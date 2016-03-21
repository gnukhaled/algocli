
#include "cmd-fs.h"

static const char *backup_root = "/mnt/backup";
extern FILE *logfp;

int fs_quota_enable(){

        FILE *proc;
        char comm[200];
        int ret;
        int len = 0;
        ret = snprintf(comm, sizeof(comm), "/build/btrfs-progs/btrfs quota enable %s",backup_root);
	init_logger(CTX_FS);

        if ( len <= sizeof(comm)){
                proc = popen(comm, "r");
		pclose(proc);
		algolog(ALGO_INFO,"Filesystem quota enabled\n");
                printf("Filesystem quota enabled\n");
        }

        return ret;
}


int fs_quota_disable(){

        FILE *proc;
        char comm[200];
        int ret;
        int len = 0;
        ret = snprintf(comm, sizeof(comm), "/build/btrfs-progs/btrfs quota disable %s",backup_root);

        if ( len <= sizeof(comm)){
                proc = popen(comm, "r");
		pclose(proc);
                printf("Filesystem quota disabled\n");
        }

        return ret;
}

int fs_quota_status(){

        int ret;

	ret = system("/build/btrfs-progs/btrfs qgroup show /mnt/backup &> /dev/null"); 
	if (ret){
		printf("Filesystem quota is disabled\n");
	}else{
		printf("Filesystem quota is enabled\n");
	}

        return ret;
}

