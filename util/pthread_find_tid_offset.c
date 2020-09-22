/* TO RUN, DO:  gcc -pthread -g3 pthread_find_tid_offset.c
 * NOTE:  This can be used to help find the offset of
 *        any tid or pid in the TCB (thread-control block).
 *        This is useful in PID_OFFSET/TID_OFFSET in src/mtcp/restore_libc.c
 */

#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>

#ifdef __GLIBC__
# define TID_OFFSET 720
#else
# define TID_OFFSET 56 /* else this is musl libc */
#endif
#define PTHREAD_MAX_OFFSET 1000

#ifdef __GLIBC__
void *foo(void *arg) {return NULL;};
void create_thread(void) {
  pthread_t thread;
  pthread_create(&thread, NULL, foo, NULL);
}
#endif

int main() {
#ifdef __GLIBC__
  // In glibc, pthread_self() returns 0 unless there's at least one more thread
  create_thread();
#endif
  pthread_t thr = pthread_self();
  errno = 99;
  printf("We have hard-wired in the pid/tid offset: %d\n",
	 TID_OFFSET);
#ifdef __GLIBC__
  printf("  FROM direct syscall: pid: %d; tid: %d\n",
	 getpid(), (int)(syscall(SYS_gettid)));
  printf("  FROM pthread_t     : pid: %d; tid: %d\n",
	 *(int *)((char *)thr+TID_OFFSET), *(int *)((char *)thr+TID_OFFSET+4));
#else
  printf("  FROM direct syscall: tid: %d; errno: %d\n",
	 (int)(syscall(SYS_gettid)), errno);
  printf("  FROM pthread_t     : tid: %d; errno: %d\n",
	 *(int *)((char *)thr+TID_OFFSET), *(int *)((char *)thr+TID_OFFSET+4));
#endif
  pid_t pid = getpid();
  int i;
  // PTHREAD_MAX_OFFSET must be large enough for tid/pid offset in pthread_t.
  printf("Here, we automatically discover the offset.\n");
  for (i = 0; i < PTHREAD_MAX_OFFSET; i += 4) {
#ifdef __GLIBC__
    // pid/tid appear in pthread_t.  Since tid == pid for primary thread,
    //   this is enough:
    if (*(int *)((char *)thr + i) == pid &&
        *(int *)((char *)thr + i + 4) == pid)
#else
    // tid/errno_val appear in pthread_t.  Since tid == pid for primary thread,
    //   this is enough:
    if (*(int *)((char *)thr + i) == pid &&
        *(int *)((char *)thr + i + 4) == errno)
#endif
    {
      printf("  tid offset discovered in pthread_t: %d\n", i);
    }
  }
  if (i == PTHREAD_MAX_OFFSET) {
    printf("  tid offset in pthread_t was not found.\n");
  }
  return 0;
}
