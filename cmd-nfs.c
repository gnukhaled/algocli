
#include "cmd-nfs.h"

char *table = "nfs_shares";

int nfs_enable(){

  int ret;
  ret = system("systemctl enable nfs-server.service &> /dev/null");
  if (!ret){
    ret = system("systemctl start nfs-server.service &> /dev/null");
    printf("NFS sharing protocol enabled\n");
  }

  return ret;
}

int nfs_disable(){

  int ret;
  ret = system("systemctl stop nfs-server.service &> /dev/null");
  if (!ret){
    ret = system("systemctl disable nfs-server.service &> /dev/null");
    printf("NFS sharing protocol disabled\n");
  }

  return ret;
}

int nfs_share(char *bp, char *host, char *params){

	char **vals = calloc(3,sizeof(char*));
	int ret;

	vals[0] = bp;
	vals[1] = host;
	vals[2] = params ? params : NFS_DEFAULT_PARAMS;

	ret = registry(REG_REPLACE, table, NULL, vals);
	free(vals);
	return ret;
}

int nfs_unshare(char *bp, char *host){

        char **vals = calloc(3,sizeof(char*));
	int ret;

	vals[0] = bp;
	vals[1] = host;

	ret = registry(REG_DELETE, table, NULL, vals);
	free(vals);
	return ret;
}

int nfs_share_list(char* bpname){

	char **vals = calloc(1,sizeof(char*));
	int ret;

	vals[0] = bpname ? bpname : NULL;

	ret = registry(REG_READ, table, NULL, vals);
	printf("DBG----------> %s\n", nfshead->bpname);
	return ret;
}
