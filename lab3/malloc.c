#define _GNU_SOURCE
#include "brk.h"
#include <unistd.h>
#include <string.h> 
#include <errno.h> 
#include <sys/mman.h>
#include <stdio.h>

#define NALLOC 1024                                     /* minimum #units to request */

typedef long Align;                                     /* for alignment to long boundary */

union header {                                          /* block header */
  struct {
    union header *ptr;                                  /* next block if on free list */
    unsigned size;                                      /* size of this block  - what unit? */ 
  } s;
  Align x;                                              /* force alignment of blocks */
};

typedef union header Header;

static Header base;                                     /* empty list to get started */
static Header *freep = NULL;                            /* start of free list */

/* free: put block ap in the free list */

void free(void * ap)
{
  Header *bp, *p;

  if(ap == NULL) return;                                /* Nothing to do */

  bp = (Header *) ap - 1;                               /* point to block header */
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;                                            /* freed block at atrt or end of arena */

  if(bp + bp->s.size == p->s.ptr) {                     /* join to upper nb */
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  }
  else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp) {                             /* join to lower nbr */
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  freep = p;
}

/* morecore: ask system for more memory */

#ifdef MMAP

static void * __endHeap = 0;

void * endHeap(void)
{
  if(__endHeap == 0) __endHeap = sbrk(0);
  return __endHeap;
}
#endif


static Header *morecore(unsigned nu)
{
  void *cp;
  Header *up;
#ifdef MMAP
  unsigned noPages;
  if(__endHeap == 0) __endHeap = sbrk(0);
#endif

  if(nu < NALLOC)
    nu = NALLOC;
#ifdef MMAP
  noPages = ((nu*sizeof(Header))-1)/getpagesize() + 1;
  cp = mmap(__endHeap, noPages*getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  nu = (noPages*getpagesize())/sizeof(Header);
  __endHeap += noPages*getpagesize();
#else
  cp = sbrk(nu*sizeof(Header));
#endif
  if(cp == (void *) -1){                                 /* no space at all */
    perror("failed to get more memory");
    return NULL;
  }
  up = (Header *) cp;
  up->s.size = nu;
  free((void *)(up+1));
  return freep;
}

void *allocate_memory(Header *prevp, Header *p, unsigned nunits) {
  if (p->s.size == nunits)
    prevp->s.ptr = p->s.ptr;
  else {
    p->s.size -= nunits;
    p += p->s.size;
    p->s.size = nunits;
  }
  freep = prevp;
  return (void *)(p+1);
}

void * ff_malloc(unsigned nunits)
{
  Header *p, *prevp;
  Header * morecore(unsigned);

  prevp = freep;
  for(p= prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
    if(p->s.size >= nunits) { /* big enough */
      return allocate_memory(prevp, p, nunits);
    }
    
    if(p == freep) {
      if((p = morecore(nunits)) == NULL) {
	return NULL;
      }
    }
  }
}

void * wf_malloc(unsigned nunits)
{
  Header *p, *prevp;
  Header *largest = NULL;
  Header *largestprev = NULL;
  Header * morecore(unsigned);

  prevp = freep;
  for(p= prevp->s.ptr;  ; prevp = p, p = p->s.ptr) {
    if(p->s.size >= nunits && (largest == NULL || p->s.size > largest->s.size)) {
      largest = p;
      largestprev = prevp;
    }

    if(p == freep) {
      if(largest != NULL) {
	break;
      }
      
      if((p = morecore(nunits)) == NULL) {
	return NULL;
      }
    }
  }

  return allocate_memory(largestprev, largest, nunits);
}

void * malloc(size_t nbytes) {
  unsigned nunits;
  
  if(nbytes == 0) {
    return NULL;
  }

  nunits = (nbytes+sizeof(Header)-1)/sizeof(Header) +1;

  if(freep == NULL) {
    base.s.ptr = freep = &base;
    base.s.size = 0;
  }
  
  if(STRATEGY == 1) {
    return ff_malloc(nunits);
  } else if(STRATEGY == 3) {
    return wf_malloc(nunits);    
  } else {
    fprintf(stderr, "Unsupported strategy! Defaulting to first-fit.\n");
    return ff_malloc(nunits);
  }
}

void * realloc(void * ptr, size_t nbytes) {
  void * newptr;
  Header *h;
  int size;
  
  if(ptr == NULL) {
    return malloc(nbytes);
  }

  if(nbytes == 0) {
    free(ptr);
    return NULL;
  }

  newptr = malloc(nbytes);
  if(newptr == NULL) {
    return NULL;
  }

  h = (Header *) ptr - 1;
  size = (h->s.size - 1) * sizeof(Header);
  if(size < nbytes) {
    memcpy(newptr, ptr, size);
  } else {
    memcpy(newptr, ptr, nbytes);
  }

  free(ptr);
  
  return newptr;
}
