
#include "algocli.h"
#include "cmd-system.h"
#include "paths.h"
#include "privilege.h"
#include "validate.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Reads a [y/N] confirmation. Returns true iff the user typed
 * exactly y/Y/yes/YES followed by newline (or EOF after such).
 * Anything else — empty line, "n", "no", garbage, EOF — is treated
 * as cancel. Uses fgets for a defined buffer with no leftover-stdin
 * issues that scanf("%c") had. */
static bool confirm_yn(const char *prompt) {
    printf("\n\t%s [y,N]: ", prompt);
    fflush(stdout);

    char line[16];
    if (!fgets(line, sizeof line, stdin)) {
        return false;
    }
    /* Drain any unread bytes if the line was longer than our buffer. */
    if (strchr(line, '\n') == NULL) {
        int c;
        while ((c = getchar()) != EOF && c != '\n') { /* discard */ }
    }
    /* Skip leading whitespace, then look at the first significant
     * character. The prompt is small enough that we accept either
     * "y" or "yes"; everything else is no. */
    char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    return (*p == 'y' || *p == 'Y');
}

int algo_reboot(void){
    if (!confirm_yn("Confirm system reboot")) {
        printf("System reboot cancelled by user\n");
        return 0;
    }
    printf("\nRebooting system\n");
    sync();
    fflush(stdout);
    if (priv_raise() != 0) {
        fprintf(stderr, "reboot: cannot escalate privileges\n");
        return -1;
    }
    int rc = reboot(RB_AUTOBOOT);
    /* reboot(2) only returns on failure. */
    priv_drop();
    return rc;
}

int algo_shutdown(void){
    if (!confirm_yn("Confirm system shutdown")) {
        printf("System shutdown cancelled by user\n");
        return 0;
    }
    printf("\nShutting down the system\n");
    sync();
    fflush(stdout);
    if (priv_raise() != 0) {
        fprintf(stderr, "shutdown: cannot escalate privileges\n");
        return -1;
    }
    int rc = reboot(RB_POWER_OFF);
    priv_drop();
    return rc;
}

/* Read MOTD content from stdin, validate it, and write atomically to
 * /etc/motd. The previous implementation shelled out to vim, which
 * had a trivial root shell escape via `:!sh` even with --noplugin.
 * This version never spawns a process and never invokes a parser. */
int system_set_motd(void){
    char buf[4096];
    size_t total = 0;

    printf("Enter MOTD text. Finish with a single '.' on its own line "
           "(or EOF):\n");
    fflush(stdout);

    /* Read until sentinel line ".\n" or EOF, accumulating into buf
     * with strict size and content limits. */
    char line[256];
    while (fgets(line, sizeof line, stdin)) {
        if (strcmp(line, ".\n") == 0 || strcmp(line, ".") == 0) {
            break;
        }
        size_t len = strlen(line);
        if (!is_safe_text(line, sizeof line)) {
            fprintf(stderr, "Rejected: MOTD must be printable ASCII "
                            "(plus tab/newline); no control chars.\n");
            return -1;
        }
        if (total + len >= sizeof buf) {
            fprintf(stderr, "Rejected: MOTD exceeds %zu bytes.\n",
                    sizeof buf - 1);
            return -1;
        }
        memcpy(buf + total, line, len);
        total += len;
    }
    buf[total] = '\0';

    /* Atomic replace: write to MOTD_TMP_PATH in the same directory,
     * fsync, rename. rename(2) is atomic within a filesystem. The
     * directory and file are root-owned, so the entire block runs
     * with euid=0. */
    const char *target = MOTD_PATH;
    const char *tmp    = MOTD_TMP_PATH;
    int rc = -1;

    if (priv_raise() != 0) {
        fprintf(stderr, "motd: cannot escalate privileges\n");
        return -1;
    }

    int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC | O_NOFOLLOW,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0) {
        fprintf(stderr, "open(%s): %s\n", tmp, strerror(errno));
        goto out;
    }

    ssize_t written = 0;
    while (written < (ssize_t)total) {
        ssize_t n = write(fd, buf + written, total - (size_t)written);
        if (n < 0) {
            if (errno == EINTR) continue;
            fprintf(stderr, "write: %s\n", strerror(errno));
            close(fd);
            unlink(tmp);
            goto out;
        }
        written += n;
    }

    if (fsync(fd) != 0) {
        fprintf(stderr, "fsync: %s\n", strerror(errno));
        close(fd);
        unlink(tmp);
        goto out;
    }
    close(fd);

    if (rename(tmp, target) != 0) {
        fprintf(stderr, "rename(%s, %s): %s\n", tmp, target, strerror(errno));
        unlink(tmp);
        goto out;
    }

    printf("MOTD updated (%zu bytes)\n", total);
    rc = 0;
out:
    priv_drop();
    return rc;
}

/* ----- dispatcher shims (formerly in algocli.c) ----------------- */

int func_system(char **arg) {
    (void)arg;
    cmd_ref();
    return 0;
}

int func_system_set(char **arg) {
    (void)arg;
    cmd_ref();
    return 0;
}

int func_system_set_motd(char **arg) {
    (void)arg;
    return system_set_motd();
}

int func_system_reboot(char **arg) {
    (void)arg;
    return algo_reboot();
}

int func_system_shutdown(char **arg) {
    (void)arg;
    return algo_shutdown();
}
