
#ifndef LOG_H
#define LOG_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

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

#include "paths.h"
#define LOG_DIR LOG_VIEW_DIR

int log_list(char *);
char*  log_view(char *);
int log_watch(char *);

#endif /* LOG_H */
