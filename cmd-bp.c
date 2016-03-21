
#include "cmd-bp.h"
#include "algo-log.h"
#include "algocli.h"

static const char *backup_root = "/mnt/backup";
extern FILE *logfp;

int bp_fs_create(char *name){

	FILE *proc;
	char comm[200];
	int ret;
	int len = 0;
	ret = snprintf(comm, sizeof(comm), "/build/btrfs-progs/btrfs subvolume create  %s/%s",backup_root, name);
	init_logger(CTX_BP);

	if ( len <= sizeof(comm)){
		proc = popen(comm, "r");
		algolog(ALGO_INFO, "Created backup point %s\n", name);
		printf("Created backup point %s\n", name);
	}

	pclose(proc);
	return ret;
}

int bp_delete(char *name){

        FILE *proc;
        char comm[200];
        int ret;
        int len = 0;
        ret = snprintf(comm, sizeof(comm), "/build/btrfs-progs/btrfs subvolume delete  %s/%s",backup_root, name);
	init_logger(CTX_BP);

        if ( len <= sizeof(comm)){
			proc = popen(comm, "r");
			algolog(ALGO_INFO, "User %s deleted backup point %s\n",
							getenv("USER"), name);
			printf("Deleted backup point %s\n", name);
        }

	pclose(proc);
        return ret;
}


int bp_rename(char *orig, char *newname){

        FILE *proc;
        char comm[200];
        int ret;
        int len = 0;
        ret = snprintf(comm, sizeof(comm), "mv %s/%s %s/%s",
				backup_root, orig, backup_root, newname);

        if ( len <= sizeof(comm)){
                proc = popen(comm, "r");
		pclose(proc);
                printf("Renamed backup point %s to %s\n", orig, newname);
        }

        return ret;
}


int bp_quota_set(char *name, char *limit){

        FILE *proc;
        char comm[200];
        int ret;
        int len = 0;
        ret = snprintf(comm, sizeof(comm), "/build/btrfs-progs/btrfs qgroup limit %s %s/%s",
						limit ,backup_root, name);

        if ( len <= sizeof(comm)){
                proc = popen(comm, "r");
		pclose(proc);
                printf("Quota set for backup point %s to %s\n", name, limit);
        }

        return ret;
}


int create_bp(char *name, char *createdon){

	char *table = "backup_points";
	char **val  = calloc(2, sizeof(char*));
	val[0] = name;
	val[1] = createdon;

	return registry(REG_INSERT, table, NULL, val);

}
