#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "../../include/database/registry_new.h"
#include "../../include/core/config.h"

/* Global database connection */
static sqlite3 *db = NULL;
static char db_path[MAX_PATH_LENGTH] = {0};

/* Initialize database connection */
int registry_init(const char *db_file) {
    if (!db_file) {
        return REG_ERROR_INVALID_ARGS;
    }

    if (db != NULL) {
        /* Already initialized */
        return REG_SUCCESS;
    }

    strncpy(db_path, db_file, MAX_PATH_LENGTH - 1);
    db_path[MAX_PATH_LENGTH - 1] = '\0';

    int rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open registry database: %s\n",
                sqlite3_errmsg(db));
        sqlite3_close(db);
        db = NULL;
        return REG_ERROR_DB_OPEN;
    }

    return REG_SUCCESS;
}

/* Close database connection */
void registry_close(void) {
    if (db != NULL) {
        sqlite3_close(db);
        db = NULL;
    }
}

/* Get database connection */
sqlite3* registry_get_connection(void) {
    return db;
}

/* Backup point operations */

int registry_insert_backup_point(const char *name, const char *created_on) {
    if (!name || !created_on || !db) {
        return REG_ERROR_INVALID_ARGS;
    }

    const char *sql = "INSERT INTO backup_points (name, createdon) VALUES (?, ?)";
    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return REG_ERROR_PREPARE;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, created_on, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert backup point: %s\n",
                sqlite3_errmsg(db));
        return REG_ERROR_EXEC;
    }

    return REG_SUCCESS;
}

int registry_delete_backup_point(const char *name) {
    if (!name || !db) {
        return REG_ERROR_INVALID_ARGS;
    }

    const char *sql = "DELETE FROM backup_points WHERE name = ?";
    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return REG_ERROR_PREPARE;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to delete backup point: %s\n",
                sqlite3_errmsg(db));
        return REG_ERROR_EXEC;
    }

    return REG_SUCCESS;
}

int registry_update_backup_point_quota(const char *name, const char *quota) {
    if (!name || !quota || !db) {
        return REG_ERROR_INVALID_ARGS;
    }

    const char *sql = "UPDATE backup_points SET quota = ? WHERE name = ?";
    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return REG_ERROR_PREPARE;
    }

    sqlite3_bind_text(stmt, 1, quota, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to update backup point quota: %s\n",
                sqlite3_errmsg(db));
        return REG_ERROR_EXEC;
    }

    return REG_SUCCESS;
}

int registry_list_backup_points(registry_callback_t callback, void *user_data) {
    if (!callback || !db) {
        return REG_ERROR_INVALID_ARGS;
    }

    const char *sql = "SELECT * FROM backup_points";

    char *err_msg = NULL;
    int rc = sqlite3_exec(db, sql, callback, user_data, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to list backup points: %s\n", err_msg);
        sqlite3_free(err_msg);
        return REG_ERROR_EXEC;
    }

    return REG_SUCCESS;
}

/* NFS share operations */

int registry_insert_nfs_share(const char *bpname, const char *host,
                              const char *params) {
    if (!bpname || !host || !params || !db) {
        return REG_ERROR_INVALID_ARGS;
    }

    const char *sql = "INSERT OR REPLACE INTO nfs_shares (bpname, host, params) "
                     "VALUES (?, ?, ?)";
    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return REG_ERROR_PREPARE;
    }

    sqlite3_bind_text(stmt, 1, bpname, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, host, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, params, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert NFS share: %s\n",
                sqlite3_errmsg(db));
        return REG_ERROR_EXEC;
    }

    return REG_SUCCESS;
}

int registry_delete_nfs_share(const char *bpname, const char *host) {
    if (!bpname || !host || !db) {
        return REG_ERROR_INVALID_ARGS;
    }

    const char *sql = "DELETE FROM nfs_shares WHERE bpname = ? AND host = ?";
    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return REG_ERROR_PREPARE;
    }

    sqlite3_bind_text(stmt, 1, bpname, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, host, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to delete NFS share: %s\n",
                sqlite3_errmsg(db));
        return REG_ERROR_EXEC;
    }

    return REG_SUCCESS;
}

int registry_list_all_nfs_shares(registry_callback_t callback, void *user_data) {
    if (!callback || !db) {
        return REG_ERROR_INVALID_ARGS;
    }

    const char *sql = "SELECT * FROM nfs_shares";

    char *err_msg = NULL;
    int rc = sqlite3_exec(db, sql, callback, user_data, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to list NFS shares: %s\n", err_msg);
        sqlite3_free(err_msg);
        return REG_ERROR_EXEC;
    }

    return REG_SUCCESS;
}

/* SSH configuration operations */

int registry_update_ssh_config(const char *field, const char *value) {
    if (!field || !value || !db) {
        return REG_ERROR_INVALID_ARGS;
    }

    /* Validate field name to prevent SQL injection */
    if (strcmp(field, "Port") != 0 &&
        strcmp(field, "Protocol") != 0 &&
        strcmp(field, "ListenAddress") != 0) {
        fprintf(stderr, "Invalid field name: %s\n", field);
        return REG_ERROR_INVALID_ARGS;
    }

    char sql[MAX_SQL_STATEMENT_LENGTH];
    snprintf(sql, sizeof(sql), "UPDATE ssh_config SET %s = ?", field);

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return REG_ERROR_PREPARE;
    }

    sqlite3_bind_text(stmt, 1, value, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to update SSH config: %s\n",
                sqlite3_errmsg(db));
        return REG_ERROR_EXEC;
    }

    return REG_SUCCESS;
}

/* SSH allowed hosts operations */

int registry_insert_ssh_allowed_host(const char *address) {
    if (!address || !db) {
        return REG_ERROR_INVALID_ARGS;
    }

    const char *sql = "INSERT OR REPLACE INTO ssh_allowedhosts (address) "
                     "VALUES (?)";
    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return REG_ERROR_PREPARE;
    }

    sqlite3_bind_text(stmt, 1, address, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert SSH allowed host: %s\n",
                sqlite3_errmsg(db));
        return REG_ERROR_EXEC;
    }

    return REG_SUCCESS;
}

int registry_delete_ssh_allowed_host(const char *address) {
    if (!address || !db) {
        return REG_ERROR_INVALID_ARGS;
    }

    const char *sql = "DELETE FROM ssh_allowedhosts WHERE address = ?";
    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return REG_ERROR_PREPARE;
    }

    sqlite3_bind_text(stmt, 1, address, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to delete SSH allowed host: %s\n",
                sqlite3_errmsg(db));
        return REG_ERROR_EXEC;
    }

    return REG_SUCCESS;
}

int registry_list_ssh_allowed_hosts(registry_callback_t callback,
                                    void *user_data) {
    if (!callback || !db) {
        return REG_ERROR_INVALID_ARGS;
    }

    const char *sql = "SELECT * FROM ssh_allowedhosts";

    char *err_msg = NULL;
    int rc = sqlite3_exec(db, sql, callback, user_data, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to list SSH allowed hosts: %s\n", err_msg);
        sqlite3_free(err_msg);
        return REG_ERROR_EXEC;
    }

    return REG_SUCCESS;
}

/* Utility functions */

void registry_free_backup_point(backup_point_t *bp) {
    if (bp) {
        free(bp->name);
        free(bp->created_on);
        free(bp->quota);
    }
}

void registry_free_ssh_config(ssh_config_t *config) {
    if (config) {
        free(config->port);
        free(config->protocol);
        free(config->listen_address);
    }
}
