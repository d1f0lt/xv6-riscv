#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "proc.h"
#include "fs.h"
#include "file.h"
#include "defs.h"

int
mutexalloc(struct file **f)
{
  struct file *mf;
  struct sleeplock *m;

  *f = 0;
  if((mf = filealloc()) == 0)
    return -1;

  m = (struct sleeplock*)kalloc();
  if(m == 0){
    fileclose(mf);
    return -1;
  }

  initsleeplock(m, "mutex");
  mf->type = FD_MUTEX;
  mf->readable = 0;
  mf->writable = 0;
  mf->mutex = m;
  *f = mf;

  return 0;
}

void
mutexclose(struct sleeplock *m)
{
  if(m)
    kfree((void*)m);
}

int
mutexlock(struct sleeplock *m)
{
  if(m == 0)
    return -1;
  acquiresleep(m);
  return 0;
}

int
mutexunlock(struct sleeplock *m)
{
  if(m == 0)
    return -1;
  if(!holdingsleep(m))
    return -1;
  releasesleep(m);
  return 0;
}

void
mutexunlockifheld(struct sleeplock *m)
{
  if(m && holdingsleep(m))
    releasesleep(m);
}
