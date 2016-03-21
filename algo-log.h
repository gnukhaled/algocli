
#ifndef ALGOLOG_H
#define ALGOLOG_H

#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#define MSG_MAX 300

/* Log Levels */
#define ALGO_CRIT 1
#define ALGO_ERR 2
#define ALGO_WARN 3
#define ALGO_NOTICE 4
#define ALGO_INFO 5

/* Log contexts*/
#define CTX_ACCESSCTRL  1
#define CTX_SYSHEALTH   2
#define CTX_SNAPSHOT    3
#define CTX_CONFIG      4
#define CTX_SYSTEM      5
#define CTX_REPL        6
#define CTX_USER        7
#define CTX_CIFS        8
#define CTX_DISK        9
#define CTX_LOG         10
#define CTX_NFS         11
#define CTX_VTL         12
#define CTX_NET         13
#define CTX_BP          14
#define CTX_FS          15

char *tstamp();
int algolog(int type, char *fmt, ...);
void init_logger(int ctx);

FILE *logfp;

//extern char *username;
//username = getenv("USER");

#endif /* ALGOLOG_H */
