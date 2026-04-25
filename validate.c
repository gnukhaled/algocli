/* validate.c — implementations for input validators.
 *
 * These predicates assume the caller passes a NUL-terminated C string.
 * NULL inputs return false. */

#include "validate.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

/* Internal helper: classify a single byte for identifier use. */
static inline bool name_first_ok(unsigned char c) {
    return isalnum(c) || c == '_';
}
static inline bool name_rest_ok(unsigned char c) {
    return isalnum(c) || c == '_' || c == '.' || c == '-';
}

bool is_safe_name(const char *s) {
    if (!s) return false;
    size_t n = strlen(s);
    if (n == 0 || n > 64) return false;
    if (strcmp(s, ".") == 0 || strcmp(s, "..") == 0) return false;
    if (!name_first_ok((unsigned char)s[0])) return false;
    for (size_t i = 1; i < n; i++) {
        if (!name_rest_ok((unsigned char)s[i])) return false;
    }
    return true;
}

static bool hostname_label_ok(const char *p, size_t len) {
    if (len == 0 || len > 63) return false;
    if (p[0] == '-' || p[len - 1] == '-') return false;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)p[i];
        if (!(isalnum(c) || c == '-')) return false;
    }
    return true;
}

bool is_valid_hostname(const char *s) {
    if (!s) return false;
    size_t total = strlen(s);
    if (total == 0 || total > 253) return false;

    const char *label = s;
    for (size_t i = 0; i <= total; i++) {
        if (s[i] == '.' || s[i] == '\0') {
            size_t lablen = (size_t)(&s[i] - label);
            if (!hostname_label_ok(label, lablen)) return false;
            label = &s[i + 1];
        }
    }
    /* Reject trailing dot followed by empty label (consequence of the
     * loop above accepting "host." — explicitly forbid). */
    if (total > 0 && s[total - 1] == '.') return false;
    return true;
}

bool is_valid_port(const char *s, int *out) {
    if (!s || !*s || !out) return false;
    /* No leading whitespace, no sign — strict digits only. */
    for (const char *p = s; *p; p++) {
        if (!isdigit((unsigned char)*p)) return false;
    }
    char *endp = NULL;
    long v = strtol(s, &endp, 10);
    if (endp == s || (endp && *endp != '\0')) return false;
    if (v < 1 || v > 65535) return false;
    *out = (int)v;
    return true;
}

bool is_valid_listen_addr(const char *s) {
    if (!s) return false;
    unsigned char buf[16];
    if (inet_pton(AF_INET, s, buf) == 1) return true;
    if (inet_pton(AF_INET6, s, buf) == 1) return true;
    return false;
}

bool is_safe_text(const char *s, size_t max_len) {
    if (!s) return false;
    size_t n = strnlen(s, max_len + 1);
    if (n > max_len) return false;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c == '\n' || c == '\t') continue;
        if (c < 0x20 || c >= 0x7f) return false;
    }
    return true;
}
