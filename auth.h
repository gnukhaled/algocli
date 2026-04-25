/* auth.h — auth-gate macro and audit helpers.
 *
 * Every state-changing func_* in algocli.c starts with AUTH_GATE("cmd").
 * The macro:
 *   1. Calls algoauth() (PAM re-authentication of the invoking user).
 *   2. On failure, writes a log line to /tmp/algocli-audit.log and
 *      returns -1 from the surrounding func_*.
 *   3. On success, writes a "permitted" log line and falls through to
 *      the command body.
 *
 * Read-only commands (help, list, view, status) do *not* gate. They
 * are safe by construction.
 */

#ifndef ALGOCLI_AUTH_H
#define ALGOCLI_AUTH_H

int  algoauth(void);   /* defined in algocli.c */
void audit_log_authok(const char *cmd);
void audit_log_authfail(const char *cmd);
void audit_log_result(const char *cmd, int rc);

#define AUTH_GATE(cmd_name) do { \
    if (!algoauth()) { \
        audit_log_authfail(cmd_name); \
        return -1; \
    } \
    audit_log_authok(cmd_name); \
} while (0)

#endif /* ALGOCLI_AUTH_H */
