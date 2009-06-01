#ifndef MP_PROTOTYPE_H
#define MP_PROTOTYPE_H

#include "types.h"
/* Prototypes are needed for functions. Prototypes are shared between
** functions and some prototypes are predefined for adding internal
** functions at startup.
** The caller member receives a pointer to a function and a list of
** pointers to values of the required type. (*func)(), void**.
*/
typedef struct {
    Type *type;
    int n;
    Type restype;
    int max;
    int (*caller)();  /* function to call a function for that prototype */
} Prototype;

/* maximum number of types in a prototype = maximum number of argument
** to a function
*/
#define MAX_PROTOTYPE 256

/*  Type of Nth argument in referenced prototype PR */
#define TypeNeeded(PR,N) (((N)<0||(N)>=(PR)->n) ? 0 : (PR)->type[N])
#define ProtoCaller(PR)  ((PR)->caller)
#define ProtoArgs(PR)    ((PR)->n)
#define ProtoResult(PR)  ((PR)->restype)

/* Create or lookup a prototype from a list of types.
** The list typelist is copied, not referenced.
*/
extern Prototype *define_prototype(const Type *typelist, const int n,
				   const Type restype, int (*caller)());

/* Create or lookup a prototype from a list of values
** Works like define_prototype, except that it extracts the types
** from the values.
*/
extern Prototype *make_value_prototype(const Value *valuelist, const int n,
				       Type restype, int (*caller)());

/* add a complete prototype to a database
** returns the entry in the database.
*/
extern  Prototype *proto_add_to_database(Prototype *p);

/* create an empty prototype which can be expanded */
extern Prototype *create_prototype(int (*caller)());

/* add a new type to a prototype 
** returns zero on success, non-zero on failure
*/
extern int add_type(Prototype *prototype, const Type type);

/* add the result type to a prototype */
extern int add_result_type(Prototype *prototype, const Type type);

/* destroy a prototype */
extern void destroy_prototype(Prototype *p);

/* predefined prototypes for adding internal functions */
extern Prototype protopredef[];
#define ProtoEmpty (&(protopredef[0]))
#define ProtoInt   (&(protopredef[1]))
#define ProtoStr   (&(protopredef[2]))

/* check if a list of types matches a prototype. */
extern int matches_prototype(Type *typelist, Prototype *p, int nr);

/* initialisation function for object file
*/
extern void protoinit(void);


#endif
