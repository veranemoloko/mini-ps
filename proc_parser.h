#ifndef PROC_PARSER_H
#define PROC_PARSER_H

#include <sys/types.h>

#define PROC_DIR "/proc"

typedef struct PsInfo {
  pid_t pid;
  uid_t uid;
  char user[64];
  char name[256];
  unsigned long utimeTicks;
  unsigned long stimeTicks;
  unsigned long rss_kb;
} PsInfo;

int get_ps_info(pid_t pid, PsInfo *info);

#endif
