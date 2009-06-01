/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
** Permission to use, copy, modify and distribute this software
** and its documentation for any purpose is hereby granted
** without fee, provided that the above copyright notice appear
** in all copies and that both that copyright notice and this
** permission notice appear in supporting documentation, and
** that the name of EUT not be used in advertising or publicity
** pertaining to distribution of the software without specific,
** written prior permission.  EUT makes no representations about
** the suitability of this software for any purpose. It is provided
** "as is" without express or implied warranty.
** 
** EUT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
** SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL EUT
** BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
** DAMAGES OR ANY DAMAGE WHATSOEVER RESULTING FROM
** LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
** CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
** OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
** OF THIS SOFTWARE.
** 
** 
** Roland Backhouse & Richard Verhoeven.
** Department of Mathematics and Computing Science.
** Eindhoven University of Technology.
**
********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

extern void *mymalloc(size_t size);
extern void *myrealloc(void *p, size_t size);
extern void myfree(void *p);
extern void *mycalloc(size_t nr, size_t size);

static void error_func(void)
{
    int i=10;
    /* function to break when something goes wrong */
    while (i>0) i--;
}

#ifdef DEBUG

/*
** On some systems free and malloc give different results. myfree
** and mymalloc try to simulate that behaviour in order to get
** a portable program.
**
** On a Sun, you can free almost verything and it will demage the
** memory management. A program will crash when it tries to malloc
** something and the core file is not usefull in such a situation.
** To work around this, a database of malloced/realloced pointers
** is used to monitor all the malloc, realloc and free calls.
**
** myfree:
**     * The first two bytes of memory are changed. Some free/malloc
**       implementations will use these bytes for their administation.
** mymalloc:
**     * It is not possible to malloc 0 bytes. Some implementations
**       return a NULL pointer instead of a pointer to a block of
**       0 bytes.
**
*/
#ifdef SK
extern int printf();
extern char *memset();
#endif

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
	printf("Pointer malloced twice.\n");
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
	printf("Pointer not allocated or freed twice.\n");
	error_func();
	return 0;
    } else {
	if (lm->first != ((char*)p)[-1]) {
	    printf("First char wrong.\n");
	    error_func();
	    return 0;
	}
	if (((char*)p)[lm->size] != 127) {
	    printf("Last char wrong.\n");
	    error_func();
	    return 0;
	}
	varlist = delete(varlist,p);
	return 1;
    }
}

void *mymalloc(size_t size)
{
    void *p;
    if (size==0) { error_func(); return NULL; }
    p = malloc(size+1);
    if (!p) {
      printf("Unable to allocate %li bytes.\n", (long int) size);
      error_func();
    }
    if (p && !insert_pointer(p,size)) p = 0;
    return p;
}

void *mycalloc(size_t nr, size_t size)
{
    void *p = mymalloc(size*nr);
    if (p) memset(p, 0,size*nr);
    return p;
}

void *myrealloc(void *p, size_t size)
{
    void *q = 0;
    int i=0;
    LISTELM *lm = member(varlist,p);
    if (p && !lm) {
	printf("Trying to realloc strange pointer.\n");
	error_func();
	return 0;
    }
    if (!size) {
      printf("Trying to realloc 0 bytes.\n");
      error_func();
    }
    if (p) {
	if (lm->first != ((char*)p)[-1]) {
	    printf("First char wrong in realloc.\n");
	    error_func();
	    i++;
	}
	if (((char*)p)[lm->size]!=127) {
	    printf("Last char wrong in realloc.\n");
	    error_func();
	    i++;
	}
    } else {
	printf("Warning: reallocing NULL pointer.\n");
	error_func();
	i++;
    }
    if (i) q = malloc(size+1);
    else   q = realloc(p, size+1);
    if (!q) {
        printf("Unable to (re)allocate %li bytes.\n", (long int) size);
	error_func();
	return 0;
    }
    if (q==p) {
        ((char*)q)[size] = 127;
	lm->size = size;
	lm->first = ((char*)q)[-1];
    } else if (q) {
        if (p && !i) varlist=delete(varlist,p);
	if (!insert_pointer(q,size)) q = 0;
    }
    return q;
}

void myfree(void *p)
{
    if (p) {
	if (remove_pointer(p)) {
	    ((char*)p)[0]='*';
	    ((char*)p)[1]='\0';
	    free(p);
	}
    }
}

#else

#ifdef SK
extern int printf();
#endif

void myfree(void *p)
{
    if (p) free(p);
}

void *mymalloc(size_t size)
{
    return malloc(size);
}

void *mycalloc(size_t nr, size_t size)
{
    return calloc(nr,size);
}

void *myrealloc(void *p, size_t size)
{
    if (p)
	return realloc(p,size);
    else {
	printf("Warning: reallocing NULL pointer.\n");
	error_func();
	return malloc(size);
    }
}

#endif

