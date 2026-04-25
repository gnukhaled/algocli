/* privilege.c — implementation of the privilege helpers. */

#include "privilege.h"

#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

static uid_t g_real_uid;
static gid_t g_real_gid;
static bool  g_can_escalate = false;
static char  g_user[64] = "(unknown)";

int priv_init(void) {
    g_real_uid = getuid();
    g_real_gid = getgid();

    /* Look up the username for audit logging. We do this *before*
     * dropping privileges so the password file lookup runs as root,
     * sidestepping any nsswitch source that needs root to read. */
    struct passwd *pw = getpwuid(g_real_uid);
    if (pw && pw->pw_name) {
        snprintf(g_user, sizeof g_user, "%s", pw->pw_name);
    }

    /* If we're already real-root or not setuid-root at all, there's
     * nothing to drop and nothing to escalate to. */
    if (geteuid() == g_real_uid) {
        g_can_escalate = (g_real_uid == 0);   /* root-as-root only */
        return 0;
    }

    /* Setuid-root case. Drop effective uid; keep saved-uid at 0 so we
     * can re-raise via seteuid(0). setresuid is the right primitive:
     * setuid() would also drop the saved-uid and lock us out. */
    if (setresuid(g_real_uid, g_real_uid, 0) != 0) {
        fprintf(stderr, "priv_init: setresuid failed: %s\n", strerror(errno));
        return -1;
    }
    g_can_escalate = true;
    return 0;
}

int priv_raise(void) {
    if (!g_can_escalate) return -EPERM;
    if (seteuid(0) != 0) {
        int e = errno;
        fprintf(stderr, "priv_raise: seteuid(0): %s\n", strerror(e));
        return -e;
    }
    return 0;
}

void priv_drop(void) {
    if (!g_can_escalate) return;
    /* Failure to drop is unrecoverable — leaving the process at root
     * after a privileged op is the security bug we are explicitly
     * trying to avoid. */
    if (seteuid(g_real_uid) != 0) {
        fprintf(stderr, "priv_drop: seteuid(%u): %s — aborting\n",
                (unsigned)g_real_uid, strerror(errno));
        abort();
    }
}

bool priv_can_escalate(void) {
    return g_can_escalate;
}

const char *priv_user(void) {
    return g_user;
}

uid_t priv_real_uid(void) {
    return g_real_uid;
}
