#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>
#include "../../include/core/safe_exec.h"
#include "../../include/core/config.h"

/* Validate alphanumeric string with optional underscores and hyphens */
int validate_alphanumeric(const char *str, size_t max_len) {
    if (!str || strlen(str) == 0 || strlen(str) > max_len) {
        return FALSE;
    }

    for (size_t i = 0; str[i] != '\0'; i++) {
        if (!isalnum(str[i]) && str[i] != '_' && str[i] != '-') {
            return FALSE;
        }
    }

    return TRUE;
}

/* Validate path component (no slashes, no parent directory references) */
int validate_path_component(const char *str, size_t max_len) {
    if (!str || strlen(str) == 0 || strlen(str) > max_len) {
        return FALSE;
    }

    /* Reject dangerous patterns */
    if (strcmp(str, ".") == 0 || strcmp(str, "..") == 0) {
        return FALSE;
    }

    /* Check for path separators and null bytes */
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == '/' || str[i] == '\0' || str[i] == '\n') {
            return FALSE;
        }
    }

    return validate_alphanumeric(str, max_len);
}

/* Validate IP address (simple check) */
int validate_ip_address(const char *str) {
    if (!str) {
        return FALSE;
    }

    int dots = 0;
    int digits = 0;
    const char *p = str;

    while (*p) {
        if (*p == '.') {
            if (digits == 0 || digits > 3) return FALSE;
            dots++;
            digits = 0;
        } else if (isdigit(*p)) {
            digits++;
        } else {
            return FALSE;
        }
        p++;
    }

    return (dots == 3 && digits > 0 && digits <= 3);
}

/* Validate port number */
int validate_port(const char *str) {
    if (!str || strlen(str) == 0) {
        return FALSE;
    }

    for (size_t i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            return FALSE;
        }
    }

    int port = atoi(str);
    return (port > 0 && port <= 65535);
}

/* Validate hostname */
int validate_hostname(const char *str, size_t max_len) {
    if (!str || strlen(str) == 0 || strlen(str) > max_len) {
        return FALSE;
    }

    for (size_t i = 0; str[i] != '\0'; i++) {
        if (!isalnum(str[i]) && str[i] != '.' && str[i] != '-') {
            return FALSE;
        }
    }

    return TRUE;
}

/* Safe string duplication */
char* safe_strdup(const char *str) {
    if (!str) {
        return NULL;
    }

    size_t len = strlen(str);
    if (len > MAX_BUFFER_SIZE) {
        fprintf(stderr, "String too long for safe_strdup\n");
        return NULL;
    }

    char *dup = malloc(len + 1);
    if (!dup) {
        perror("malloc");
        return NULL;
    }

    memcpy(dup, str, len + 1); /* Copy including null terminator */

    return dup;
}

/* Safe snprintf wrapper */
int safe_snprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int ret = vsnprintf(buf, size, fmt, args);

    va_end(args);

    if (ret < 0 || (size_t)ret >= size) {
        fprintf(stderr, "safe_snprintf: buffer overflow detected\n");
        return -1;
    }

    return ret;
}

/* Sanitize string by removing dangerous characters */
char* sanitize_string(const char *input, size_t max_len) {
    if (!input) {
        return NULL;
    }

    size_t len = strlen(input);
    if (len > max_len) {
        len = max_len;
    }

    char *output = malloc(len + 1);
    if (!output) {
        perror("malloc");
        return NULL;
    }

    size_t j = 0;
    for (size_t i = 0; i < len && input[i] != '\0'; i++) {
        /* Only allow printable ASCII characters except dangerous ones */
        if (isprint(input[i]) && input[i] != ';' && input[i] != '&' &&
            input[i] != '|' && input[i] != '`' && input[i] != '$' &&
            input[i] != '(' && input[i] != ')' && input[i] != '<' &&
            input[i] != '>') {
            output[j++] = input[i];
        }
    }

    output[j] = '\0';
    return output;
}

/* Safe execution using fork/exec (no shell interpretation) */
int safe_exec(const char *program, char *const argv[]) {
    if (!program || !argv) {
        return EXEC_INVALID_ARGS;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return EXEC_FORK_FAILED;
    }

    if (pid == 0) {
        /* Child process */
        execv(program, argv);
        /* If execv returns, it failed */
        perror("execv");
        _exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            return EXEC_ERROR;
        }

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }

        return EXEC_ERROR;
    }
}

/* Safe execution with output capture */
int safe_exec_output(const char *program, char *const argv[],
                     char *output, size_t output_size) {
    if (!program || !argv || !output) {
        return EXEC_INVALID_ARGS;
    }

    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return EXEC_ERROR;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return EXEC_FORK_FAILED;
    }

    if (pid == 0) {
        /* Child process */
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        execv(program, argv);
        perror("execv");
        _exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        close(pipefd[1]);

        ssize_t bytes_read = read(pipefd[0], output, output_size - 1);
        if (bytes_read >= 0) {
            output[bytes_read] = '\0';
        }

        close(pipefd[0]);

        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            return EXEC_ERROR;
        }

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }

        return EXEC_ERROR;
    }
}

/* Safe systemctl wrapper */
int safe_systemctl(const char *action, const char *service) {
    if (!action || !service) {
        return EXEC_INVALID_ARGS;
    }

    /* Validate inputs */
    if (!validate_alphanumeric(action, SMALL_BUFFER_SIZE)) {
        fprintf(stderr, "Invalid action: %s\n", action);
        return EXEC_INVALID_ARGS;
    }

    if (!validate_alphanumeric(service, SMALL_BUFFER_SIZE)) {
        fprintf(stderr, "Invalid service: %s\n", service);
        return EXEC_INVALID_ARGS;
    }

    /* Build argv array */
    char service_arg[MEDIUM_BUFFER_SIZE];
    safe_snprintf(service_arg, sizeof(service_arg), "%s.service", service);

    char *argv[] = {
        (char *)app_config.systemctl_binary,
        (char *)action,
        service_arg,
        NULL
    };

    return safe_exec(app_config.systemctl_binary, argv);
}

/* Check if file exists */
int safe_file_exists(const char *path) {
    if (!path) {
        return FALSE;
    }

    struct stat st;
    return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
}

/* Check if directory exists */
int safe_dir_exists(const char *path) {
    if (!path) {
        return FALSE;
    }

    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

/* Safely create directory */
int safe_create_directory(const char *path, mode_t mode) {
    if (!path) {
        return EXEC_INVALID_ARGS;
    }

    if (safe_dir_exists(path)) {
        return EXEC_SUCCESS;
    }

    if (mkdir(path, mode) < 0) {
        perror("mkdir");
        return EXEC_ERROR;
    }

    return EXEC_SUCCESS;
}
