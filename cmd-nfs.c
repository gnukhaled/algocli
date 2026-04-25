
#include "algocli.h"
#include "cmd-nfs.h"
#include "paths.h"
#include "proc.h"
#include "validate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYSTEMCTL SYSTEMCTL_BIN

/* Helper shared with cmd-accessctrl.c — both manage systemd units. */
static int systemctl(const char *action, const char *unit) {
    char action_buf[16];
    char unit_arg[64];
    if (snprintf(action_buf, sizeof action_buf, "%s", action)
            >= (int)sizeof action_buf) {
        return -1;
    }
    int n = snprintf(unit_arg, sizeof unit_arg, "%s.service", unit);
    if (n < 0 || (size_t)n >= sizeof unit_arg) {
        return -1;
    }
    char systemctl_bin[] = SYSTEMCTL;
    char *const argv[] = { systemctl_bin, action_buf, unit_arg, NULL };
    return spawn(argv);
}

int nfs_enable(void) {
    int rc = systemctl("enable", "nfs-server");
    if (rc != 0) return rc;
    rc = systemctl("start", "nfs-server");
    if (rc == 0) printf("NFS sharing protocol enabled\n");
    return rc;
}

int nfs_disable(void) {
    int rc = systemctl("stop", "nfs-server");
    if (rc != 0) return rc;
    rc = systemctl("disable", "nfs-server");
    if (rc == 0) printf("NFS sharing protocol disabled\n");
    return rc;
}

int nfs_share(char *bp, char *host, char *params) {
    if (!is_safe_name(bp)) {
        fprintf(stderr, "Invalid backup-point name '%s'\n",
                bp ? bp : "(null)");
        return -1;
    }
    if (!is_valid_hostname(host)) {
        fprintf(stderr, "Invalid host '%s'\n", host ? host : "(null)");
        return -1;
    }
    /* params may be NULL → use default. If user-supplied, restrict to
     * a fixed character set that NFS export options actually use. */
    if (params) {
        for (const char *p = params; *p; p++) {
            unsigned char c = (unsigned char)*p;
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                  (c >= '0' && c <= '9') || c == ',' || c == '_')) {
                fprintf(stderr, "Invalid NFS params (allowed: alnum , _)\n");
                return -1;
            }
        }
    }

    return reg_nfs_replace(bp, host, params ? params : NFS_DEFAULT_PARAMS);
}

int nfs_unshare(char *bp, char *host) {
    if (!is_safe_name(bp)) {
        fprintf(stderr, "Invalid backup-point name '%s'\n",
                bp ? bp : "(null)");
        return -1;
    }
    if (!is_valid_hostname(host)) {
        fprintf(stderr, "Invalid host '%s'\n", host ? host : "(null)");
        return -1;
    }
    return reg_nfs_delete(bp, host);
}

int nfs_share_list(char *bpname) {
    if (bpname && !is_safe_name(bpname)) {
        fprintf(stderr, "Invalid backup-point name '%s'\n", bpname);
        return -1;
    }

    enum { MAX_ROWS = 256 };
    reg_nfs_row_t rows[MAX_ROWS];
    size_t n = 0;
    if (reg_nfs_list(bpname, rows, MAX_ROWS, &n) != 0) {
        return -1;
    }

    if (n == 0) {
        printf("(no shares%s%s)\n",
               bpname ? " for " : "",
               bpname ? bpname : "");
        return 0;
    }

    printf("%-32s  %-32s  %s\n", "Backup point", "Host", "Options");
    printf("--------------------------------  --------------------------------  -------\n");
    for (size_t i = 0; i < n; i++) {
        printf("%-32s  %-32s  %s\n",
               rows[i].bpname, rows[i].host, rows[i].params);
    }
    return 0;
}

/* ----- dispatcher shims (formerly in algocli.c) ----------------- */

int func_nfs(char **arg) {
    (void)arg;
    cmd_ref();
    return 0;
}

int func_nfs_enable(char **arg) {
    (void)arg;
    return nfs_enable();
}

int func_nfs_disable(char **arg) {
    (void)arg;
    return nfs_disable();
}

int func_nfs_share(char **arg) {
    return nfs_share(arg[0], arg[1], arg[2]);
}

int func_nfs_unshare(char **arg) {
    return nfs_unshare(arg[0], arg[1]);
}

int func_nfs_share_list(char **arg) {
    return nfs_share_list(arg[0]);
}
