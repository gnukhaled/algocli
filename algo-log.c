
#include "algo-log.h"

FILE *logfp = NULL;
static char timestamp[255];

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
	case CTX_AUDIT:
                filename = "/tmp/algocli-audit.log";
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

int algolog(int type, const char *fmt, ...){
  va_list ap;
  int count = 0;
  char hostname[HOST_NAME_MAX + 1];

  if (!logfp)
      return -1;

  if (gethostname(hostname, sizeof hostname) != 0) {
      perror("gethostname");
      snprintf(hostname, sizeof hostname, "localhost");
  }
  hostname[sizeof hostname - 1] = '\0';

  count += fprintf(logfp, "%s %s ", tstamp(), hostname);

  switch (type){
        case ALGO_CRIT:   count += fprintf(logfp, "CRITICAL: "); break;
        case ALGO_ERR:    count += fprintf(logfp, "ERROR: ");    break;
        case ALGO_WARN:   count += fprintf(logfp, "WARNING: ");  break;
        case ALGO_NOTICE: count += fprintf(logfp, "NOTICE: ");   break;
        case ALGO_INFO:   count += fprintf(logfp, "INFO: ");     break;
        default: break;
  }

  va_start(ap, fmt);
  count += vfprintf(logfp, fmt, ap);
  va_end(ap);

  fflush(logfp);
  /* TODO(phase3): keep the FILE* open across calls instead of
   * fclose-ing per call; require init_logger() once at startup. */
  fclose(logfp);
  logfp = NULL;
  return count;
}

