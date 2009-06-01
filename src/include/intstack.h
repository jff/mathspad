#ifndef MP_INTSTACK_H
#define MP_INTSTACK_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Technical University of Eindhoven (TUE)
** 
********************************************************************/

typedef struct INTSTACK INTSTACK;
struct INTSTACK {
    INTSTACK *next;
    int nr;
};

extern int push_int(INTSTACK **s, int nr);
extern int pop_int(INTSTACK **s);
extern void remove_int(INTSTACK **s, int nr);
extern void free_int(INTSTACK *s);

#define head_int(A) ((A)? (A)->nr : 0)
#define tail_stack(A) ((A)? (A)->next : 0)
#endif
