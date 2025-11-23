#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../../include/core/command_dispatcher.h"
#include "../../include/core/config.h"

/* Global command context for completion */
static command_t *completion_context = NULL;

/* Strip leading and trailing whitespace */
char* strip_whitespace(char *string) {
    char *start, *end;

    /* Skip leading whitespace */
    for (start = string; isspace((unsigned char)*start); start++)
        ;

    if (*start == '\0')
        return start;

    /* Skip trailing whitespace */
    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end))
        end--;

    *(end + 1) = '\0';

    return start;
}

/* Find a base command by name */
command_t* find_command(const char *name) {
    if (!name)
        return NULL;

    for (int i = 0; commands[i].name; i++) {
        if (strcmp(name, commands[i].name) == 0) {
            return &commands[i];
        }
    }

    return NULL;
}

/* Find the last (deepest) command in a command string */
command_t* find_last_command(char *line) {
    if (!line)
        return NULL;

    char *line_copy = strdup(line);
    if (!line_copy)
        return NULL;

    char *tokens[32];  /* Max command depth */
    int token_count = 0;

    /* Tokenize the line */
    char *token = strtok(line_copy, " ");
    while (token && token_count < 32) {
        tokens[token_count++] = token;
        token = strtok(NULL, " ");
    }

    if (token_count == 0) {
        free(line_copy);
        return NULL;
    }

    /* Find base command */
    command_t *cmd = find_command(tokens[0]);
    if (!cmd) {
        free(line_copy);
        return NULL;
    }

    /* Walk down the command tree */
    for (int i = 1; i < token_count; i++) {
        if (!cmd->subcmd)
            break;

        command_t *subcmd = NULL;
        for (int j = 0; cmd->subcmd[j].name; j++) {
            if (strcmp(cmd->subcmd[j].name, tokens[i]) == 0) {
                subcmd = &cmd->subcmd[j];
                break;
            }
        }

        if (subcmd) {
            cmd = subcmd;
        } else {
            break;
        }
    }

    free(line_copy);
    return cmd;
}

/* Execute a command line */
int dispatch_command(char *line) {
    if (!line)
        return ERROR_INVALID_ARGS;

    char *line_copy = strdup(line);
    if (!line_copy)
        return ERROR_MEMORY;

    char *tokens[32];
    int token_count = 0;

    /* Tokenize the line */
    char *token = strtok(line_copy, " ");
    while (token && token_count < 32) {
        tokens[token_count++] = token;
        token = strtok(NULL, " ");
    }

    if (token_count == 0) {
        free(line_copy);
        return SUCCESS;
    }

    /* Find base command */
    command_t *cmd = find_command(tokens[0]);
    if (!cmd) {
        printf("Unknown command: %s\n", tokens[0]);
        printf("Type 'help' for available commands.\n");
        free(line_copy);
        return ERROR_NOT_FOUND;
    }

    /* Walk down the command tree */
    int cmd_depth = 1;
    for (int i = 1; i < token_count; i++) {
        if (!cmd->subcmd)
            break;

        command_t *subcmd = NULL;
        for (int j = 0; cmd->subcmd[j].name; j++) {
            if (strcmp(cmd->subcmd[j].name, tokens[i]) == 0) {
                subcmd = &cmd->subcmd[j];
                break;
            }
        }

        if (subcmd) {
            cmd = subcmd;
            cmd_depth = i + 1;
        } else {
            break;
        }
    }

    /* Prepare arguments (everything after the command) */
    char *args[32];
    int arg_count = 0;
    for (int i = cmd_depth; i < token_count && arg_count < 32; i++) {
        args[arg_count++] = tokens[i];
    }
    args[arg_count] = NULL;

    /* Execute command function */
    int result = SUCCESS;
    if (cmd->func) {
        result = cmd->func(args);
    } else if (cmd->subcmd) {
        /* Command has subcommands but none specified */
        print_command_help(line);
    } else {
        printf("Command not yet implemented\n");
    }

    free(line_copy);
    return result;
}

/* Print help for a command */
void print_command_help(const char *line) {
    char *line_copy = strdup(line);
    if (!line_copy)
        return;

    command_t *cmd = find_last_command(line_copy);

    if (!cmd || !cmd->subcmd) {
        printf("No help available\n");
        free(line_copy);
        return;
    }

    printf("Available subcommands:\n\n");

    /* Find longest command name for formatting */
    size_t longest = 0;
    for (int i = 0; cmd->subcmd[i].name; i++) {
        size_t len = strlen(cmd->subcmd[i].name);
        if (len > longest)
            longest = len;
    }

    /* Print commands with documentation */
    for (int i = 0; cmd->subcmd[i].name; i++) {
        printf("  %-*s  %s\n",
               (int)longest,
               cmd->subcmd[i].name,
               cmd->subcmd[i].doc ? cmd->subcmd[i].doc : "");
    }

    printf("\n");
    free(line_copy);
}

/* Initialize readline with command completion */
void initialize_readline(const char *progname) {
    rl_readline_name = progname;
    rl_attempted_completion_function = command_completion;
}

/* Readline completion function */
char** command_completion(const char *text, int start, int end) {
    (void)end;  /* Unused parameter */

    char **matches = NULL;

    if (start == 0) {
        /* Complete base commands */
        matches = rl_completion_matches(text, base_command_generator);
    } else {
        /* Complete subcommands based on context */
        char *line_copy = strdup(rl_line_buffer);
        if (line_copy) {
            completion_context = find_last_command(line_copy);
            matches = rl_completion_matches(text, subcommand_generator);
            free(line_copy);
        }
    }

    /* Don't fall back to filename completion */
    rl_attempted_completion_over = 1;

    return matches;
}

/* Generator for base commands */
char* base_command_generator(const char *text, int state) {
    static int index;
    static size_t len;

    /* Initialize on first call */
    if (!state) {
        index = 0;
        len = strlen(text);
    }

    /* Return matching commands */
    while (commands[index].name) {
        const char *name = commands[index].name;
        index++;

        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    return NULL;
}

/* Generator for subcommands */
char* subcommand_generator(const char *text, int state) {
    static int index;
    static size_t len;

    /* Initialize on first call */
    if (!state) {
        index = 0;
        len = strlen(text);
    }

    /* Return matching subcommands if we have a context */
    if (completion_context && completion_context->subcmd) {
        while (completion_context->subcmd[index].name) {
            const char *name = completion_context->subcmd[index].name;
            index++;

            if (strncmp(name, text, len) == 0) {
                return strdup(name);
            }
        }
    }

    return NULL;
}
