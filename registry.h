
#ifndef REGISTRY_H
#define REGISTRY_H

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

/* Registry database path */
#define REGISTRY_PATH  "registry.db"

/* Registry open, close macros */
#define _algoreg_dbclose() sqlite3_close(db);
#define  _algoreg_dbopen()					\
        sqlite3 *db;						\
        char *err_msg = 0;					\
        int rc = sqlite3_open(REGISTRY_PATH, &db);		\
        if (rc != SQLITE_OK) {					\
		fprintf(stderr, "Cannot open registry %s\n"	\
					,sqlite3_errmsg(db));	\
		sqlite3_close(db);				\
	}

/* SQL statement max length */
#define STMT_MAX 300

/* Registry access modes */
#define REG_READ        (1 << 0)
#define REG_INSERT      (1 << 1)
#define REG_UPDATE      (1 << 2)
#define REG_REPLACE     (1 << 3)
#define REG_DELETE      (1 << 4)

/* Registry read contexts */
#define NFS_CTX         (1 << 0)
#define BP_CTX          (1 << 1)
#define CIFS_CTX        (1 << 2)
#define REPL_CTX        (1 << 3)

char sql[STMT_MAX];

typedef struct nfs_share{
	char *bpname;
	char *host;
	char *params;
	struct nfs_share *next;
} nfs_share_t;

/* Registry row types */
nfs_share_t *nfscurr;
extern nfs_share_t *nfshead;

/* Registry manipulation functions */
int registry(int, char*, char*, char**);
static int algoreg_read(char*, char*, char**);
static int algoreg_write(int, char*, char*, char**);

/* Dynamic statement functions */
static char* prepare_stmt(int, char*, char*, char**);
static char** get_cols(char*);
static inline void get_where_cols(char*, int);
static inline int get_current_context(char*);
static inline void populate_cols(char*);
static inline int val_idx(char**, char*);

/* Callback functions */
static int (*callback)(void*,int,char**,char**);
static int pragma_callback(void*,int,char**,char**);
static int nfs_callback(void*,int,char**,char**);
static int cifs_callback(void*,int,char**,char**);
static int bp_callback(void*,int,char**,char**);
static int repl_callback(void*,int,char**,char**);

/* Registry row types list functions */
static void llnfs_push(nfs_share_t *);
static void llnfs_flush(nfs_share_t *);

#endif /* REGISTRY_H */
