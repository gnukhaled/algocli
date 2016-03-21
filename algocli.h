
#ifndef ALGOCLI_H
#define ALGOCLI_H

#define _GNU_SOURCE
//#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include "algo-log.h"
#include "cmd-log.h"

#define TRUE 1
#define FALSE 0

#define NULL_COMMAND {(char *)NULL, NULL, (command_t *)NULL, \
								(char *)NULL}

/* The command type definition, each command 
 * (command_t) should link to an array of other 
 * sub commands (command_t) */

typedef struct command {
    char *name;
    int (*func)(char**);
    struct command *subcmd;
    char *doc;
} command_t;

/* readline executable commands callable functions */
int func_exit();
int func_accessctrl(), func_bp(), func_syshealth(), func_config(), func_disk();
int func_fs(), func_help(), func_repl(), func_cifs(), func_vtl();
int func_net(), func_nfs(), func_snapshot();
int func_system(), func_user(), func_log();
int func_log_list(), func_log_view(), func_log_watch();
int func_bp_create(), func_bp_delete(), func_bp_rename(), func_bp_list();
int func_bp_quota(), func_bp_quota_set();
int func_fs_quota(), func_fs_quota_enable();
int func_fs_quota_disable(), func_fs_quota_status();
int func_accessctrl_ssh(), func_accessctrl_ftp(), func_accessctrl_web();
int func_accessctrl_ssh_disable(),func_accessctrl_ssh_enable();
int func_accessctrl_web_enable(), func_accessctrl_web_disable();
int func_accessctrl_ftp_enable(), func_accessctrl_ftp_disable();
int func_accessctrl_ssh_set(),func_accessctrl_ssh_set_port();
int func_accessctrl_ssh_set_protocol(),func_accessctrl_ssh_set_listen_address();
int func_accessctrl_ssh_list();
int func_accessctrl_ssh_list_config(), func_accessctrl_ssh_list_allowedhosts();
int func_accessctrl_ssh_add(), func_accessctrl_ssh_del();
int func_accessctrl_ssh_add_allowedhost(),func_accessctrl_ssh_del_allowedhost();
int func_accessctrl_ftp_set(),func_accessctrl_ftp_set_port();
int func_accessctrl_web_set(),func_accessctrl_web_set_http_port();
int func_accessctrl_web_set_https_port();
int func_accessctrl_web_set_http_session();
int func_system_reboot(), func_system_shutdown();
int func_system_set(), func_system_set_motd();
int func_nfs_share(), func_nfs_unshare();
int func_nfs_enable(), func_nfs_disable();
int func_nfs_share_list();

/* readline functions */
void initialize_readline(char*);
int execute_command(char*);
char* algoclibase_generator();
char* algoclisub_generator();
char** algocli_completion();
char* stripwhite();
command_t *find_command(char*);
command_t *find_last_command(char*);
void cmd_ref();
char* format_doc(int, const char *);

int algoauth();
void sigint_handler(int signal);

#endif /* ALGOCLI_H */
