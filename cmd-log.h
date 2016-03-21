
#ifndef LOG_H
#define LOG_H

#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/select.h>
#include <sys/inotify.h>

#define LOG_DIR "/mnt/support"

int log_list(char *);
char*  log_view(char *);
int log_watch(char *);

#endif /* LOG_H */
