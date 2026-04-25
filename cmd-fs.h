
#ifndef FS_H
#define FS_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "algo-log.h"

int fs_quota_enable(void);
int fs_quota_disable(void);
int fs_quota_status(void);

#endif /* FS_H */
