
#ifndef BP_H
#define BP_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "registry.h"



int bp_fs_create(char *name);
int bp_delete(char *name);
int bp_rename(char *orig, char *newname);
int bp_quota_set(char *name, char *size);
int bp_list();

int create_bp(char *, char *);

extern FILE *logfp;

#endif /* BP_H */
