#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../include/core/config.h"
#include "../../include/core/safe_exec.h"
#include "../../include/database/registry_new.h"

/* Create a backup point (BTRFS subvolume) */
int backup_point_create(const char *name) {
    if (!name) {
        fprintf(stderr, "Error: Backup point name is required\n");
        return ERROR_INVALID_ARGS;
    }

    /* Validate backup point name */
    if (!validate_path_component(name, MAX_USERNAME_LENGTH)) {
        fprintf(stderr, "Error: Invalid backup point name '%s'\n", name);
        fprintf(stderr, "Name must be alphanumeric with underscores/hyphens only\n");
        return ERROR_INVALID_ARGS;
    }

    /* Build full path */
    char full_path[MAX_PATH_LENGTH];
    int ret = safe_snprintf(full_path, sizeof(full_path), "%s/%s",
                           config_get_backup_root(), name);
    if (ret < 0) {
        return ERROR_GENERAL;
    }

    /* Check if already exists */
    if (safe_dir_exists(full_path)) {
        fprintf(stderr, "Error: Backup point '%s' already exists\n", name);
        return ERROR_GENERAL;
    }

    /* Create BTRFS subvolume using safe execution */
    char *argv[] = {
        (char *)app_config.btrfs_binary,
        "subvolume",
        "create",
        full_path,
        NULL
    };

    char output[LARGE_BUFFER_SIZE];
    ret = safe_exec_output(app_config.btrfs_binary, argv, output,
                          sizeof(output));

    if (ret != SUCCESS) {
        fprintf(stderr, "Error: Failed to create backup point '%s'\n", name);
        fprintf(stderr, "BTRFS output: %s\n", output);
        return ERROR_SYSTEM_CALL;
    }

    /* Get current timestamp */
    time_t now = time(NULL);
    char timestamp[SMALL_BUFFER_SIZE];
    struct tm *tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    /* Add to registry */
    ret = registry_insert_backup_point(name, timestamp);
    if (ret != REG_SUCCESS) {
        fprintf(stderr, "Warning: Backup point created but failed to add to registry\n");
        return ERROR_DB_ERROR;
    }

    printf("Successfully created backup point '%s'\n", name);
    return SUCCESS;
}

/* Delete a backup point */
int backup_point_delete(const char *name) {
    if (!name) {
        fprintf(stderr, "Error: Backup point name is required\n");
        return ERROR_INVALID_ARGS;
    }

    /* Validate backup point name */
    if (!validate_path_component(name, MAX_USERNAME_LENGTH)) {
        fprintf(stderr, "Error: Invalid backup point name '%s'\n", name);
        return ERROR_INVALID_ARGS;
    }

    /* Build full path */
    char full_path[MAX_PATH_LENGTH];
    int ret = safe_snprintf(full_path, sizeof(full_path), "%s/%s",
                           config_get_backup_root(), name);
    if (ret < 0) {
        return ERROR_GENERAL;
    }

    /* Check if exists */
    if (!safe_dir_exists(full_path)) {
        fprintf(stderr, "Error: Backup point '%s' does not exist\n", name);
        return ERROR_NOT_FOUND;
    }

    /* Delete BTRFS subvolume using safe execution */
    char *argv[] = {
        (char *)app_config.btrfs_binary,
        "subvolume",
        "delete",
        full_path,
        NULL
    };

    char output[LARGE_BUFFER_SIZE];
    ret = safe_exec_output(app_config.btrfs_binary, argv, output,
                          sizeof(output));

    if (ret != SUCCESS) {
        fprintf(stderr, "Error: Failed to delete backup point '%s'\n", name);
        fprintf(stderr, "BTRFS output: %s\n", output);
        return ERROR_SYSTEM_CALL;
    }

    /* Remove from registry */
    ret = registry_delete_backup_point(name);
    if (ret != REG_SUCCESS) {
        fprintf(stderr, "Warning: Backup point deleted but failed to remove from registry\n");
        return ERROR_DB_ERROR;
    }

    printf("Successfully deleted backup point '%s'\n", name);
    return SUCCESS;
}

/* Rename a backup point */
int backup_point_rename(const char *old_name, const char *new_name) {
    if (!old_name || !new_name) {
        fprintf(stderr, "Error: Both old and new names are required\n");
        return ERROR_INVALID_ARGS;
    }

    /* Validate names */
    if (!validate_path_component(old_name, MAX_USERNAME_LENGTH) ||
        !validate_path_component(new_name, MAX_USERNAME_LENGTH)) {
        fprintf(stderr, "Error: Invalid backup point name\n");
        return ERROR_INVALID_ARGS;
    }

    /* Build full paths */
    char old_path[MAX_PATH_LENGTH];
    char new_path[MAX_PATH_LENGTH];

    int ret = safe_snprintf(old_path, sizeof(old_path), "%s/%s",
                           config_get_backup_root(), old_name);
    if (ret < 0) {
        return ERROR_GENERAL;
    }

    ret = safe_snprintf(new_path, sizeof(new_path), "%s/%s",
                       config_get_backup_root(), new_name);
    if (ret < 0) {
        return ERROR_GENERAL;
    }

    /* Check old exists and new doesn't */
    if (!safe_dir_exists(old_path)) {
        fprintf(stderr, "Error: Backup point '%s' does not exist\n", old_name);
        return ERROR_NOT_FOUND;
    }

    if (safe_dir_exists(new_path)) {
        fprintf(stderr, "Error: Backup point '%s' already exists\n", new_name);
        return ERROR_GENERAL;
    }

    /* Rename using mv command (safe for BTRFS subvolumes) */
    char *argv[] = {
        "/bin/mv",
        old_path,
        new_path,
        NULL
    };

    char output[LARGE_BUFFER_SIZE];
    ret = safe_exec_output("/bin/mv", argv, output, sizeof(output));

    if (ret != SUCCESS) {
        fprintf(stderr, "Error: Failed to rename backup point\n");
        return ERROR_SYSTEM_CALL;
    }

    /* Update registry - delete old, insert new */
    registry_delete_backup_point(old_name);

    time_t now = time(NULL);
    char timestamp[SMALL_BUFFER_SIZE];
    struct tm *tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    registry_insert_backup_point(new_name, timestamp);

    printf("Successfully renamed backup point '%s' to '%s'\n",
           old_name, new_name);
    return SUCCESS;
}

/* Set quota for a backup point */
int backup_point_set_quota(const char *name, const char *limit) {
    if (!name || !limit) {
        fprintf(stderr, "Error: Backup point name and limit are required\n");
        return ERROR_INVALID_ARGS;
    }

    /* Validate backup point name */
    if (!validate_path_component(name, MAX_USERNAME_LENGTH)) {
        fprintf(stderr, "Error: Invalid backup point name '%s'\n", name);
        return ERROR_INVALID_ARGS;
    }

    /* Build full path */
    char full_path[MAX_PATH_LENGTH];
    int ret = safe_snprintf(full_path, sizeof(full_path), "%s/%s",
                           config_get_backup_root(), name);
    if (ret < 0) {
        return ERROR_GENERAL;
    }

    /* Check if exists */
    if (!safe_dir_exists(full_path)) {
        fprintf(stderr, "Error: Backup point '%s' does not exist\n", name);
        return ERROR_NOT_FOUND;
    }

    /* Set quota using BTRFS qgroup limit */
    char *argv[] = {
        (char *)app_config.btrfs_binary,
        "qgroup",
        "limit",
        (char *)limit,
        full_path,
        NULL
    };

    char output[LARGE_BUFFER_SIZE];
    ret = safe_exec_output(app_config.btrfs_binary, argv, output,
                          sizeof(output));

    if (ret != SUCCESS) {
        fprintf(stderr, "Error: Failed to set quota for backup point '%s'\n",
                name);
        fprintf(stderr, "BTRFS output: %s\n", output);
        return ERROR_SYSTEM_CALL;
    }

    /* Update registry */
    ret = registry_update_backup_point_quota(name, limit);
    if (ret != REG_SUCCESS) {
        fprintf(stderr, "Warning: Quota set but failed to update registry\n");
    }

    printf("Successfully set quota for backup point '%s' to %s\n", name, limit);
    return SUCCESS;
}

/* List callback for displaying backup points */
static int list_callback(void *data, int argc, char **argv, char **col_names) {
    /* argv[0] = name, argv[1] = createdon, argv[2] = quota */
    printf("%-20s  %-20s  %-10s\n",
           argv[0] ? argv[0] : "",
           argv[1] ? argv[1] : "",
           argv[2] ? argv[2] : "none");
    return 0;
}

/* List all backup points */
int backup_point_list(void) {
    printf("%-20s  %-20s  %-10s\n", "Name", "Created On", "Quota");
    printf("%-20s  %-20s  %-10s\n", "----", "----------", "-----");

    int ret = registry_list_backup_points(list_callback, NULL);

    if (ret != REG_SUCCESS) {
        fprintf(stderr, "Error: Failed to list backup points\n");
        return ERROR_DB_ERROR;
    }

    return SUCCESS;
}
