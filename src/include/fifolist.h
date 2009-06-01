#ifndef MP_FIFOLIST_H
#define MP_FIFOLIST_H

typedef struct _FifoList FifoList;

struct _FifoList {
  void *first;
  void *last;
  int length;
};

#define EmptyFifoList {0,0,0}

/* make a new fifo list */
extern FifoList *FifoMake(void);

/* push an item on the list */
extern void FifoPush(FifoList *flist, void *object);

/* get an item from the list */
extern void *FifoPop(FifoList *flist);

/* get the length of the list */
#define FifoLength(FL) ((FL)->length)

/* Clear the list and call the destroy function
** for each object in the list.
*/
extern void FifoClear(FifoList *flist, void (*destfunc)(void *obj));


/* Destroy the list. That is clear it if needed and free the list itself.
*/
extern void FifoDestroy(FifoList *flist, void (*destfunc)(void *obj));


#endif
