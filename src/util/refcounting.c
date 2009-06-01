#include <stdlib.h>
#include <stdio.h>

#include "memman.h"

typedef struct RefElem RefElem;

struct RefElem {
  void *pointer;
  void (*freefunc)(void*);
  int count;
  RefElem *next;     /* linked list */
};

/* Pointers are usually aligned at 8 or 16 byte positions. As a
** result, the lower 3 or 4 bits are not useful. As pointers
** are usually allocated in a sequential order, the higher bits
** can be used instead of the lower bits.
** The hashsize is a power of two, as it simplifies calculations.
**
*/

#define HASHSIZE (1<<12)
#define HASHVALUE(P)   (((((int)(P))>>12)^((int)(P)))&(HASHSIZE-1))

static RefElem *hash_table[HASHSIZE];

static FreeList refelems= FreeListWithBlockSize(sizeof(RefElem));

#ifdef DEBUG
#define LOG_MESSAGE(S,I)  fprintf(stderr, S,I)
#else
#define LOG_MESSAGE(S,I)
#endif

void increase_refcount(void *pointer, void (*freefunc)(void*))
{
  int i=HASHVALUE(pointer);
  RefElem *re= hash_table[i];
  if (!pointer) return;
  while (re && re->pointer!=pointer) re=re->next;
  if (re) {
    re->count++;
    LOG_MESSAGE("setting counter to %i\n", re->count);
  } else {
    LOG_MESSAGE("new pointer %x\n", pointer);
    re=FreeList_malloc(&refelems);
    re->next=hash_table[i];
    re->pointer=pointer;
    re->count=1;
    re->freefunc=freefunc;
    hash_table[i]=re;
  }
}

static void refcount_break(int i)
{
  while (i>0) { i=i>>1; }
}

void decrease_refcount(void *pointer)
{
  int i=HASHVALUE(pointer);
  RefElem **re=&(hash_table[i]);
  
  if (!pointer) return;
  while (*re && (*re)->pointer!=pointer)
     re=&(*re)->next;
  if (*re) {
    (*re)->count--;
    LOG_MESSAGE("setting counter to %i\n", (*re)->count);
    if (!(*re)->count) {
      RefElem *h;
      LOG_MESSAGE("counter at 0, freeing %x\n", pointer);
      h=*re;
      if (h->freefunc) {
	(*h->freefunc)(pointer);
      }
      *re = h->next;
      FreeList_free(&refelems, h);
    }
  } else {
    LOG_MESSAGE("No reference counting on supplied pointer %x\n",pointer);
    refcount_break(16);
  }
}


