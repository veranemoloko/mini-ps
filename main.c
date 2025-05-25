#include "proc_parser.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  DIR *dir = opendir("/proc");
  if (!dir) {
    perror("proc");
    return 1;
  }

  struct dirent *dir_item;
  long ticks_sec = sysconf(_SC_CLK_TCK);

  printf("%-6s %-8s %-20s %-10s %-10s\n", "PID", "USER", "NAME", "CPU(s)",
         "MEM(KB)");

  while ((dir_item = readdir(dir)) != NULL) {
    if (!is_numeric(dir_item->d_name))
      continue;
    pid_t pid = atoi(dir_item->d_name);

    PsInfo info;
    if (get_ps_info(pid, &info) == 0) {
      double cpu_sec =
          (info.utime_ticks + info.stime_ticks) / (double)ticks_sec;

      printf("%-6d %-8s %-20s %10.2f %10lu\n", info.pid, info.user, info.name,
             cpu_sec, info.rss_kb);
    }
  }

  closedir(dir);
  return 0;
}
