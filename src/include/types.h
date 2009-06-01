#ifndef MP_TYPES_H
#define MP_TYPES_H

typedef unsigned int Type;

/* some predefined types */
#define  IntType 1
#define  StringType 2
#define  RealType 3
#define  LazyEvalType 4
#define  FirstFreeType 5

#define ToRefType(A) ((A)|0x8000)
#define NoRefType(A) ((A)&0x7fff)
#define BaseType(A)  ((A)&0x1fff)
#define ToConstType(A) ((A)|0x4000)
#define ToArrayType(A) ((A)|0x2000)
#define TypeMatch(a,b)  ((!(((a)^(b))&0x1FFF)) && (((a)^(b))!=0xC000))

#define IsRef(a) (((a)&0x8000)>0)
#define IsConst(a)   (((a)&0x4000)>0)
#define IsArray(a)   (((a)&0x2000)>0)


/* one argument/variable */
/* The members of the union val are not used, but are needed to get correct
** alignment restrictions. (They are handy for debugging purposes)
** The addresses of all the members are equal to the address of val itself
** and the address of val is used to pass the actual values or their
** reference.
*/
typedef union {
  int ival;      /* for scalar types    */
  double rval;   /* for float  types      */
  void *stval;   /* for all other types */
  int *iref;     /* for referenced scalar types */
  double *rref;  /* for referenced float types  */
  void **stref;  /* for referenced other types  */
} TVal;

typedef struct {
    Type type;
    TVal val;
} Value;

/* values are automatically converted between the base type and their
** reference type, depending on the prototype of a function.
** This conversion is done in the function call_function in function.c.
*/


/* define a new type
** The functions are used to construct or destroy objects and have to
** keep track of clearing memory.  The copy function creates a new
** object by copying an existing object and might use reference counters
** to increase sharing and performance.
** If reference counters are used, each functions which changes an object,
** should make an non-shared copy of it and adjust the counters.  Furthermore,
** the destruct function should only destruct the object if it is not
** shared anymore.
*/

/* add a new type to a type database */
extern Type define_type(char *name, int size,
			void (*construct)(void**),
			void (*destruct)(void*),
			void (*copy)(void **, void*));
/* lookup a type in the type database */
extern Type lookup_type(char *name);
extern char *lookup_typename(Type t);

/*
** An Argument is a reference to a Value, which is part of a list
** There are three lists when a function is executed:
**
** List         Argument      Purpose
**              value
** call_stack   -1..-1000     (arguments to user-defined functions)
** local_var    -1000..       (local variables)
** global_var   1..           (global variables and constants)
**
** These list are internal to the variable module.
*/

typedef int Argument;

/*
** Functions are passed using pointers.
** Using void* hides the implementation details and
** makes modules less dependable on eachother.
*/

typedef struct {
    char *name;
    char *description;
    char nr_args;    /* number of arguments */
    char functype;   /* internal or user defined */
    Type rettype;    /* return type */
    void *prototype; /* function prototype */
    /* additional information depend on functype */
} FuncDef;

typedef FuncDef *FuncRef;

extern void ConstructValue(Value* v);
extern void CopyValue(Value* dest, Value* orig);
extern void DestructValue(Value* v);
#endif
