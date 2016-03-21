
#ifndef SYSTEM_H
#define SYSTEM_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/reboot.h>

int algo_reboot();
int algo_shutdown();
int system_set_motd();

#endif /* SYSTEM_H */
