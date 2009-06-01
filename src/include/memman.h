#ifndef MP_MEMMAN_H
#define MP_MEMMAN_H
/*
** memman is a layer around the alloc function and the free function.
** The function behave equally, except that some minor or major checks
** are preformed, depending on the compiler option -DDEBUG.
**
** If the -DDEBUG option is used, all memory that is alloced through
** this extra layer, should also be freed through this layer, in order
** to keep the database correct.
**
** This module also provides functions to manage freelists. The
** freelists are automatically reduced if the size gets too long.
** If an alloc function is called and there is no memory available,
** the freelists are cleared to try to solve get the needed memory.
**
** It is also possible to register function which should be called
** in the event that there is really no memory. Such a function
** can clear its cache, make backups or realloc buffer where needed.
*/
#include <stdlib.h>

/* needed to make sure that the system allocation functions are not
** redefined in memman.c.
*/
#ifndef MEMMAN_H_INSIDE_MEMMAN_C

#ifdef free
#undef free
#endif

#ifdef malloc
#undef malloc
#endif

#ifdef remalloc
#undef remalloc
#endif

#ifdef calloc
#undef calloc
#endif

#define free(A) mm_free(A)
#define malloc(A) mm_malloc(A)
#define realloc(A,B) mm_realloc(A,B)
#define calloc(A,B) mm_calloc(A,B)

#endif

/* The alloc functions will allocate memory. If they fail,
** it seems useless to go one, since there is either an
** error or memory is just full.
*/
extern void *mm_malloc(size_t size);
extern void *mm_realloc(void *p, size_t size);
extern void *mm_calloc(size_t n, size_t size);
extern void mm_free(void *p);

/*  Freelists are automatically freed for general use if normal allocation
**  fails.  A freelist should only be used for temporary and small objects,
**  although it is not a restriction.
*/

typedef struct {
  void *data;
  int length;  /* current length */
  int regis;   /* a magic number for the memory manager */
  int size;    /* size of blocks (if freelist is empty) */
  int min;     /* minimal freelist size (used when list gets too long) */
  int max;     /* maximal freelist size (free until MIN items remain) */
} FreeList;

/* 2 versions for initializing a static FreeList. */
#define FreeListWithBlockSize(S) { 0,0,0,(S),-1,-1 }
#define FreeListWithSizeAndMax(S,M) { 0,0,0,(S),-1,(M) }

/* The regis field is used as a magic number.  After the first call to
** FreeList_malloc, regis is given that number and the list is added to
** a freelist database. If the system malloc fails, the freelists
** in the database are all destroyed if the fields all make sense
** (regis==MAGIC, all>=0, length<=max, min<=max)
** (otherwise, it could have been a temporary freelist and a memory leak
**  occured)
*/

/* add the memory of data to the freelist */
extern void FreeList_free(FreeList *fl, void *data);

/* get one element from the freelist */
extern void *FreeList_malloc(FreeList *fl);

/* fill the freelist with the minimal number of elements */
extern void FreeList_fill(FreeList *fl);

/* clear the freelist */
extern void FreeList_clear(FreeList *fl);


/* A function can be registered to be called when memory is really low
** Such a function is called when the freelists are cleared and memory
** is still not available.  Usually, this situation would lead to a
** crash or undefined behaviour.  The problem might be caused by 
** requesting a large memory block in combination with too much
** memory fragmentation.
**
** The function should try to free memory by freeing unused buffers and
** reallocing semi-used buffers. (realloc will be in a special
** malloc+free mode.
*/
extern void RegisterPaybackFunction(void (*function)(void));

extern void RemovePaybackFunction(void (*function)(void));

#endif
