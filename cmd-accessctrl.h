
#ifndef ACCESSCTRL_H
#define ACCESSCTRL_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define SSH_PORT  	(1 << 0)
#define SSH_LISTENADDR  (1 << 1)
#define SSH_PROTO 	(1 << 2)

#define ssh_config_tbl  "ssh_config"
#define ssh_allowedhosts_tbl  "ssh_allowedhosts"



int accessctrl_ssh_enable(void);
int accessctrl_ssh_disable(void);
int accessctrl_web_disable(void);
int accessctrl_web_enable(void);
int accessctrl_ftp_disable(void);
int accessctrl_ftp_enable(void);

int ssh_set(int, char *);
int ssh_add_host(char *);
int ssh_del_host(char *);

#endif /* ACCESSCTRL_H */
