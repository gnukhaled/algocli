#ifndef COMMAND_DISPATCHER_H
#define COMMAND_DISPATCHER_H

#include <readline/readline.h>

/* Command structure for hierarchical commands */
typedef struct command {
    const char *name;                    /* Command name */
    int (*func)(char **args);           /* Handler function */
    struct command *subcmd;             /* Subcommand array */
    const char *doc;                    /* Documentation string */
} command_t;

/* Null command terminator */
#define NULL_COMMAND {NULL, NULL, NULL, NULL}

/* Command dispatcher functions */
int dispatch_command(char *line);
command_t* find_command(const char *name);
command_t* find_last_command(char *line);

/* Readline integration */
void initialize_readline(const char *progname);
char** command_completion(const char *text, int start, int end);
char* base_command_generator(const char *text, int state);
char* subcommand_generator(const char *text, int state);

/* Helper functions */
void print_command_help(const char *line);
char* strip_whitespace(char *string);

/* External command array */
extern command_t commands[];

#endif /* COMMAND_DISPATCHER_H */
