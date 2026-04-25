
#ifndef SYSTEM_H
#define SYSTEM_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/reboot.h>

int algo_reboot(void);
int algo_shutdown(void);
int system_set_motd(void);

#endif /* SYSTEM_H */
