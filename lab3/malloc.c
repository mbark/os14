#define _GNU_SOURCE
#include "brk.h"
#include <unistd.h>
#include <string.h> 
#include <errno.h> 
#include <sys/mman.h>
#include <stdio.h>

#define NALLOC 1024

typedef long Align;

union header {
  struct {
    union header *ptr;
    unsigned size;
  } s;
  Align x;
};

typedef union header Header;

/*
The first element in the list of free memory. Used as base for the list.
*/
static Header base;
/*
A pointer that points to the most recently used node in the circular
list of memory pointers.
*/
static Header *freep = NULL;

/*
Frees up the memory previously allocated. The parameter, ap, is the pointer
returned by a previous malloc or realloc call. The method will find the pointer
saved by malloc and move it to the list of freed nodes. It will also, if
possible, merge the memory of two nodes to create a node with more memory.
ap is the pointer to the previosly allocated memory
*/
void free(void * ap)
{
  /* bp points to the header of ap. */
  /* p points to the free memory chunk closest to bp with a lower address. */
  Header *bp, *p;

  if(ap == NULL) {
    return;
  }

  bp = (Header *) ap - 1;
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr) {
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr)) {
      break;
    }
  }

  if(bp + bp->s.size == p->s.ptr) {
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else {
    bp->s.ptr = p->s.ptr;
  }
  
  if(p + p->s.size == bp) {
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else {
    p->s.ptr = bp;
  }
  freep = p;
}

/* morecore: ask system for more memory */

#ifdef MMAP

/* Holds the address where the heap ends. */
static void * __endHeap = 0;

/* Returns the address where the heap ends. */
void * endHeap(void) {
  if(__endHeap == 0) {
    __endHeap = sbrk(0);
  }
  return __endHeap;
}
#endif

/* morecore uses mmap to allocate more memory from the operating system. nu units
  of memory will be allocated (rounded up to the nearest integer number of
  pages). The newly allocated memory will be added to the free list.
  morecore returns freep. */
static Header *morecore(unsigned nu) {
  /* Holds the return value of mmap. */
  void *cp;
  /* A pointer to the header of the newly allocated memory chunk. */
  Header *up;
#ifdef MMAP
  /* The number of pages to allocate. */
  unsigned noPages;
  if(__endHeap == 0) {
    __endHeap = sbrk(0);
  }
#endif

  if(nu < NALLOC) {
    nu = NALLOC;
  }
#ifdef MMAP
  noPages = ((nu*sizeof(Header))-1)/getpagesize() + 1;
  cp = mmap(__endHeap, noPages*getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  nu = (noPages*getpagesize())/sizeof(Header);
  __endHeap += noPages*getpagesize();
#else
  cp = sbrk(nu*sizeof(Header));
#endif
  if(cp == (void *) -1){
    return NULL;
  }
  up = (Header *) cp;
  up->s.size = nu;
  free((void *)(up+1));
  return freep;
}

/*
Allocates memory by reducing the size of p's free size. The pointer's
free size will be split if necessary.
prevp is the pointer to the header that points to p
p is the pointer pointing to the place where we will allocate the
memory.
nunits is the memory to be allocated given as a number of headers (units).
*/
void *allocate_memory(Header *prevp, Header *p, unsigned nunits) {
  if (p->s.size == nunits) {
    prevp->s.ptr = p->s.ptr;
  } else {
    p->s.size -= nunits;
    p += p->s.size;
    p->s.size = nunits;
  }
  freep = prevp;
  return (void *)(p+1);
}

/*
Allocate memory using the First Fit algorithm. It will go through
the list of free space and upon finding a spot large enough allocate that.
nunits is the amount of memory to be allocated given as a number of headers (units).
*/
void * ff_malloc(unsigned nunits) {
  /*
  The pointers used when traversing the list, p will be the current pointer
  and prevp the previous one.
  */
  Header *p, *prevp;
  /*
  A function prototype
  */
  Header * morecore(unsigned);

  prevp = freep;
  for(p= prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
    if(p->s.size >= nunits) {
      return allocate_memory(prevp, p, nunits);
    }

    if(p == freep) {
      if((p = morecore(nunits)) == NULL) {
       return NULL;
     }
   }
 }
}

/*
Allocate memory using the Worst Fit algorithm. Goes through the list of
free space saving the largest free memory. Then allocates the largest found memory.
nunits is the amount of memory to be allocated given as a number of headers (units).
*/
void * wf_malloc(unsigned nunits)
{
  /*
  The pointers used when traversing the list, p will be the current pointer
  and prevp the previous one.
  */
  Header *p, *prevp;
  /*
  The largest free memory found so far.
  */
  Header *largest = NULL;
  /*
  The previous pointer to the largest memory found.
  */
  Header *largestprev = NULL;
  /*
  A function prototype.
  */
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
/*
Allocates memory of the size given by the parameter, nbytes. The method will
convert the given bytes to units, that is convert it to the number of headers
it corresponds to. The reason for this is that we want all data to align properly.
It will then check what strategy is used and use the corresponding malloc algorithm.
nbytes is the number of bytes to allocate.
*/
void * malloc(size_t nbytes) {
  /*
  The memory to allocate as units (number of headers)
  */
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

/*
Reallocates memory, that is increases or decreases the memory for previously allocated
memory. It will allocate new memory of the given size, and then copy the data to the
new allocated space. The size of the data copied is determined by min(nbytes, nprev)
where nprev is the previously allocated size.
ptr is the pointer to the previously allocated memory
nbytes is the number of bytes to allocate
*/
void * realloc(void * ptr, size_t nbytes) {
  /*
  The pointer to the new allocated memory.
  */
  void * newptr;
  /*
  The header for the previously allocated memory.
  */
  Header *h;
  /*
  The size of the previously allocated memory.
  */
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
