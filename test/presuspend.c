#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>

#include "dmtcp.h"

#define NUM_THREADS 10
#define GETTID() syscall(SYS_gettid)

// Inform the presuspend plugin.
void setChildPidPtr(pid_t *masterChildPidPtr) __attribute((weak));
#define setChildPidPtr(arg) (setChildPidPtr ? setChildPidPtr(arg) : 0)
int in_presuspend = 0;
pid_t childPid = 0;

int main()
{
  setChildPidPtr(&childPid);
  if (in_presuspend) {
    printf("PRESUSPEND event has been received at start of main().\n");
    exit(1);
  }

  while (1) {
    childPid = fork();
    if (childPid == 0) {
      int childCounter = 1;
      while (1) {
        printf("Child %d\n", childCounter++);
        fflush(stdout);
        sleep(1);
      }
    } else {
      int parentCounter = 1;
      while(childPid && kill(childPid, 0) == 0) {
        printf("Parent %d\n", parentCounter++);
        fflush(stdout);
        sleep(1);
      }
      printf("Parent: Child exited; will fork another child.\n");
      fflush(stdout);

      // Wait for resume before forking another child.
      // TODO(Kapil): Add support for creating  new processes in Presuspend
      //              phase.
      while (childPid != 0) {
        sleep(1);
      }
    }
  }

  return 0;
}
