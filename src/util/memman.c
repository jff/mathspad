
#include <stdlib.h>
#include <stdio.h>

#define MEMMAN_H_INSIDE_MEMMAN_C
#include "memman.h"


static void error_func(void)
{
    int i=10;
    /* function to break when something goes wrong */
    while (i>0) i--;
}

extern char *memset();

/*
** On some systems free and malloc give different results. mm_free
** and mm_malloc try to simulate that behaviour in order to get
** a portable program.
**
** There are two modes for this module.
** 1  This mode preforms minor checks to simplify programming:
**    * allocation of 0 bytes returns the NULL pointer.
**    * reallocation of NULL pointer.
**    * freeing of NULL pointer.
**
** 2  This mode does a better check by keeping a database of malloced
**    pointers.  It performs the following checks:
**    * is the argument to free indeed allocated?
**    * is the result of an alloc function not already used?
**    * is freed memory correctly used, that is, are the boundaries
**      obeyed?  This is a rather simple check: allocate extra memory at
**      the end and store some values there, store the values in front of
**      the allocated memory in the database. Check if these values are
**      changed before calling free. (This check does not work on some
**      systems, since the values in front of a merory block can be changed
**      by the systems memory manager).
**    * change the contents for freed memory, forsing the program to correctly
**      use the free function (after the last use of the memory).
**      Incorrect programs will show undefined behaviour near the incorrect
**      free call.
**
**  If suspicious memory manipulations are performed, a special dummy
**  function is called which can be used as breakpoint.  In mode 2, 
**  an error message is written to stderr and the called function fails
**  (returns NULL or does not free anything).
**
*/

#ifdef DEBUG

/* mode 2: extra checks */

typedef struct {
    void *ptr;
    char first;
    size_t size;
} LISTELM;

typedef struct ltnode *lfptr;

typedef struct ltnode {
    int kind;
    LISTELM *val;
    void *min;
    lfptr left;
    lfptr right;
    int ht;
    int bal;
} LfNode;

#define Leaf 0
#define Breanch 1

/* This should be done with a tree */

LfNode *varlist = 0;


#define printval(A) printf("%p, %i\n", (A)->ptr, (int) (A)->size)

static lfptr update(lfptr t)
{
    if (t->kind==Leaf) {
        t->ht=1;
        t->bal=0;
	t->min=t->val->ptr;
    } else {
        t->bal = t->right->ht - t->left->ht;
        /* rotate tree if out of balance
        ** (using update to recalculate values )
        */
        if (t->bal < -1) {
            if (t->left->bal>0) {
                lfptr b;
                lfptr c;
                b=t->left;
                c=b->right;
                t->left=c->right;
                b->right=c->left;
                c->left=b;
                c->right=t;
                (void) update(b);
                (void) update(t);
                t=c;
                t->bal = t->right->ht - t->left->ht;
            } else {
                lfptr b;
                b=t->left;
                t->left=b->right;
                b->right=t;
                (void) update(t);
                t=b;
                t->bal = t->right->ht - t->left->ht;
            }
        } else if (t->bal > 1) {
            if (t->right->bal<0) {
                lfptr b;
                lfptr c;
                b=t->right;
                c=b->left;
                t->right=c->left;
                b->left=c->right;
                c->right=b;
                c->left=t;
                (void) update(b);
                (void) update(t);
                t=c;
                t->bal = t->right->ht - t->left->ht;
            } else {
                lfptr b;
                b=t->right;
                t->right=b->left;
                b->left=t;
                (void) update(t);
                t=b; 
                t->bal = t->right->ht - t->left->ht;
            }
        }
        if (t->bal>0)
            t->ht=t->right->ht+1;
        else
            t->ht=t->left->ht+1;
	t->min=t->left->min;
    }
    return t;
}

#define NewNode (lfptr) malloc(sizeof(LfNode))

static void clear(lfptr t)
{
    if (!t) return;
    if (t->kind==Leaf) {
	free(t->val);
        free(t);
    } else {
        clear(t->left);
        clear(t->right);
        free(t);
    }
}

static void printnode(lfptr t)
{
    if (t->kind==Leaf)
        printval( t->val);
    else {
        printnode(t->left);
        printnode(t->right);
    }
}


static lfptr insert(lfptr t, LISTELM *val)
{
    if (!t) {
        t=NewNode;
        t->kind=Leaf;
        t->val=val;
    } else {
        if (t->kind==Breanch) {
            if (val->ptr < t->right->min)
                t->left = insert(t->left, val);
            else
                t->right = insert(t->right, val);
        } else {
            lfptr l;
            lfptr b;
            l = insert(NULL, val);
            b = NewNode;
            b->kind=Breanch;
            if (t->val->ptr <= val->ptr) {
                b->left=t;
                b->right=l;
            } else {
                b->left=l;
                b->right=t;
            }
            t=b;
        }
    }
    return update(t);
}

static lfptr delete(lfptr t, void* val)
{
    if (!t) return NULL;
    if (t->kind==Leaf) {
        if (val == t->val->ptr) {
	    free(t->val);
            free(t);
            t=NULL;
            return t;
        }
    } else {
        if (val < t->right->min) {
            t->left = delete(t->left, val);
            if (!t->left) {
                lfptr b;
                b=t;
                t=t->right;
                free(b);
            }
        } else {
            t->right = delete(t->right, val);
            if (!t->right) {
                lfptr b;
                b=t;
                t=t->left;
                free(b);
            }
        }
    }
    return update(t);
}

static LISTELM *member(lfptr t, void* val)
{
    if (!t) return 0;
    if (t->kind==Breanch)
        if (val <t->right->min)
            return member(t->left, val);
        else
            return member(t->right,val);
    else
        if (t->val->ptr == val) return t->val; else return 0;
}

static int insert_pointer(void *p, size_t size)
{
    LISTELM *lm = member(varlist, p);
    if (lm) {
	fprintf(stderr,"Pointer malloced twice.\n");
	error_func();
	return 0;
    } else {
	lm = (LISTELM*) malloc(sizeof(LISTELM));
	lm->ptr = p;
	lm->first = ((char*)p)[-1];
	lm->size = size;
	((char*)p)[size]=127;
	varlist = insert(varlist, lm);
	return 1;
    }
}

static int remove_pointer(void *p)
{
    LISTELM *lm = member(varlist, p);
    if (!lm) {
	fprintf(stderr,"Pointer not allocated or freed twice.\n");
	error_func();
	return 0;
    } else {
        int size;
	if (lm->first != ((char*)p)[-1]) {
	    fprintf(stderr,"First char wrong.\n");
	    error_func();
	    return 0;
	}
	if (((char*)p)[lm->size] != 127) {
	    fprintf(stderr,"Last char wrong.\n");
	    error_func();
	    return 0;
	}
	size=lm->size;
	varlist = delete(varlist,p);
	return sl->size;
    }
}

/* realloc_check return is it is save to realloc this pointer (1).
** If not, a malloc should be used.
*/
static int realloc_check(void *p, size_t size)
{
  LISTELM *lm = member(varlist,p);

  if (p && !lm) {
    fprintf(stderr,"Trying to realloc strange pointer.\n");
    error_func();
    return 0;
  }
  if (p) {
    int i=0;
    if (lm->first != ((char*)p)[-1]) {
      fprintf(stderr,"First char wrong in realloc.\n");
      error_func();
      i++;
    }
    if (((char*)p)[lm->size]!=127) {
      fprintf(stderr,"Last char wrong in realloc.\n");
      error_func();
      i++;
    }
    return (i==0);
  } else {
    fprintf(stderr,"Warning: reallocing NULL pointer.\n");
    error_func();
    return 0;
  }
  return 1;
}

static void uncon_remove_pointer(void *p)
{
  varlist=delete(varlist,p);
}

static void replace_pointer(void *p, size_t size)
{
  LISTELM *lm=member(varlist,p);
  if (lm) {
    ((char*)p)[size]=127;
    lm->size=size;
    lm->first=((char*)p)[-1];
  }  
}
/* number of extra bytes for checking memory boundaries */
#define ARRAYCHECK 1
/* MEM_CHECK is used in guard as "if (MEM_CHECK && ....)".
** A decent compiler should optimize it away.
*/
#define MEM_CHECK 1
#else

#define insert_pointer(P,S) 1
#define remove_pointer(P) 1
#define uncon_remove_pointer(P)
#define replace_pointer(P,SIZE) 
#define realloc_check(P,S) ((P)!=0)
#define ARRAYCHECK 0
#define MEM_CHECK 0

#endif

/* free all freelists */
static void ClearFreeLists(void);

/* call the payback functions to free memory or save something. */
static void CallPayback(void);

/* P is a pointer, FUNC is one of malloc(..), calloc(..) or realloc(..,..) */
#define TryHard(P,FUNC) do { \
        P=FUNC; \
	if (!P) { ClearFreeLists(); P=FUNC; } \
	if (!P) { CallPayback(); ClearFreeLists(); P=FUNC; } \
      } while (0)

void *mm_malloc(size_t size)
{
    void *p;
    if (size==0) { error_func(); return NULL; }
    TryHard(p,malloc(size+ARRAYCHECK));
    if (!p) {
      fprintf(stderr,"Unable to allocate %li bytes.\n", (long) size);
      error_func();
    }
    if (MEM_CHECK && p && !insert_pointer(p,size)) p = 0;
    return p;
}

void *mm_calloc(size_t nr, size_t size)
{
    void *p;
    if (size==0 || nr==0) { error_func(); return NULL; }
    TryHard(p,calloc(nr, size));
    if (!p) {
      fprintf(stderr,"Unable to allocate %li blocks of %li bytes.\n",
	      (long) nr, (long) size);
      error_func();
    }
    if (MEM_CHECK && p && !insert_pointer(p,size)) p = 0;
    return p;
}

void *mm_realloc(void *p, size_t size)
{
    void *q = 0;

    if (!size) {
      fprintf(stderr, "Trying to realloc 0 bytes.\n");
      error_func();
    }
    if (!realloc_check(p,size)) {
      TryHard(q,malloc(size+ARRAYCHECK));
      if (MEM_CHECK && q && !insert_pointer(q,size)) q=0;
    } else {
      TryHard(q,realloc(p, size+ARRAYCHECK));
      if (MEM_CHECK) {
	if (q==p) { replace_pointer(q,size); }
	else if (q) {
	  uncon_remove_pointer(p);
	  if (!insert_pointer(q,size)) q=0;
	}
      }
    }
    if (!q) {
        fprintf(stderr,"Unable to (re)allocate %li bytes.\n", (long)size);
	error_func();
	return 0;
    }
    return q;
}

void mm_free(void *p)
{
    if (p) {
      if (MEM_CHECK) {
	int size;
	size=remove_pointer(p);
	/* set it to uneven characters to cause "bus error" for many
	** pointer types.
	*/
	if (size) memset(p, ((((char *)p)[0]^0xCC)|0x1), size);
      }
      free(p);
    }
}


#define FreeListMagic 0x5482AC35

/* 2000 different freelists should be sufficient.
** * A freelist is automatically registered in a
**   FreeList_free or FreeList_fill.
**   The magic number will be set to FreeListMagic
** * The list of freelists cleared in a ClearAllLists(),
**   which frees all freelists.
**   The magic number will be set to 0.
** * A FreeList_clear will remove the freelist from the
**   list of freelists, and set the magic number to 0.
*/

#define MAXFREELIST 2000
static FreeList *alllist[MAXFREELIST];
static int maxfreelist=0;
static int fragmented=0;

typedef struct _FLItem FLItem;
struct _FLItem { FLItem *next; };

static void add_freelist(FreeList *fl)
{
  if (maxfreelist==MAXFREELIST) {
    if (fragmented) {
      /* shift all freelists down where possible */
      int i,j;
      for (i=j=0;i<maxfreelist; i++) {
	if (alllist[i]) alllist[j++]=alllist[i];
      }
      maxfreelist=j;
      fragmented=0;
    } else {
      ClearFreeLists();
    }
  }
  if (fl->size<sizeof(FLItem)) fl->size=sizeof(FLItem);
  if (fl->max<0) fl->max= (fl->size>2500 ? 4: 10000/fl->size);
  if (fl->min<0) fl->min= (fl->max/4);
  if (fl->length<0) {
    fprintf(stderr,"Strange freelist. Negative length.\n");
    error_func();
    fl->length=0;
  }
  if (fl->length==0 && fl->data) {
    fprintf(stderr,"Strange freelist. Should be empty (Memory leak).\n");
    fl->data=0;
  }
  if (!fl->data && fl->length>0) {
    fprintf(stderr, "Strange freelist. Positive length, but empty.\n");
    error_func();
    fl->length=0;
  }
  fl->regis=FreeListMagic;
  alllist[maxfreelist++]=fl;
}

static void reduce_freelist(FreeList *fl, int len)
{
  FLItem *fli;
  while (fl->length>len) {
    fli=(FLItem*)fl->data;
    fl->length--;
    fl->data=fli->next;
    mm_free(fli);
  }
}

/* add the memory of data to the freelist */
void FreeList_free(FreeList *fl, void *data)
{
  FLItem *fli;
  if (!fl) return;
  if (fl->regis!=FreeListMagic) add_freelist(fl);
  if (!data) return;
  fli=(FLItem*)data;
  fli->next=fl->data;
  fl->data=fli;
  fl->length++;
  if (fl->length>fl->max) reduce_freelist(fl,fl->min);
}

/* get one element from the freelist */
void *FreeList_malloc(FreeList *fl)
{
  FLItem *fli;
  if (!fl) return 0;
  if (!fl->length) {
    return mm_malloc(fl->size);
  }
  fli=fl->data;
  fl->data=fli->next;
  fl->length--;
  return fli;
}

/* fill the freelist with the minimal number of elements */
void FreeList_fill(FreeList *fl)
{
  /* This is a bit tricky. This freelist should not be registered,
  ** since it could be cleared due to a ClearFreeLists()
  ** Therefore,the magic number is first cleared to make sure the
  ** the list is not destroyed inbetween. Afterwards the list is
  ** added again.
  */
  FLItem *fli;
  if (!fl) return;
  if (fl->regis!=FreeListMagic) add_freelist(fl);
  fl->regis=0;
  fragmented=1;
  while (fl->length<fl->min) {
    fli=mm_malloc(fl->size);
    fli->next=fl->data;
    fl->data=fli;
    fl->length++;
  }
  add_freelist(fl);
}

/* clear the freelist */
void FreeList_clear(FreeList *fl)
{
  if (!fl) return;
  if (fl->regis!=FreeListMagic) add_freelist(fl);
  fl->regis=0;
  reduce_freelist(fl,0);
  if (fl->data) {
    fprintf(stderr, "Freelist should be empty. Memory leak.\n");
    error_func();
    fl->data=0;
  }
}

static void ClearFreeLists(void)
{
  int i;
  FreeList *fl;
  
  for (i=0; i<maxfreelist; i++) {
    fl=alllist[i];
    if (fl && fl->regis==FreeListMagic) {
      if (fl->length>0 && fl->size>0 && fl->data &&
	  fl->max>=0 && fl->min>=0) {
	reduce_freelist(fl,0);
      }
      fl->regis=0;
    }
  }
  fragmented=0;
  maxfreelist=0;
}

/* A function can be registered to be called when memory is really low
** Such a function is called when the freelists are cleared and memory
** is still not available.  Usually, this situation would lead to a
** crash or undefined behaviour.  The problem might be caused by 
** requesting a large memory block in combination with too much
** memory fragmentation.
**
** The function should try to free memory by freeing unused buffers and
** reallocing semi-used buffers. (realloc will be in a special
** malloc+free mode).
*/

typedef struct _PaybackFunc PaybackFunc;

struct _PaybackFunc {
  void (*function)(void);
  PaybackFunc *next;
};

static PaybackFunc *paybacklist=0;

void RegisterPaybackFunction(void (*function)(void))
{
  PaybackFunc *pbf;
  pbf= mm_malloc(sizeof(PaybackFunc));
  pbf->function=function;
  pbf->next=paybacklist;
  paybacklist=pbf;
}

void RemovePaybackFunction(void (*function)(void))
{
  PaybackFunc **pbf;
  pbf=&paybacklist;
  while ((*pbf) && (*pbf)->function != function) pbf=&(*pbf)->next;
  if (*pbf) {
    PaybackFunc *h;
    h=*pbf;
    *pbf = h->next;
    mm_free(h);
  }
}

static void CallPayback(void)
{
  PaybackFunc *pbf;
  pbf=paybacklist;
  while (pbf) {
    if (pbf->function) (*(pbf->function))();
    pbf=pbf->next;
  }
}
