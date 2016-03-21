
#ifndef ALGOCLIDEFS_H
#define ALGOCLIDEFS_H

#include "algocli.h"

// ------------- log commands ---------- 
command_t cmds_log[] = {
    { "list",   func_log_list, (command_t *)NULL,"Log files list"},
    { "view",   func_log_view, (command_t *)NULL,"View log file"},
    { "watch",  func_log_watch, (command_t *)NULL,"Monitor log file"},
    NULL_COMMAND
};
//--------------------------------------


// -------------- fs commands -------------
command_t cmds_fs_quota[] = {
    { "enable",    func_fs_quota_enable, (command_t *)NULL,"Enable filesystem quota"},
    { "disable",   func_fs_quota_disable, (command_t *)NULL,"Disable filesystem quota"},
    { "status",    func_fs_quota_status, (command_t *)NULL,"Check filesystem quota status"},
    NULL_COMMAND
};

command_t cmds_fs[] = {
    { "quota",    func_fs_quota, cmds_fs_quota,"Filesystem quota operations"},
    NULL_COMMAND
};
//--------------------------------------


// -------------- bp commands -----------
command_t cmds_bp_quota[] = {
    { "set",   func_bp_quota_set, (command_t *)NULL,"Set backup point quota"},
    NULL_COMMAND
};

command_t cmds_bp[] = {
    { "create",   func_bp_create, (command_t *)NULL,"Create a backup point"},
    { "delete",   func_bp_delete, (command_t *)NULL,"Delete a backup point"},
    { "rename",   func_bp_rename, (command_t *)NULL,"Rename a backup point"},
    { "quota",    func_bp_quota,  cmds_bp_quota,"Manage backup point quota"},
    { "list",     func_bp_list,   (command_t *)NULL,"List backup point(s)"},
    NULL_COMMAND
};
//----------------------------------------


//--------- accessctrl commands -----------

command_t cmds_accessctrl_ftp_set[] = {
    { "port",   func_accessctrl_ftp_set_port, (command_t *)NULL,"Set ftp port"},
    NULL_COMMAND
};

command_t cmds_accessctrl_web_set[] = {
    { "http-port",   func_accessctrl_web_set_http_port, (command_t *)NULL,"Set http port"},
    { "https-port",   func_accessctrl_web_set_https_port, (command_t *)NULL,"Set https port"},
    { "session-timeout",   func_accessctrl_web_set_http_session, (command_t *)NULL,"Set session timeout"},
    NULL_COMMAND
};

command_t cmds_accessctrl_ssh_set[] = {
    { "port",   func_accessctrl_ssh_set_port, (command_t *)NULL,"Set ssh port"},
    { "protocol",   func_accessctrl_ssh_set_protocol, (command_t *)NULL,"Set ssh protocol version"},
    { "listen-address",   func_accessctrl_ssh_set_listen_address, (command_t *)NULL,"Set ssh network listen address"},
    NULL_COMMAND
};

command_t cmds_accessctrl_ssh_add[] = {
    { "allowed-host",   func_accessctrl_ssh_add_allowedhost, (command_t *)NULL,"Add ssh allowed host"},
    NULL_COMMAND
};

command_t cmds_accessctrl_ssh_del[] = {
    { "allowed-host",   func_accessctrl_ssh_del_allowedhost, (command_t *)NULL,"Delete ssh allowed host"},
    NULL_COMMAND
};

command_t cmds_accessctrl_ssh_list[] = {
    { "config",   func_accessctrl_ssh_list_config, (command_t *)NULL,"Show ssh configurations"},
    { "allowed-hosts",   func_accessctrl_ssh_list_allowedhosts, (command_t *)NULL,"List ssh allowed hosts"},
    NULL_COMMAND
};

command_t cmds_accessctrl_ssh[] = {
    { "disable",   func_accessctrl_ssh_disable, (command_t *)NULL,"Disable ssh access"},
    { "enable",   func_accessctrl_ssh_enable, (command_t *)NULL,"Enable ssh access"},
    { "list",   func_accessctrl_ssh_list, cmds_accessctrl_ssh_list,"List ssh parameters"},
    { "set",   func_accessctrl_ssh_set, cmds_accessctrl_ssh_set,"Set ssh parameters"},
    { "add",   func_accessctrl_ssh_add, cmds_accessctrl_ssh_add,"Add ssh parameter(s)"},
    { "del",   func_accessctrl_ssh_del, cmds_accessctrl_ssh_del,"Delete ssh parameter(s)"},
    NULL_COMMAND
};

command_t cmds_accessctrl_ftp[] = {
    { "disable",   func_accessctrl_ftp_disable, (command_t *)NULL,"Disable ftp access"},
    { "enable",   func_accessctrl_ftp_enable, (command_t *)NULL,"Enable ftp access"},
    { "set",   func_accessctrl_ftp_set, cmds_accessctrl_ftp_set,"Set ftp parameters"},
    NULL_COMMAND
};

command_t cmds_accessctrl_web[] = {
    { "disable",   func_accessctrl_web_disable, (command_t *)NULL,"Disable web access"},
    { "enable",   func_accessctrl_web_enable, (command_t *)NULL,"Enable web access"},
    { "set",   func_accessctrl_web_set, cmds_accessctrl_web_set,"Set web management parameters"},
/*    { "add",   func_accessctrl_web_set, cmds_accessctrl_web_set,"Add web allowed-host"},
    { "del",   func_accessctrl_web_set, cmds_accessctrl_web_set,"Delete web allowed-host"},
    { "reset",   func_accessctrl_web_set, cmds_accessctrl_web_set,"Reset web allowed-host and ports"},*/
    NULL_COMMAND
};

command_t cmds_accessctrl[] = {
    { "ssh",     func_accessctrl_ssh, cmds_accessctrl_ssh,"Control ssh access"},
    { "ftp",     func_accessctrl_ftp, cmds_accessctrl_ftp,"Control ftp access"},
    { "web",     func_accessctrl_web, cmds_accessctrl_web,"Control web access"},
    NULL_COMMAND
};
//----------------------------------------------


//------------------- system command -----------
command_t cmds_system_set[] = {
    {"motd",  func_system_set_motd, (command_t *)NULL, "System message of the day"},
    NULL_COMMAND
};

command_t cmds_system[] = {
    {"reboot",  func_system_reboot, (command_t *)NULL, "System reboot"},
    {"shutdown",  func_system_shutdown, (command_t *)NULL, "System reboot"},
    {"set",  func_system_set, cmds_system_set, "Set system parameter"},
    NULL_COMMAND
};
//----------------------------------------------


//------------------- NFS command---------------
command_t cmds_nfs[] = {
    {"enable",  func_nfs_enable, (command_t *)NULL, "Enable NFS sharing protocol"},
    {"disable",  func_nfs_disable, (command_t *)NULL, "Disable NFS sharing protocol"},
    {"share",   func_nfs_share, (command_t *)NULL, "NFS share a backup point"},
    {"unshare",   func_nfs_unshare, (command_t *)NULL, "NFS unshare a backup point"},
    {"list",   func_nfs_share_list, (command_t *)NULL, "List configured shares"},
    NULL_COMMAND
};
//----------------------------------------------


//--------------------BASE COMMANDS-----------------------
command_t commands[] = {
    { "accessctrl",func_accessctrl, cmds_accessctrl, "Manage administrative access control" },
    { "syshealth", func_syshealth,  (command_t *)NULL,"System health administration and reporting" },
    { "disk",      func_disk,       (command_t *)NULL,"Manage disk hardware operations" },
    { "bp",        func_bp,         cmds_bp,"Create, Delete a backup point" },
    { "fs",        func_fs,         cmds_fs,"Filesystem management" },
    { "snapshot",  func_snapshot,   (command_t *)NULL,"Data snapshot operations" },
    { "vtl",       func_vtl,        (command_t *)NULL,"Virtual tape library management" },
    { "repl",      func_repl,       (command_t *)NULL,"Manage replication operations" },
    { "net",       func_net,        (command_t *)NULL ,"Network administration" },
    { "nfs",       func_nfs,        cmds_nfs ,"NFS sharing operations" },
    { "cifs",      func_cifs,       (command_t *)NULL,"CIFS sharing operations" },
    { "system",    func_system,     cmds_system,"System operations" },
    { "config",    func_config,     (command_t *)NULL ,"Configure system parameters" },
    { "user",      func_user,       (command_t *)NULL,"User management" },
    { "log",       func_log,        cmds_log ,"Logging information" },
    { "help",      func_help,       commands,"Display help" },
    { "exit",      func_exit,       (command_t *)NULL,"Logout" },
    { "?",         func_help,       (command_t *)NULL ,"Synonym for `help'" },
    NULL_COMMAND
};
//--------------------------------------------------------

#endif /* ALGOCLIDEFS_H */
