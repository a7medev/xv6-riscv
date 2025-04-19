// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct metadata {
  int ref; // reference count for physical page used in COW
};

struct {
  struct spinlock lock;
  struct run *freelist;
  struct metadata *metadata;
  void *pagesstart;
} kmem;

static struct metadata*
kmetadata(void *pa)
{
  return &kmem.metadata[(pa - kmem.pagesstart) / PGSIZE];
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");

  int pagesz = PGSIZE + sizeof(struct metadata); // page size + metadata overhead
  uint64 npages = (PHYSTOP - (uint64)end) / pagesz;
  uint64 metadatasz = npages * sizeof(struct metadata);

  kmem.metadata = (struct metadata *)end;
  kmem.pagesstart = (char *)PGROUNDUP((uint64)end + metadatasz);

  for (int i = 0; i < npages; i++) {
    kmem.metadata[i].ref = 1; // init ref to 1 for initial kfree to work
  }

  freerange(kmem.pagesstart, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p = pa_start;
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

static int
kinvalid(void *pa)
{
  return ((uint64)pa % PGSIZE) != 0 || (void*)pa < kmem.pagesstart || (uint64)pa >= PHYSTOP;
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  struct metadata *md;

  if(kinvalid(pa))
    panic("kfree");

  md = kmetadata(pa);

  acquire(&kmem.lock);
  if (--md->ref > 0) {
    release(&kmem.lock);
    return;
  }
  release(&kmem.lock);

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  struct metadata *md;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) {
    kmem.freelist = r->next;
    md = kmetadata(r);
    md->ref = 1;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void
kretain(void *pa)
{
  struct metadata *md;

  if(kinvalid(pa))
    panic("kretain");

  md = kmetadata(pa);

  acquire(&kmem.lock);
  if (md->ref == 0)
    panic("kretain: ref is 0");
  md->ref++;
  release(&kmem.lock);
}
