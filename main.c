
#include "algocli.h"

sigjmp_buf ctrlc_buf;

void sigint_handler(int signal){
	printf("\n");
	siglongjmp(ctrlc_buf, 1);
}

int main(int argc, char **argv){
    char *line, *s;
    char *homedir = getenv("HOME");
    char histfile[PATH_MAX];
    struct sigaction sa;

    /* Safely construct histfile path using snprintf to prevent buffer overflow */
    if (homedir != NULL) {
        snprintf(histfile, sizeof(histfile), "%s/.algohist", homedir);
    } else {
        snprintf(histfile, sizeof(histfile), ".algohist");
    }

    setuid(0);

    sa.sa_handler = &sigint_handler;
    sa.sa_flags = SA_RESTART;
    sigfillset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1){
        perror("SIGINT handler error");
    }

    char *progname = argv[0];
    initialize_readline(progname);

    read_history(histfile);

    int ret;
    char hostname[128];        /* Fixed: was char *hostname[128] (array of pointers) */
    char *osname = "AlgoOS";
    char prompt[512];          /* Fixed: was char *prompt[512] (array of pointers) */
    char *username;
    username = getenv("USER");

    /* Handle NULL username to prevent crash */
    if (username == NULL) {
        username = "unknown";
    }

    if ((ret = gethostname(hostname, sizeof(hostname))) != 0)
        perror("gethostname");

    /* Use snprintf instead of multiple strcat calls to prevent buffer overflow */
    snprintf(prompt, sizeof(prompt), "[%s@%s] %s -> ", username, hostname, osname);

    while ( sigsetjmp( ctrlc_buf, 1 ) != 0 );

        while ((line = readline(prompt)) != NULL){
		rl_bind_key('\t', rl_complete);

	if (!line)
	    break;

	s = stripwhite(line);

	if (*s){
	   add_history(s);
	   write_history(histfile);
           execute_command(s);
	}

	free(line);
    }

    return (0);
}
