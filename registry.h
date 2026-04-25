/* registry.h — typed sqlite3-backed configuration store.
 *
 * Replaces the original strcat-based DML builder. Every public
 * function uses sqlite3_prepare_v2 with bound parameters internally;
 * SQL injection is impossible by construction because no user-
 * supplied string ever reaches the SQL parser.
 *
 * Lifecycle:
 *   reg_init(path)   — call once at startup; opens (and creates if
 *                      missing) the database, runs schema migrations.
 *   reg_close()      — call once at shutdown.
 *
 * Threading: the database handle is global. Not thread-safe (this
 * shell has no threads).
 */

#ifndef ALGOCLI_REGISTRY_H
#define ALGOCLI_REGISTRY_H

#include <stdbool.h>
#include <stddef.h>

/* Default on-disk location. Tests override this path via reg_init. */
#define REGISTRY_PATH_DEFAULT "registry.db"

/* Return-code convention used throughout this module:
 *    0  — success
 *   -1  — failure (an error message has already been printed)
 *   >0  — domain-specific (e.g. reg_*_get returns 1 for "found",
 *         0 for "not found"). */

int  reg_init(const char *path);
void reg_close(void);

/* ----- backup_points ---------------------------------------------- */

int reg_bp_insert(const char *name, const char *createdon);
int reg_bp_delete_row(const char *name);
int reg_bp_rename(const char *old_name, const char *new_name);
/* 1 = found, 0 = not found, -1 = error. createdon_out may be NULL. */
int reg_bp_get(const char *name, char *createdon_out, size_t out_size);

/* ----- nfs_shares ------------------------------------------------- */

typedef struct {
    char bpname[65];
    char host[256];
    char params[128];
} reg_nfs_row_t;

int reg_nfs_replace(const char *bp, const char *host, const char *params);
int reg_nfs_delete(const char *bp, const char *host);
/* List up to max_rows rows. filter_bp may be NULL to list all. *out_n
 * receives the count actually written. Returns 0/-1. */
int reg_nfs_list(const char *filter_bp,
                 reg_nfs_row_t *rows, size_t max_rows, size_t *out_n);

/* ----- ssh_config (single-row table) ------------------------------ */

typedef enum {
    REG_SSH_PORT,
    REG_SSH_LISTEN_ADDRESS,
    REG_SSH_PROTOCOL,
} reg_ssh_field_t;

int reg_ssh_set(reg_ssh_field_t field, const char *value);
/* 1 = found, 0 = no row, -1 = error. */
int reg_ssh_get(reg_ssh_field_t field, char *out, size_t out_size);

/* ----- ssh_allowedhosts ------------------------------------------- */

int reg_ssh_add_host(const char *host);
int reg_ssh_del_host(const char *host);
/* 1 = exists, 0 = not, -1 = error. */
int reg_ssh_host_exists(const char *host);

#endif /* ALGOCLI_REGISTRY_H */
