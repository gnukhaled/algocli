/* proc.h — privileged subprocess execution.
 *
 * Every command-side caller that used to invoke popen() or system()
 * goes through spawn(). spawn() runs a child via fork + execve with a
 * caller-supplied argv and the global sanitized environment. There is
 * no shell involved, so user-supplied arguments cannot be re-parsed
 * as commands. Each argv element is treated as a single argument by
 * the kernel.
 *
 * Return values:
 *   ≥ 0   — child's exit status (0 = success, n = child returned n)
 *   < 0   — spawn itself failed; value is -errno from the failing
 *           syscall (fork, execve, waitpid, ...)
 */

#ifndef ALGOCLI_PROC_H
#define ALGOCLI_PROC_H

/* Run a privileged subprocess. argv[0] must be an absolute path; argv
 * is a NULL-terminated vector. The child inherits the current process
 * environment (which main() sanitizes at startup; see env.h). stdin/
 * stdout/stderr are inherited so the user sees command output. */
int spawn(char *const argv[]);

/* Initialize the global sanitized environment. Must be called once
 * from main() before any spawn(). Clears the environment, then re-
 * exports a known-safe PATH plus a curated allowlist of variables
 * that were inherited from the invoking user (HOME, USER, TERM, LANG,
 * LC_ALL). After this call, every fork+execve descendant inherits
 * exactly those variables and nothing else. */
void env_sanitize(void);

#endif /* ALGOCLI_PROC_H */
