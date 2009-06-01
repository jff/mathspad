/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Technical University of Eindhoven (TUE)
** 
** Permission to use, copy, modify and distribute this software
** and its documentation for any purpose is hereby granted
** without fee, provided that the above copyright notice appear
** in all copies and that both that copyright notice and this
** permission notice appear in supporting documentation, and
** that the name of TUE not be used in advertising or publicity
** pertaining to distribution of the software without specific,
** written prior permission.  TUE makes no representations about
** the suitability of this software for any purpose. It is provided
** "as is" without express or implied warranty.
** 
** TUE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
** SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL TUE
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
** Technical University of Eindhoven.
**
********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "leaftree.h"

#define False 0
#define True 1
#define Leaf 0
#define Breanch 1


#define MINSIZE (sizeof(LeafNode)-sizeof(DummyAlign))
#define LVal(A) (&(((LeafNode*)A)->val))
static int leafsize=MINSIZE;
static void (*free_leaf)(void*);
static int (*cmp_leaf)(const void*,const void*);

static int newbaltop[15] = {-4,-3,-2,-2,-2,-3,-2,-1,-1,-1,-3,-2,-1,0,0};
static int newbalright[15] = {-1,-1,-1,-2,-3,0,0,0,-1,-2,1,1,1,0,-1};

static BreanchNode* rrot(BreanchNode* t)
{
    BreanchNode* b;
    int i;
    b=t->left;
    t->left=b->right;
    b->right=t;
    i= -t->bal*5-b->bal+2;
    b->bal = -newbaltop[i];
    t->bal = -newbalright[i];
    return b;
}

static BreanchNode* lrot(BreanchNode* t)
{
    BreanchNode* b;
    int i;
    b=t->right;
    t->right=b->left;
    b->left=t;
    i=t->bal*5+b->bal+2;
    b->bal = newbaltop[i];
    t->bal = newbalright[i];
    return b;
}

static BreanchNode* rotate(BreanchNode* t)
{
    /*
    ** rotate tree if out of balance
    */
    if (t->bal < -1) {
	if (t->left->bal>0) t->left=lrot(t->left);
	t=rrot(t);
    } else if (t->bal > 1) {
	if (t->right->bal<0) t->right=rrot(t->right);
	t=lrot(t);
    }
    return t;
}

#define NewNode ((BreanchNode*) malloc(sizeof(BreanchNode)))
#define NewLeaf ((BreanchNode*) calloc(1,leafsize))
#define EmptyLeaf(A)  memset((A),0,leafsize)

static void free_breanchnode(BreanchNode* t)
{
    if (!t) return;
    if (t->kind==Leaf) {
	if (free_leaf) (*free_leaf)((void*) t);
	free(t);
    } else {
	free_breanchnode(t->left);
	free_breanchnode(t->right);
	free(t);
    }
}

static int dchange=0;
static void* rmin;
static BreanchNode* lastleaf=NULL;

static BreanchNode* insert_node(BreanchNode* t, void* val)
{
    if (!t) {
	t=NewLeaf;
	t->kind=Leaf;
	memcpy(LVal(t),val, leafsize-MINSIZE);
	lastleaf=t;
	dchange=1;
    } else if (t->kind==Breanch) {
	if ((*cmp_leaf)(val, t->val) < 0) {
	    t->left = insert_node(t->left, val);
	    if (dchange) {
		t->bal=t->bal-1;
		dchange= (t->bal == -1);
		if (t->bal < -1) t=rotate(t);
	    }
	} else {
	    t->right = insert_node(t->right, val);
	    if (dchange) {
		t->bal=t->bal+1;
		dchange = (t->bal == 1);
		if (t->bal > 1) t=rotate(t);
	    }
	}
    } else {
	int cmpres=(*cmp_leaf)(LVal(t),val);
	if (!cmpres) {
	    lastleaf=t;
	    dchange=0;
	} else {
	    BreanchNode* l = NewLeaf;
	    BreanchNode* b = NewNode;
	    l->kind=Leaf;
	    b->kind=Breanch;
	    b->bal=0;
	    lastleaf=l;
	    dchange=1;
	    memcpy(LVal(l),val, leafsize-MINSIZE);
	    if (cmpres<0) {
		b->left=t;
		b->right=l;
		b->val=LVal(l);
	    } else {
		b->left=l;
		b->right=t;
		b->val=LVal(t);
	    }
	    t=b;
	}
    }
    return t;
}

static BreanchNode* delete_node(BreanchNode* t, void* val)
{
    if (!t) return NULL;
    if (t->kind==Leaf) {
	if (!(*cmp_leaf)(val, LVal(t))) {
	    if (free_leaf) (*free_leaf)(LVal(t));
	    free(t);
	    t=NULL;
	    dchange=1;
	} else
	    dchange=0;
    } else {
	int cmpres= (*cmp_leaf)(val,t->val);
	if (cmpres<0) {
	    t->left = delete_node(t->left, val);
	    if (dchange) {
		if (!t->left) {
		    BreanchNode* b;
		    b=t;
		    t=t->right;
		    rmin=LVal(t);
		    free(b);
		} else {
		    t->bal = t->bal+1;
		    if (t->bal>1) {
			dchange = (t->right->bal!=0);
			t=rotate(t);
		    } else
			dchange = !t->bal;
		}
	    }
	} else {
	    t->right = delete_node(t->right, val);
	    if (!cmpres) t->val=rmin;
	    if (dchange) {
		if (!t->right) {
		    BreanchNode* b;
		    b=t;
		    t=t->left;
		    free(b);
		    rmin=LVal(t);
		} else {
		    t->bal = t->bal-1;
		    if (t->bal<-1) {
			dchange = (t->left->bal!=0);
			t=rotate(t);
		    } else
			dchange = !t->bal;
		}
	    }
	}
    }
    return t;
}

static BreanchNode *add_breanchnode(BreanchNode *lt, BreanchNode *ltadd)
{
    if (!ltadd) return lt;
    if (ltadd->kind==Breanch) {
	lt = add_breanchnode(lt, ltadd->left);
	return add_breanchnode(lt, ltadd->right);
    } else
	return insert_node(lt, ltadd->val);
}

static void *member_breanchnode(BreanchNode* t, void *val)
{
    if (!t) {
	lastleaf=NULL;
	return NULL;
    }
    while (t->kind==Breanch) {
	if ((*cmp_leaf)(val,t->val)<0)
	    t=t->left;
	else
	    t=t->right;
    }
    if (!(*cmp_leaf)(LVal(t),val)) {
	lastleaf=t;
	return LVal(t);
    } else {
	lastleaf=NULL;
	return 0;
    }
}

static char pline[70]=
      "                                                                  -- ";

int incorrect_tree(BreanchNode *b, int *hi)
{
    switch (b->kind) {
    case Breanch:
	{ BreanchNode *h; void *v;int res=0, hil,hir;
	  h=b->right;
	  while (h->kind==Breanch) h=h->left;
	  v=&(((LeafNode*)h)->val);
	  if (b->val!=v) {
	      printf("Invalid Reference. %p <-> %p [%i]\n", b->val,v,*((int*)v));
	      res=1;
	  }
	  res+=incorrect_tree(b->left, &hil);
	  res+=incorrect_tree(b->right, &hir);
	  if (b->bal != (hir-hil)) {
	      res++;
	      printf("Incorrect Ballance (%i <-> %i) [%i]\n", b->bal, hir-hil, *((int*)v));
	  }
	  if (hir<hil) *hi=hil+1; else *hi=hir+1;
	  return res;
      }
    default:
	*hi=1;
	return 0;
    }
}

void print_tree(BreanchNode *b, int n)
{
    if (!b) printf("Empty\n"); else
    if (b->kind==Leaf)
	printf("%s %p [%i]\n", pline+(66-2*n), b, 
	       *((int*)(&(((LeafNode*)b)->val))));
    else {
	print_tree(b->right, n+1);
	printf("%s [%p] [%i]\n", pline+(66-2*n), b->val, *((int*) b->val));
	print_tree(b->left, n+1);
    }
}



void* LT_found_leaf(void)
{
    return (void*) LVal(lastleaf);
}

void LT_free(LeafTree *lt)
{
    leafsize=MINSIZE+lt->leafsize;
    free_leaf=lt->freeleaf;
    cmp_leaf=lt->cmp;
    free_breanchnode(lt->tree);
    lt->tree=NULL;
}

void LT_insert(LeafTree *lt, void *val)
{
    leafsize=MINSIZE+lt->leafsize;
    cmp_leaf=lt->cmp;
    lt->tree=insert_node(lt->tree,val);
}

void LT_delete(LeafTree *lt, void *val)
{
    free_leaf=lt->freeleaf;
    cmp_leaf=lt->cmp;
    lt->tree=delete_node(lt->tree,val);
}

void *LT_member(LeafTree *lt, void *val)
{
    cmp_leaf=lt->cmp;
    return member_breanchnode(lt->tree,val);
}

void LT_add_leaftree(LeafTree *lt, LeafTree *ltadd)
{
    if (lt->freeleaf || ltadd->freeleaf ||
	lt->leafsize!=ltadd->leafsize ||
	lt->cmp != ltadd->cmp) return;
    cmp_leaf=lt->cmp;
    leafsize=lt->leafsize;
    lt->tree = add_breanchnode(lt->tree, ltadd->tree);
}
