#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "bitarray.h"
#include "filelock.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

void
initflock(struct flock *lk, char *name)
{
  initlock(&lk->lk, "filelock");
  lk->locked = 0;
  lk->readers = 0;
  lk->writer = 0;
}

// acquires internal sleep lock. lk->lk must be acquired when calling this.
static void
acquirelk(struct flock *lk)
{
  while (lk->locked) {
    sleep(lk, &lk->lk);
  }
  lk->locked = 1;
}

// releases internal sleep lock. lk->lk must be acquired when calling this.
static void
releaselk(struct flock *lk)
{
  lk->locked = 0;
  wakeup(lk);
}

// Acquires shared access to file lock
int
flocksh(struct flock *lk)
{
  acquire(&lk->lk);
  if (BIT(lk->holders, myprocidx())) {
    release(&lk->lk);
    return -1;
  }

  lk->readers++;
  if (lk->readers == 1)
    acquirelk(lk);
  SETBIT(lk->holders, myprocidx());
  release(&lk->lk);

  return 0;
}

// Acquires exclusive access to file lock
int
flockex(struct flock *lk)
{
  acquire(&lk->lk);
  if (BIT(lk->holders, myprocidx())) {
    release(&lk->lk);
    return -1;
  }

  acquirelk(lk);
  lk->writer = myproc()->pid;
  SETBIT(lk->holders, myprocidx());
  release(&lk->lk);

  return 0;
}

// Releases the lock from a writer. Must be holding lk->lk before calling it.
static void
funlockex(struct flock *lk)
{
  lk->writer = 0;
  releaselk(lk);
}

// Releases the lock from a reader. Must be holding lk->lk before calling it.
static int
funlocksh(struct flock *lk)
{
  if (lk->readers < 1)
    return -1;

  lk->readers--;
  if (lk->readers == 0)
    releaselk(lk);
  
  return 0;
}

// Releases the file lock
// Holding lk->lk is done separately to allow users to maintain data structures
// about the holders of the filelock inside the critical section of funlock.
int
funlock(struct flock *lk)
{
  acquire(&lk->lk);
  
  if (!BIT(lk->holders, myprocidx())) {
    release(&lk->lk);
    return -1;
  }

  CLEARBIT(lk->holders, myprocidx());

  if (lk->writer == myproc()->pid) {
    funlockex(lk);
    release(&lk->lk);
    return 0;
  }

  int r = funlocksh(lk);
  release(&lk->lk);
  return r;
}

int
holdingflock(struct flock *lk)
{
  acquire(&lk->lk);
  int r = BIT(lk->holders, myprocidx());
  release(&lk->lk);
  return r;
}
