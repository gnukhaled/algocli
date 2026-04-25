
#ifndef ALGOCLI_H
#define ALGOCLI_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdbool.h>
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

/* Stub helper used by func_* shims for commands not yet implemented.
 * Prints a one-line "not implemented yet" message and returns -ENOSYS
 * so callers and scripts can detect failure. When the real
 * implementation lands, delete the using line entirely. */
#define STUB_NOT_IMPLEMENTED(name) do { \
    fprintf(stderr, "%s: not implemented yet\n", name); \
    return -ENOSYS; \
} while (0)

#define NULL_COMMAND { (char *)NULL, (int (*)(char **))NULL, \
                       (command_t *)NULL, (char *)NULL, 0 }

/* The command type definition. Each command (command_t) may link to
 * an array of sub-commands (command_t) terminated by NULL_COMMAND.
 *
 * `flags` is a bit-set declaring policy on the command. The
 * dispatcher consults these flags before invoking `func`, so each
 * func_* implementation stays focused on its actual work and the
 * security policy is visible alongside the command's name and doc. */

/* CMD_AUTH: command requires PAM re-authentication of the invoking
 * user before it runs. The dispatcher wraps the call with AUTH_GATE
 * — failure aborts the dispatch, success records an audit-log entry. */
#define CMD_AUTH (1u << 0)

typedef struct command {
    char *name;
    int (*func)(char **);
    struct command *subcmd;
    char *doc;
    unsigned flags;
} command_t;

/* Readline-invoked command callbacks. Every callback takes a NULL-
 * terminated argument vector; callbacks that ignore their args still
 * accept char ** for type compatibility with command_t.func. */
int func_exit(char **);
int func_accessctrl(char **), func_bp(char **), func_syshealth(char **);
int func_config(char **), func_disk(char **);
int func_fs(char **), func_help(char **), func_repl(char **);
int func_cifs(char **), func_vtl(char **);
int func_net(char **), func_nfs(char **), func_snapshot(char **);
int func_system(char **), func_user(char **), func_log(char **);
int func_log_list(char **), func_log_view(char **), func_log_watch(char **);
int func_bp_create(char **), func_bp_delete(char **);
int func_bp_rename(char **), func_bp_list(char **);
int func_bp_quota(char **), func_bp_quota_set(char **);
int func_fs_quota(char **), func_fs_quota_enable(char **);
int func_fs_quota_disable(char **), func_fs_quota_status(char **);
int func_accessctrl_ssh(char **), func_accessctrl_ftp(char **);
int func_accessctrl_web(char **);
int func_accessctrl_ssh_disable(char **), func_accessctrl_ssh_enable(char **);
int func_accessctrl_web_enable(char **), func_accessctrl_web_disable(char **);
int func_accessctrl_ftp_enable(char **), func_accessctrl_ftp_disable(char **);
int func_accessctrl_ssh_set(char **), func_accessctrl_ssh_set_port(char **);
int func_accessctrl_ssh_set_protocol(char **);
int func_accessctrl_ssh_set_listen_address(char **);
int func_accessctrl_ssh_list(char **);
int func_accessctrl_ssh_list_config(char **);
int func_accessctrl_ssh_list_allowedhosts(char **);
int func_accessctrl_ssh_add(char **), func_accessctrl_ssh_del(char **);
int func_accessctrl_ssh_add_allowedhost(char **);
int func_accessctrl_ssh_del_allowedhost(char **);
int func_accessctrl_ftp_set(char **), func_accessctrl_ftp_set_port(char **);
int func_accessctrl_web_set(char **), func_accessctrl_web_set_http_port(char **);
int func_accessctrl_web_set_https_port(char **);
int func_accessctrl_web_set_http_session(char **);
int func_system_reboot(char **), func_system_shutdown(char **);
int func_system_set(char **), func_system_set_motd(char **);
int func_nfs_share(char **), func_nfs_unshare(char **);
int func_nfs_enable(char **), func_nfs_disable(char **);
int func_nfs_share_list(char **);

/* Readline integration */
void initialize_readline(const char *progname);
int execute_command(char *line);
char *algoclibase_generator(const char *text, int state);
char *algoclisub_generator(const char *text, int state);
char **algocli_completion(const char *text, int start, int end);
char *stripwhite(char *string);
command_t *find_command(const char *name);
command_t *find_last_command(char *line);
void cmd_ref(void);
char *format_doc(int spcount, const char *text);

int algoauth(void);
void sigint_handler(int signal);

#endif /* ALGOCLI_H */
