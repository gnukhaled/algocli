
#include "cmd-log.h"

int log_list(char *filename){

	int temp;
	static struct stat sb;

	chdir(LOG_DIR);
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
   int string_size,read_size;

   chdir(LOG_DIR);
   FILE *logfile = fopen(filename, "r");

   if (logfile)
   {
       fseek(logfile,0,SEEK_END);
       string_size = ftell(logfile);
       rewind(logfile);

       buffer = (char*)malloc(sizeof(char) *(string_size + 1));
       read_size = fread(buffer,sizeof(char), string_size,logfile);
       buffer[string_size] = '\0';

       if (string_size != read_size) {
           free(buffer);
           buffer = NULL;
        }
    }

    return buffer;
}


int log_watch(char *filename){


	return 0;
}

