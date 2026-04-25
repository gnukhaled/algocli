/* algocli-defs.c — the master command table.
 *
 * This file owns the entire static command tree. Each subarray is
 * file-static (the header doesn't need to see them — the only thing
 * external code touches is `commands[]`).
 *
 * Adding a command:
 *   1. Implement the operation in cmd-<area>.c.
 *   2. Add a func_* shim in the same cmd-<area>.c (with whatever
 *      validation it needs); declare in cmd-<area>.h or algocli.h.
 *   3. Add the entry below. If the command mutates state, OR it with
 *      CMD_AUTH — the dispatcher will call algoauth() before
 *      invoking func_*. No need to AUTH_GATE inside the func itself.
 *
 * The original 5-field layout was:
 *   { "name", func, subcmd, "doc" }
 * We added a 5th `flags` field. Read-only commands omit CMD_AUTH and
 * default to flags=0; state-changing commands set CMD_AUTH.
 */

#include "algocli.h"

/* ----- log ------------------------------------------------------ */
static command_t cmds_log[] = {
    { "list",  func_log_list,  (command_t *)NULL, "Log files list",  0 },
    { "view",  func_log_view,  (command_t *)NULL, "View log file",   0 },
    { "watch", func_log_watch, (command_t *)NULL, "Monitor log file",0 },
    NULL_COMMAND
};

/* ----- fs ------------------------------------------------------- */
static command_t cmds_fs_quota[] = {
    { "enable",  func_fs_quota_enable,  (command_t *)NULL, "Enable filesystem quota",       CMD_AUTH },
    { "disable", func_fs_quota_disable, (command_t *)NULL, "Disable filesystem quota",      CMD_AUTH },
    { "status",  func_fs_quota_status,  (command_t *)NULL, "Check filesystem quota status", 0 },
    NULL_COMMAND
};

static command_t cmds_fs[] = {
    { "quota", func_fs_quota, cmds_fs_quota, "Filesystem quota operations", 0 },
    NULL_COMMAND
};

/* ----- bp ------------------------------------------------------- */
static command_t cmds_bp_quota[] = {
    { "set", func_bp_quota_set, (command_t *)NULL, "Set backup point quota", CMD_AUTH },
    NULL_COMMAND
};

static command_t cmds_bp[] = {
    { "create", func_bp_create, (command_t *)NULL, "Create a backup point", CMD_AUTH },
    { "delete", func_bp_delete, (command_t *)NULL, "Delete a backup point", CMD_AUTH },
    { "rename", func_bp_rename, (command_t *)NULL, "Rename a backup point", CMD_AUTH },
    { "quota",  func_bp_quota,  cmds_bp_quota,    "Manage backup point quota", 0 },
    { "list",   func_bp_list,   (command_t *)NULL, "List backup point(s)", 0 },
    NULL_COMMAND
};

/* ----- accessctrl ----------------------------------------------- */
static command_t cmds_accessctrl_ftp_set[] = {
    { "port", func_accessctrl_ftp_set_port, (command_t *)NULL, "Set ftp port", CMD_AUTH },
    NULL_COMMAND
};

static command_t cmds_accessctrl_web_set[] = {
    { "http-port",       func_accessctrl_web_set_http_port,    (command_t *)NULL, "Set http port",       CMD_AUTH },
    { "https-port",      func_accessctrl_web_set_https_port,   (command_t *)NULL, "Set https port",      CMD_AUTH },
    { "session-timeout", func_accessctrl_web_set_http_session, (command_t *)NULL, "Set session timeout", CMD_AUTH },
    NULL_COMMAND
};

static command_t cmds_accessctrl_ssh_set[] = {
    { "port",           func_accessctrl_ssh_set_port,           (command_t *)NULL, "Set ssh port",                  CMD_AUTH },
    { "protocol",       func_accessctrl_ssh_set_protocol,       (command_t *)NULL, "Set ssh protocol version",      CMD_AUTH },
    { "listen-address", func_accessctrl_ssh_set_listen_address, (command_t *)NULL, "Set ssh network listen address", CMD_AUTH },
    NULL_COMMAND
};

static command_t cmds_accessctrl_ssh_add[] = {
    { "allowed-host", func_accessctrl_ssh_add_allowedhost, (command_t *)NULL, "Add ssh allowed host", CMD_AUTH },
    NULL_COMMAND
};

static command_t cmds_accessctrl_ssh_del[] = {
    { "allowed-host", func_accessctrl_ssh_del_allowedhost, (command_t *)NULL, "Delete ssh allowed host", CMD_AUTH },
    NULL_COMMAND
};

static command_t cmds_accessctrl_ssh_list[] = {
    { "config",        func_accessctrl_ssh_list_config,       (command_t *)NULL, "Show ssh configurations", 0 },
    { "allowed-hosts", func_accessctrl_ssh_list_allowedhosts, (command_t *)NULL, "List ssh allowed hosts",  0 },
    NULL_COMMAND
};

static command_t cmds_accessctrl_ssh[] = {
    { "disable", func_accessctrl_ssh_disable, (command_t *)NULL,         "Disable ssh access",     CMD_AUTH },
    { "enable",  func_accessctrl_ssh_enable,  (command_t *)NULL,         "Enable ssh access",      CMD_AUTH },
    { "list",    func_accessctrl_ssh_list,    cmds_accessctrl_ssh_list,  "List ssh parameters",    0 },
    { "set",     func_accessctrl_ssh_set,     cmds_accessctrl_ssh_set,   "Set ssh parameters",     0 },
    { "add",     func_accessctrl_ssh_add,     cmds_accessctrl_ssh_add,   "Add ssh parameter(s)",   0 },
    { "del",     func_accessctrl_ssh_del,     cmds_accessctrl_ssh_del,   "Delete ssh parameter(s)", 0 },
    NULL_COMMAND
};

static command_t cmds_accessctrl_ftp[] = {
    { "disable", func_accessctrl_ftp_disable, (command_t *)NULL,         "Disable ftp access",     CMD_AUTH },
    { "enable",  func_accessctrl_ftp_enable,  (command_t *)NULL,         "Enable ftp access",      CMD_AUTH },
    { "set",     func_accessctrl_ftp_set,     cmds_accessctrl_ftp_set,   "Set ftp parameters",     0 },
    NULL_COMMAND
};

static command_t cmds_accessctrl_web[] = {
    { "disable", func_accessctrl_web_disable, (command_t *)NULL,         "Disable web access",     CMD_AUTH },
    { "enable",  func_accessctrl_web_enable,  (command_t *)NULL,         "Enable web access",      CMD_AUTH },
    { "set",     func_accessctrl_web_set,     cmds_accessctrl_web_set,   "Set web management parameters", 0 },
    NULL_COMMAND
};

static command_t cmds_accessctrl[] = {
    { "ssh", func_accessctrl_ssh, cmds_accessctrl_ssh, "Control ssh access", 0 },
    { "ftp", func_accessctrl_ftp, cmds_accessctrl_ftp, "Control ftp access", 0 },
    { "web", func_accessctrl_web, cmds_accessctrl_web, "Control web access", 0 },
    NULL_COMMAND
};

/* ----- system --------------------------------------------------- */
static command_t cmds_system_set[] = {
    { "motd", func_system_set_motd, (command_t *)NULL, "System message of the day", CMD_AUTH },
    NULL_COMMAND
};

static command_t cmds_system[] = {
    { "reboot",   func_system_reboot,   (command_t *)NULL, "System reboot",        CMD_AUTH },
    { "shutdown", func_system_shutdown, (command_t *)NULL, "System shutdown",      CMD_AUTH },
    { "set",      func_system_set,      cmds_system_set,   "Set system parameter", 0 },
    NULL_COMMAND
};

/* ----- nfs ------------------------------------------------------ */
static command_t cmds_nfs[] = {
    { "enable",  func_nfs_enable,     (command_t *)NULL, "Enable NFS sharing protocol",  CMD_AUTH },
    { "disable", func_nfs_disable,    (command_t *)NULL, "Disable NFS sharing protocol", CMD_AUTH },
    { "share",   func_nfs_share,      (command_t *)NULL, "NFS share a backup point",     CMD_AUTH },
    { "unshare", func_nfs_unshare,    (command_t *)NULL, "NFS unshare a backup point",   CMD_AUTH },
    { "list",    func_nfs_share_list, (command_t *)NULL, "List configured shares",       0 },
    NULL_COMMAND
};

/* ----- root ----------------------------------------------------- */
command_t commands[] = {
    { "accessctrl", func_accessctrl, cmds_accessctrl, "Manage administrative access control", 0 },
    { "syshealth",  func_syshealth,  (command_t *)NULL, "System health administration and reporting", 0 },
    { "disk",       func_disk,       (command_t *)NULL, "Manage disk hardware operations", 0 },
    { "bp",         func_bp,         cmds_bp,         "Create, Delete a backup point", 0 },
    { "fs",         func_fs,         cmds_fs,         "Filesystem management", 0 },
    { "snapshot",   func_snapshot,   (command_t *)NULL, "Data snapshot operations", 0 },
    { "vtl",        func_vtl,        (command_t *)NULL, "Virtual tape library management", 0 },
    { "repl",       func_repl,       (command_t *)NULL, "Manage replication operations", 0 },
    { "net",        func_net,        (command_t *)NULL, "Network administration", 0 },
    { "nfs",        func_nfs,        cmds_nfs,        "NFS sharing operations", 0 },
    { "cifs",       func_cifs,       (command_t *)NULL, "CIFS sharing operations", 0 },
    { "system",     func_system,     cmds_system,     "System operations", 0 },
    { "config",     func_config,     (command_t *)NULL, "Configure system parameters", 0 },
    { "user",       func_user,       (command_t *)NULL, "User management", 0 },
    { "log",        func_log,        cmds_log,        "Logging information", 0 },
    { "help",       func_help,       commands,        "Display help", 0 },
    { "exit",       func_exit,       (command_t *)NULL, "Logout", 0 },
    { "?",          func_help,       (command_t *)NULL, "Synonym for `help'", 0 },
    NULL_COMMAND
};
