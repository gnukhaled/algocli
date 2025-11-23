#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/core/config.h"
#include "../../include/core/safe_exec.h"
#include "../../include/database/registry_new.h"

/* Service management functions using safe systemctl wrapper */

int access_control_ssh_enable(void) {
    int ret = safe_systemctl("enable", "sshd");
    if (ret != EXEC_SUCCESS) {
        fprintf(stderr, "Error: Failed to enable SSH service\n");
        return ERROR_SYSTEM_CALL;
    }

    ret = safe_systemctl("start", "sshd");
    if (ret != EXEC_SUCCESS) {
        fprintf(stderr, "Error: Failed to start SSH service\n");
        return ERROR_SYSTEM_CALL;
    }

    printf("SSH service enabled\n");
    return SUCCESS;
}

int access_control_ssh_disable(void) {
    int ret = safe_systemctl("stop", "sshd");
    if (ret != EXEC_SUCCESS) {
        fprintf(stderr, "Error: Failed to stop SSH service\n");
        return ERROR_SYSTEM_CALL;
    }

    ret = safe_systemctl("disable", "sshd");
    if (ret != EXEC_SUCCESS) {
        fprintf(stderr, "Error: Failed to disable SSH service\n");
        return ERROR_SYSTEM_CALL;
    }

    printf("SSH service disabled\n");
    return SUCCESS;
}

int access_control_ftp_enable(void) {
    int ret = safe_systemctl("enable", "vsftpd");
    if (ret != EXEC_SUCCESS) {
        fprintf(stderr, "Error: Failed to enable FTP service\n");
        return ERROR_SYSTEM_CALL;
    }

    ret = safe_systemctl("start", "vsftpd");
    if (ret != EXEC_SUCCESS) {
        fprintf(stderr, "Error: Failed to start FTP service\n");
        return ERROR_SYSTEM_CALL;
    }

    printf("FTP service enabled\n");
    return SUCCESS;
}

int access_control_ftp_disable(void) {
    int ret = safe_systemctl("stop", "vsftpd");
    if (ret != EXEC_SUCCESS) {
        fprintf(stderr, "Error: Failed to stop FTP service\n");
        return ERROR_SYSTEM_CALL;
    }

    ret = safe_systemctl("disable", "vsftpd");
    if (ret != EXEC_SUCCESS) {
        fprintf(stderr, "Error: Failed to disable FTP service\n");
        return ERROR_SYSTEM_CALL;
    }

    printf("FTP service disabled\n");
    return SUCCESS;
}

int access_control_web_enable(void) {
    int ret = safe_systemctl("enable", "httpd");
    if (ret != EXEC_SUCCESS) {
        fprintf(stderr, "Error: Failed to enable web service\n");
        return ERROR_SYSTEM_CALL;
    }

    ret = safe_systemctl("start", "httpd");
    if (ret != EXEC_SUCCESS) {
        fprintf(stderr, "Error: Failed to start web service\n");
        return ERROR_SYSTEM_CALL;
    }

    printf("Web interface enabled\n");
    return SUCCESS;
}

int access_control_web_disable(void) {
    int ret = safe_systemctl("stop", "httpd");
    if (ret != EXEC_SUCCESS) {
        fprintf(stderr, "Error: Failed to stop web service\n");
        return ERROR_SYSTEM_CALL;
    }

    ret = safe_systemctl("disable", "httpd");
    if (ret != EXEC_SUCCESS) {
        fprintf(stderr, "Error: Failed to disable web service\n");
        return ERROR_SYSTEM_CALL;
    }

    printf("Web interface disabled\n");
    return SUCCESS;
}

/* SSH configuration functions */

int access_control_ssh_set_port(const char *port) {
    if (!port) {
        fprintf(stderr, "Error: Port number is required\n");
        return ERROR_INVALID_ARGS;
    }

    if (!validate_port(port)) {
        fprintf(stderr, "Error: Invalid port number '%s'\n", port);
        return ERROR_INVALID_ARGS;
    }

    int ret = registry_update_ssh_config("Port", port);
    if (ret != REG_SUCCESS) {
        fprintf(stderr, "Error: Failed to update SSH port in registry\n");
        return ERROR_DB_ERROR;
    }

    printf("SSH port set to %s\n", port);
    printf("Note: Restart SSH service for changes to take effect\n");
    return SUCCESS;
}

int access_control_ssh_set_protocol(const char *protocol) {
    if (!protocol) {
        fprintf(stderr, "Error: Protocol is required\n");
        return ERROR_INVALID_ARGS;
    }

    /* Validate protocol (should be 1, 2, or "1,2") */
    if (strcmp(protocol, "1") != 0 && strcmp(protocol, "2") != 0 &&
        strcmp(protocol, "1,2") != 0) {
        fprintf(stderr, "Error: Invalid protocol '%s' (must be 1, 2, or 1,2)\n",
                protocol);
        return ERROR_INVALID_ARGS;
    }

    int ret = registry_update_ssh_config("Protocol", protocol);
    if (ret != REG_SUCCESS) {
        fprintf(stderr, "Error: Failed to update SSH protocol in registry\n");
        return ERROR_DB_ERROR;
    }

    printf("SSH protocol set to %s\n", protocol);
    printf("Note: Restart SSH service for changes to take effect\n");
    return SUCCESS;
}

int access_control_ssh_set_listen_address(const char *address) {
    if (!address) {
        fprintf(stderr, "Error: Listen address is required\n");
        return ERROR_INVALID_ARGS;
    }

    if (!validate_ip_address(address)) {
        fprintf(stderr, "Error: Invalid IP address '%s'\n", address);
        return ERROR_INVALID_ARGS;
    }

    int ret = registry_update_ssh_config("ListenAddress", address);
    if (ret != REG_SUCCESS) {
        fprintf(stderr, "Error: Failed to update SSH listen address in registry\n");
        return ERROR_DB_ERROR;
    }

    printf("SSH listen address set to %s\n", address);
    printf("Note: Restart SSH service for changes to take effect\n");
    return SUCCESS;
}

/* SSH allowed hosts management */

int access_control_ssh_add_allowed_host(const char *host) {
    if (!host) {
        fprintf(stderr, "Error: Host address is required\n");
        return ERROR_INVALID_ARGS;
    }

    if (!validate_ip_address(host) && !validate_hostname(host, MAX_HOSTNAME_LENGTH)) {
        fprintf(stderr, "Error: Invalid host address '%s'\n", host);
        return ERROR_INVALID_ARGS;
    }

    int ret = registry_insert_ssh_allowed_host(host);
    if (ret != REG_SUCCESS) {
        fprintf(stderr, "Error: Failed to add SSH allowed host\n");
        return ERROR_DB_ERROR;
    }

    printf("Added SSH allowed host: %s\n", host);
    return SUCCESS;
}

int access_control_ssh_delete_allowed_host(const char *host) {
    if (!host) {
        fprintf(stderr, "Error: Host address is required\n");
        return ERROR_INVALID_ARGS;
    }

    int ret = registry_delete_ssh_allowed_host(host);
    if (ret != REG_SUCCESS) {
        fprintf(stderr, "Error: Failed to delete SSH allowed host\n");
        return ERROR_DB_ERROR;
    }

    printf("Deleted SSH allowed host: %s\n", host);
    return SUCCESS;
}

/* List callback for displaying allowed hosts */
static int list_hosts_callback(void *data, int argc, char **argv,
                               char **col_names) {
    printf("  %s\n", argv[0] ? argv[0] : "");
    return 0;
}

int access_control_ssh_list_allowed_hosts(void) {
    printf("SSH Allowed Hosts:\n");

    int ret = registry_list_ssh_allowed_hosts(list_hosts_callback, NULL);

    if (ret != REG_SUCCESS) {
        fprintf(stderr, "Error: Failed to list SSH allowed hosts\n");
        return ERROR_DB_ERROR;
    }

    return SUCCESS;
}
