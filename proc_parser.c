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

int isNumericLine(const char *str) {
  if (!str || !*str)
    return 0;
  for (; *str; str++) {
    if (!isdigit(*str))
      return 0;
  }
  return 1;
}

void getUsername(uid_t uid, char *buf, size_t buflen) {
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
              long *rssPages, char *comm, size_t comm_len) {
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
  *rssPages = rss;
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
  static unsigned long pageKb = 0;
  if (pageKb == 0) {
    pageKb = sysconf(_SC_PAGESIZE) / 1024;
    if (pageKb == 0)
      pageKb = 4;
  }
  return pageKb;
}

int getPsInfo(pid_t pid, PsInfo *info) {
  info->pid = pid;
  if (getUid(pid, &info->uid) != 0)
    return -1;
  getUsername(info->uid, info->user, sizeof(info->user));
  unsigned long utime = 0, stime = 0;
  long rssPages = 0;
  char comm[256] = {0};
  if (parseStat(pid, &utime, &stime, &rssPages, comm, sizeof(comm)) != 0)
    return -1;

  info->utimeTicks = utime;
  info->stimeTicks = stime;
  strncpy(info->name, comm, sizeof(info->name));
  info->name[sizeof(info->name) - 1] = '\0';

  unsigned long pageKb = getPageSizeKb();
  if (rssPages > 0)
    info->rssKb = rssPages * pageKb;
  else
    info->rssKb = 0;
  return 0;
}
