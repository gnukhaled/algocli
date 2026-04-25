/* paths.h — compile-time configurable filesystem paths.
 *
 * Every hardcoded path in the codebase is centralized here. Each can
 * be overridden at build time:
 *
 *   make CFLAGS+="-DBACKUP_ROOT=\\\"/srv/backups\\\"   \\
 *               -DLOG_VIEW_DIR=\\\"/var/log/algocli\\\"  \\
 *               -DBTRFS_BIN=\\\"/sbin/btrfs\\\""
 *
 * Tests, distro packages, and unusual deployments can pick their own
 * layout without patching source. Default values match the original
 * AlgoOS appliance layout. */

#ifndef ALGOCLI_PATHS_H
#define ALGOCLI_PATHS_H

/* Where backup-point subvolumes live. */
#ifndef BACKUP_ROOT
#define BACKUP_ROOT "/mnt/backup"
#endif

/* Directory `log view`/`log list` reads from. Distinct from where
 * algolog *writes* per-context logs (those go to /tmp/, see algo-log.c). */
#ifndef LOG_VIEW_DIR
#define LOG_VIEW_DIR "/mnt/support"
#endif

/* MOTD target. */
#ifndef MOTD_PATH
#define MOTD_PATH "/etc/motd"
#endif
#ifndef MOTD_TMP_PATH
#define MOTD_TMP_PATH "/etc/motd.new"
#endif

/* Privileged binaries spawned via fork+execve. Absolute paths only. */
#ifndef BTRFS_BIN
#define BTRFS_BIN "/usr/sbin/btrfs"
#endif

#ifndef MV_BIN
#define MV_BIN "/usr/bin/mv"
#endif

#ifndef SYSTEMCTL_BIN
#define SYSTEMCTL_BIN "/usr/bin/systemctl"
#endif

/* Registry database. Phase 6 default keeps the project-relative path
 * for dev convenience; production deployments should set this to
 * /var/lib/algocli/registry.db. */
#ifndef REGISTRY_PATH_DEFAULT_OVERRIDE
/* registry.h provides REGISTRY_PATH_DEFAULT — leave as-is. */
#endif

#endif /* ALGOCLI_PATHS_H */
