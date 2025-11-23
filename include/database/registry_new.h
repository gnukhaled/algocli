#ifndef REGISTRY_NEW_H
#define REGISTRY_NEW_H

#include <sqlite3.h>
#include "../core/config.h"

/* Registry access modes */
typedef enum {
    REG_MODE_READ,
    REG_MODE_INSERT,
    REG_MODE_UPDATE,
    REG_MODE_REPLACE,
    REG_MODE_DELETE
} registry_mode_t;

/* Registry result codes */
typedef enum {
    REG_SUCCESS = 0,
    REG_ERROR = -1,
    REG_ERROR_DB_OPEN = -2,
    REG_ERROR_PREPARE = -3,
    REG_ERROR_BIND = -4,
    REG_ERROR_EXEC = -5,
    REG_ERROR_NOT_FOUND = -6,
    REG_ERROR_INVALID_ARGS = -7
} registry_result_t;

/* Data structures */
typedef struct {
    char *name;
    char *created_on;
    char *quota;
} backup_point_t;

typedef struct {
    char *bpname;
    char *host;
    char *params;
} nfs_share_t;

typedef struct {
    char *address;
} ssh_allowed_host_t;

typedef struct {
    char *port;
    char *protocol;
    char *listen_address;
} ssh_config_t;

/* Callback types for queries */
typedef int (*registry_callback_t)(void *user_data, int argc,
                                   char **argv, char **col_names);

/* Database connection management */
int registry_init(const char *db_path);
void registry_close(void);
sqlite3* registry_get_connection(void);

/* Backup point operations */
int registry_insert_backup_point(const char *name, const char *created_on);
int registry_delete_backup_point(const char *name);
int registry_update_backup_point_quota(const char *name, const char *quota);
int registry_get_backup_point(const char *name, backup_point_t *bp);
int registry_list_backup_points(registry_callback_t callback, void *user_data);

/* NFS share operations */
int registry_insert_nfs_share(const char *bpname, const char *host,
                              const char *params);
int registry_delete_nfs_share(const char *bpname, const char *host);
int registry_get_nfs_shares(const char *bpname, registry_callback_t callback,
                            void *user_data);
int registry_list_all_nfs_shares(registry_callback_t callback, void *user_data);

/* SSH configuration operations */
int registry_insert_ssh_config(const char *port, const char *protocol,
                               const char *listen_addr);
int registry_update_ssh_config(const char *field, const char *value);
int registry_get_ssh_config(ssh_config_t *config);

/* SSH allowed hosts operations */
int registry_insert_ssh_allowed_host(const char *address);
int registry_delete_ssh_allowed_host(const char *address);
int registry_list_ssh_allowed_hosts(registry_callback_t callback,
                                    void *user_data);

/* Utility functions */
void registry_free_backup_point(backup_point_t *bp);
void registry_free_ssh_config(ssh_config_t *config);

#endif /* REGISTRY_NEW_H */
