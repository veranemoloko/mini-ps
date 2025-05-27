#include "proc_parser.h"
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int isNumeric(const char *str) {
  if (!str || !*str)
    return 0;
  for (; *str; str++) {
    if (!isdigit(*str))
      return 0;
  }
  return 1;
}

int main() {

  DIR *dir = opendir("/proc");
  if (!dir) {
    perror("proc");
    return 1;
  }

  struct dirent *dirItem;
  printf("%-6s %-8s %-20s %-10s %-10s\n", "PID", "USER", "NAME", "CPU(s)",
         "MEM(KB)");

  while ((dirItem = readdir(dir)) != NULL) {
    if (!isNumeric(dirItem->d_name))
      continue;

    pid_t pid = atoi(dirItem->d_name);
    PsInfo inf;
    if (getPsInfo(pid, &inf) == 0) {
      long ticksSec = sysconf(_SC_CLK_TCK);
      double cpu_sec = (inf.utimeTicks + inf.stimeTicks) / (double)ticksSec;
      printf("%-6d %-8s %-20s %10.2f %10lu\n", inf.pid, inf.user, inf.name,
             cpu_sec, inf.rss_kb);
    }
  }
  closedir(dir);
  return 0;
}
