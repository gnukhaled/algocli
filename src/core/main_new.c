#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <setjmp.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include "../../include/core/config.h"
#include "../../include/core/safe_exec.h"
#include "../../include/core/command_dispatcher.h"
#include "../../include/database/registry_new.h"

/* SECURITY NOTE: This version does NOT use setuid(0) */
/* Run this with proper sudo/capabilities instead */

static sigjmp_buf ctrlc_buf;

/* Signal handler for SIGINT (Ctrl+C) */
static void sigint_handler(int signal) {
    (void)signal; /* Unused parameter */
    printf("\n");
    siglongjmp(ctrlc_buf, 1);
}

/* Build shell prompt safely */
static char* build_prompt(void) {
    static char prompt[LARGE_BUFFER_SIZE];
    char hostname[MAX_HOSTNAME_LENGTH];
    const char *username;
    struct passwd *pw;

    /* Get username */
    pw = getpwuid(getuid());
    username = pw ? pw->pw_name : "unknown";

    /* Get hostname */
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        strncpy(hostname, "localhost", sizeof(hostname) - 1);
        hostname[sizeof(hostname) - 1] = '\0';
    }

    /* Build prompt safely */
    safe_snprintf(prompt, sizeof(prompt), "[%s@%s] %s -> ",
                 username, hostname, APP_PROMPT_NAME);

    return prompt;
}

/* Note: strip_whitespace is provided by command_dispatcher module */

/* Main function */
int main(int argc, char **argv) {
    (void)argc; /* Unused parameter */
    (void)argv; /* Unused parameter */

    char *line;
    char *stripped;
    char history_file[MAX_PATH_LENGTH];
    const char *home_dir;
    struct passwd *pw;
    struct sigaction sa;

    /* SECURITY: Do NOT call setuid(0) here */
    /* This application should be run with appropriate sudo/capabilities */

    /* Initialize configuration */
    if (config_init() != SUCCESS) {
        fprintf(stderr, "Failed to initialize configuration\n");
        return EXIT_FAILURE;
    }

    /* Initialize registry database */
    if (registry_init(config_get_registry_db()) != REG_SUCCESS) {
        fprintf(stderr, "Failed to initialize registry database\n");
        return EXIT_FAILURE;
    }

    /* Setup signal handler for SIGINT */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sa.sa_flags = SA_RESTART;
    sigfillset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    /* Get home directory safely */
    pw = getpwuid(getuid());
    home_dir = pw ? pw->pw_dir : getenv("HOME");

    if (!home_dir) {
        fprintf(stderr, "Warning: Could not determine home directory\n");
        home_dir = "/tmp";
    }

    /* Build history file path safely */
    safe_snprintf(history_file, sizeof(history_file), "%s/%s",
                 home_dir, DEFAULT_HISTORY_FILE);

    /* Load command history */
    read_history(history_file);

    /* Build prompt */
    char *prompt = build_prompt();

    /* Setup readline with command completion */
    initialize_readline(APP_NAME);

    /* Main loop with SIGINT handling */
    while (sigsetjmp(ctrlc_buf, 1) != 0)
        ;

    while ((line = readline(prompt)) != NULL) {
        if (!line)
            break;

        stripped = strip_whitespace(line);

        if (*stripped) {
            add_history(stripped);
            write_history(history_file);
            dispatch_command(stripped);
        }

        free(line);
    }

    /* Cleanup */
    registry_close();

    return EXIT_SUCCESS;
}
