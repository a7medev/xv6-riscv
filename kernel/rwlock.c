#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "rwlock.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

// TODO: Fix rwlock implementation

void
initrwlock(struct rwlock *lk, char *name)
{
    initlock(&lk->lk, "rwlock");
    initsleeplock(&lk->rwlk, name);
    lk->readers = 0;
    lk->writer = 0;
}

void
acquireread(struct rwlock *lk)
{
    acquire(&lk->lk);
    int r = lk->readers++;
    release(&lk->lk);
    if (r == 0)
        acquiresleep(&lk->rwlk);
}

void
acquirewrite(struct rwlock *lk)
{
    acquiresleep(&lk->rwlk);
    acquire(&lk->lk);
    lk->writer = myproc()->pid;
    release(&lk->lk);
}

// Releases the lock from a writer. Must be holding lk->lk before calling it.
static void
releasewrite(struct rwlock *lk)
{
    lk->writer = 0;
    releasesleep(&lk->rwlk);
}

// Releases the lock from a reader. Must be holding lk->lk before calling it.
static int
releaseread(struct rwlock *lk)
{
    if (lk->readers < 1) {
        return -1;
    }

    lk->readers--;
    if (lk->readers == 0)
        releasesleep(&lk->rwlk);

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
