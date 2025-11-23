#ifndef CONFIG_H
#define CONFIG_H

/* Application Information */
#define APP_NAME "AlgoBackup CLI"
#define APP_VERSION "0.2.0"
#define APP_PROMPT_NAME "AlgoOS"

/* Buffer Sizes */
#define SMALL_BUFFER_SIZE 64
#define MEDIUM_BUFFER_SIZE 256
#define LARGE_BUFFER_SIZE 512
#define MAX_BUFFER_SIZE 1024
#define MAX_COMMAND_LENGTH 2048
#define MAX_PATH_LENGTH 4096
#define MAX_MESSAGE_LENGTH 300
#define MAX_SQL_STATEMENT_LENGTH 1024
#define MAX_HOSTNAME_LENGTH 256
#define MAX_USERNAME_LENGTH 64

/* File Paths */
#define DEFAULT_HISTORY_FILE ".algohist"
#define DEFAULT_CONFIG_FILE "/etc/algobackup/algocli.conf"
#define DEFAULT_REGISTRY_DB "registry.db"
#define DEFAULT_BACKUP_ROOT "/mnt/backup"
#define DEFAULT_LOG_DIR "/var/log/algobackup"
#define DEFAULT_SUPPORT_DIR "/mnt/support"

/* External Tools */
#define BTRFS_BINARY "/usr/sbin/btrfs"
#define SYSTEMCTL_BINARY "/usr/bin/systemctl"
#define EDITOR_BINARY "/usr/bin/vim"

/* Boolean Values */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* Return Codes */
#define SUCCESS 0
#define ERROR_GENERAL 1
#define ERROR_INVALID_ARGS 2
#define ERROR_AUTH_FAILED 3
#define ERROR_DB_ERROR 4
#define ERROR_SYSTEM_CALL 5
#define ERROR_MEMORY 6
#define ERROR_NOT_FOUND 7
#define ERROR_PERMISSION 8

/* Configuration Structure */
typedef struct {
    char backup_root[MAX_PATH_LENGTH];
    char log_dir[MAX_PATH_LENGTH];
    char support_dir[MAX_PATH_LENGTH];
    char registry_db[MAX_PATH_LENGTH];
    char btrfs_binary[MAX_PATH_LENGTH];
    char systemctl_binary[MAX_PATH_LENGTH];
    char editor_binary[MAX_PATH_LENGTH];
    int debug_mode;
    int log_level;
} app_config_t;

/* Global configuration instance */
extern app_config_t app_config;

/* Configuration functions */
int config_init(void);
int config_load(const char *config_file);
void config_set_defaults(void);
const char* config_get_backup_root(void);
const char* config_get_log_dir(void);
const char* config_get_registry_db(void);

#endif /* CONFIG_H */
