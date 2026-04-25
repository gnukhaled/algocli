
#include "algocli.h"
#include "cmd-log.h"

int log_list(char *filename){

	static struct stat sb;

	if (chdir(LOG_DIR) != 0) {
		perror("chdir(LOG_DIR)");
		return -1;
	}
	if (stat(filename, &sb) == -1){
		perror("stat error");
	}

	char *mtime = ctime(&sb.st_mtime);

	char *pos;
	if ((pos = strchr(mtime, '\n')) != NULL)
		*pos = '\0';

	printf("%9s      %2f      %s\n", filename, (float)sb.st_size/1024/1024 ,mtime);
	return errno;

}

//XXX
//FIXME buffer must be deallocated
//XXX
 
char* log_view(char *filename){

   char *buffer = NULL;
   long string_size;
   size_t read_size;

   if (chdir(LOG_DIR) != 0) {
       perror("chdir(LOG_DIR)");
       return NULL;
   }
   FILE *logfile = fopen(filename, "r");

   if (logfile)
   {
       fseek(logfile, 0, SEEK_END);
       string_size = ftell(logfile);
       rewind(logfile);

       if (string_size < 0) {
           fclose(logfile);
           return NULL;
       }

       buffer = malloc((size_t)string_size + 1);
       if (!buffer) {
           fclose(logfile);
           return NULL;
       }
       read_size = fread(buffer, 1, (size_t)string_size, logfile);
       buffer[string_size] = '\0';

       if ((size_t)string_size != read_size) {
           free(buffer);
           buffer = NULL;
        }
        fclose(logfile);
    }

    return buffer;
}


int log_watch(char *filename){
        (void)filename;
        return 0;
}


/* ----- dispatcher shims (formerly in algocli.c) ----------------- */

int func_log(char **arg) {
    (void)arg;
    cmd_ref();
    return 0;
}

int func_log_list(char **arg) {
    /* TODO(phase4): this function's filter logic is still broken
     * (the first non-matching entry returns -1 immediately). Kept
     * as-is during the Phase 7 move; the fix is its own Phase 4 task. */
    int ret = 0;
    DIR *logdir;
    struct dirent *dir;
    const char *filter = (arg && arg[0]) ? arg[0] : "";

    logdir = opendir(LOG_DIR);
    if (!logdir) {
        perror("opendir");
        return -1;
    }

    while ((dir = readdir(logdir)) != NULL) {
        if (*filter && strncmp(dir->d_name, filter, strlen(filter)) != 0)
            continue;
        printf("No such log file %s\n", filter);
        ret = -1;
        break;
    }

    printf("\n");
    printf("%s      %s     %s\n", "File Name", "Size (MB)", "Modified");
    printf("---------      ---------     ------------------------\n");

    rewinddir(logdir);
    if (*filter == '\0') {
        while ((dir = readdir(logdir)) != NULL) {
            if (dir->d_name[0] != '.') {
                ret = log_list(dir->d_name);
            }
        }
    }
    printf("\n");
    closedir(logdir);
    return ret;
}

int func_log_view(char **arg) {
    char *string = log_view(arg[0]);
    if (string) {
        puts(string);
    }
    return 0;
}

int func_log_watch(char **arg) {
    (void)arg;
    STUB_NOT_IMPLEMENTED("log watch");
}
