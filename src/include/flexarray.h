#ifndef MP_FLEX_H
#define MP_FLEX_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Technical University of Eindhoven (TUE)
** 
********************************************************************/
typedef
struct { void *arr;
	 int nr;
	 int max;
	 int size;
	 int (*comp)(const void*,const void*);
     } FlexArray;

extern int  fx_contains(FlexArray *fl, void *item);
extern void fx_remove(FlexArray *fl, void *item);
extern void fx_add(FlexArray *fl, void *item);
/*extern void fx_insert(FlexArray *fl, int pos, void *item);*/
extern int  fx_switch(FlexArray *fl, void *olditem, void *newitem);
extern void fx_init(FlexArray *fl, int sz, int (*cmp)(const void*,const void*));
extern void fx_clear(FlexArray *fl);
extern int  fx_set(FlexArray *fl, int pos, void *item);
extern void *fx_copy(FlexArray *fl);

#define fx_item(A,B) (((char*)(A)->arr) + (B)*((A)->size))

#define fx_max(A) ((A)->nr)


extern int int_cmp(const void *a, const void *b);


#define int_contains(A,B) fx_contains(&(A), (void*) &(B))
#define int_remove(A,B)   fx_remove(&(A), (void*) &(B))
#define int_add(A,B)      fx_add(&(A),(void*) &(B))
/*#define int_insert(A,B,C) fx_insert(&(A),(B), (void*) &(C))*/
#define int_switch(A,B,C) fx_switch(&(A), (void*) &(B), (void*) &(C))
#define int_init(A)       fx_init(&(A), sizeof(int), int_cmp)
#define int_clear(A)      fx_clear(&(A))
#define int_set(A,B,C)    fx_set(&(A), (B), &(C))
#define int_item(A,B)     (*((int*) fx_item(&(A),B))) 
#define int_max(A)        fx_max(&(A))
#define int_copy(A)       ((int*) fx_copy(&(A)))
/* other types possible */

#endif
