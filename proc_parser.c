#define _GNU_SOURCE
#include "proc_parser.h"
#include <ctype.h>
#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

static void getUsername(uid_t uid, char *buf, size_t buflen) {
  struct passwd *pw = getpwuid(uid);
  if (pw) {
    if (buflen > 0) {
      strncpy(buf, pw->pw_name, buflen - 1);
      buf[buflen - 1] = '\0';
    }
  } else {
    snprintf(buf, buflen, "%d", uid);
  }
}

int parseStat(pid_t pid, unsigned long *utime, unsigned long *stime,
              long *rss_pages, char *comm, size_t comm_len) {
  char path[64], buf[1024];
  FILE *fp;
  snprintf(path, sizeof(path), PROC_DIR "/%d/stat", pid);
  fp = fopen(path, "r");
  if (!fp)
    return -1;

  if (!fgets(buf, sizeof(buf), fp)) {
    fclose(fp);
    return -1;
  }
  fclose(fp);

  char *start = strchr(buf, '(');
  char *end = strrchr(buf, ')');
  if (!start || !end || end <= start + 1)
    return -1;
  size_t nameLen = end - start - 1;
  if (nameLen >= comm_len)
    nameLen = comm_len - 1;
  strncpy(comm, start + 1, nameLen);
  comm[nameLen] = '\0';

  char tmp[1024];
  if (sizeof(tmp) > 0) {
    strncpy(tmp, buf, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
  }

  tmp[start - buf] = ' ';
  tmp[end - buf] = ' ';

  unsigned long ut = 0, st = 0;
  long rss = 0;

  int fieldNum = 0;
  char *tok = strtok(tmp, " ");
  while (tok) {
    fieldNum++;
    if (fieldNum == 14)
      ut = strtoul(tok, NULL, 10);
    else if (fieldNum == 15)
      st = strtoul(tok, NULL, 10);
    else if (fieldNum == 24) {
      rss = strtol(tok, NULL, 10);
      break;
    }
    tok = strtok(NULL, " ");
  }

  *utime = ut;
  *stime = st;
  *rss_pages = rss;
  return 0;
}

int getUid(pid_t pid, uid_t *uid) {
  char path[64], line[256];
  FILE *fp;
  snprintf(path, sizeof(path), PROC_DIR "/%d/status", pid);
  fp = fopen(path, "r");
  if (!fp)
    return -1;

  while (fgets(line, sizeof(line), fp)) {
    if (strncmp(line, "Uid:", 4) == 0) {
      unsigned int ruid;
      if (sscanf(line, "Uid:\t%u", &ruid) == 1) {
        *uid = (uid_t)ruid;
        fclose(fp);
        return 0;
      }
    }
  }

  fclose(fp);
  return -1;
}

unsigned long getPageSizeKb() {
  static unsigned long page_kb = 0;
  if (page_kb == 0) {
    page_kb = sysconf(_SC_PAGESIZE) / 1024;
    if (page_kb == 0)
      page_kb = 4;
  }
  return page_kb;
}

int getPsInfo(pid_t pid, PsInfo *info) {
  info->pid = pid;

  if (getUid(pid, &info->uid) != 0)
    return -1;
  getUsername(info->uid, info->user, sizeof(info->user));

  unsigned long utime = 0, stime = 0;
  long rss_pages = 0;
  char comm[256] = {0};
  if (parseStat(pid, &utime, &stime, &rss_pages, comm, sizeof(comm)) != 0)
    return -1;

  info->utimeTicks = utime;
  info->stimeTicks = stime;
  strncpy(info->name, comm, sizeof(info->name));
  info->name[sizeof(info->name) - 1] = '\0';

  unsigned long page_kb = getPageSizeKb();
  if (rss_pages > 0)
    info->rss_kb = rss_pages * page_kb;
  else
    info->rss_kb = 0;

  return 0;
}
