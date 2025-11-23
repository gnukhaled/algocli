#ifndef SAFE_EXEC_H
#define SAFE_EXEC_H

#include <sys/types.h>

/* Maximum number of arguments for safe_exec */
#define MAX_EXEC_ARGS 32

/* Safe execution return codes */
#define EXEC_SUCCESS 0
#define EXEC_ERROR -1
#define EXEC_INVALID_ARGS -2
#define EXEC_FORK_FAILED -3
#define EXEC_TIMEOUT -4

/* Input validation */
int validate_alphanumeric(const char *str, size_t max_len);
int validate_path_component(const char *str, size_t max_len);
int validate_ip_address(const char *str);
int validate_port(const char *str);
int validate_hostname(const char *str, size_t max_len);

/* Safe string operations */
char* safe_strdup(const char *str);
int safe_snprintf(char *buf, size_t size, const char *fmt, ...);
char* sanitize_string(const char *input, size_t max_len);

/* Safe execution functions */
int safe_exec(const char *program, char *const argv[]);
int safe_exec_output(const char *program, char *const argv[],
                     char *output, size_t output_size);
int safe_systemctl(const char *action, const char *service);

/* Safe file operations */
int safe_file_exists(const char *path);
int safe_dir_exists(const char *path);
int safe_create_directory(const char *path, mode_t mode);

#endif /* SAFE_EXEC_H */
