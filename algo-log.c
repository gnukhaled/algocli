
#include "algo-log.h"

extern FILE *logfp;
char timestamp[255];

void init_logger(int ctx){

  char *filename;
  char error[MSG_MAX];

  switch(ctx){
	case CTX_ACCESSCTRL:
		filename = "/tmp/accessctrl.log";
		break;
	case CTX_SYSHEALTH:
                filename = "/tmp/syshealth.log";
                break;
	case CTX_SNAPSHOT:
                filename = "/tmp/snapshot.log";
                break;
	case CTX_CONFIG:
                filename = "/tmp/config.log";
                break;
	case CTX_SYSTEM:
                filename = "/tmp/system.log";
                break;
	case CTX_REPL:
                filename = "/tmp/repl.log";
                break;
	case CTX_USER:
                filename = "/tmp/user.log";
                break;
	case CTX_CIFS:
                filename = "/tmp/cifs.log";
                break;
	case CTX_DISK:
                filename = "/tmp/disk.log";
                break;
	case CTX_LOG:
                filename = "/tmp/log.log";
                break;
	case CTX_NFS:
                filename = "/tmp/nfs.log";
                break;
	case CTX_VTL:
                filename = "/tmp/vtl.log";
                break;
	case CTX_NET:
                filename = "/tmp/net.log";
                break;
	case CTX_BP:
                filename = "/tmp/bp.log";
                break;
	case CTX_FS:
                filename = "/tmp/fs.log";
                break;
	default:
                filename = "/dev/null";
  }


  if (!(logfp = fopen(filename, "a+"))){
    sprintf(error, "Logger setup: failed to open log file %s.\n", filename);
    perror(error);
  }
}

char *tstamp() {

  char *month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  time_t ltime;
  struct tm *Tm;
  ltime=time(NULL);
  Tm=localtime(&ltime);

  sprintf((char*)timestamp,"%s %d %d:%d:%d %d",
    month[Tm->tm_mon],
    Tm->tm_mday,
    Tm->tm_hour,
    Tm->tm_min,
    Tm->tm_sec,
    Tm->tm_year+1900);

    return timestamp;
}

int algolog(int type, char *fmt, ...){

  va_list ap;
  char format[MSG_MAX];
  int count = 0;
  int i, j;
  char *s;

  int ret;
  char hostname[128];

  if ((ret = gethostname(hostname, sizeof hostname)) != 0){
	perror("gethostname");
  }

  time_t timestamp = time(NULL);
  char *ts = ctime(&timestamp);

  fprintf(logfp,"%s %s ", tstamp(), hostname);

  switch (type){
	case 1:
		fprintf(logfp,"%s ", "CRITICAL:");
                break;
        case 2:
                fprintf(logfp,"%s ", "ERROR:");
                break;
        case 3:
                fprintf(logfp,"%s ", "WARNING:");
                break;
        case 4:
                fprintf(logfp,"%s ", "NOTICE:");
                break;
        case 5:
                fprintf(logfp,"%s ", "INFO:");
                break;
  }


  va_start(ap, fmt);

  while (*fmt){

    for (j = 0; fmt[j] && fmt[j] != '%'; j++)
      format[j] = fmt[j];

    if (j){
      format[j] = '\0';
      count += fprintf(logfp, format);
      fmt += j;
    }else{
      for (j = 0; !isalpha(fmt[j]); j++){
        format[j] = fmt[j];
        if (j && fmt[j] == '%')
          break;
      }
      format[j] = fmt[j];
      format[j + 1] = '\0';
      fmt += j + 1;

      switch (format[j]) {
	case 'd':
		i = va_arg(ap, int);
		count += fprintf(logfp, format, i);
		break;

	case 's':
		s = va_arg(ap, char *);
		count += fprintf(logfp, format, s);
		break;

	case 'n':
		count += fprintf(logfp, "%d", count);
		break;

	case '%':
		count += fprintf(logfp, "%%");
		break;

		default:
		fprintf(stderr, "Invalid log message format\n");
      }
    }
 }

  va_end(ap);
  fclose(logfp);
  return count;
}

