
#include "cmd-bp.h"
#include "algo-log.h"
#include "algocli.h"
#include "paths.h"
#include "proc.h"
#include "validate.h"

#include <ctype.h>

/* Each function below validates its identifier inputs through
 * is_safe_name() and then dispatches via spawn() with absolute paths.
 * No /bin/sh involved at any point — argv elements pass through the
 * kernel as opaque strings, so shell metacharacters in user input
 * cannot reparse into commands. */

/* Not declared const — see cmd-fs.c for explanation (execve argv). */
static char backup_root[] = BACKUP_ROOT;

extern FILE *logfp;

/* Helper: build "<backup_root>/<name>" into a heap buffer. Returns
 * NULL on validation failure or allocation error. Caller frees. */
static char *bp_path(const char *name) {
    if (!is_safe_name(name)) {
        fprintf(stderr, "Invalid backup-point name '%s' "
                "(allowed: A-Z a-z 0-9 _ . - up to 64 chars)\n",
                name ? name : "(null)");
        return NULL;
    }
    size_t need = strlen(backup_root) + 1 + strlen(name) + 1;
    char *p = malloc(need);
    if (!p) return NULL;
    snprintf(p, need, "%s/%s", backup_root, name);
    return p;
}

int bp_fs_create(char *name) {
    char *path = bp_path(name);
    if (!path) return -1;

    init_logger(CTX_BP);
    char *const argv[] = {
        (char *)BTRFS_BIN, "subvolume", "create", path, NULL
    };
    int rc = spawn(argv);
    if (rc == 0) {
        algolog(ALGO_INFO, "Created backup point %s\n", name);
        printf("Created backup point %s\n", name);
    } else {
        fprintf(stderr, "Failed to create backup point '%s' (rc=%d)\n",
                name, rc);
    }
    free(path);
    return rc;
}

int bp_delete(char *name) {
    char *path = bp_path(name);
    if (!path) return -1;

    init_logger(CTX_BP);
    char *const argv[] = {
        (char *)BTRFS_BIN, "subvolume", "delete", path, NULL
    };
    int rc = spawn(argv);
    if (rc == 0) {
        const char *user = getenv("USER");
        algolog(ALGO_INFO, "User %s deleted backup point %s\n",
                user ? user : "(unknown)", name);
        printf("Deleted backup point %s\n", name);
    } else {
        fprintf(stderr, "Failed to delete backup point '%s' (rc=%d)\n",
                name, rc);
    }
    free(path);
    return rc;
}

int bp_rename(char *orig, char *newname) {
    char *src = bp_path(orig);
    if (!src) return -1;
    char *dst = bp_path(newname);
    if (!dst) { free(src); return -1; }

    char *const argv[] = { (char *)MV_BIN, src, dst, NULL };
    int rc = spawn(argv);
    if (rc == 0) {
        printf("Renamed backup point %s to %s\n", orig, newname);
    } else {
        fprintf(stderr, "Failed to rename '%s' to '%s' (rc=%d)\n",
                orig, newname, rc);
    }
    free(src);
    free(dst);
    return rc;
}

int bp_quota_set(char *name, char *limit) {
    char *path = bp_path(name);
    if (!path) return -1;

    /* The btrfs qgroup limit takes a size string like "10G", "500M",
     * or a raw byte count. Validate as identifier-class characters:
     * digits, optional single suffix char K/M/G/T. Reject anything
     * weirder so it can't smuggle metacharacters even though spawn()
     * wouldn't reparse them anyway — defense in depth. */
    if (!limit || !*limit) {
        fprintf(stderr, "missing quota limit\n");
        free(path);
        return -1;
    }
    for (const char *p = limit; *p; p++) {
        if (!(isdigit((unsigned char)*p) || strchr("KMGTkmgt", *p))) {
            fprintf(stderr, "invalid quota limit '%s'\n", limit);
            free(path);
            return -1;
        }
    }

    char *const argv[] = {
        (char *)BTRFS_BIN, "qgroup", "limit", limit, path, NULL
    };
    int rc = spawn(argv);
    if (rc == 0) {
        printf("Quota set for backup point %s to %s\n", name, limit);
    } else {
        fprintf(stderr, "Failed to set quota for '%s' (rc=%d)\n",
                name, rc);
    }
    free(path);
    return rc;
}

int create_bp(char *name, char *createdon) {
    if (!is_safe_name(name)) {
        fprintf(stderr, "Invalid backup-point name '%s'\n",
                name ? name : "(null)");
        return -1;
    }
    return reg_bp_insert(name, createdon);
}

/* ----- dispatcher shims (formerly in algocli.c) ----------------- */

int func_bp(char **arg) {
    (void)arg;
    cmd_ref();
    return 0;
}

int func_bp_create(char **arg) {
    time_t bpctime = time(NULL);
    char *createdon = ctime(&bpctime);
    char *pos;
    if ((pos = strchr(createdon, '\n')) != NULL)
        *pos = '\0';
    return create_bp(arg[0], createdon);
}

int func_bp_delete(char **arg) {
    return bp_delete(arg[0]);
}

int func_bp_rename(char **arg) {
    return bp_rename(arg[0], arg[1]);
}

int func_bp_quota(char **arg) {
    (void)arg;
    cmd_ref();
    return 0;
}

int func_bp_quota_set(char **arg) {
    return bp_quota_set(arg[0], arg[1]);
}

int func_bp_list(char **arg) {
    (void)arg;
    STUB_NOT_IMPLEMENTED("bp list");
}
