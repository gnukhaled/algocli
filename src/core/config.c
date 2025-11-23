#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../include/core/config.h"

/* Global configuration instance */
app_config_t app_config;

/* Initialize configuration with defaults */
void config_set_defaults(void) {
    strncpy(app_config.backup_root, DEFAULT_BACKUP_ROOT, MAX_PATH_LENGTH - 1);
    strncpy(app_config.log_dir, DEFAULT_LOG_DIR, MAX_PATH_LENGTH - 1);
    strncpy(app_config.support_dir, DEFAULT_SUPPORT_DIR, MAX_PATH_LENGTH - 1);
    strncpy(app_config.registry_db, DEFAULT_REGISTRY_DB, MAX_PATH_LENGTH - 1);
    strncpy(app_config.btrfs_binary, BTRFS_BINARY, MAX_PATH_LENGTH - 1);
    strncpy(app_config.systemctl_binary, SYSTEMCTL_BINARY, MAX_PATH_LENGTH - 1);
    strncpy(app_config.editor_binary, EDITOR_BINARY, MAX_PATH_LENGTH - 1);

    app_config.backup_root[MAX_PATH_LENGTH - 1] = '\0';
    app_config.log_dir[MAX_PATH_LENGTH - 1] = '\0';
    app_config.support_dir[MAX_PATH_LENGTH - 1] = '\0';
    app_config.registry_db[MAX_PATH_LENGTH - 1] = '\0';
    app_config.btrfs_binary[MAX_PATH_LENGTH - 1] = '\0';
    app_config.systemctl_binary[MAX_PATH_LENGTH - 1] = '\0';
    app_config.editor_binary[MAX_PATH_LENGTH - 1] = '\0';

    app_config.debug_mode = 0;
    app_config.log_level = 5; /* INFO level */
}

/* Initialize configuration system */
int config_init(void) {
    config_set_defaults();
    return SUCCESS;
}

/* Load configuration from file (stub for now) */
int config_load(const char *config_file) {
    (void)config_file; /* Unused parameter - TODO: Implement file parsing */
    /* TODO: Implement configuration file parsing */
    /* For now, just use defaults */
    config_set_defaults();
    return SUCCESS;
}

/* Getter functions */
const char* config_get_backup_root(void) {
    return app_config.backup_root;
}

const char* config_get_log_dir(void) {
    return app_config.log_dir;
}

const char* config_get_registry_db(void) {
    return app_config.registry_db;
}
