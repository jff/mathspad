
#include <stdlib.h>
#include <stdio.h>
#include "fifolist.h"
#include "memman.h"


typedef struct _ObjectList ObjectList;
struct _ObjectList {
  void *object;
  ObjectList *next;
};

FreeList freeobjects = FreeListWithBlockSize(sizeof(ObjectList));

#define NewObjectList() FreeList_malloc(&freeobjects)
#define DeleteObjectList(P)  FreeList_free(&freeobjects, (P))


/* other definition of FIFOList, matching FifoList, with typed first
** and last fields.  Used to reduce the typecasts.
*/

typedef struct _FIFOList FIFOList;
struct _FIFOList {
  ObjectList *first;
  ObjectList *last;
  int length;
};

FifoList *FifoMake(void)
{
  FIFOList *fl;
  
  fl=malloc(sizeof(FIFOList));
  if (!fl) return (FifoList*)fl;
  fl->first=0;
  fl->last=0;
  fl->length=0;
  return (FifoList*)fl;
}

void FifoPush(FifoList *flist, void *object)
{
  FIFOList *fl = (FIFOList *)flist;
  ObjectList *obl;
  if (!fl) return;
  obl=NewObjectList();
  if (!obl) return;
  obl->next=0;
  obl->object=object;
  if (!fl->length) {
    fl->first=fl->last= obl;
  } else {
    fl->last->next=obl;
    fl->last=obl;
  }
  fl->length++;
}

void *FifoPop(FifoList *flist)
{
  FIFOList *fl = (FIFOList*) flist;
  void *res;
  ObjectList *obl;

  if (!fl || !fl->length) return 0;
  fl->length--;
  if (!fl->first) {
    fprintf(stderr, "Bad FIFO stack. Should contain elements\n");
    return 0;
  }
  res=fl->first->object;
  obl=fl->first;
  fl->first=obl->next;
  DeleteObjectList(obl);
  if (!fl->length) {
    if (fl->first) {
      fprintf(stderr, "Bad FIFO stack. Should be empty (memory leaked)\n");
      fl->first=0;
    }
    fl->last=fl->first;
  }
  return res;
}

void FifoClear(FifoList *flist, void (*destfunc)(void *obj))
{
  FIFOList *fl = (FIFOList*) flist;
  ObjectList *obl;
  
  while (fl->length) {
    fl->length--;
    if (!fl->first) {
      fprintf(stderr, "Bad FIFO stack. Should contain elements.\n");
      continue;
    }
    obl=fl->first;
    fl->first=obl->next;
    if (destfunc) {
      (*destfunc)(obl->object);
    }
    DeleteObjectList(obl);
  }
  fl->length=0;
  if (fl->first) {
    /* error and memory leak, should not occure  */
    fprintf(stderr, "Bad Fifo Stack. Should be empty (memory leaked)\n");
    fl->first=0;
  }
  fl->last=fl->first;
}

void FifoDestroy(FifoList *flist, void (*destfunc)(void*))
{
  FifoClear(flist,destfunc);
  free(flist);
}
