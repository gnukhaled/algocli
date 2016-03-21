
#ifndef NFS_H
#define NFS_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libsmartcols/libsmartcols.h>
#include "registry.h"

#define NFS_DEFAULT_PARAMS "rw,sync,no_all_squash,no_root_squash"

nfs_share_t *nfshead;

int nfs_enable();
int nfs_disable();
int nfs_share(char*, char*, char*);
int nfs_unshare(char*, char*);

int nfs_share_list(char*);

#endif /* NFS_H */
