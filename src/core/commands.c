#include <stdio.h>
#include <stdlib.h>
#include "../../include/core/command_dispatcher.h"
#include "../../include/core/config.h"

/* Forward declarations for command handlers */
static int cmd_exit(char **args);
static int cmd_help(char **args);
static int cmd_bp(char **args);
static int cmd_bp_create(char **args);
static int cmd_bp_delete(char **args);
static int cmd_bp_rename(char **args);
static int cmd_bp_list(char **args);
static int cmd_bp_quota(char **args);
static int cmd_bp_quota_set(char **args);
static int cmd_accessctrl(char **args);
static int cmd_accessctrl_ssh(char **args);
static int cmd_accessctrl_ssh_enable(char **args);
static int cmd_accessctrl_ssh_disable(char **args);
static int cmd_accessctrl_ftp(char **args);
static int cmd_accessctrl_ftp_enable(char **args);
static int cmd_accessctrl_ftp_disable(char **args);
static int cmd_accessctrl_web(char **args);
static int cmd_accessctrl_web_enable(char **args);
static int cmd_accessctrl_web_disable(char **args);

/* External command implementations */
extern int backup_point_create(const char *name);
extern int backup_point_delete(const char *name);
extern int backup_point_rename(const char *old_name, const char *new_name);
extern int backup_point_list(void);
extern int backup_point_set_quota(const char *name, const char *limit);

extern int access_control_ssh_enable(void);
extern int access_control_ssh_disable(void);
extern int access_control_ftp_enable(void);
extern int access_control_ftp_disable(void);
extern int access_control_web_enable(void);
extern int access_control_web_disable(void);

/* Backup point quota subcommands */
static command_t bp_quota_commands[] = {
    {"set", cmd_bp_quota_set, NULL, "Set backup point quota"},
    NULL_COMMAND
};

/* Backup point commands */
static command_t bp_commands[] = {
    {"create", cmd_bp_create, NULL, "Create a backup point"},
    {"delete", cmd_bp_delete, NULL, "Delete a backup point"},
    {"rename", cmd_bp_rename, NULL, "Rename a backup point"},
    {"list",   cmd_bp_list,   NULL, "List backup points"},
    {"quota",  cmd_bp_quota,  bp_quota_commands, "Manage backup point quota"},
    NULL_COMMAND
};

/* Access control SSH subcommands */
static command_t accessctrl_ssh_commands[] = {
    {"enable",  cmd_accessctrl_ssh_enable,  NULL, "Enable SSH access"},
    {"disable", cmd_accessctrl_ssh_disable, NULL, "Disable SSH access"},
    NULL_COMMAND
};

/* Access control FTP subcommands */
static command_t accessctrl_ftp_commands[] = {
    {"enable",  cmd_accessctrl_ftp_enable,  NULL, "Enable FTP access"},
    {"disable", cmd_accessctrl_ftp_disable, NULL, "Disable FTP access"},
    NULL_COMMAND
};

/* Access control Web subcommands */
static command_t accessctrl_web_commands[] = {
    {"enable",  cmd_accessctrl_web_enable,  NULL, "Enable web interface"},
    {"disable", cmd_accessctrl_web_disable, NULL, "Disable web interface"},
    NULL_COMMAND
};

/* Access control commands */
static command_t accessctrl_commands[] = {
    {"ssh", cmd_accessctrl_ssh, accessctrl_ssh_commands, "Control SSH access"},
    {"ftp", cmd_accessctrl_ftp, accessctrl_ftp_commands, "Control FTP access"},
    {"web", cmd_accessctrl_web, accessctrl_web_commands, "Control web access"},
    NULL_COMMAND
};

/* Main command array */
command_t commands[] = {
    {"bp",         cmd_bp,         bp_commands,         "Backup point management"},
    {"accessctrl", cmd_accessctrl, accessctrl_commands, "Access control management"},
    {"help",       cmd_help,       NULL,                "Display help"},
    {"exit",       cmd_exit,       NULL,                "Exit the program"},
    {"quit",       cmd_exit,       NULL,                "Exit the program"},
    NULL_COMMAND
};

/* Command handler implementations */

static int cmd_exit(char **args) {
    (void)args;
    printf("Goodbye!\n");
    exit(EXIT_SUCCESS);
}

static int cmd_help(char **args) {
    if (args[0]) {
        /* Help for specific command */
        char cmdline[256];
        snprintf(cmdline, sizeof(cmdline), "%s", args[0]);
        print_command_help(cmdline);
    } else {
        /* General help */
        printf("AlgoBackup CLI - Available Commands:\n\n");

        size_t longest = 0;
        for (int i = 0; commands[i].name; i++) {
            size_t len = strlen(commands[i].name);
            if (len > longest)
                longest = len;
        }

        for (int i = 0; commands[i].name; i++) {
            printf("  %-*s  %s\n",
                   (int)longest,
                   commands[i].name,
                   commands[i].doc ? commands[i].doc : "");
        }

        printf("\nType '<command> help' for more information on a specific command.\n");
        printf("Use TAB for command completion.\n\n");
    }
    return SUCCESS;
}

/* Backup point command handlers */

static int cmd_bp(char **args) {
    (void)args;
    print_command_help("bp");
    return SUCCESS;
}

static int cmd_bp_create(char **args) {
    if (!args[0]) {
        fprintf(stderr, "Usage: bp create <name>\n");
        return ERROR_INVALID_ARGS;
    }
    return backup_point_create(args[0]);
}

static int cmd_bp_delete(char **args) {
    if (!args[0]) {
        fprintf(stderr, "Usage: bp delete <name>\n");
        return ERROR_INVALID_ARGS;
    }
    return backup_point_delete(args[0]);
}

static int cmd_bp_rename(char **args) {
    if (!args[0] || !args[1]) {
        fprintf(stderr, "Usage: bp rename <old_name> <new_name>\n");
        return ERROR_INVALID_ARGS;
    }
    return backup_point_rename(args[0], args[1]);
}

static int cmd_bp_list(char **args) {
    (void)args;
    return backup_point_list();
}

static int cmd_bp_quota(char **args) {
    (void)args;
    print_command_help("bp quota");
    return SUCCESS;
}

static int cmd_bp_quota_set(char **args) {
    if (!args[0] || !args[1]) {
        fprintf(stderr, "Usage: bp quota set <name> <limit>\n");
        fprintf(stderr, "Example: bp quota set mybackup 10G\n");
        return ERROR_INVALID_ARGS;
    }
    return backup_point_set_quota(args[0], args[1]);
}

/* Access control command handlers */

static int cmd_accessctrl(char **args) {
    (void)args;
    print_command_help("accessctrl");
    return SUCCESS;
}

static int cmd_accessctrl_ssh(char **args) {
    (void)args;
    print_command_help("accessctrl ssh");
    return SUCCESS;
}

static int cmd_accessctrl_ssh_enable(char **args) {
    (void)args;
    return access_control_ssh_enable();
}

static int cmd_accessctrl_ssh_disable(char **args) {
    (void)args;
    return access_control_ssh_disable();
}

static int cmd_accessctrl_ftp(char **args) {
    (void)args;
    print_command_help("accessctrl ftp");
    return SUCCESS;
}

static int cmd_accessctrl_ftp_enable(char **args) {
    (void)args;
    return access_control_ftp_enable();
}

static int cmd_accessctrl_ftp_disable(char **args) {
    (void)args;
    return access_control_ftp_disable();
}

static int cmd_accessctrl_web(char **args) {
    (void)args;
    print_command_help("accessctrl web");
    return SUCCESS;
}

static int cmd_accessctrl_web_enable(char **args) {
    (void)args;
    return access_control_web_enable();
}

static int cmd_accessctrl_web_disable(char **args) {
    (void)args;
    return access_control_web_disable();
}
