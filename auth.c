/* auth.c — audit log helpers backing the AUTH_GATE macro.
 *
 * Each helper opens (or reopens) the audit log via init_logger and
 * writes a single line via algolog. The current init_logger / algolog
 * pair re-opens the file per call (Phase 3 in the plan keeps this
 * behavior; Phase 6 will keep the FILE* open across calls).
 */

#include "auth.h"
#include "algo-log.h"
#include "privilege.h"

void audit_log_authok(const char *cmd) {
    init_logger(CTX_AUDIT);
    algolog(ALGO_NOTICE, "auth-ok user=%s cmd=%s\n",
            priv_user(), cmd ? cmd : "(unknown)");
}

void audit_log_authfail(const char *cmd) {
    init_logger(CTX_AUDIT);
    algolog(ALGO_WARN, "auth-fail user=%s cmd=%s\n",
            priv_user(), cmd ? cmd : "(unknown)");
}

void audit_log_result(const char *cmd, int rc) {
    init_logger(CTX_AUDIT);
    algolog(ALGO_NOTICE, "result user=%s cmd=%s rc=%d\n",
            priv_user(), cmd ? cmd : "(unknown)", rc);
}
