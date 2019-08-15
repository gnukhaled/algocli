
#include "algocli.h"

sigjmp_buf ctrlc_buf;

void sigint_handler(int signal){
	printf("\n");
	siglongjmp(ctrlc_buf, 1);
}

int main(int argc, char **argv){
    char *line, *s;
    char *homedir = getenv("HOME");
    const char *histfile = strcat(homedir, "/.algohist");
    struct sigaction sa;

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
    char *hostname[128];
    char *osname = "AlgoOS";
    char *prompt[512];
    char *username;
    username  = getenv("USER");

    if ((ret = gethostname(hostname, sizeof hostname)) != 0)
        perror("gethostname");

    strcat((char *)prompt, "[");
    strcat((char *)prompt, username);
    strcat((char *)prompt, "@");
    strcat((char *)prompt, hostname);
    strcat((char *)prompt, "] ");
    strcat((char *)prompt, osname);
    strcat((char *)prompt, " -> ");

    while ( sigsetjmp( ctrlc_buf, 1 ) != 0 );

        while ((line = readline((char *)prompt)) != NULL){
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
