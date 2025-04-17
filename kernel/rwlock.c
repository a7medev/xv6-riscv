#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "rwlock.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

void
initrwlock(struct rwlock *lk, char *name)
{
  initlock(&lk->lk, "rwlock");
  lk->locked = 0;
  lk->readers = 0;
  lk->writer = 0;
}

// acquires internal sleep lock. lk->lk must be acquired when calling this.
static void
acquirelk(struct rwlock *lk)
{
  while (lk->locked) {
    sleep(lk, &lk->lk);
  }
  lk->locked = 1;
}

// releases internal sleep lock. lk->lk must be acquired when calling this.
static void
releaselk(struct rwlock *lk)
{
  lk->locked = 0;
  wakeup(lk);
}

void
acquireread(struct rwlock *lk)
{
  acquire(&lk->lk);
  lk->readers++;
  if (lk->readers == 1)
    acquirelk(lk);
  release(&lk->lk);
}

void
acquirewrite(struct rwlock *lk)
{
  acquire(&lk->lk);
  acquirelk(lk);
  lk->writer = myproc()->pid;
  release(&lk->lk);
}

// Releases the lock from a writer. Must be holding lk->lk before calling it.
static void
releasewrite(struct rwlock *lk)
{
  lk->writer = 0;
  releaselk(lk);
}

// Releases the lock from a reader. Must be holding lk->lk before calling it.
static int
releaseread(struct rwlock *lk)
{
  if (lk->readers < 1)
    return -1;

  lk->readers--;
  if (lk->readers == 0)
    releaselk(lk);

  return 0;
}

int
releaserw(struct rwlock *lk)
{
  int r;
  acquire(&lk->lk);
  if (lk->writer == myproc()->pid) {
    releasewrite(lk);
    r = 0;
  } else {
    r = releaseread(lk);
  }
  release(&lk->lk);
  return r;
}
