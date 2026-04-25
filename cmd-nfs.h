
#ifndef NFS_H
#define NFS_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "registry.h"

#define NFS_DEFAULT_PARAMS "rw,sync,no_all_squash,no_root_squash"

int nfs_enable(void);
int nfs_disable(void);
int nfs_share(char *, char *, char *);
int nfs_unshare(char *, char *);

int nfs_share_list(char *);

#endif /* NFS_H */
