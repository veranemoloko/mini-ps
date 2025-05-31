#include "proc_parser.h"
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {

  DIR *dir = opendir("/proc");
  if (!dir) {
    perror("open /proc");
    return 1;
  }

  puts("PID    USER     NAME                    CPU(s)     MEM(KB)");
  struct dirent *dirItem; // Structure containing directory entry information
  while ((dirItem = readdir(dir)) != NULL) {
    if (!isNumericLine(dirItem->d_name)) // Skip if name is not a PID
      continue;

    PsInfo inf;
    pid_t pid = atoi(dirItem->d_name);
    if (getPsInfo(pid, &inf) == 0) {

      long ticksSec = sysconf(_SC_CLK_TCK); // Get system clock ticks per secon
      double cpu_sec = (inf.utimeTicks + inf.stimeTicks) /
                       (double)ticksSec; // Calculate total CPU time in seconds

      printf("%-6d %-8s %-20s %10.2f %10lu\n",
             inf.pid,    // Process ID
             inf.user,   // Username
             inf.name,   // Process name
             cpu_sec,    // CPU time
             inf.rssKb); // Memory usage in KB
    }
  }
  closedir(dir);
  return 0;
}
