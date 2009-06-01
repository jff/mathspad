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
// mark.cc

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include "mathpad.h"

extern Bool interrupted(void);
}

#define INTERRUPT_COUNT 1000

#include "mathpad.hh"
#include "mark.hh"
#include "marker.hh"
#include "node.hh"

#include "mathpad.icc"

const Mark& Mark::base() const
{
    const Mark* pm = this;
    while (!!pm->above()) pm = &pm->above();
    return *pm;
}

Bool operator == (const Mark& m1, const Mark& m2)
{
    return m1.node == m2.node && m1.pos == m2.pos;
}

Bool operator != (const Mark& m1, const Mark& m2)
{
    return m1.node != m2.node || m1.pos != m2.pos;
}

Mark::Mark()
{
    node=Empty;
}

Mark::Mark(const Mark& m)
{
    node=m.node;
    pos=m.pos;
}

Mark::Mark(Node* pn, Index i)
{
    node=pn;
    pos=i;
}

Mark::~Mark() {}

Char Mark::traverse()
{
    Char c = (*node)(pos);
    switch (Char2Node(c)) {
    case 0:
        if (!!node->above()) {
	    *this = node->above();
	    c = (*node)[pos];
	    pos += 1;
	}
        break;
    case NodeCode:
	*this = node->under(pos);
	pos = 0;
        break;
    default:
        pos += 1;
        break;
    }
    return c;
}

Char Mark::esrevart()
{
    Char c = (*node)(pos - 1);
    switch (Char2Node(c)) {
    case 0:
        if (!!node->above()) {
            *this = node->above();
            c = (*node)[pos];
        }
        break;
    case NodeCode:
        {
            Node* nu = node->under(pos - 1);
            if (nu) {
                *this = nu;
                pos = nu->size();
            } else {
                pos -= 1;
            }
        }
        break;
    default:
        pos -= 1;
    }
    return c;
}

Bool Mark::find(Char *str, int n)
{
    Bool found;
    Char c = 1;
    Index ir = INTERRUPT_COUNT;
    Bool irt = MP_False;
    found = node->matches(str, n, pos);
    while (!found && c && !irt) {
        c = traverse();
        found = node->matches(str,n,pos);
	ir--;
	if (!ir) {
	    irt = 0;  /*interrupted(); */
	    ir = INTERRUPT_COUNT;
	}
    }
    if (!found) {
        *this = Empty;
    }
    return irt;
}

Bool Mark::findback(Char *str, int n)
{
    Bool found;
    Char c = 1;
    Index ir = INTERRUPT_COUNT;
    Bool irt = MP_False;
    found = node->matches(str, n, pos);
    while (!found && c && !irt) {
        c = esrevart();
        found = node->matches(str,n,pos);
	ir--;
	if (!ir) {
	    irt = 0; /* interrupted(); */
	    ir = INTERRUPT_COUNT;
	}
    }
    if (!found) {
        *this = Empty;
    }
    return irt;
}

Bool Mark::find_stencil(Index nnr)
{
    Bool found;
    Index ir = INTERRUPT_COUNT;
    Node *pn = node;
    Bool irt = MP_False;
    if (pn->kind()==MP_Text) pn = pn->right_of(pos);
    found = pn && pn!=node && pn->is_stencil() && pn->stencil_nr() == nnr;
    while (!found && pn && !irt) {
	if (pn->first())
	    pn = pn->first();
	else if (pn->right())
	    pn = pn->right();
	else {
	    while (pn && !pn->right())
		pn = pn->above();
	    if (pn) pn = pn->right();
	}
	if (pn)
	    found = pn->is_stencil() && pn->stencil_nr()==nnr;
	ir--;
	if (!ir) {
	    irt = 0; /* interrupted(); */
	    ir = INTERRUPT_COUNT;
	}
    }
    if (!found)
        *this = Empty;
    else
        *this = pn;
    pos=0;
    return irt; 
}

Bool Mark::findback_stencil(Index nnr)
{
    Bool found;
    Index ir = INTERRUPT_COUNT;
    Node *pn = node;
    Bool irt = MP_False;
    if (pn->kind()==MP_Text) pn = pn->left_of(pos);
    found = pn && pn!=node && pn->is_stencil() && pn->stencil_nr() == nnr;
    while (!found && pn && !irt) {
	if (pn->left()) {
	    pn = pn->left();
	    while (pn->last()) pn = pn->last();
	} else
	    pn = pn->above();
	found = pn->is_stencil() && pn->stencil_nr() == nnr;
	ir--;
	if (!ir) {
	    irt = 0; /* interrupted(); */
	    ir = INTERRUPT_COUNT;
	}
    }
    if (!found) {
        *this = Empty;
    } else
	*this = pn;
    return irt;
}

Bool Mark::find_tree(Node *n)
{
    Bool found;
    Char c = 1;
    Index ir = INTERRUPT_COUNT;
    Bool irt = MP_False;
    Index ne = n->number_empty();
    Node** nlist = (Node**) malloc(ne*sizeof(Node*));
    if (ne!=0 && !nlist) { *this = Empty; return MP_False; }
    Char* tlist = (Char*) malloc(ne*sizeof(Char));
    if (ne!=0 && !tlist) { free((char*)nlist); *this = Empty; return MP_False; }
    found = node->match_tree(n, pos,nlist, tlist);
    while (!found && c && !irt) {
        c = traverse();
	if (!node->is_stencil() || !pos)
	    found = node->match_tree(n, pos, nlist, tlist);
	ir--;
	if (!ir) {
	    ir=INTERRUPT_COUNT;
	    irt = 0; /* interrupted(); */
	}
    }
    if (!found) {
        *this = Empty;
    }
    free((char*)nlist);
    free((char*)tlist);
    return irt;
}

void Mark::cond_right(int n)
{
    while (n && (*node)(pos) && !IsPh((*node)(pos))) {
	n--;
	pos++;
	if (pos == node->size()) n=0;
    }
    if (n)
	traverse();
}

void Mark::cond_left(int n)
{
    while (n && pos && !IsPh((*node)(pos))) {
	n--;
	pos--;
    }
    if (n)
	esrevart();
}

void Mark::nextline()
{
    Char c;
    do {
	c = traverse();
    } while (c && !IsNewline(c));
}

void Mark::prevline()
{
    Char c;
    do {
	c = esrevart();
    } while (c && !IsNewline(c));
}

void Mark::close_parens()
{
    Node *pn = node;
    Index i;

    if (pn->text()) return;
    if (pn->id()) {
        Index j = 1, k = pn->size(), l = 1;
	i=pos;
	if (i>k) i=k; 
	while (j && i) {
	    i--;
	    if ((*pn)[i] == ')') j++;
	    else if ((*pn)[i] == '(') j--;
	}
	while (l && k) {
	    k--;
	    if ((*pn)[k] == ')') l++;
	    else if ((*pn)[k] == '(') l--;
	}
	if (!j && !l) return;
    }
    i = pos;
    while ((IsDispOrExpr(pn->kind()) || pn->id()) &&
	   (!pn->parens() || !pn->id())) {
	if (pn->parens()) {
	    pn->set_parens(MP_False);
	    if (!pn->parens()) pn->set_parens(MP_True);
	    *this = pn;
	    pos = pn->size();
	    return;
	}
	i = pn->above().pos;
	pn = pn->above();
    }
    *this = pn;
    pos = i;
}

