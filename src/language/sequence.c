
#include <stdlib.h>
#include <stdio.h>
#include "sequence.h"
#include "prototype.h"
#include "function.h"
#include "userdef.h"
#include "variable.h"

#define INTERNALFUNC 0
#define USERFUNC 1


void free_sequence(Sequence *s)
{
    if (s->arglist) free_expression(s->arglist);
    if (s->free_next && s->next) { free_sequence(s->next);}
    if (s->free_alt && s->alternative) free_sequence(s->alternative);
}

/* primitive functions (one functioncall) */
/* skip function */
Sequence *seq_empty(void)
{
    Sequence *res;
    res = (Sequence *) malloc(sizeof(Sequence));
    if (res) {
	res->free_next=1;
        res->free_alt=1;
        res->arglist=0;
        res->next=res->alternative=0;
    }
    return res;
}

Sequence *seq_expression(Expression *expr)
{
    Sequence *res;
    res=seq_empty();
    if (res) {
	res->arglist=expr;
    }
    return res;
}

Sequence *seq_assign(int nr, Argument *alist, Expression *elist)
{
    return seq_expression(assign_expression(nr, alist, elist));
}

/* Construct a sequence that checks if an expression is true.
** It assigns the value of the expression to an internal variable
** which is used by the interpreter.
*/
static Sequence *make_ifcheck(Expression *expr)
{
    /* ifcheck uses the eval function to evaluate the expression
    ** and leave the result on the stack */
    Sequence *res;
    if (expr && left_on_stack(expr) != 1) {
      /* This is a strange case. Multiple values or no values left on
      ** the stack, which might result in undefined behaviour
      */
      fprintf(stderr, "Warning: incorrect expression in guard.\n");
    }
    res = seq_empty();
    if (res) {
        res->arglist=expr;
    }
    return res;
}

/* combined sequences  */

/* if statement
** Info:
**    if-statement of the form:  if (E) then ST else SF fi
**    The sequences ST and SF each have one termination node where the
**    execution of ST or SF ends. These nodes are connected to the
**    termination node of the if statement. The expression E is evalutated
**    and determines which sequence (ST or SF) is taken.
**    if ST or SF is not defined, the arguments need to be zero
** Argument:
**    EXPR     expression which evaluates the guard  (E)
**    TRUEPS   start of the True sequence  (ST start)
**    TRUEPE   end of the True sequence    (ST end)
**    FALSEPS  start of the False sequence (SF start)
**    FALSEPE  end of the False sequence   (SF end)
**    ENDRET   returns the end of the if-statement sequence
** Returns:
**    start of the if-statement sequence
*/
Sequence *seq_if_statement(Expression *expr,
			   Sequence *trueps, Sequence *truepe,
			   Sequence *falseps, Sequence *falsepe,
			   Sequence **endret)
{
    Sequence *start,*end;
    start = make_ifcheck(expr);
    end = seq_empty();
    if (trueps) {
	start->next=trueps;
	truepe->next=end;
    } else {
	start->next=end;
    }
    if (falseps) {
	start->alternative=falseps;
	falsepe->next=end;
	falsepe->free_next=0;
    } else {
	start->alternative=end;
	start->free_alt=0;
    }
    *endret=end;
    return start;
}

/* elseif statement
** Info:
**    elseif-statement of the form:  S elseif (E) then SD
**    In other words: it is an if statement with a dangling else.
**    The sequences S and SD each have one termination node where the
**    execution of S or SD ends. These nodes are connected to the
**    termination node of the elseif statement. The expression E is evalutated
**    and determines if the SD needs to be executed.
**    The sequence S needs to be the result of an elseif-statement.
**    If E is not defined, the result will be "S elseif (True) then SD".
**    If SD is not defined, then the result will be "S elseif (E) then skip".
**    If S is not defined, then the result will just be "if (E) then SD".
** Argument:
**    IFPS     start of the elseif sequence  (S start)
**    IFPE     end of the elseif sequence    (S end)
**    EXPR     expression which evaluates the guard  (E)
**    TRUEPS   start of the True sequence    (SD start)
**    TRUEPE   end of the True sequence      (SD end)
**    ENDRET   returns the end of the elseif-statement sequence
** Returns:
**    start of the elseif-statement sequence
*/
Sequence *seq_elseif(Sequence *ifps, Sequence *ifpe,
		     Expression *expr,
		     Sequence *trueps, Sequence *truepe,
		     Sequence **endret)
{
    Sequence *start,*end;
    Sequence *ifchk=0;
    end = ifpe;
    if (!end) end = seq_empty();
    *endret = end;
    if (truepe) {
      truepe->next = end;
      truepe->free_next = (!ifpe);
    }
    if (expr) ifchk = make_ifcheck(expr);
    if (ifchk) {
      if (trueps) {
	ifchk->next=trueps;
	ifchk->free_next=1;
      } else {
	ifchk->next=end;
	ifchk->free_next = (!ifpe);
      }
      ifchk->alternative=end;
      ifchk->free_alt=0;
    } else {
      ifchk=trueps;
    }
    if (ifps) {
      start = ifps;
      if (ifchk) {
	Sequence *dangelse;
	dangelse = start;
	while (dangelse && dangelse->alternative!=ifpe)
	  dangelse=dangelse->alternative;
	if (dangelse) {
	  dangelse->alternative=ifchk;
	  dangelse->free_alt=1;
	}
      }
    } else {
	start = ifchk;
    }
    return start;
}


/* do statement
** Info:
**    do-statement of the form:  do (E) then ST elsedo SF od
**    The sequences ST and SF each have one termination node where the
**    execution of ST or SF ends. These nodes are connected to the
**    termination node of the do statement. The expression E is evalutated
**    and determines which sequence (ST or SF) is executed. After execution
**    the expression is evaluated again.
**    If SF is not defined, the result will be "do (E) then ST od"
**    If E is not defined, the result will be "do (True) then ST elsedo SF od"
** Argument:
**    EXPR     expression which evaluates the guard  (E)
**    TRUEPS   start of the True sequence   (ST start)
**    TRUEPE   end of the True sequence     (ST end)
**    FALSEPS  start of the elsedo sequence (SF start)
**    FALSEPE  end of the elsedo sequence   (SF end)
**    ENDRET   returns the end of the do-statement sequence
** Returns:
**    start of the do-statement sequence
*/
Sequence *seq_do_statement(Expression *expr,
			   Sequence *trueps, Sequence *truepe,
			   Sequence *falseps, Sequence *falsepe,
			   Sequence **endret)
{
    Sequence *start, *end;
    Sequence *ifchk;
    start=ifchk=make_ifcheck(expr);
    end = seq_empty();
    if (trueps) {
	start->next=trueps;
	truepe->next=start;
	truepe->free_next=0;
    } else {
	/* Possible infinite loop:
	**  * guard true
	**  * no sequence
	** Protection by creating a fall-throught.
	** If multiple threats are possible, blocking this one would be
	** an option. 
	*/
	if (falseps) start->next=falseps;
	else start->next=end;
	start->free_next=0;
    }
    if (falseps) {
	Sequence *h;
	start->alternative=falseps;
	falsepe->next = start;
	falsepe->free_next = 0;
	h=falseps;
	while (h->alternative != falsepe) {
	    if (h->next == falsepe) {
		/* infinite loop */
		h->next = h->alternative;
		h->free_next=0;
	    }
	    h=h->alternative;
	}
	h->alternative=end;
	h->free_alt=1;
    } else {
	start->alternative=end;
	start->free_alt=1;
    }
    *endret=end;
    return start;
}

/* else-do statement.
** Info:
**    else-do-statement of the form:  S elsedo (E) then SD
**    (the extra part of the do statement from GCL)
**    do  B1 -> S1  []  B2 -> S2  []  ...  []  Bn -> Sn  od
**                  -----------------------------------
**    It just calls the seq_elseif function to combine the sequences.
*/
Sequence *seq_elsedo(Sequence *dops, Sequence *dope,
		     Expression *expr,
		     Sequence *trueps, Sequence *truepe,
		     Sequence **endret)
{
    return seq_elseif(dops, dope, expr, trueps, truepe, endret);
}

static int interupted=0;

static int use_alternative(void)
{
    return !non_zero_on_stack();
}

static void clear_use_alternative(void)
{
    clear_expr_stack();
}

static void print_sequence(Sequence *seq, int indent)
{
  if (seq) {
    printf("%*s%p (Expr:%p) (Next: %p, Alt: %p)\n", indent, "", seq,
	   seq->arglist, seq->next, seq->alternative);
    if (seq->free_next) print_sequence(seq->next, indent+3);
    if (seq->free_alt) print_sequence(seq->alternative, indent+3);
  }
}

int run_sequence(Sequence *seq)
{
    while (seq && !interupted) {
	calculate_expression(seq->arglist);
	if (seq->alternative && use_alternative()) {
	    clear_use_alternative();
	    seq=seq->alternative;
	} else {
	    seq=seq->next;
	}
	clear_expr_stack();
    }
    return interupted;
}

int eval_sequence(Sequence *seq)
{
    int i;
    push_eval_stack();
    i=run_sequence(seq);
    clear_expr_stack();
    pop_eval_stack();
    return i;
}
