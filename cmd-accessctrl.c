
#include "algocli.h"
#include "cmd-accessctrl.h"
#include "paths.h"
#include "registry.h"
#include "proc.h"

#include <stdio.h>

#define SYSTEMCTL SYSTEMCTL_BIN

/* Run `systemctl <action> <unit>.service`. Both arguments come from
 * fixed string literals at the call sites (see service callers below),
 * not from user input. We dup them into local mutable buffers because
 * execve takes char *const argv[] — pointer-to-const-pointer-to-char,
 * not pointer-to-pointer-to-const-char. */
static int systemctl(const char *action, const char *unit) {
    char action_buf[16];
    char unit_arg[64];
    if (snprintf(action_buf, sizeof action_buf, "%s", action)
            >= (int)sizeof action_buf) {
        fprintf(stderr, "internal: systemctl action too long\n");
        return -1;
    }
    int n = snprintf(unit_arg, sizeof unit_arg, "%s.service", unit);
    if (n < 0 || (size_t)n >= sizeof unit_arg) {
        fprintf(stderr, "internal: unit name too long\n");
        return -1;
    }
    char systemctl_bin[] = SYSTEMCTL;
    char *const argv[] = { systemctl_bin, action_buf, unit_arg, NULL };
    return spawn(argv);
}

static int service_enable(const char *unit, const char *user_msg) {
    int rc = systemctl("enable", unit);
    if (rc != 0) return rc;
    rc = systemctl("start", unit);
    if (rc == 0) printf("%s enabled\n", user_msg);
    return rc;
}

static int service_disable(const char *unit, const char *user_msg) {
    int rc = systemctl("stop", unit);
    if (rc != 0) return rc;
    rc = systemctl("disable", unit);
    if (rc == 0) printf("%s disabled\n", user_msg);
    return rc;
}

int accessctrl_ssh_enable(void)  { return service_enable ("sshd",   "SSH service"); }
int accessctrl_ssh_disable(void) { return service_disable("sshd",   "SSH service"); }
int accessctrl_ftp_enable(void)  { return service_enable ("vsftpd", "FTP service"); }
int accessctrl_ftp_disable(void) { return service_disable("vsftpd", "FTP service"); }
int accessctrl_web_enable(void)  { return service_enable ("httpd",  "Web interface"); }
int accessctrl_web_disable(void) { return service_disable("httpd",  "Web interface"); }

int ssh_set(int directive, char *val) {
    if (!val) return -1;
    reg_ssh_field_t field;
    switch (directive) {
        case SSH_PORT:       field = REG_SSH_PORT;           break;
        case SSH_LISTENADDR: field = REG_SSH_LISTEN_ADDRESS; break;
        case SSH_PROTO:      field = REG_SSH_PROTOCOL;       break;
        default: return -1;
    }
    return reg_ssh_set(field, val);
}

int ssh_add_host(char *host) {
    return reg_ssh_add_host(host);
}

int ssh_del_host(char *host) {
    return reg_ssh_del_host(host);
}

/* ----- dispatcher shims (formerly in algocli.c) ----------------- */

int func_accessctrl(char **arg) {
    (void)arg;
    cmd_ref();
    return 0;
}

int func_accessctrl_ssh(char **arg) {
    (void)arg;
    cmd_ref();
    return 0;
}

int func_accessctrl_ftp(char **arg) {
    (void)arg;
    cmd_ref();
    return 0;
}

int func_accessctrl_web(char **arg) {
    (void)arg;
    cmd_ref();
    return 0;
}

int func_accessctrl_ssh_list(char **arg)             { (void)arg; return 0; }
int func_accessctrl_ssh_list_config(char **arg)      { (void)arg; return 0; }
int func_accessctrl_ssh_list_allowedhosts(char **arg){ (void)arg; return 0; }

int func_accessctrl_ssh_enable(char **arg)  { (void)arg; return accessctrl_ssh_enable(); }
int func_accessctrl_ssh_disable(char **arg) { (void)arg; return accessctrl_ssh_disable(); }
int func_accessctrl_ftp_enable(char **arg)  { (void)arg; return accessctrl_ftp_enable(); }
int func_accessctrl_ftp_disable(char **arg) { (void)arg; return accessctrl_ftp_disable(); }
int func_accessctrl_web_enable(char **arg)  { (void)arg; return accessctrl_web_enable(); }
int func_accessctrl_web_disable(char **arg) { (void)arg; return accessctrl_web_disable(); }

int func_accessctrl_ssh_set(char **arg) { (void)arg; cmd_ref(); return 0; }
int func_accessctrl_ftp_set(char **arg) { (void)arg; cmd_ref(); return 0; }
int func_accessctrl_web_set(char **arg) { (void)arg; cmd_ref(); return 0; }

int func_accessctrl_ssh_set_port(char **arg) {
    int ret = ssh_set(SSH_PORT, arg[0]);
    if (ret) printf("Failed to set the SSH port\n");
    return ret;
}

int func_accessctrl_ssh_set_protocol(char **arg) {
    int ret = ssh_set(SSH_PROTO, arg[0]);
    if (ret) printf("Failed to set the SSH protocol version\n");
    return ret;
}

int func_accessctrl_ssh_set_listen_address(char **arg) {
    int ret = ssh_set(SSH_LISTENADDR, arg[0]);
    if (ret) printf("Failed to set the SSH network listen address\n");
    return ret;
}

int func_accessctrl_ssh_add(char **arg) { (void)arg; cmd_ref(); return 0; }
int func_accessctrl_ssh_del(char **arg) { (void)arg; cmd_ref(); return 0; }

int func_accessctrl_ssh_add_allowedhost(char **arg) {
    int ret = 0;
    for (int i = 0; arg[i] != NULL; i++) {
        ret = ssh_add_host(arg[i]);
        if (ret) return 1;
    }
    return ret;
}

int func_accessctrl_ssh_del_allowedhost(char **arg) {
    int ret = 0;
    for (int i = 0; arg[i] != NULL; i++) {
        ret = ssh_del_host(arg[i]);
        if (ret) return 1;
    }
    return ret;
}

int func_accessctrl_ftp_set_port(char **arg) {
    (void)arg;
    STUB_NOT_IMPLEMENTED("accessctrl ftp set port");
}

int func_accessctrl_web_set_http_port(char **arg) {
    (void)arg;
    STUB_NOT_IMPLEMENTED("accessctrl web set http-port");
}

int func_accessctrl_web_set_https_port(char **arg) {
    (void)arg;
    STUB_NOT_IMPLEMENTED("accessctrl web set https-port");
}

int func_accessctrl_web_set_http_session(char **arg) {
    (void)arg;
    STUB_NOT_IMPLEMENTED("accessctrl web set session-timeout");
}
