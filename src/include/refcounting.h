#ifndef MP_REFCOUNTING_H

#define MP_REFCOUNTING_H


void increase_refcount(void *pointer, void (*freefunc)(void*));
void decrease_refcount(void *pointer);

#endif
