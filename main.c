
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <sysexits.h>
#include "algocli.h"
#include "privilege.h"
#include "proc.h"
#include "registry.h"

#ifndef ALGOCLI_VERSION
#define ALGOCLI_VERSION "dev"
#endif

sigjmp_buf ctrlc_buf;

void sigint_handler(int signal){
    (void)signal;
    /* async-signal-safe: write(2) is, printf is not. The result is
     * intentionally ignored — there is nothing meaningful to do if
     * writing a single byte fails inside a signal handler. */
    ssize_t n = write(STDOUT_FILENO, "\n", 1);
    (void)n;
    siglongjmp(ctrlc_buf, 1);
}

static void print_usage(const char *prog) {
    printf("Usage: %s [OPTIONS]\n"
           "\n"
           "Interactive admin shell for the AlgoBackup appliance.\n"
           "\n"
           "Options:\n"
           "  -h, --help     Show this help and exit.\n"
           "  -v, --version  Print version and exit.\n"
           "\n"
           "With no options, %s drops to an interactive prompt. Type\n"
           "`help` at the prompt to see available commands, or read\n"
           "the algocli(1) man page.\n",
           prog ? prog : "algocli",
           prog ? prog : "algocli");
}

static int handle_argv(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        if (strcmp(a, "-v") == 0 || strcmp(a, "--version") == 0) {
            printf("algocli %s\n", ALGOCLI_VERSION);
            return 0;
        }
        if (strcmp(a, "--") == 0) {
            return -1;   /* fall through to interactive mode */
        }
        fprintf(stderr, "%s: unrecognized option '%s'\n"
                        "Try '%s --help' for more information.\n",
                argv[0] ? argv[0] : "algocli", a,
                argv[0] ? argv[0] : "algocli");
        return EX_USAGE;
    }
    return -1;   /* no flags; proceed to interactive mode */
}

int main(int argc, char **argv){
    char *line, *s;
    /* Capture user-provided env vars to local storage *before*
     * env_sanitize() clears the process environment. After that
     * point getenv() pointers from the original env are gone (the
     * passthrough allowlist re-installs HOME/USER/TERM/etc., so
     * those still work via getenv after sanitize, but anything
     * outside the allowlist — like XDG_STATE_HOME — is lost). */
    char homedir[PATH_MAX] = {0};
    char username[64] = {0};
    char xdg_state[PATH_MAX] = {0};
    {
        const char *e;
        if ((e = getenv("HOME")))           snprintf(homedir,   sizeof homedir,   "%s", e);
        if ((e = getenv("USER")))           snprintf(username,  sizeof username,  "%s", e);
        if ((e = getenv("XDG_STATE_HOME"))) snprintf(xdg_state, sizeof xdg_state, "%s", e);
    }

    char histfile[PATH_MAX];
    char hostname[HOST_NAME_MAX + 1];
    char prompt[256 + HOST_NAME_MAX];
    struct sigaction sa;

    int argv_rc = handle_argv(argc, argv);
    if (argv_rc >= 0) return argv_rc;

    /* Sanitize the environment before any fork+execve runs. We do this
     * before priv_init so the passthrough allowlist captures the
     * invoking user's HOME/USER/TERM/LANG, not root's. */
    env_sanitize();

    /* Drop euid to the invoking user, keeping saved-uid=0 so each
     * privileged op can re-escalate via priv_raise(). priv_init also
     * captures the username for audit logging. */
    if (priv_init() != 0) {
        fprintf(stderr, "warning: privilege init failed; "
                        "privileged commands will not work\n");
    } else if (!priv_can_escalate()) {
        fprintf(stderr, "warning: not running setuid-root; "
                        "privileged commands will return EPERM\n");
    }

    sa.sa_handler = &sigint_handler;
    sa.sa_flags = SA_RESTART;
    sigfillset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1){
        perror("SIGINT handler error");
    }

    initialize_readline(argv[0]);

    /* History file location, in priority order:
     *   1. $XDG_STATE_HOME/algocli/history   (XDG-compliant systems)
     *   2. $HOME/.local/state/algocli/history (XDG default base)
     *   3. $HOME/.algocli_history             (back-compat with original)
     * mkdir -p the parent directory so read_history won't fail on
     * first run. */
    char histdir[PATH_MAX - 16];   /* leave room for "/history" + NUL */
    if (*xdg_state) {
        snprintf(histdir, sizeof histdir, "%s/algocli", xdg_state);
    } else if (*homedir) {
        snprintf(histdir, sizeof histdir, "%s/.local/state/algocli", homedir);
    } else {
        snprintf(histdir, sizeof histdir, "/tmp/algocli");
    }
    /* mkdir -p the directory tree, best effort. */
    char tmp[sizeof histdir];
    snprintf(tmp, sizeof tmp, "%s", histdir);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0700);
            *p = '/';
        }
    }
    mkdir(tmp, 0700);
    snprintf(histfile, sizeof histfile, "%s/history", histdir);
    read_history(histfile);
    stifle_history(1000);

    if (reg_init(NULL) != 0) {
        fprintf(stderr, "warning: registry unavailable; "
                        "configuration commands will fail\n");
    }

    if (gethostname(hostname, sizeof hostname) != 0) {
        perror("gethostname");
        snprintf(hostname, sizeof hostname, "localhost");
    }
    hostname[sizeof hostname - 1] = '\0';

    snprintf(prompt, sizeof prompt, "[%s@%s] AlgoOS -> ",
             *username ? username : "user", hostname);

    while (sigsetjmp(ctrlc_buf, 1) != 0) {
        /* return point for SIGINT — re-enter the readline loop */
    }

    while ((line = readline(prompt)) != NULL){
        s = stripwhite(line);
        if (*s){
            add_history(s);
            write_history(histfile);
            execute_command(s);
        }
        free(line);
    }

    reg_close();
    return 0;
}
