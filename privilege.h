/* privilege.h — least-privilege helpers for the setuid-root shell.
 *
 * The binary is installed mode 4755 (setuid root). On exec, the
 * kernel sets euid=0 while real-uid stays as the invoking user.
 * Running every command at full root is over-privileged; we instead:
 *
 *   - At startup, drop euid back to the invoking user with
 *     setresuid(real, real, 0). The saved-uid stays at 0 so we can
 *     re-escalate when needed.
 *   - Each privileged syscall (reboot, fork+execve of root tools,
 *     writes under /etc) is bracketed by priv_raise() and priv_drop().
 *
 * If the binary is NOT setuid-root (e.g. running as the real root
 * user, or as a normal user during development), priv_init still
 * succeeds in degraded mode: priv_raise/priv_drop become no-ops and
 * the privileged syscalls will return the same -EPERM they would
 * have without the wrapper. priv_can_escalate() lets callers tell
 * the difference up front.
 */

#ifndef ALGOCLI_PRIVILEGE_H
#define ALGOCLI_PRIVILEGE_H

#include <stdbool.h>
#include <sys/types.h>

/* Call once, early in main(). Returns 0 on success, -1 on a hard
 * failure (which is logged but not necessarily fatal — the caller
 * may continue in unprivileged mode). */
int priv_init(void);

/* Promote effective uid to 0. Returns 0 on success, -errno on
 * failure. A failure here means we either weren't setuid-root or
 * something has set CAP_SETUID-blocking flags; the caller should
 * abandon the privileged operation. */
int priv_raise(void);

/* Demote effective uid back to the invoking user. Returns 0; on
 * failure, calls abort() — failure to drop privileges is treated as
 * a security-critical bug, not a recoverable error. */
void priv_drop(void);

/* True iff priv_raise() can succeed (i.e. we are setuid-root). */
bool priv_can_escalate(void);

/* Username of the invoking user (real-uid), looked up once via
 * getpwuid at priv_init time. Returns "(unknown)" if the lookup
 * failed. The pointer is valid for the program's lifetime. */
const char *priv_user(void);

/* Real uid captured at priv_init time. */
uid_t priv_real_uid(void);

#endif /* ALGOCLI_PRIVILEGE_H */
