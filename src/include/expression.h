#ifndef MP_EXPRESSION
#define MP_EXPRESSION

#include "types.h"
/* an operator could be an enumerated type */
/* operators are added with some datatype in mind, but can be used for
** other datatypes too.  The type checker will assure that the operators
** are used with the correct type, so overloading is possible.
**
** The parser will use labels to refer to the characters used to represent
** these operators.  Using different symbols for the same operator would
** be possible by adjusting the lexical scanner. So, a union symbol can
** be used for sets, while a + is used for integers. Internally, the
** same operator number is used, which allows the union of two
** integers to be equal to adding the two integers.
**
** Room is left open to allow adding new operators to a group.
*/
/* standard operators */
#define OPASSIGN 1
#define OPARRAY 2
/* integer related operators */
#define OPADD 11
#define OPSUB 12
#define OPMULT 13
#define OPDIV 14
#define OPREMAIN 15
/* boolean related operators */
#define OPLOGICAND 21
#define OPLOGICOR 22
#define OPLOGICXOR 23
#define OPLOGICNOT 24
#define OPEQUAL 25
#define OPNOTEQUAL 26
#define OPGREATER 27
#define OPLESS 28
#define OPLESSEQUAL 29
#define OPGREATEREQUAL 30
/* bitlevel related operators */
#define OPAND 41
#define OPOR 42
#define OPXOR 43
#define OPNOT 44
#define OPSHIFTLEFT 45
#define OPSHIFTRIGHT 46

#define Operator int

/* An Expression contains an expression in reverse polish notation
** functions are provided to calculate the result of an Expression
** An assignment is also an expression
*/

typedef struct Expression Expression;
struct Expression {
    char type;          /* operator, argument or function */
    signed char delta;  /* delta on stack (arg -> +1 | op -> 0..-127) */
    Type restype;       /* the result type of this expression */
    union {
	Operator op;
	Argument arg;
	Expression *le;
	FuncRef ifunc;
	FuncRef ufunc;
    } val;
    Expression *next;
};

typedef struct {
    Value *vlist;
    int len;
    Expression *expr;
} LazyExpression;

/* make an expression for only one argument */
extern Expression *make_expression(Argument arg);

/* combine two expression with an operator. Unary operators
** only use the first argument.
** The function automatically chooses that operator that correctly
** converts the sub expressions to the correct types.
*/
extern Expression *combine_expression(Expression *e1, Expression *e2,
				      Operator op);

/* combine NR variables and expressions into a concurrent assignment
** expression. ALIST and ELIST should each contain NR elements and
** each element from ELIST is assigned to the corresponding element
** in ALIST.
*/
extern Expression *assign_expression(int nr, Argument *alist,
				     Expression *elist);


/* combine a list of expression and a function reference into
** a new expression, which calls that function on those arguments
*/
extern Expression *func_expression(Expression *args, int num, FuncRef func,
				   int is_userfunc);

/* create a lazy expression from EXPR, such that EXPR is not evaluate
** now, but evaluated later, but in the current context (local variables)
*/
extern Expression *make_lazy_expression(Expression *expr);

/* Calculate an expression. */
extern void calculate_expression(Expression *e);

/* Calculate a lazy expression. */
extern void calculate_lazy_expression(void *e);

/* Create/return a list of value references from the last NR elements
** on the expression stack
extern Value **expr_list(Expression *expr, int nr);
*/

/* Clear the stack used to calculate an expression */
extern void clear_expr_stack(void);


/* return if last value on expression stack is non-zero. */

extern int non_zero_on_stack(void);

/* returns a list of result types of an expression */
extern Type *type_of_expression(Expression *e);

/* return number of values on stack after expression is evaluated */
extern int left_on_stack(Expression *e);

/* free an expression */
extern void free_expression(Expression *e);

/* define an operator for a given type */
extern void define_operator(Operator op, FuncRef function);

extern void push_eval_stack(void);
extern void pop_eval_stack(void);

#endif
