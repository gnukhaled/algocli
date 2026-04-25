/* validate.h — input validators for command-side user data.
 *
 * Every value that crosses the boundary into a system call (execve
 * argv, sqlite3_bind_text) or back into our own SQL must be vetted
 * here first. The predicates are deliberately strict — false negatives
 * (a legitimate but unusual hostname rejected) are far cheaper than
 * false positives (a malicious string accepted into a shell or query).
 *
 * No external dependencies beyond libc and POSIX.
 */

#ifndef ALGOCLI_VALIDATE_H
#define ALGOCLI_VALIDATE_H

#include <stdbool.h>
#include <stddef.h>

/* Identifier (backup-point names, share names, log filenames):
 *   - 1..64 chars
 *   - first char alphanumeric or underscore
 *   - subsequent chars: A-Z a-z 0-9 _ . -
 *   - explicitly rejects "." and ".." which are filesystem traps */
bool is_safe_name(const char *s);

/* Hostname per RFC-952/1123 character class:
 *   - 1..253 chars total
 *   - labels of 1..63 chars separated by '.'
 *   - each label: A-Z a-z 0-9 -, must not start or end with '-' */
bool is_valid_hostname(const char *s);

/* TCP/UDP port. Returns true on success and fills *out (1..65535).
 * Strictly numeric; no leading whitespace, no sign, no trailing chars. */
bool is_valid_port(const char *s, int *out);

/* IPv4 or IPv6 listen address — accepts only what inet_pton accepts. */
bool is_valid_listen_addr(const char *s);

/* Plain ASCII printable text plus newline, ≤max_len. Used for MOTD
 * content and other free-form fields where we want to reject control
 * characters and non-ASCII before persisting. */
bool is_safe_text(const char *s, size_t max_len);

#endif /* ALGOCLI_VALIDATE_H */
