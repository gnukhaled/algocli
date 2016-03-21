
#ifndef FS_H
#define FS_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "algo-log.h"

int fs_quota_enable();
int fs_quota_disable();
int fs_quota_status();

#endif /* FS_H */
