/* registry.c — typed configuration store backed by sqlite3.
 *
 * The legacy strcat-based DML builder is gone. Every operation here
 * follows the same template:
 *
 *     1. sqlite3_prepare_v2(db, "... ?N ...", -1, &stmt, NULL);
 *     2. sqlite3_bind_text(stmt, N, value, -1, SQLITE_TRANSIENT);
 *     3. step / fetch
 *     4. sqlite3_finalize(stmt)
 *
 * SQL injection is impossible: parameter values never reach the SQL
 * parser. The only way for a user-supplied string to influence query
 * structure would be for someone to concatenate it into a literal SQL
 * string, which this file never does.
 *
 * Memory: result rows are copied into caller-owned storage
 * (reg_nfs_row_t, char buffers). No globals are kept across calls
 * except the database handle itself.
 */

#include "registry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

static sqlite3 *g_db = NULL;

/* Schema applied on first open. Idempotent: CREATE IF NOT EXISTS. */
static const char *SCHEMA_SQL =
    "CREATE TABLE IF NOT EXISTS backup_points("
    "  name TEXT,"
    "  createdon TEXT"
    ");"
    "CREATE TABLE IF NOT EXISTS nfs_shares("
    "  bpname TEXT,"
    "  host   TEXT,"
    "  params TEXT,"
    "  PRIMARY KEY (bpname, host)"
    ");"
    "CREATE TABLE IF NOT EXISTS ssh_config("
    "  Port INT,"
    "  ListenAddress TEXT,"
    "  Protocol TEXT,"
    "  SyslogFacility TEXT,"
    "  PasswordAuthentication TEXT,"
    "  PubkeyAuthentication TEXT,"
    "  MaxAuthTries INT"
    ");"
    "CREATE TABLE IF NOT EXISTS ssh_allowedhosts("
    "  address TEXT PRIMARY KEY"
    ");";

static const char *ssh_column(reg_ssh_field_t f) {
    switch (f) {
        case REG_SSH_PORT:           return "Port";
        case REG_SSH_LISTEN_ADDRESS: return "ListenAddress";
        case REG_SSH_PROTOCOL:       return "Protocol";
    }
    return NULL;
}

/* Internal helper. Prepares a statement, binds a single text param at
 * index 1 (or none if value == NULL), steps, expects no rows, and
 * finalizes. Returns 0/−1. Used for INSERT/DELETE/UPDATE. */
static int exec_stmt(sqlite3_stmt *stmt) {
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "registry: %s\n", sqlite3_errmsg(g_db));
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);
    return 0;
}

int reg_init(const char *path) {
    if (g_db) return 0;            /* already initialized */
    if (!path) path = REGISTRY_PATH_DEFAULT;

    int rc = sqlite3_open(path, &g_db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "registry: cannot open %s: %s\n",
                path, sqlite3_errmsg(g_db));
        sqlite3_close(g_db);
        g_db = NULL;
        return -1;
    }

    char *err = NULL;
    rc = sqlite3_exec(g_db, SCHEMA_SQL, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "registry: schema setup failed: %s\n", err);
        sqlite3_free(err);
        sqlite3_close(g_db);
        g_db = NULL;
        return -1;
    }
    return 0;
}

void reg_close(void) {
    if (g_db) {
        sqlite3_close(g_db);
        g_db = NULL;
    }
}

/* ----- backup_points ---------------------------------------------- */

int reg_bp_insert(const char *name, const char *createdon) {
    if (!g_db || !name) return -1;
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db,
            "INSERT INTO backup_points(name, createdon) VALUES (?1, ?2)",
            -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "registry: prepare: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, createdon ? createdon : "", -1, SQLITE_TRANSIENT);
    return exec_stmt(stmt);
}

int reg_bp_delete_row(const char *name) {
    if (!g_db || !name) return -1;
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db,
            "DELETE FROM backup_points WHERE name = ?1",
            -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "registry: prepare: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    return exec_stmt(stmt);
}

int reg_bp_rename(const char *old_name, const char *new_name) {
    if (!g_db || !old_name || !new_name) return -1;
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db,
            "UPDATE backup_points SET name = ?1 WHERE name = ?2",
            -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "registry: prepare: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    sqlite3_bind_text(stmt, 1, new_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, old_name, -1, SQLITE_TRANSIENT);
    return exec_stmt(stmt);
}

int reg_bp_get(const char *name, char *createdon_out, size_t out_size) {
    if (!g_db || !name) return -1;
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db,
            "SELECT createdon FROM backup_points WHERE name = ?1",
            -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "registry: prepare: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    int result = -1;
    if (rc == SQLITE_ROW) {
        if (createdon_out && out_size > 0) {
            const unsigned char *t = sqlite3_column_text(stmt, 0);
            snprintf(createdon_out, out_size, "%s", t ? (const char *)t : "");
        }
        result = 1;
    } else if (rc == SQLITE_DONE) {
        result = 0;
    } else {
        fprintf(stderr, "registry: %s\n", sqlite3_errmsg(g_db));
    }
    sqlite3_finalize(stmt);
    return result;
}

/* ----- nfs_shares ------------------------------------------------- */

int reg_nfs_replace(const char *bp, const char *host, const char *params) {
    if (!g_db || !bp || !host) return -1;
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db,
            "INSERT OR REPLACE INTO nfs_shares(bpname, host, params) "
            "VALUES (?1, ?2, ?3)",
            -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "registry: prepare: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    sqlite3_bind_text(stmt, 1, bp,     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, host,   -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, params ? params : "", -1, SQLITE_TRANSIENT);
    return exec_stmt(stmt);
}

int reg_nfs_delete(const char *bp, const char *host) {
    if (!g_db || !bp || !host) return -1;
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db,
            "DELETE FROM nfs_shares WHERE bpname = ?1 AND host = ?2",
            -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "registry: prepare: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    sqlite3_bind_text(stmt, 1, bp,   -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, host, -1, SQLITE_TRANSIENT);
    return exec_stmt(stmt);
}

int reg_nfs_list(const char *filter_bp,
                 reg_nfs_row_t *rows, size_t max_rows, size_t *out_n) {
    if (!g_db || !rows || !out_n) return -1;
    *out_n = 0;

    sqlite3_stmt *stmt = NULL;
    const char *sql = filter_bp
        ? "SELECT bpname, host, params FROM nfs_shares "
          "WHERE bpname = ?1 ORDER BY bpname, host"
        : "SELECT bpname, host, params FROM nfs_shares "
          "ORDER BY bpname, host";

    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "registry: prepare: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    if (filter_bp) {
        sqlite3_bind_text(stmt, 1, filter_bp, -1, SQLITE_TRANSIENT);
    }

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW && *out_n < max_rows) {
        reg_nfs_row_t *r = &rows[*out_n];
        const unsigned char *bp = sqlite3_column_text(stmt, 0);
        const unsigned char *h  = sqlite3_column_text(stmt, 1);
        const unsigned char *p  = sqlite3_column_text(stmt, 2);
        snprintf(r->bpname, sizeof r->bpname, "%s", bp ? (const char *)bp : "");
        snprintf(r->host,   sizeof r->host,   "%s", h  ? (const char *)h  : "");
        snprintf(r->params, sizeof r->params, "%s", p  ? (const char *)p  : "");
        (*out_n)++;
    }
    if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
        fprintf(stderr, "registry: %s\n", sqlite3_errmsg(g_db));
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);
    return 0;
}

/* ----- ssh_config (single-row table) ------------------------------ */

int reg_ssh_set(reg_ssh_field_t field, const char *value) {
    if (!g_db || !value) return -1;
    const char *col = ssh_column(field);
    if (!col) return -1;

    /* Ensure exactly one row exists. INSERT a default row first if
     * empty; then UPDATE the requested column. The INSERT is a no-op
     * if the table already has a row (we check via SELECT COUNT). */
    sqlite3_stmt *stmt = NULL;
    int has_row = 0;
    if (sqlite3_prepare_v2(g_db,
            "SELECT COUNT(*) FROM ssh_config", -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            has_row = sqlite3_column_int(stmt, 0) > 0;
        }
        sqlite3_finalize(stmt);
    }
    if (!has_row) {
        if (sqlite3_exec(g_db,
                "INSERT INTO ssh_config(Port) VALUES (NULL)",
                NULL, NULL, NULL) != SQLITE_OK) {
            fprintf(stderr, "registry: %s\n", sqlite3_errmsg(g_db));
            return -1;
        }
    }

    /* Build the UPDATE with a literal column name from a fixed allow-
     * list (ssh_column above) — never user-supplied — so this is
     * still injection-safe. */
    char sql[128];
    snprintf(sql, sizeof sql, "UPDATE ssh_config SET %s = ?1", col);
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "registry: prepare: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    sqlite3_bind_text(stmt, 1, value, -1, SQLITE_TRANSIENT);
    return exec_stmt(stmt);
}

int reg_ssh_get(reg_ssh_field_t field, char *out, size_t out_size) {
    if (!g_db || !out || out_size == 0) return -1;
    const char *col = ssh_column(field);
    if (!col) return -1;

    char sql[128];
    snprintf(sql, sizeof sql, "SELECT %s FROM ssh_config LIMIT 1", col);
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "registry: prepare: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    int result = 0;
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char *t = sqlite3_column_text(stmt, 0);
        snprintf(out, out_size, "%s", t ? (const char *)t : "");
        result = 1;
    } else if (rc != SQLITE_DONE) {
        fprintf(stderr, "registry: %s\n", sqlite3_errmsg(g_db));
        result = -1;
    }
    sqlite3_finalize(stmt);
    return result;
}

/* ----- ssh_allowedhosts ------------------------------------------- */

int reg_ssh_add_host(const char *host) {
    if (!g_db || !host) return -1;
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db,
            "INSERT OR REPLACE INTO ssh_allowedhosts(address) VALUES (?1)",
            -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "registry: prepare: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    sqlite3_bind_text(stmt, 1, host, -1, SQLITE_TRANSIENT);
    return exec_stmt(stmt);
}

int reg_ssh_del_host(const char *host) {
    if (!g_db || !host) return -1;
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db,
            "DELETE FROM ssh_allowedhosts WHERE address = ?1",
            -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "registry: prepare: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    sqlite3_bind_text(stmt, 1, host, -1, SQLITE_TRANSIENT);
    return exec_stmt(stmt);
}

int reg_ssh_host_exists(const char *host) {
    if (!g_db || !host) return -1;
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db,
            "SELECT 1 FROM ssh_allowedhosts WHERE address = ?1",
            -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "registry: prepare: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    sqlite3_bind_text(stmt, 1, host, -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    int result = -1;
    if (rc == SQLITE_ROW)      result = 1;
    else if (rc == SQLITE_DONE) result = 0;
    else fprintf(stderr, "registry: %s\n", sqlite3_errmsg(g_db));
    sqlite3_finalize(stmt);
    return result;
}
