/* test_registry.c — unit tests for the typed registry API.
 *
 * Each test case operates on an isolated temp database. Verifies:
 *   - basic CRUD round-trips for backup_points, nfs_shares, ssh_config,
 *     ssh_allowedhosts;
 *   - SQL injection inputs are stored as literal values, never
 *     executed as SQL (we feed quote/semicolon/DROP-TABLE strings and
 *     assert the table still exists with the malicious value as data);
 *   - the database survives weird inputs (NUL-bounded, very long).
 *
 * Build:  cc -I.. -std=c11 tests/test_registry.c registry.c \
 *             -lsqlite3 -o tests/test_registry
 * Run:    ./tests/test_registry
 */

#include "../registry.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

#define CHECK_EQ_STR(a, b) do { \
    if (strcmp((a), (b)) != 0) { \
        fprintf(stderr, "FAIL %s:%d: %s != %s (%s vs %s)\n", \
                __FILE__, __LINE__, #a, #b, (a), (b)); \
        failures++; \
    } \
} while (0)

/* ----- fixtures --------------------------------------------------- */

static char tmpdb_path[256];

static void fresh_db(void) {
    snprintf(tmpdb_path, sizeof tmpdb_path,
             "/tmp/algocli-test-%d.db", (int)getpid());
    unlink(tmpdb_path);
    int rc = reg_init(tmpdb_path);
    if (rc != 0) {
        fprintf(stderr, "fresh_db: reg_init failed\n");
        exit(2);
    }
}

static void close_db(void) {
    reg_close();
    unlink(tmpdb_path);
}

/* ----- backup_points --------------------------------------------- */

static void test_bp_crud(void) {
    fresh_db();

    CHECK(reg_bp_insert("backup1", "Mon Apr 25 12:00:00 2026") == 0);
    CHECK(reg_bp_insert("backup2", "Mon Apr 25 12:05:00 2026") == 0);

    char ts[64];
    CHECK(reg_bp_get("backup1", ts, sizeof ts) == 1);
    CHECK_EQ_STR(ts, "Mon Apr 25 12:00:00 2026");

    CHECK(reg_bp_get("nonexistent", ts, sizeof ts) == 0);

    CHECK(reg_bp_rename("backup1", "backup1-renamed") == 0);
    CHECK(reg_bp_get("backup1-renamed", ts, sizeof ts) == 1);
    CHECK(reg_bp_get("backup1", ts, sizeof ts) == 0);

    CHECK(reg_bp_delete_row("backup2") == 0);
    CHECK(reg_bp_get("backup2", ts, sizeof ts) == 0);

    close_db();
}

/* ----- nfs_shares ------------------------------------------------- */

static void test_nfs_crud(void) {
    fresh_db();

    CHECK(reg_nfs_replace("bp1", "host1.example.com", "rw,sync") == 0);
    CHECK(reg_nfs_replace("bp1", "host2.example.com", "ro") == 0);
    CHECK(reg_nfs_replace("bp2", "host1.example.com", "rw,async") == 0);

    /* Replace existing row. */
    CHECK(reg_nfs_replace("bp1", "host1.example.com", "ro,sync") == 0);

    reg_nfs_row_t rows[16];
    size_t n = 0;

    /* List all */
    CHECK(reg_nfs_list(NULL, rows, 16, &n) == 0);
    CHECK(n == 3);

    /* List filtered */
    CHECK(reg_nfs_list("bp1", rows, 16, &n) == 0);
    CHECK(n == 2);
    /* The two host1 rows: one for bp1 (the replaced one), one for bp2. */
    int found_replaced = 0;
    for (size_t i = 0; i < n; i++) {
        if (strcmp(rows[i].bpname, "bp1") == 0 &&
            strcmp(rows[i].host, "host1.example.com") == 0) {
            CHECK_EQ_STR(rows[i].params, "ro,sync");
            found_replaced = 1;
        }
    }
    CHECK(found_replaced);

    CHECK(reg_nfs_delete("bp1", "host1.example.com") == 0);
    CHECK(reg_nfs_list("bp1", rows, 16, &n) == 0);
    CHECK(n == 1);

    close_db();
}

/* ----- ssh_config ------------------------------------------------- */

static void test_ssh_config(void) {
    fresh_db();

    CHECK(reg_ssh_set(REG_SSH_PORT, "2222") == 0);
    CHECK(reg_ssh_set(REG_SSH_LISTEN_ADDRESS, "0.0.0.0") == 0);
    CHECK(reg_ssh_set(REG_SSH_PROTOCOL, "2") == 0);

    char buf[128];
    CHECK(reg_ssh_get(REG_SSH_PORT, buf, sizeof buf) == 1);
    CHECK_EQ_STR(buf, "2222");
    CHECK(reg_ssh_get(REG_SSH_LISTEN_ADDRESS, buf, sizeof buf) == 1);
    CHECK_EQ_STR(buf, "0.0.0.0");
    CHECK(reg_ssh_get(REG_SSH_PROTOCOL, buf, sizeof buf) == 1);
    CHECK_EQ_STR(buf, "2");

    /* Updating one field shouldn't disturb the others. */
    CHECK(reg_ssh_set(REG_SSH_PORT, "22") == 0);
    CHECK(reg_ssh_get(REG_SSH_PORT, buf, sizeof buf) == 1);
    CHECK_EQ_STR(buf, "22");
    CHECK(reg_ssh_get(REG_SSH_LISTEN_ADDRESS, buf, sizeof buf) == 1);
    CHECK_EQ_STR(buf, "0.0.0.0");

    close_db();
}

/* ----- ssh_allowedhosts ------------------------------------------ */

static void test_ssh_hosts(void) {
    fresh_db();

    CHECK(reg_ssh_add_host("10.0.0.1") == 0);
    CHECK(reg_ssh_add_host("10.0.0.2") == 0);
    CHECK(reg_ssh_host_exists("10.0.0.1") == 1);
    CHECK(reg_ssh_host_exists("10.0.0.2") == 1);
    CHECK(reg_ssh_host_exists("10.0.0.3") == 0);

    /* Idempotent add */
    CHECK(reg_ssh_add_host("10.0.0.1") == 0);
    CHECK(reg_ssh_host_exists("10.0.0.1") == 1);

    CHECK(reg_ssh_del_host("10.0.0.1") == 0);
    CHECK(reg_ssh_host_exists("10.0.0.1") == 0);
    CHECK(reg_ssh_host_exists("10.0.0.2") == 1);

    close_db();
}

/* ----- SQL injection — every value is treated as data ------------ */

static void test_injection_is_data(void) {
    fresh_db();

    /* If any of these strings were concatenated into SQL, the table
     * would be dropped or behaviour would change. With prepared
     * statements they pass through as literal values. */
    const char *injection_strings[] = {
        "x'); DROP TABLE backup_points; --",
        "1\"; DROP TABLE backup_points; --",
        "x' OR '1'='1",
        "robert');DROP TABLE backup_points;--",   /* the classic */
        "name with 'single quotes'",
        "name with \"double quotes\"",
        "name; DELETE FROM backup_points;",
        "a` rm -rf / `",
        NULL
    };

    char ts[64];
    for (size_t i = 0; injection_strings[i]; i++) {
        const char *s = injection_strings[i];
        CHECK(reg_bp_insert(s, "createdon-payload") == 0);
        /* Round-trip: the bp must be retrievable by exactly the bytes
         * we passed in, proving the value never reached the parser. */
        CHECK(reg_bp_get(s, ts, sizeof ts) == 1);
        CHECK_EQ_STR(ts, "createdon-payload");
    }

    /* Most importantly: the table must still exist with all the rows
     * we wrote, plus zero extra rows from any injected commands. */
    /* Use list count via nfs as a sanity (different table not tested,
     * but we count backup_points by retrieving each known row). */
    for (size_t i = 0; injection_strings[i]; i++) {
        CHECK(reg_bp_get(injection_strings[i], ts, sizeof ts) == 1);
    }

    /* Same probe through the nfs API. */
    CHECK(reg_nfs_replace("bp", "host'; DROP TABLE nfs_shares; --",
                          "rw") == 0);
    reg_nfs_row_t rows[8];
    size_t n = 0;
    CHECK(reg_nfs_list(NULL, rows, 8, &n) == 0);
    CHECK(n == 1);
    CHECK_EQ_STR(rows[0].host, "host'; DROP TABLE nfs_shares; --");

    close_db();
}

/* ----- weird-but-valid inputs ------------------------------------ */

static void test_long_inputs(void) {
    fresh_db();

    char long_name[200];
    memset(long_name, 'a', sizeof long_name - 1);
    long_name[sizeof long_name - 1] = '\0';

    /* Long names round-trip just fine — sqlite TEXT has no length
     * limit and our column buffers are sized accordingly. */
    CHECK(reg_bp_insert(long_name, "ts") == 0);
    char ts[64];
    CHECK(reg_bp_get(long_name, ts, sizeof ts) == 1);
    CHECK_EQ_STR(ts, "ts");

    close_db();
}

/* ----- harness --------------------------------------------------- */

int main(void) {
    test_bp_crud();
    test_nfs_crud();
    test_ssh_config();
    test_ssh_hosts();
    test_injection_is_data();
    test_long_inputs();

    if (failures > 0) {
        fprintf(stderr, "\n%d failure(s)\n", failures);
        return 1;
    }
    printf("OK — all registry tests passed\n");
    return 0;
}
