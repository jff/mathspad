#ifndef MP_LEAFTREE_H
#define MP_LEAFTREE_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Technical University of Eindhoven (TUE)
** 
********************************************************************/

/*
** leaftree.h
**
** The leaftree represents a set of values.  Only the leafs contain valid
** information and the tree structure is used to find these values.
** The tree is height balanced and the values are sorted according to a
** supplied compare function.
**
** A leaftree uses 3 different types, one for the leafs, one for the
** breanches and one to store the leafsize, compare function, free function
** and rootnode. The type definition for the nodes is:
*/
typedef struct bnode BreanchNode;

struct bnode {
    char kind;       /* 0=leaf, 1=breanch */
    signed char bal; /* balance count */
    void *val;       /* points to a value in a leaf */
    BreanchNode *left;
    BreanchNode *right;
};

/*
** The type of a leaf is general and dynamically allocated.
** It looks like:
*/
typedef union { double al1; long al2; void *al3; int al4; } DummyAlign;

typedef struct lnode {
    char kind;
    DummyAlign val;
} LeafNode;

/* 
** The size of LeafNode will depend on the size of the information needed
** in a leaf.  The adres of LeafNode.val will be used as return value or
** argument for the supplied compare function.  The union structure is used
** to make sure the alignment is correct.
**
** The LeafTree consists of a node, two functions and a size. The definition
** is:
*/

typedef struct ltree {
    BreanchNode *tree;
    int (*cmp)(const void*,const void*);
    void (*freeleaf)(void*);
    int leafsize;
} LeafTree;

/*
** The functions that manipulate leaftrees only use the size of this
** leaf type and the fact that the first couple of bytes contain these
** fields.
*/


extern void LT_free(LeafTree * lt);
extern void LT_insert(LeafTree *lt, void *val);
extern void LT_delete(LeafTree *lt, void *val);
extern void LT_add_leaftree(LeafTree *lt, LeafTree *ltadd);
extern void *LT_member(LeafTree *lt, void *val);
extern void *LT_found_leaf(void);
#endif
