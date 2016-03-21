
#include <libsmartcols/libsmartcols.h>
#include "algocli.h"
#include "registry.h"



static char **colnames  = (char**)NULL;
static char **wherecols = (char**)NULL;

int registry(int mode, char *tbl, char *col, char **val){

	if (mode == REG_READ){
		return algoreg_read(tbl, col, val);
	}else{
		return algoreg_write(mode, tbl, col, val);
	}
}

static int algoreg_write(int mode, char *tbl, char *col, char **val) {

	_algoreg_dbopen();

	char *stmt = prepare_stmt(mode, tbl, col, val);
	callback = &pragma_callback;
	rc = sqlite3_exec(db, stmt, callback, 0, &err_msg);

	if (rc != SQLITE_OK ) {
		goto fail;
	}
	_algoreg_dbclose();
	return 0;

fail:
        fprintf(stderr, "Registry transaction error in %s: %s\n"
                        ,__FUNCTION__, err_msg);
        sqlite3_free(err_msg);
        _algoreg_dbclose();
        return 1;
}

static int algoreg_read(char *tbl, char *col,char **val){

	int mode = REG_READ;
	int cbctx = get_current_context(tbl);
	char *stmt = prepare_stmt(mode, tbl, col, val);
	_algoreg_dbopen();

      switch (cbctx){
                case NFS_CTX:
                        callback = &nfs_callback;

                        break;

		case BP_CTX:
                        callback = &bp_callback;
                        break;

		case CIFS_CTX:
                        callback = &cifs_callback;
                        break;

                case REPL_CTX:
                        callback = &repl_callback;
                        break;

                default:
                        callback = 0;
        }

	rc = sqlite3_exec(db, stmt, callback, 0, &err_msg);

        if (rc != SQLITE_OK ) {
		goto fail;
        }

	_algoreg_dbclose();
	return 0;

fail:
	fprintf(stderr, "Registry transaction error in %s: %s\n"
                        ,__FUNCTION__, err_msg);
        sqlite3_free(err_msg);
        _algoreg_dbclose();
        return 1;
}

static char* prepare_stmt(int mode, char *tbl,
			  char *col, char **val){

	sql[0] = '\0';
	char **cols = calloc(20,sizeof(char*));
	char **vals = (char**)val;
	int max_cols = 0;
	int where_cols = 0;
	int valcounter = 0;
	int i = 0;
	int j = 0;

	if (col == NULL){
		cols = get_cols(tbl);
	}else{
		cols[0] = col;
	}

	while (cols[max_cols] != NULL)
		max_cols++;

	while (vals[valcounter] != NULL)
		valcounter++;

	get_where_cols(tbl,mode);
	while (wherecols[where_cols] != NULL)
		where_cols++;

	switch(mode){
		case REG_UPDATE:
			strcat(sql,"UPDATE ");
                        strncat(sql, tbl, strlen(tbl));
			strcat(sql," SET ");
                        while (i < max_cols ){
                                strncat(sql,cols[i], strlen(cols[i]));
                                strcat(sql,"=");
                                strcat(sql,"\"");
                                strncat(sql, vals[i], strlen(vals[i]));
                                strcat(sql,"\"");
				if (i < max_cols-1)
					strcat(sql,",");
				i++;
			}

			if (where_cols > 1){
				strcat(sql," WHERE ");

				while (j < where_cols ){
					strncat(sql,wherecols[j],
						strlen(wherecols[j]));
					strcat(sql,"=");
					strcat(sql,"\"");
					strncat(sql, vals[val_idx(cols,
						wherecols[j])],
						strlen(vals[val_idx(cols,
                                                wherecols[j])]));
					strcat(sql,"\"");
					if (j < where_cols-1)
						strcat(sql," AND ");
				j++;
				}
			}
			break;

		case REG_REPLACE:
			strcat(sql,"INSERT OR REPLACE INTO ");
			strncat(sql, tbl, strlen(tbl));
			strcat (sql, " (");
			while (i < max_cols ){
                                strncat(sql, cols[i], strlen(cols[i]));
                                if (i <  max_cols-1)
                                        strcat(sql,",");
                        i++;
                        }

			strcat(sql,") ");
			strcat(sql, "VALUES (");
			while (j < max_cols ){
				strcat(sql,"\"");
                                strncat(sql, vals[j], strlen(vals[j]));
                                if (j <  max_cols-1){
					strcat(sql,"\"");
                                        strcat(sql,",");
				}
                        j++;
                        }
			strcat(sql,"\"");
			strcat(sql,") ");
			break;

		case REG_DELETE:
			strcat(sql,"DELETE FROM ");
                        strncat(sql, tbl, strlen(tbl));
			strcat(sql," WHERE ");
			while (i < where_cols ){
				strncat(sql,wherecols[i], strlen(wherecols[i]));
				strcat(sql,"=");
				strcat(sql,"\"");
				strncat(sql, vals[val_idx(cols, wherecols[i])],
					strlen(vals[val_idx(cols,
						wherecols[i])]));
				strcat(sql,"\"");
				if (i < where_cols-1)
					strcat(sql," AND ");
			i++;
			}
			break;

		case REG_INSERT:
			strcat(sql,"INSERT INTO ");
                        strncat(sql, tbl, strlen(tbl));
                        strcat (sql, " (");
                        while (i < max_cols ){
                                strncat(sql, cols[i], strlen(cols[i]));
                                if (i <  max_cols-1)
                                        strcat(sql,",");
                        i++;
                        }

                        strcat(sql,") ");
                        strcat(sql, "VALUES (");
                        while (j < max_cols ){
                        strcat(sql,"\"");
                                strncat(sql, vals[j], strlen(vals[j]));
                                if (j <  max_cols-1){
                                        strcat(sql,"\"");
                                        strcat(sql,",");
                                }
                        j++;
                        }
                        strcat(sql,"\"");
                        strcat(sql,") ");
                        break;

		case REG_READ:
			strcat(sql,"SELECT * FROM ");
			strncat(sql, tbl, strlen(tbl));
			if (valcounter >= 1)
				strcat(sql," WHERE ");

			if (valcounter >= 1){
				if (where_cols >= 1){
					while (j < where_cols ){
						strncat(sql,wherecols[j],
                                                strlen(wherecols[j]));
						strcat(sql,"=");
						strcat(sql,"\"");
						strncat(sql, vals[val_idx(cols,
							wherecols[j])],
							strlen(vals[val_idx(cols
							,wherecols[j])]));
						strcat(sql,"\"");
						if (j < where_cols-1)
							strcat(sql," AND ");
					j++;
					}
				}
			}
			break;

		default:
			goto fail;
	}
	printf("%s\n", sql);
	return sql;

fail:
	fprintf(stderr, "Error constructing the registry statement\n");
	return (char *)NULL;
}

static char** get_cols(char *table){

	char **columns = calloc(20, sizeof(char*));
	int x = 0;
	_algoreg_dbopen();
	char stmt[50];
	sprintf(stmt,"PRAGMA table_info(%s)", table);
        rc = sqlite3_exec(db, stmt, pragma_callback, 0, &err_msg);

        if (rc != SQLITE_OK ) {
		goto fail;
	}
        _algoreg_dbclose();

		while (colnames[x] != NULL){
			columns[x] = colnames[x];
			colnames[x] = NULL;
			x++;
		}
        return columns;

fail:
	fprintf(stderr, "Registry transaction error in %s: %s\n"
                        ,__FUNCTION__, err_msg);
        sqlite3_free(err_msg);
        _algoreg_dbclose();
}

static inline void populate_cols(char *col){
	int elements = 0;
	while (colnames[elements] != NULL)
		elements++;
	colnames[elements] = col;
}

static inline int val_idx(char **columns , char *wclause){

        int i = 0;
        while (columns[i]){
                if (strncmp(columns[i], wclause, strlen(wclause)) == 0)
                        return i;
                i++;
        }
        return -1;
}

static inline void get_where_cols(char *table, int mode){

	wherecols = calloc(10,sizeof(char*));

	if (strcmp(table, "nfs_shares") == 0){
		if (mode == REG_READ){
			wherecols[0] = "bpname";
		}else{
			wherecols[0] = "bpname";
			wherecols[1] = "host";
		}
	}else if (strcmp(table, "backup_points") == 0){
		wherecols[0] = "name";

	}else if (strcmp(table, "ssh_allowedhosts") == 0){
		wherecols[0] = "address";
	}

}

static inline int get_current_context(char *table){

	int ctx;

	if (strcmp(table, "nfs_shares") == 0){
		ctx = NFS_CTX;
	}else if (strcmp(table, "backup_points") == 0){
		ctx = BP_CTX;
	}

	return ctx;
}

static void llnfs_push(nfs_share_t *entry){

//		nfshead = NULL;
        nfscurr = (nfs_share_t*)malloc(sizeof(nfs_share_t));
        if (nfscurr == NULL){
              fprintf(stderr,
                     "Error allocating memory for nfs share");
        }

	nfscurr = entry;
	nfscurr->next = nfshead;
	nfshead = nfscurr;
	
	
}

static void llnfs_flush(nfs_share_t *head){

}

static int pragma_callback(void *data, int argc,
		char **argv, char **azColName) {

    data = 0;
    int i;
    char *desiredcol = "name";

    if (colnames == NULL)
          colnames = calloc(20, sizeof(char*));

    for ( i = 0; i < argc; i++) {
        if ((strncmp(azColName[i], desiredcol
                     ,strlen(desiredcol))) == 0){
                populate_cols(strdup(argv[i]));
        }
    }
    return 0;
}

static int nfs_callback(void *data, int argc, char **argv,
                    char **azColName){

	//printf("%10s  %10s  %10s\n", argv[0], argv[1], argv[2]);

	nfs_share_t *entry = (nfs_share_t*)malloc(sizeof(nfs_share_t));
	entry->bpname = argv[0];
	entry->host = argv[1];
	entry->params = argv[2];
	llnfs_push(entry);
	free(entry);

	return 0;

}

static int bp_callback(void *data, int argc, char **argv,
                    char **azColName){

}

static int cifs_callback(void *data, int argc, char **argv,
                    char **azColName){

}

static int repl_callback(void *data, int argc, char **argv,
                    char **azColName){

}

