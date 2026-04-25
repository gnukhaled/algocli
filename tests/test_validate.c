/* test_validate.c — table-driven tests for the validators.
 *
 * Build:  cc -I.. -std=c11 -Wall -Wextra tests/test_validate.c \
 *             validate.c -o tests/test_validate
 * Run:    ./tests/test_validate
 *
 * Exits 0 on success, prints the first failing case and exits 1
 * otherwise. No external test framework. */

#include "../validate.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static void test_safe_name(void) {
    /* Accept */
    CHECK(is_safe_name("backup1"));
    CHECK(is_safe_name("nightly_full"));
    CHECK(is_safe_name("snap.2026-04-25"));
    CHECK(is_safe_name("a"));
    CHECK(is_safe_name("_underscore_first"));
    CHECK(is_safe_name("with-dash"));

    /* Reject — empty / overlong / NULL */
    CHECK(!is_safe_name(NULL));
    CHECK(!is_safe_name(""));
    char overlong[80];
    memset(overlong, 'a', sizeof overlong - 1);
    overlong[sizeof overlong - 1] = '\0';
    CHECK(!is_safe_name(overlong));

    /* Reject — filesystem traps */
    CHECK(!is_safe_name("."));
    CHECK(!is_safe_name(".."));
    CHECK(!is_safe_name(".hidden"));        /* leading dot */
    CHECK(!is_safe_name("-leadingdash"));   /* leading dash */

    /* Reject — shell metacharacters and quoting */
    CHECK(!is_safe_name("a;rm -rf /"));
    CHECK(!is_safe_name("a$(id)"));
    CHECK(!is_safe_name("a`id`"));
    CHECK(!is_safe_name("a|b"));
    CHECK(!is_safe_name("a&b"));
    CHECK(!is_safe_name("a>b"));
    CHECK(!is_safe_name("a<b"));
    CHECK(!is_safe_name("a b"));            /* whitespace */
    CHECK(!is_safe_name("a\tb"));
    CHECK(!is_safe_name("a\nb"));
    CHECK(!is_safe_name("a'b"));
    CHECK(!is_safe_name("a\"b"));
    CHECK(!is_safe_name("a\\b"));

    /* Reject — path traversal */
    CHECK(!is_safe_name("a/b"));
    CHECK(!is_safe_name("../etc/passwd"));

    /* Reject — SQL metacharacters */
    CHECK(!is_safe_name("a';--"));
    CHECK(!is_safe_name("a)"));
}

static void test_hostname(void) {
    /* Accept */
    CHECK(is_valid_hostname("localhost"));
    CHECK(is_valid_hostname("backup.example.com"));
    CHECK(is_valid_hostname("a"));
    CHECK(is_valid_hostname("a-b"));
    CHECK(is_valid_hostname("123host"));      /* RFC-1123 allows leading digit */
    CHECK(is_valid_hostname("host-1.subnet-2.example.com"));

    /* Reject */
    CHECK(!is_valid_hostname(NULL));
    CHECK(!is_valid_hostname(""));
    CHECK(!is_valid_hostname("-leadingdash.com"));
    CHECK(!is_valid_hostname("trailingdash-.com"));
    CHECK(!is_valid_hostname("host..double-dot"));
    CHECK(!is_valid_hostname("trailing.dot."));
    CHECK(!is_valid_hostname("under_score.com"));   /* not allowed in hostname */
    CHECK(!is_valid_hostname("host with space"));
    CHECK(!is_valid_hostname("host;rm"));
    CHECK(!is_valid_hostname("host';DROP TABLE--"));

    /* Length limits */
    char overlong_label[70];
    memset(overlong_label, 'a', sizeof overlong_label - 1);
    overlong_label[sizeof overlong_label - 1] = '\0';
    CHECK(!is_valid_hostname(overlong_label));

    char overlong_total[260];
    memset(overlong_total, 'a', sizeof overlong_total - 1);
    overlong_total[sizeof overlong_total - 1] = '\0';
    CHECK(!is_valid_hostname(overlong_total));
}

static void test_port(void) {
    int p;
    CHECK(is_valid_port("22", &p) && p == 22);
    CHECK(is_valid_port("1", &p) && p == 1);
    CHECK(is_valid_port("65535", &p) && p == 65535);
    CHECK(is_valid_port("80", &p) && p == 80);

    CHECK(!is_valid_port(NULL, &p));
    CHECK(!is_valid_port("", &p));
    CHECK(!is_valid_port("0", &p));
    CHECK(!is_valid_port("65536", &p));
    CHECK(!is_valid_port("99999", &p));
    CHECK(!is_valid_port("-1", &p));
    CHECK(!is_valid_port(" 22", &p));        /* leading whitespace */
    CHECK(!is_valid_port("22 ", &p));        /* trailing whitespace */
    CHECK(!is_valid_port("22a", &p));
    CHECK(!is_valid_port("22;ls", &p));
    CHECK(!is_valid_port("0x16", &p));
}

static void test_listen_addr(void) {
    CHECK(is_valid_listen_addr("0.0.0.0"));
    CHECK(is_valid_listen_addr("127.0.0.1"));
    CHECK(is_valid_listen_addr("192.168.1.1"));
    CHECK(is_valid_listen_addr("::"));
    CHECK(is_valid_listen_addr("::1"));
    CHECK(is_valid_listen_addr("2001:db8::1"));
    CHECK(is_valid_listen_addr("fe80::1"));

    CHECK(!is_valid_listen_addr(NULL));
    CHECK(!is_valid_listen_addr(""));
    CHECK(!is_valid_listen_addr("999.0.0.1"));
    CHECK(!is_valid_listen_addr("1.2.3"));
    CHECK(!is_valid_listen_addr("0.0.0.0;ls"));
    CHECK(!is_valid_listen_addr("not-an-address"));
}

static void test_safe_text(void) {
    CHECK(is_safe_text("hello world", 4096));
    CHECK(is_safe_text("multi\nline\ntext", 4096));
    CHECK(is_safe_text("", 4096));
    CHECK(is_safe_text("tabs\tare\tok", 4096));

    CHECK(!is_safe_text(NULL, 4096));
    CHECK(!is_safe_text("\x01control", 4096));
    CHECK(!is_safe_text("\x7f", 4096));
    CHECK(!is_safe_text("non-ascii: \xc3\xa9", 4096));   /* UTF-8 */
    CHECK(!is_safe_text("escape: \x1b[31m", 4096));      /* ANSI esc */
    char overlong[5000];
    memset(overlong, 'a', sizeof overlong - 1);
    overlong[sizeof overlong - 1] = '\0';
    CHECK(!is_safe_text(overlong, 4096));
}

int main(void) {
    test_safe_name();
    test_hostname();
    test_port();
    test_listen_addr();
    test_safe_text();

    if (failures > 0) {
        fprintf(stderr, "\n%d failure(s)\n", failures);
        return 1;
    }
    printf("OK — all validator tests passed\n");
    return 0;
}
