#ifndef MP_SEQUENCE_H
#define MP_SEQUENCE_H

#include "types.h"
#include "expression.h"

typedef struct FuncSequence FuncSequenceRef;

#ifndef Sequence
#define Sequence FuncSequenceRef
#endif

/* To call a function. A sequence forms a graph. Each node contains a
** function and at most two outgoing arrows to other nodes.  Depending
** on a special variable, one of the two next nodes is selected.
** An if/do/while/case is created by assigning the guard to that special
** variable.
** 
** FREE_NEXT  :  needed to cleanup this structure
** FREE_ALT 
** ARGLIST    :  expression to be evaluated. It might contain function
**               or procedure calls
** NEXT       :  sequence to execute in True case
** ALTERNATIVE:  sequence to execute in False case
*/
struct FuncSequence {
    char free_next; /* needed for freeing sequences */
    char free_alt;
    Expression *arglist;   /* list of arguments */
    Sequence *next;        /* true case */
    Sequence *alternative; /* false case (if defined) */
};

/* primitive functions (one functioncall) */
/* skip function */
extern Sequence *seq_empty(void);
/* evaluate an expression */
extern Sequence *seq_expression(Expression *expr);
/* concurrent assignment   (a,b,c:=c-b,a-c,b-a */
extern Sequence *seq_assign(int nr, Argument *alist, Expression *elist);

/* combined sequences  */

extern Sequence *seq_if_statement(Expression *expr,
				  Sequence *trueps, Sequence *truepe,
				  Sequence *falseps, Sequence *falsepe,
				  Sequence **endret);
extern Sequence *seq_elseif(Sequence *ifps, Sequence *ifpe,
			    Expression *expr,
			    Sequence *trueps, Sequence *truepe,
			    Sequence **endret);
extern Sequence *seq_do_statement(Expression *expr,
				  Sequence *trueps, Sequence *truepe,
				  Sequence *falseps, Sequence *falsepe,
				  Sequence **endret);
extern Sequence *seq_elsedo(Sequence *dops, Sequence *dope,
			    Expression *expr,
			    Sequence *trueps, Sequence *truepe,
			    Sequence **endret);

extern int run_sequence(Sequence *seq);
extern int eval_sequence(Sequence *seq);
extern void free_sequence(Sequence *seq);

#endif
