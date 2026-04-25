
#include "algocli.h"
#include "cmd-fs.h"
#include "paths.h"
#include "proc.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

/* Not declared const because execve's argv has type char *const[] —
 * each element must be a `char *`. The string literal still lives in
 * read-only memory; the type just lets us pass it to execve. */
static char backup_root[] = BACKUP_ROOT;

extern FILE *logfp;

int fs_quota_enable(void) {
    init_logger(CTX_FS);
    char *const argv[] = {
        (char *)BTRFS_BIN, "quota", "enable", (char *)backup_root, NULL
    };
    int rc = spawn(argv);
    if (rc == 0) {
        algolog(ALGO_INFO, "Filesystem quota enabled\n");
        printf("Filesystem quota enabled\n");
    } else {
        fprintf(stderr, "Failed to enable quota (rc=%d)\n", rc);
    }
    return rc;
}

int fs_quota_disable(void) {
    char *const argv[] = {
        (char *)BTRFS_BIN, "quota", "disable", (char *)backup_root, NULL
    };
    int rc = spawn(argv);
    if (rc == 0) {
        printf("Filesystem quota disabled\n");
    } else {
        fprintf(stderr, "Failed to disable quota (rc=%d)\n", rc);
    }
    return rc;
}

int fs_quota_status(void) {
    /* spawn() inherits the current stdio, so probing whether a btrfs
     * subcommand succeeds requires redirecting its output to /dev/null
     * within the child. We do that by forking ourselves: the child
     * reopens fd 1 and 2 onto /dev/null, then execve's btrfs. Cleaner
     * than relying on shell redirection. */
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    if (pid == 0) {
        int devnull = open("/dev/null", O_WRONLY | O_CLOEXEC);
        if (devnull >= 0) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
        char *const argv[] = {
            (char *)BTRFS_BIN, "qgroup", "show", (char *)backup_root, NULL
        };
        execve(argv[0], argv, environ);
        _exit(127);
    }
    int status = 0;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) { perror("waitpid"); return -1; }
    }
    int rc = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    if (rc != 0) {
        printf("Filesystem quota is disabled\n");
    } else {
        printf("Filesystem quota is enabled\n");
    }
    return rc;
}

/* ----- dispatcher shims (formerly in algocli.c) ----------------- */

int func_fs(char **arg) {
    (void)arg;
    cmd_ref();
    return 0;
}

int func_fs_quota(char **arg) {
    (void)arg;
    cmd_ref();
    return 0;
}

int func_fs_quota_enable(char **arg) {
    (void)arg;
    return fs_quota_enable();
}

int func_fs_quota_disable(char **arg) {
    (void)arg;
    return fs_quota_disable();
}

int func_fs_quota_status(char **arg) {
    (void)arg;
    return fs_quota_status();
}
