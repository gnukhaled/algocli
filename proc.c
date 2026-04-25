/* proc.c — fork/execve helper and environment sanitizer.
 *
 * Replaces popen()/system() across the codebase. Two reasons:
 *
 *   1. Security. popen()/system() invoke /bin/sh -c <string>, which
 *      reparses metacharacters. Any user-controlled string in the
 *      command becomes a command-injection vector. fork+execve
 *      passes argv to the kernel as an opaque vector — there is no
 *      shell to interpret semicolons, dollar-paren, backticks, etc.
 *
 *   2. Correctness. waitpid + WEXITSTATUS lets us return a clean
 *      "child exit status" int instead of system()'s wait-status
 *      bitfield, which was being silently mishandled at every call
 *      site.
 */

#include "proc.h"
#include "privilege.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

int spawn(char *const argv[]) {
    if (!argv || !argv[0]) {
        errno = EINVAL;
        return -EINVAL;
    }
    /* argv[0] must be an absolute path. We don't search PATH ourselves
     * because that's the whole class of bug we are eliminating. */
    if (argv[0][0] != '/') {
        fprintf(stderr, "spawn: refusing to run non-absolute path '%s'\n",
                argv[0]);
        errno = EINVAL;
        return -EINVAL;
    }

    /* Raise privilege so the child inherits euid=0. We drop again on
     * the parent side after fork; the child's euid is unaffected by
     * the parent's later seteuid. If we are not setuid-root,
     * priv_raise returns -EPERM and we proceed unprivileged so the
     * caller can still test/dry-run on a developer machine. */
    int raise_rc = priv_raise();
    if (raise_rc != 0 && raise_rc != -EPERM) {
        return raise_rc;
    }

    pid_t pid = fork();
    if (pid < 0) {
        int e = errno;
        perror("fork");
        if (raise_rc == 0) priv_drop();
        return -e;
    }

    if (pid == 0) {
        /* Child. Replace the process image. environ has already been
         * sanitized by main() via env_sanitize(); we pass it through
         * explicitly via execve so this is auditable in strace. */
        execve(argv[0], argv, environ);
        /* execve only returns on failure. */
        fprintf(stderr, "execve(%s): %s\n", argv[0], strerror(errno));
        _exit(127);
    }

    /* Parent path. Drop our euid back to the invoking user immediately
     * — the privileged work is now happening in the child. */
    if (raise_rc == 0) priv_drop();

    int status = 0;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno == EINTR) continue;
        int e = errno;
        perror("waitpid");
        return -e;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        fprintf(stderr, "%s: terminated by signal %d\n",
                argv[0], WTERMSIG(status));
        return 128 + WTERMSIG(status);
    }
    return -EIO;
}

void env_sanitize(void) {
    /* Capture a small allowlist of variables that callers expect to
     * remain available (terminal config, locale, home dir, username).
     * Everything else — including LD_PRELOAD, IFS, PATH, BASH_ENV,
     * etc. — is dropped before any spawn() runs. */
    static const char *const passthrough[] = {
        "HOME", "USER", "LOGNAME", "TERM", "LANG", "LC_ALL", NULL
    };
    char *saved[8] = {0};
    for (size_t i = 0; passthrough[i]; i++) {
        const char *v = getenv(passthrough[i]);
        if (v) {
            saved[i] = strdup(v);
        }
    }

    if (clearenv() != 0) {
        perror("clearenv");
    }

    /* Known-safe PATH for the privileged operations we run. Real
     * deployments only need standard system directories. */
    setenv("PATH", "/usr/sbin:/usr/bin:/sbin:/bin", 1);
    setenv("IFS", " \t\n", 1);

    for (size_t i = 0; passthrough[i]; i++) {
        if (saved[i]) {
            setenv(passthrough[i], saved[i], 1);
            free(saved[i]);
        }
    }
}
