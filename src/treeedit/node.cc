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
// node.cc
// malloc, realloc, and free are used in Node::set_gap and Node::~Node

extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "memman.h"
#include "mathpad.h"
#include "flexarray.h"
#include "notatype.h"
#include "output.h"
#include "latexout.h"
#include "message.h"
#include "fileread.h"

#include "unistring.h"

}
#include <string.h>
#include "mathpad.hh"
#include "mark.hh"
#include "marker.hh"
#include "node.hh"
#include "mathpad.icc"

Bool text_only=MP_False;
Bool all_parens=MP_False;
// Helper functions for saves

Char fgetChar(FILE* f)
{
    int ch1 = fgetc(f);
    int ch0 = fgetc(f);
    return (Char)(ch0 | (ch1 << 8));
}

Char fgetRune(FILE* f)
{
    Char ch = (Char)fgetc(f);
    if (ch == '\n') return Newline;
    if ((ch &0x80) == 0) return ch;
    ch = (Char)((ch << 6) | (fgetc(f) & 0x3F));
    if ((ch & 0x2800) == 0x2000) return (Char)(ch & 0x07FF);
    ch = (Char)((ch << 6) | (fgetc(f) & 0x3F));
    return ch;
}

Index fgetIndex(FILE* f)
{
    int i3 = fgetc(f);
    int i2 = fgetc(f);
    int i1 = fgetc(f);
    int i0 = fgetc(f);
    return i0 | (i1 << 8) | (i2 << 16) | (i3 << 24);
}

Bool fgetBool(FILE* f)
{
    return fgetc(f);
}


static void membwd(Char* d, Char* s, Char* e)
{
    while (s != e) {
	*d++ = *s++;
    }
}

static void memfwd(Char* d, Char* s, Char* e)
{
    while (s != e) {
	*--d = *--s;
    }
}

const Mark& Node::above()
{
    if (!!father && father->stencil)
	father.pos=stencil_position(father->innr, _kind);
    return father;
}

Node * Node::right() const
{
    if (!!father && father->stencil) {
	Offset i=stencil_position_right(father->innr, _kind);
	return (i<0?Empty:father->under((Index)i));
    } else
	return _right;
}

Node * Node::left() const
{
    if (!!father && father->stencil) {
	Offset i = stencil_position_left(father->innr,_kind);
	return (i<0?Empty:father->under((Index)i));
    } else
	return _left;
}

Node * Node::first()
{
    if (stencil) {
	Offset i = stencil_position_first(innr);
	return (i<0?Empty:under((Index) i));
    } else
	return _first;
}

Node * Node::last()
{
    if (stencil) {
	Offset i=stencil_position_last(innr);
	return (i<0?Empty:under((Index)i));
    } else
	return _last;
}

Char Node::operator () (Index pos) const
{
    return (pos < size()) ? (*this)[pos] : (Char)0;
}

// Compare the text of a node with a string.
Bool Node::operator == (const Char* s) const
{
    Index i = 0;
    if (stencil) return MP_False;
    while (i != size1) {
	if (p1[i] != s[i]) return MP_False;
	i += 1;
    }
    while (i != size1+size2) {
	if (p2[i] != s[i]) return MP_False;
	i += 1;
    }
    return s[i] == 0;
}

// construct a Node out of nothing, with *pn as an example
Node::Node(const Node* pn) : father(Tree)
{
    father.pos = 0;
    _left = _right = _first = _last = Empty;
    p1 = p2 = 0;
    size1 = size2 = gap = 0;
    list = 0;
    changed = MP_True;
    stencil = MP_False;
    innr = 0;
    _kind = pn->_kind;
    _parens = pn->_parens;
    _findnr = pn->_findnr;
    _opdelta = pn->_opdelta;
    _display_pos = pn->_display_pos;
    _lines = 0;
}

// construct a node of kind nk, precedence pr
Node::Node(Char nk) : father(Tree)
{
    father.pos = 0;
    _left = _right = _first = _last = Empty;
    p1 = p2 = 0;
    size1 = size2 = gap = 0;
    list = 0;
    changed = MP_True;
    _kind = nk;
    _parens = MP_False;
    stencil = MP_False;
    innr = 0;
    _findnr = 0;
    _opdelta = 0;
    _display_pos = 0;
    _lines = 0;
}

// remove the node from the tree
void Node::unlink()
{
    if (father) {
	if (_left)  _left->_right = _right; else father->_first = _right;
	if (_right) _right->_left = _left;  else father->_last  = _left;
	father->changed = MP_True;
    }
    father = _left = _right = Empty;
}

// destruct a node
Node::~Node()
{
    if (p1) free((char*)p1);
    if (stencil) unlock_stencil(innr);
    while (_first) delete _first;
    de_alias_node();
    unlink();
}

// move known aliases to father for this node only
void Node::de_alias_node()
{
    Marker* pm = list;
    while (pm) {
	Marker* hpm = pm->next();
	if (pm->kind() != Tree) *pm = father;
	pm = hpm;
    }
}

// move known aliases to father for entire subtree
void Node::de_alias_tree()
{
    for (Node* pn = _first; pn; pn = pn->_right) {
	pn->de_alias_tree();
    }
    de_alias_node();
}

// find son at position pos
Node* Node::under(Index pos)
{
    if (stencil) {
	Char c = (*this)[pos];
	if (!IsPh(c)) return Empty;
	Node *pn = _first;
	while (pn && pn->_kind != c) pn = pn->_right;
	if (pn && pn->_kind == c)
	    return pn;
	else {
	    Node* nn = new Node(c);
	    nn->father = this;
	    nn->father.pos = pos;
	    nn->_right = _first;
	    if (_first)
		_first->_left = nn;
	    else
		_last = nn;
	    _first = nn;
	    return nn;
	}
    } else if (pos<=size()/2) {
	Node* pn = _first;
	while (pn && pn->father.pos < pos) pn = pn->_right;
	if (pn && pn->father.pos == pos)
	    return pn;
    } else {
	Node* pn = _last;
	while (pn && pn->father.pos > pos) pn = pn->_left;
	if (pn && pn->father.pos == pos)
	    return pn;
    }
    return Empty;
}

// find son left of position pos
Node* Node::left_of(Index pos)
{
    if (stencil) {
	Char *c = stencil_screen(innr);
	Index i = stencil_size(innr);
	if (pos>=i) pos=i-1;
	while (pos && !IsPh(c[pos])) pos--;
	if (IsPh(c[pos])) return under(pos);
	else return Empty;
    } else if (pos <= size()/2) {
	Node* pn = right_of(pos);
	if (!pn) pn = _last;
	else pn = pn->_left;
	if (!pn) return Empty;
	if (pn->father.pos == pos) pn = pn->_left;
	return pn;
    } else {
	Node* pn = _last;
	while (pn && pn->father.pos >= pos) pn = pn->_left;
	return pn;
    }
}

// find son right of position pos (!stencil)
Node* Node::right_of(Index pos)
{
    if (stencil) {
	Char *c = stencil_screen(innr);
	Index i = stencil_size(innr);
	pos++;
	while (pos<i && !IsPh(c[pos])) pos++;
	if (pos<i) return under(pos);
	else return Empty;
    } else if (pos > size()/2) {
	Node* pn = left_of(pos);
	if (!pn) pn = _first;
	else pn = pn->_right;
	if (!pn) return Empty;
	if (pn->father.pos == pos) pn = pn->_right;
	return pn;
    } else {
	Node* pn = _first;
	while (pn && pn->father.pos <= pos) pn = pn->_right;
	return pn;
    }
}

// find depth of node in tree
Index Node::depth() const
{
    Index d = 0;
    for (Node* pn = father; pn; pn = pn->father) d += 1;
    return d;
}

//find the next empty node in the tree
Node* Node::next()
{
    Node* pn = this;
    do {
	if (pn->right()) for (pn = pn->right(); pn->first(); pn = pn->first());
	else pn = pn->father;
    } while (pn && pn->size());
    return pn;
}

Mark Node::next_text()
{
    Node* pn = this;
    while (pn) {
	if (pn->size() == 0 && pn != this) {
	    Mark m(pn,0);
	    return m;
	}
	if (pn->father && pn->father->text()) {
	    for (Node* h = father; h; h = h->father) {
		if (h == pn->father) {
		    Mark m(pn->above());
		    m.pos++;
		    return m;
		}
	    }
	}
	if (pn->right())
	    for (pn = pn->right(); pn->first(); pn = pn->first());
	else
	    pn = pn->father;
    }
    Mark m(0,0);
    return m;
}

Mark Node::next_node()
{
    Node* pn = this;
    while (pn->first()) pn = pn->first();
    if (pn->size() == 0) {
	return Mark(pn,0);
    } else {
	return pn->next_text();
    }
}

Bool Node::normal_identifier() const
{
    Index i=0;
    while (i<size1)
	if (!Char2Font(p1[i]) && isalpha(Char2ASCII(p1[i]))) i++;
	else return MP_False;
    while (i<size2)
	if (!Char2Font(p2[i]) && isalpha(Char2ASCII(p2[i]))) i++;
	else return MP_False;
    return MP_True;
}

// find height qua expressions of node in tree
Index Node::height()
{
    Index h = 0;
    for (Node* pn = first(); pn; pn = pn->right()) {
	if (pn->expr() && !pn->parens()) {
	    Index hh = pn->height() + 1;
	    if (h < hh) h = hh;
	} else if (Ph((*this)[pn->above().pos]) == MP_Expr) {
	    if (h < 1) h = 1;
	}
    }
    return h;
}

// check whether the tree has been changed, and reset the changed indicators
Bool Node::poll()
{
    Bool b = changed;
    changed = MP_False;
    for (Node* pn = _first; pn; pn = pn->_right) {
	 b |= pn->poll();
    }
    return b;
}

// calculate the minimum of the precedences
Index Node::min_preced()
{
    Index p = MaxPrecedence;
    for (Node* pn = first(); pn; pn = pn->right()) {
	if (IsOp((*this)[pn->above().pos]) && pn->size() && pn->preced() < p) {
	    p = pn->preced();
	}
    }
    return p;
}

// calculate the maximum of the precedences
Index Node::max_preced()
{
    Index p = 0;
    for (Node* pn = first(); pn; pn = pn->right()) {
	if (IsOp((*this)[pn->above().pos]) && pn->size() && pn->preced() > p) {
	    p = pn->preced();
	}
    }
    return p;
}

Bool Node::can_be_raised()
{
    Node* pn;
    Opkind ok=None;
    Index pr=0;
    if (!father) return MP_False;
    if (text() && father->text()) return MP_True;
    if (!expr() || ! father->expr()) return MP_False;
    if (stencil || father->stencil) return MP_False;
    for (pn = _first; pn; pn = pn->_right) {
	if (pn->stencil && IsOp(pn->above()(0))) {
	    pr = pn->preced();
	    ok = pn->opkind();
	}
    }
    for (pn = father->_first; pn; pn = pn->_right) {
	if (pn->stencil && IsOp(pn->above()(0))) {
	    if (pr != pn->preced()) return MP_False;
	    if (ok != pn->opkind()) {
		if (ok==Prefix  && pn->opkind()!=Postfix) return MP_False;
		if (ok==Postfix && pn->opkind()!=Prefix) return MP_False;
	    }
	}
    }
    return MP_True;
}

Bool Node::may_be_raised()
{
    if (!father) return MP_False;
    if (!size()) return MP_False;
    if (text() && father->text()) return MP_True;
    if (!expr() || !father->expr()) return MP_False;
    if (stencil || father->stencil) return MP_False;
    for (Node* pn = _first; pn; pn = pn->_right) {
	if (!pn->op() || !pn->stencil) continue;
	Node* qn = father->first();
	while (qn) {
	    if (qn->op() && qn->stencil && (qn->innr == pn->innr))
		break;
	    qn = qn->right();
	}
	if (!qn) return MP_False;
    }
    return MP_True;
}

Char Node::opspace(Bool ins)
{
    Index h = (ins? height() : father->height());
    Index os = (stencil ? stencil_space(innr):0);
    if (h>0) h--;
    if (stencil && (stencil_kind(innr)==Postfix ||
		    stencil_kind(innr)==Prefix)) h=0;
    h = (h>=os ? h : os);
    return Opspace((Offset)h+_opdelta >0 ? (Char)(h+_opdelta) : 0);
}

void Node::fill_find_nr(Char* nlist, Char &n)
{
    if (!size()) {
	nlist[n] = _findnr;
	n++;
    } else {
	Node* pn = first();
	while (pn) {
	    pn->fill_find_nr(nlist,n);
	    pn = pn->right();
	}
    }
}

Bool Node::equal_diff(Node *compare_with, Node **leftres, Node **rightres)
{
  if (!compare_with ||
      (_kind != compare_with->_kind) ||
      ( op() && (innr != compare_with->innr)) ||
      (!op() && (size() != compare_with->size())) ||
      (!_first &&  compare_with->_first) ||
      ( _first && !compare_with->_first)) {
    *leftres=this;
    *rightres=compare_with;
    return MP_False;
  }
  if (!stencil) {
    int i;
    for (i=0; i<size(); i++) {
      if ((*this)[i] != (*compare_with)[i]) {
	*leftres=this;
	*rightres=compare_with;
	return MP_False;
      }
    }
  }
  /* compare subnodes */
  Node *l, *r,*lres, *rres;
  lres=Empty; rres=Empty;
  l=first(); r=compare_with->first();
  while (l && r) {
    Node *lt, *rt;
    if (!(l->equal_diff(r, &lt, &rt))) {
      if (!lres) {
	lres=lt;
	rres=rt;
      } else {
	Node *lrub, *rrub;
	if (!lres->equal_diff(lt,&lrub,&rrub) ||
	    !rres->equal_diff(rt,&lrub,&rrub)) {
	  /* Two subtrees are different and the differences are different
	  ** as in 'a+a != 'b+c'  versus 'a+a' != 'b+b'
	  ** So, assume that this complete tree is different.
	  */
	  *leftres=this;
	  *rightres=compare_with;
	  return MP_False;
	}
      }
    }
    l=l->right();
    r=r->right();
  }
  if (lres) {
    *leftres=lres;
    *rightres=rres;
    return MP_False;
  } else {
    return MP_True;
  }
}

Char Node::last_used_find_nr()
{
    if (!size()) { 
	return _findnr;
    } else {
	Node* pn = first();
	Char max=0;
	while (pn) {
	    Char c = pn->last_used_find_nr();
	    if (max<c) max = c;
	    pn = pn->right();
	}
	return max;
    }
}

Char Node::first_unused_find_nr()
{
    Char ne = number_empty();
    Char* nlist = (Char*) malloc(ne*sizeof(Char));
    Char n = 0;
    fill_find_nr(nlist, n);
    n = 1;
    while (n<=ne) {
	Index i;
	for (i=0; i<ne && nlist[i]!=n; i++);
	if (i<ne)
	    n++;
	else {
	    free((char*)nlist);
	    return n;
	}
    }
    free((char*)nlist);
    return n;
}

void Node::set_find_nr_rec(Char& c)
{
    if (!size()) {
	_findnr = c;
	c++;
    } else {
	Node *pn = first();
	while (pn) {
	    pn->set_find_nr_rec(c);
	    pn = pn->right();
	}
    }
}

void Node::clear_find_nr_rec()
{
    _findnr = 0;
    Node *pn = first();
    while (pn) {
	pn->clear_find_nr_rec();
	pn = pn->right();
    }
}

void Node::set_parens(Bool b)
{
    Bool oldpar=_parens;
    _parens = (!!father) &&
	(b || !father->text() && min_preced() <= father->max_preced());
    changed= changed || oldpar!=_parens;
}

void Node::display_left(Index n)
{
    Node *pn = this;
    if (!father) return;
    while (pn->father && !IsDisp((pn->above())(0))) pn = pn->father;
    if (pn->father) {
	pn->_display_pos-=n;
	changed=MP_True;
    }
}

void Node::display_right(Index n)
{
    Node *pn = this;
    if (!father) return; 
    while (pn->father && !IsDisp((pn->above())(0))) pn = pn->father;
    if (pn->father) {
	pn->_display_pos+=n;
	changed=MP_True;
    }
}

void Node::increase_spacing(Index count)
{
    changed=MP_True;
    _opdelta += count;
}

void Node::decrease_spacing(Index count)
{
    changed=MP_True;
    _opdelta -= count;
}

void Node::reset_spacing()
{
    changed=(changed || _opdelta);
    _opdelta = 0;
}

// Return the number of Newlines in the tree.
Index Node::lines()
{
    Index h = (stencil ? stencil_lines(innr) : _lines);
    for (Node* pn = first(); pn; pn = pn->right()) h += pn->lines();
    return h;
}

// Change the character on a given position (word wrap)
void Node::change_to(Char newval, Index pos)
{
    if (text() && pos<size()) {
	if (IsNewline((*this)[pos])) _lines--;
	if (IsNewline(newval)) _lines++;
	(*this)[pos]=newval;
    }
}

// insert c at pos count times
Bool Node::insert(Index pos, Char c, Index count)
{
    Index i;
    if (!count) return MP_False;
    if (stencil) return MP_False;
    if (!set_gap(pos,count)) return MP_False;
    for (i = 0; i != count; i += 1) {
	(p1+size1)[i] = c;
    }
    size1 += count;
    gap -= count;
    p2 -= count;
    changed = MP_True;
    for (Marker* pm = list; pm; pm = pm->next()) {
	if (pos<pm->pos || pos==pm->pos && Right<=pm->kind()) {
	    pm->pos += count;
	}
    }
    if (IsPh(c)) {
	if (Ph(c) == MP_Disp) c = PhNum2Char(MP_Expr,Num(c));
	for (i = 0; i != count; i += 1) {
	    Node* nn  = new Node(c);
	    if (!nn) return MP_False;
	    nn->father = this;
	    nn->father.pos = pos + i;
	    nn->_left  = left_of(pos+i);
	    nn->_right = right_of(pos+i);
	    if (nn->_left)  nn->_left->_right = nn; else _first = nn;
	    if (nn->_right) nn->_right->_left = nn; else _last  = nn;
	}
    }
    if (IsNewline(c)) _lines += count;
    return MP_True;
}

void Node::insert_string(Index pos, Char *s, Index count)
{
    if (stencil || !s) return;
    if (!count) count = Ustrlen(s);
    if (!count) return;
    if (!set_gap(pos,count)) return;
    for (Index i = 0; i != count; i += 1) {
	if (s[i]=='\n') {
	    (p1+size1)[i] = Newline;
	    _lines++; 
	} else if (s[i]=='\t')
	    (p1+size1)[i] = Rtab;
	else
	    (p1+size1)[i] = s[i];
    }
    size1 += count;
    gap -= count;
    p2 -= count;
    changed = MP_True;
    for (Marker* pm = list; pm; pm = pm->next()) {
	if  (pos<pm->pos || pos==pm->pos && Right<=pm->kind()) {
	    pm->pos += count;
	}
    }
}

// insert an operator string s in node
Bool Node::insert(Index nnr)
{
    stencil = MP_True;
    innr = nnr;
    lock_stencil(nnr);
    if (p1) {
	free((char*)p1);
	p1 = p2 = 0;
	size1 = size2 = gap = 0;
    }
    Index count = stencil_size(nnr);
    changed = MP_True;
    for (Marker* pm = list; pm; pm = pm->next()) {
	if (0<pm->pos || 0==pm->pos && Right<=pm->kind()) {
	    pm->pos = count;
	}
    }
    _lines = stencil_lines(innr);
/*    Char *c = stencil_screen(innr);
    if (c) {
	Index i=0;
	while (*c) {
	    if (IsPh(*c)) {
		Node* nn = new Node(*c);
		if (!nn) return MP_False;
		replace(i,nn);
	    }
	    c++;i++;
	}
    }
*/    return MP_True;
}

Bool Node::transpose_chars(Index &pos, Index n)
{
    Bool new_or_ph = MP_False;
    if (!text() || !pos || stencil) return MP_False;
    if (pos+n>size()) n = size()-pos;
    if (!n) return MP_False;
    Index i=0;
    Char c = (*this)[pos-1];
    Node *pn = under(pos-1);
    Node *pn2=Empty;
    changed=MP_True;
    while (i<n) {
	(*this)[pos-1+i] = (*this)[pos+i];
	new_or_ph |= (IsNewline((*this)[pos-1+i]) || IsPh((*this)[pos-1+i])); 
	if (IsPh((*this)[pos-1+i]) && aig(pn2 = under(pos+i))) {
	    if (pn) {
		if (pn->_left) {
		    pn2->_left = pn->_left;
		    pn->_left->_right = pn2;
		} else {
		    _first = pn2;
		    pn2->_left = Empty;
		}
		if (pn2->_right) {
		    pn->_right = pn2->_right;
		    pn2->_right->_left = pn;
		} else {
		    _last = pn;
		    pn->_right = Empty;
		}
		pn->_left = pn2;
		pn2->_right = pn;
	    }
	    pn2->father.pos -= 1;
	}
	i++;
    }
    if (pn) pn->father.pos = pos+i-1;
    (*this)[pos+i-1] = c;
    new_or_ph |= (IsNewline(c) || IsPh(c));
    i = pos;
    for (Marker* pm = list; pm; pm = pm->next()) {
	if (pm->kind() != Tree) {
	    if (pm->pos >= i && pm->pos<i+n) pm->pos -= 1;
	}
    }
    pos = i+n;
    return new_or_ph;
}

// swap part [bp,ep) with part [bq,eq), both without place holders.
Bool Node::transpose_words(Index &bp, Index &ep, Index &bq, Index &eq)
{
    if (ep<bp || eq<bq || stencil) return MP_False;
    Index i,n;
    Bool retval = MP_False;
    Char swt;
    if (bq<bp) {
	n=bq; bq=bp; bp=n;
	n=eq; eq=ep; ep=n;
    }
    if (ep>bq) return MP_False;
    for (n=bp; n<ep; n++)
	if (IsPh((*this)[n])) return MP_False;
    for (n=bq; n<eq; n++)
	if (IsPh((*this)[n])) return MP_False;
    set_gap(eq,0);
    changed=MP_True;
    for (i=ep; i<bq; i++)
	retval |= (IsPh(p1[i]) || (IsNewline(p1[i])));
    if (ep-bp>eq-bq) {
	n = ep-bp +bq-eq;
	Char *h = (Char*) malloc(n*sizeof(Char));
	for (i=0; i< n; i++)
	    h[i]=p1[ep-n+i];
	membwd(p1+ep-n, p1+ep, p1+eq);
	Marker* pm = list;
	while (pm) {
	    if (ep<= pm->pos && pm->pos<bq) pm->pos -= n;
	    pm = pm->next();
	}
	bq -= n;
	ep -= n;
	for (i=0; i<n; i++)
	    p1[eq-n+i] = h[i];
	n = ep - bp;
	free((char*)h);
    } else {
	n = eq-bq +bp-ep;
	Char *h = (Char*) malloc(n*sizeof(Char));
	for (i=0; i<n; i++)
	    h[i]=p1[eq-n+i];
	memfwd(p1+eq, p1+eq-n, p1+ep);
	Marker* pm = list;
	while (pm) {
	    if (ep<= pm->pos && pm->pos<bq) pm->pos += n;
	    pm = pm->next();
	}
	bq += n;
	ep += n;
	for (i=0; i<n; i++)
	    p1[ep-n+i] = h[i];
	n = eq-bq;
	free((char*)h);
    }
    for (i=0; i<n; i++) {
	swt = p1[bp+i];
	p1[bp+i] = p1[bq+i];
	p1[bq+i] = swt;
    }
    return MP_True;
}

//  should be removed (MP_Var placeholders are almost never used)
void Node::var_comma_adjust(Index& begin, Index& end)
{
    if (!var() || stencil) return;
    if (((*this)(begin)==MP_Id && (*this)(end-1)==',') ||
	((*this)(begin)==',' && (*this)(end-1)==MP_Id)) return;
    if ((*this)(begin)==',' && (*this)(end-1)==',') {
	begin += 1;
	return;
    }
    if ((*this)(begin-1)==',') {
	begin -= 1;
	return;
    }
    if ((*this)(end)==',') {
	end += 1;
	return;
    }
    return;
}

// remove [begin,end) from node
Bool Node::remove(Index begin, Index end)
{
    Bool new_or_ph = MP_False;
    Index i;
    if (var()) var_comma_adjust(begin, end);
    const Index count = end - begin;
    for (i=begin; !new_or_ph && i<end; i++)
	new_or_ph = (IsNewline((*this)[i]) || IsPh((*this)[i]));
    if (stencil || (begin == 0 && end == size())) {
	if (p1) free((char*)p1);
	p1 = p2 = 0;
	size1 = size2 = gap = 0;
	if (father) {
	    if (IsDisp(above()(0))) _kind = PhNum2Char(MP_Disp,Num(_kind));
	}
	if (stencil) unlock_stencil(innr);
	innr=0;
	stencil=MP_False;
	_lines = 0;
	_findnr = 0;
	_parens = 0;
    } else {
	set_gap(begin);
	for (i = 0; i != count; i++) {
	    if (IsNewline(p2[begin+i])) _lines -= 1;
	}
	size2 -= count;
	gap += count;
	p2 += count;
    }
    changed = MP_True;
    Node* pn = _first;
    while (pn) {
	Node* hn = pn->_right;
	if (stencil || (begin<=pn->father.pos && pn->father.pos<end))
	    delete pn;
	pn = hn;
    }
    for (Marker* pm = list; pm; pm = pm->next()) {
	if (end < pm->pos) pm->pos -= count;
	else if (begin < pm->pos) pm->pos = begin;
    }
    return new_or_ph;
}

//remove pos + count, from node
Bool Node::remove(Index pos, Offset count)
{
    Index begin = pos;
    Index end = pos;
    if (count <0) {
	while (count++) {
	    Char c = (*this)(begin-1);
	    if (!c || IsPh(c)) break;
	    begin -= 1;
	}
    } else {
	while (count--) {
	    Char c = (*this)(end);
	    if (!c || IsPh(c)) break;
	    end += 1;
	}
    }
    return remove(begin,end);
}

// cut [begin,end) from a node
Node* Node::cut(Index begin, Index end)
{
    var_comma_adjust(begin, end);
    const Index count = end - begin;
    Node* nn = new Node(this);
    if (!nn || !nn->set_gap(0,count)) {
	delete nn;
	return Empty;
    }
    if ((begin == 0 && end == size()) || stencil) {
	if (nn->p1) free((char*) nn->p1);
	nn->p1 = p1;
	nn->p2 = p2;
	nn->size1 = size1;
	nn->size2 = size2;
	nn->gap = gap;
	p1 = p2 = 0;
	size1 = size2 = gap = 0;
	nn->stencil = stencil;
	nn->innr = innr;
	if (father) {
	    if (IsDisp(above()(0))) _kind = PhNum2Char(MP_Disp,Num(_kind));
	}
	nn->_first = _first;
	nn->_last = _last;
	Node *pn = _first;
	while (pn) {
	    pn->father = nn;
	    pn=pn->_right;
	}
	_first = _last = Empty;
	for (Marker* pm = list; pm; pm = pm->next()) pm->pos = 0;
	nn->_lines = _lines;
	stencil = MP_False;
	innr = 0;
	_findnr = 0;
	_parens = 0;
    } else {
	set_gap(begin);
	membwd(nn->p1, p2+begin, p2+end);
	nn->size1 = count;
	nn->gap -= count;
	nn->p2 -= count;
	size2 -= count;
	gap += count;
	p2 += count;
	Node* pn = _first;
	while (pn) {
	    Node* hn = pn->_right;
	    Index  i = pn->father.pos;
	    if (begin<=i && i<end) {
		pn->de_alias_tree();
		pn->unlink();
		nn->replace(i-begin,pn);
	    }
	    pn = hn;
	}
	for (Marker* pm = list; pm; pm = pm->next()) {
	    if (end < pm->pos) pm->pos -= count;
	    else if (begin < pm->pos) pm->pos = begin;
	}
	for (Index i = 0; i != count; i++) {
	    if (IsNewline(nn->p1[i])) nn->_lines += 1;
	}
    }
    changed = MP_True;
    _lines -= nn->_lines;
    return nn;
}

// copy [begin,end) from a node
Node* Node::copy(Index begin, Index end) const
{
    if (stencil) {
	Node* nn = new Node(this);
	if (!nn) return Empty;
	nn->stencil = MP_True;
	nn->innr=innr;
	nn->_lines = _lines;
	lock_stencil(innr);
	Node* pn = _first;
	while (pn) {
	    Node* hn = pn->copy(0, pn->size());
	    if (hn) {
		hn->father = nn;
		hn->father.pos = 0;
		hn->_left = nn->_last;
		if (nn->_last)
		    hn->_left->_right = hn;
		else
		    nn->_first = hn;
		nn->_last = hn;
	    } else {
		delete nn;
		return Empty;
	    }
	    pn = pn->_right;
	}
	return nn;
    } else {
	const Index count = end - begin;
	Node* nn = new Node(this);
	if (!nn || !nn->set_gap(0,count)) {
	    delete nn;
	    return Empty;
	}
	if (end <= size1) {
	    membwd(nn->p1, p1+begin, p1+end);
	} else if (size1 <= begin) {
	    membwd(nn->p1, p2+begin, p2+end);
	} else {
	    membwd(nn->p1, p1+begin, p1+size1);
	    membwd(nn->p1+size1-begin, p2+size1, p2+end);
	}
	nn->stencil = MP_False;
	nn->innr = innr;
	nn->size1 = count;
	nn->gap -= count;
	nn->p2 -= count;
	for (Node* pn = _first; pn; pn = pn->_right) {
	    Index i = pn->father.pos;
	    if (begin<=i && i<end) {
		Node* hn = pn->copy(0,pn->size());
		if (hn) {
		    nn->replace(i-begin,hn);
		} else {
		    delete nn;
		    return Empty;
		}
	    }
	}
	for (Index i = 0; i != count; i++) {
	    if (IsNewline(nn->p1[i])) nn->_lines += 1;
	}
	return nn;
    }
}

Char* Node::copy_name()
{
    if (id()) {
	Char *c;
	Index i = 0;
	if (aig(c = (Char*) malloc((size()+1)*sizeof(Char)))) {
	    while (i<size()) {
		c[i] = (*this)[i];
		i++;
	    }
	    c[i] = 0;
	}
	return c;
    }
    return NULL;
}

Char *Node::content_ref()
{
  if (!set_gap(size1+size2,1)) return 0;
  p1[size1]=0;
  return p1;
}

void delete_copied_name(Char* name)
{
    if (name) free((char*)name);
}

Bool Node::can_rename(Char *name)
{
    Node *pn = first();

    while (pn) {
	if (pn->var()) {
	    Node *pm = pn->first();
	    while (pm) {
		if (*pm == name) return MP_False;
		pm = pm->right();
	    }
	}
	pn = pn->right();
    }
    return MP_True;
}

void Node::rename(Index begin, Index end, Char *oldn, int ol,
		  Char *newn, int nl)
{
    if (id() && !stencil) {
	if ((*this) == oldn) {
	    gap += size();
	    p2 = p1+gap;
	    size1 = size2 = 0;
	    if (set_gap(0, nl)) {
		for (int i=0; i<nl; i++)
		    p1[i] = newn[i];
		size1+=nl;
		gap -= nl;
		p2 -= nl;
	    } else {
		for (int i=0; i<ol; i++)
		    p1[i] = oldn[i];
		size1+= ol;
		gap -= ol;
		p2 -= ol;
	    }
	}
    } else {
	Node *pn = _first;
	while (pn && !stencil && pn->father.pos <begin) pn = pn->_right;
	while (pn && (stencil || pn->father.pos <end)) {
	    if (pn->can_rename(oldn))
		pn->rename(0, pn->size(), oldn, ol, newn, nl);
	    pn = pn->_right;
	}
    }
} 

// insert the contents of a node
Bool Node::paste(Index pos, Node* pn)
{
    if (stencil) return MP_False;
    if (text() && !pn->text()) {
	insert(pos,MP_Expr);
	replace(pos,pn);
	return MP_True;
    }
    if (pn->stencil) {
	if (!size() && !pos && (expr() || op())) {
	    stencil = MP_True;
	    pn->stencil=MP_False;
	    innr = pn->innr;
	    Node *pm=_first;
	    _first = pn->_first;
	    pn->_first = pm;
	    pm = _last;
	    _last = pn->_last;
	    pn->_last = pm;
	    for (pm=_first; pm; pm=pm->_right) pm->father = this;
	    _lines = pn->_lines;
	    if (p1) free((char*)p1);
	    p1 = p2 = 0;
	    size1=size2=gap=0;
	    for (Marker* pml = list; pml; pml = pml->next()) {
	        if (pos<pml->pos || pos==pml->pos && Right<=pml->kind()) {
		    pml->pos += stencil_size(pn->innr);
	        }
	    }
	    delete pn;
	    return MP_True;
	} else
	    return MP_False;
    }
    if (!set_gap(pos,pn->size())) return MP_False;
    pn->set_gap(pn->size());
    if (!pn->size())
	_findnr = pn->_findnr;
    membwd(p1+size1, pn->p1, pn->p1+pn->size1);
    size1 += pn->size1;
    gap -= pn->size1;
    p2 -= pn->size1;
    changed = MP_True;
    for (Marker* pm = list; pm; pm = pm->next()) {
	if (pos<pm->pos || pos==pm->pos && Right<=pm->kind()) {
	    pm->pos += pn->size1;
	}
    }
    for (Node* hn = pn->_first; hn; hn = hn->_right) {
	hn->father = this;
	hn->father.pos += pos;
    }
    if (pn->_first) {
	pn->_first->_left = left_of(pos);
	pn->_last->_right = right_of(pos);
	if (pn->_first->_left)  pn->_first->_left->_right = pn->_first;
	else _first = pn->_first;
	if (pn->_last->_right) pn->_last->_right->_left = pn->_last;
	else _last = pn->_last;
	pn->_first = pn->_last = Empty;
    }
    _lines += pn->_lines;
    innr = pn->innr;
    delete pn;
    if (expr() && !!father && father->expr()) set_parens(_parens);
    return MP_True;
}

// replace the node at pos with the given one
Node* Node::replace(Index pos, Node* pn)
{
    Node* hn = under(pos);
    if (hn) {
	if (pn) {
	    pn->_left   = hn->_left;
	    pn->_right  = hn->_right;
	    pn->father = hn->father;
	    pn->_kind = PhNum2Char(Ph(pn->kind()),Num(hn->kind()));
	} else {
	    if (hn->_left)  hn->_left->_right = hn->_right;
	    else _first = hn->_right;
	    if (hn->_right) hn->_right->_left = hn->_left;
	    else _last  = hn->_left;
	}
	hn->father = hn->_left = hn->_right = Empty;
    } else if (pn) {
	pn->_left   = left_of(pos);
	pn->_right  = right_of(pos);
	pn->father = this;
	pn->father.pos = pos;
	pn->_kind = PhNum2Char(Ph(pn->kind()),Num((*this)[pos]));
    }
    if (pn) {
	if (pn->_left)  pn->_left->_right = pn; else _first = pn;
	if (pn->_right) pn->_right->_left = pn; else _last  = pn;
    }
    changed = MP_True;
    return hn;
}

// replace the node with this one, which may not be part of a tree
void Node::replaces(Node* n)
{
    if (!n) return;
    father = n->father;
    _kind = PhNum2Char(Ph(_kind),Num(n->_kind));
    _left  = n->_left;
    _right = n->_right;
    if (_left)  _left->_right = this; else father->_first = this;
    if (_right) _right->_left = this; else father->_last  = this;
    n->father = n->_left = n->_right = Empty;
    father->changed = MP_True;
    if (expr() && size() && father->expr()) {
	set_parens(parens());
    }
}

Node* Node::tree_walk(Node *stop)
{
    if (first())
	return first();
    else if (this == stop)
	return 0;
    else if (right())
	return right();
    else {
	Node *pn = father;
	while (pn && pn != stop && !pn->right()) pn = pn->father;
	if (pn && pn!=stop)
	    return pn->right();
	else
	    return 0;
    }
}

Node* Node::tree_walk_skip(Node *stop)
{
    if (this == stop)
	return 0;
    if (right())
	return right();
    Node *pn = father;
    while (pn && pn != stop && !pn->right()) pn = pn->father;
    if (pn && pn!=stop)
	return pn->right();
    else
	return 0;
}

//Bool Node::tree_diff(Node *other, Node **subnode, Node **subother)
//{
//   // should return the difference between 'this' and 'other'
//}  

Bool Node::match_tree(Node *n, Index pos, Node **nlist, Char *tlist)
{
    Node *pn = n;
    Node *pt = this;
    Index nlp = 0;
    Index i=pos;
    if (Ph(kind())!=Ph(n->kind())) return MP_False;
    while (pn && pt) {
	if (pt->_findnr) return MP_False;
	if (!pn->size()) {
	    if (!pn->_findnr && pt->size()) return MP_False;
	    if (nlist) {
		nlist[nlp] = pt;
		tlist[nlp] = pn->_findnr;
		nlp++;
	    }
	    pt = pt->tree_walk_skip(this);
	} else {
	    Index j=0;
	    Index underj = 0;
	    Bool underjset = MP_False;
	    if (pn->stencil != pt->stencil) return MP_False;
	    if (pn->stencil) {
		if (i) return MP_False;
		if (which_version(pn->innr) != which_version(pt->innr))
		    return MP_False;
		// other possible tests:
                // pn->innr != pt->innr
                // which_notation(pn->innr) != which_notation(pt->innr) 
	    } else {
		if (i+pn->size()>pt->size() ||
		    (pn!=n && pn->size() != pt->size()) ||
		    (pn->id() &&
		     (!pt->id() || pn->stencil_nr()!=pt->stencil_nr())))
		    return MP_False;
		while (j<pn->size()) {
		    if ((*pt)[i+j] != (*pn)[j]) return MP_False;
		    if (i && IsPh((*pt)[i+j]) && !underjset) {
			underj = i+j;
			underjset = MP_True;
		    }
		    j++;
		}
	    }
	    if (i) {
		if (underjset)
		    pt = pt->under(underj);
		else
		    pt = pt->tree_walk_skip(this);
	    } else
		pt = pt->tree_walk(this);
	}
	pn = pn->tree_walk(n);
    }
    if (nlist) {
	for (i=0; i<nlp; i++)
	    for (Index j=i+1; j<nlp; j++) {
		if (tlist[i]==tlist[j])
		    if (!nlist[i]->match_tree(nlist[j],0)) return MP_False;
	    }
    }
    return MP_True;
}

Mark Node::search_label(Char *str)
{
    Node *np;
    Bool found = MP_False;
    Index n=0;
    while (str[n]) n++;
    np = this;
    while (np->_first) np=np->_first;
    while (!found && np) {
	if (np->size() == n && *np == str)
	    found = MP_True;
	else {
	    while (!np->_right && !!np->father) np = np->above();
	    np = np->_right;
	    if (np)
		while (np->_first) np=np->_first;
	}
    }
    Mark mp;
    mp=np;
    while (!!mp) {
	mp=mp->above();
	if (!!mp && mp.pos > mp->size()) np = mp;
    }
    mp = np;
    mp.pos=0;
    return mp;
}


Node* Node::replace_tree(Node *oldn, Node *newn, Index pos)
{
    Index ne = oldn->number_empty();
    Index i;
    Node** nlist = (Node**) malloc(ne*sizeof(Node*));
    if (ne!=0 && !nlist) return Empty;
    Char* tlist = (Char*) malloc(ne*sizeof(Char));
    if (ne!=0 && !tlist) { free((char*)nlist); return Empty; }
    for (i = 0; i<ne; i++) {
	nlist[i] = Empty;
	tlist[i] = 0;
    }
    if (!match_tree(oldn, pos, nlist, tlist)) {
	free((char*)tlist);
	free((char*)nlist);
	return Empty;
    }
    i=0;
    Node *n = newn->copy(0,newn->size());
    Char fnrst=1;
    n->set_find_nr_rec(fnrst);
    Node *pn = n;
    Node *tn = newn;
    while (tn) {
	if (!tn->size()) {
	    if (!tn->_findnr) {
		pn = pn->tree_walk(n);
	    } else {
		for (i=0; i<ne && tlist[i]!=tn->_findnr; i++);
		if (i<ne) {
		    Node *nn = nlist[i]->copy(0,nlist[i]->size());
		    pn->paste(0,nn);
		    pn = pn->tree_walk_skip(nn);
		} else
		    pn = pn->tree_walk(n);
	    }
	} else
	    pn = pn->tree_walk(n);
	tn = tn->tree_walk(newn);
    }
    free((char*)nlist);
    free((char*)tlist);
    pn = cut(pos, pos+oldn->size());
    paste(pos, n);
    return pn;
}

Index Node::number_empty()
{
    if (size()==0) return 1;
    Index n=0;
    Node *pn = first();
    while (pn) {
	n+= pn->number_empty();
	pn = pn->right();
    }
    return n;
}

Bool Node::check_ph(Node *n)
// check if the place holders in this have a findnr and kind as used in n
{
    Index i;
    Index ne = n->number_empty();
    Char *tlist = (Char*) malloc(ne*2*sizeof(Char));
    if (ne!=0 && !tlist) return MP_False;
    Char *klist = tlist+ne;
    for (i = 0; i<ne*2; tlist[i++]=0);
    Node *pn = n;
    i=0;
    while (pn) {
	if (pn->size()==0){
	    klist[i] = Ph(pn->kind());
	    tlist[i] = pn->find_nr();
	    i++;
	}
	pn = pn->tree_walk(n);
    }
    pn = this;
    Bool result = MP_True;
    while (pn) {
	if (pn->size()==0 && pn->find_nr()) {
	    for (i=0; i<ne && tlist[i] != pn->find_nr(); i++);
	    if (i==ne || klist[i] != Ph(pn->kind())) {
		result = MP_False;
		pn->clear_find_nr();
	    }
	}
	pn = pn->tree_walk(this);
    }
    free((char*)tlist);
    return result;
}

Bool Node::matches(Char *str, Index n, Index pos)
{
    Index i=0;

    if (pos+n > size()) return MP_False;
    if (stencil) return MP_False;
    if (!text() && !id() && pos && n!=size()) return MP_False;
    while (i<n) {
	if ((str[i]==(*this)[pos+i]) ||
	    (IsPh(str[i]) && (Ph(str[i])==Ph((*this)[pos+i]))) ||
	    (IsNewline(str[i]) && IsNewline((*this)[pos+i])))
	    i++;
	else
	    return MP_False;
    }
    return MP_True;
}

Node* Node::replace_notation(Index onnr, Index nnnr)
{
    if (stencil && innr==onnr) {
	lock_stencil(nnnr);
	unlock_stencil(onnr);
	innr = nnnr;
	// Should the sub=expressions be relinked?
	if (!father->text()) father->set_parens(MP_False);
    }
    return Empty;
}

Node* Node::replacestr(Char *oldstr, const Index n, Char *newstr,
		      const Index m, Index pos)
{
    Node *rn = Empty;

    if (matches(oldstr, n, pos)) {
	Index i, j;
	int okpos, openpos;
	int* old2new = (int*) malloc(m*sizeof(int));
	int* new2old = (int*) malloc(n*sizeof(int));
	Bool backup_removed = MP_False;
	Char c;
	for (i=0; i<n; new2old[i++] = -1);
	for (i=0; i<m; i++) {
	    old2new[i] = -1;
	    if (IsPh(newstr[i])) {
		okpos = openpos = -1;
		for (j=0; j<n; j++) {
		    if (Ph(oldstr[j]) == Ph(newstr[i])) {
			if (Num(oldstr[j]) == Num(newstr[i]))
			    okpos = j;
			else
			    if (new2old[j]<0 && openpos<0) openpos = j;
		    }
		}
		if (okpos>=0) {
		    if (new2old[okpos]>=0) {
			if (openpos>=0) {
			    new2old[openpos] = new2old[okpos];
			    old2new[new2old[openpos]] = openpos;
			} else
			    old2new[new2old[okpos]] = -1;
		    }
		    new2old[okpos] = i;
		    old2new[i] = okpos;
		} else
		    if (openpos>=0) {
			new2old[openpos] = i;
			old2new[i] = openpos;
		    }
	    }
	}
	Node *pn = new Node(this);
	pn->set_gap(0,m);
	for (i=0; i<m; i++)
	    pn->insert(i,newstr[i],1);
	i=0;
	while (i<n) {
	    c = (*this)[i+pos];
	    if (IsPh(c))
		if (new2old[i]>=0)
		    replace(pos+i,
			    pn->replace(new2old[i],replace(pos+i,Empty)));
		else
		    if (under(pos+i)->size()) backup_removed = MP_True;
	    i++;
	}
	free((char*)old2new);
	free((char*)new2old);
	if (backup_removed)
	    rn = cut(pos,pos+n);
	else
	    (void) remove(pos,pos+n);
	paste(pos,pn);
    }
    return rn;
}

// commute the expressions in [begin,end)
void Node::commute(Index begin, Index end)
{
    if (begin == end || !_first) {	
	if (father) {
	    // try to commute in father
	    Node *pn;
	    Index nbegin, nend;
	    Char searchsym;
	    pn = father;
	    nbegin = nend = above().pos;
	    searchsym = Ph((*pn)(nbegin));
	    if (nbegin>0) {
		nbegin = nbegin-1;
		while (nbegin>0 && Ph((*pn)(nbegin))!=searchsym) nbegin--;
		if (Ph((*pn)(nbegin))==searchsym)
		    nend = nend+1;
		else
		    nbegin = nend;
	    }
	    if (nbegin==nend) {
		if (nend < pn->size()) {
		    nend = nend+1;
		    while (nend<pn->size() && Ph((*pn)(nend))!=searchsym)
			nend++;
		    if (Ph((*pn)(nend))==searchsym)
			nend = nend+1;
		    else
			nend = nbegin;
		}
	    }
	    if (nbegin!=nend) pn->commute(nbegin, nend);
	}
	return;
    }
    changed = MP_True;
    for (;;) {
	Char c;
	Char d;
	end -= 1;
	//  Also MP_Op and MP_Id can be commuted
	while (aig(c = (*this)(begin)) && !IsOp(c) && !IsId(c) && !IsExpr(c))
	    begin += 1;
	while (aig(d = (*this)(end))   && !IsOp(d) && !IsId(d) && !IsExpr(d))
	    end   -= 1;
	if ((IsOp(c) || IsId(c)) && IsExpr(d))
	    while (aig(c = (*this)(begin)) && !IsExpr(c)) begin += 1;
	if (IsExpr(c) && (IsOp(d)||IsId(d)))
	    while (aig(d = (*this)(end))   && !IsExpr(d)) end   -= 1;
	if (IsId(c) && IsOp(d))
	    while (aig(c = (*this)(begin)) && !IsOp(c)) begin += 1;
	if (IsOp(c) && IsId(d))
	    while (aig(d = (*this)(end))   && !IsOp(d)) end  -= 1;
	if (!c || !d || begin >= end) break;
	replace(begin,replace(end,replace(begin,Empty)));
	begin += 1;
    }
}

// distribute (fn,fm) over [begin,end)
Bool Node::distribute(Index begin, Index end, Node* fn, Node* fa,
		      Mark &mf, Mark &ma)
{
    changed = MP_True;
    Bool dist_done=MP_False;
    Node* pn = first();
    while (pn) {
	Node* hn = pn->right();
	Index i = pn->above().pos;
	if (begin<=i && i<end && IsExpr(pn->above()(0))) {
	    Node* ga = Empty;
	    Node* gn = fn->fcopy(0,fn->size(),fa,0,fa->size(),ga);
	    if (!gn || !ga) return MP_False;
	    gn->replaces(pn);
	    pn->replaces(ga);
	    if (!dist_done) {
		dist_done = MP_True;
		mf = gn;
		ma = pn;
	    }
	    delete ga;
	}
	pn = hn;
    }
    return dist_done;
}

// factorise (fn,fm) out of [begin,end)
Bool Node::factorise(Index begin, Index end, Node* fn, Node* fa)
{
    changed = MP_True;
    Node* pn = first();
    while (pn) {
	Node* hn = pn->right();
	Index i = pn->above().pos;
	if (begin<=i && i<end && IsExpr(pn->above()(0))) {
	    Node* an; Index ab; Index ae;
	    if (pn->match(fn,fa,an,ab,ae)) {
		Node* nn;
		if (ab == 0 && ae == an->size()) {
		    nn = an;
		    nn->unlink();
		} else {
		    nn = an->cut(ab,ae);
		}
		if (!nn) return MP_False;
		nn->replaces(pn);
		delete pn;
	    }
	}
	pn = hn;
    }
    return MP_True;
}

// copy a function, given argument as region, fa becomes mark at argument
Node* Node::fcopy(Index begin,Index end,Node* an,Index ab,Index ae,Node*& fa)
{
    Node* nn = new Node(this);
    if (!nn) return Empty;
    if (this == an) {
	if (stencil || (ab == begin && ae == end)) {
	    nn->_kind = PhNum2Char(MP_Expr,Num(nn->_kind));
	    fa = nn;
	} else if (ab<begin || ae >end || ab>=end || ae<=begin) {
	    delete nn;
	    return Empty;
	} else {
	    if (!nn->set_gap(0,end-begin-ae+ab+1)) {
		delete nn;
		return Empty;
	    }
	    if (ab <= size1) {
		membwd(nn->p1, p1+begin, p1+ab);
	    } else if (size1 <= begin) {
		membwd(nn->p1, p2+begin, p2+ab);
	    } else {
		membwd(nn->p1, p1+begin, p1+size1);
		membwd(nn->p1+size1-begin, p2+size1, p2+ab);
	    }
	    if (end <= size1) {
		membwd(nn->p1+ab-begin+1, p1+ae, p1+end);
	    } else if (size1 <= ae) {
		membwd(nn->p1+ab-begin+1, p2+ae, p2+end);
	    } else {
		membwd(nn->p1+ab-begin+1, p1+ae, p1+size1);
		membwd(nn->p1+ab-begin+1+size1-ae, p2+size1, p2+end);
	    }
	    nn->p1[ab] = MP_Expr;
	    fa = new Node(MP_Expr);
	    if (!fa) {
		delete nn;
		return Empty;
	    }
	    nn->replace(ab,fa);
	    nn->size1 = end-begin-ae+ab+1;
	    nn->gap -= nn->size1;
	    nn->p2 -= nn->size1;
	    for (Node* pn = _first; pn; pn = pn->_right) {
		Index i = pn->father.pos;
		if (begin<=i && i<ab  ||  ae<=i && i<end) {
		    Node* hn = pn->fcopy(0,pn->size(),an,ab,ae,fa);
		    if (hn) {
			i = (i<ab ? i-begin : i-begin-ae+ab+1);
			nn->replace(i,hn);
		    } else {
			delete nn;
			return Empty;
		    }
		}
	    }
	}
    } else {
	if (stencil) {
	    nn->stencil = MP_True;
	    nn->innr = innr;
	    lock_stencil(innr);
	    for (Node* pn=_first; pn; pn = pn->_right) {
		Node *hn = pn->fcopy(0, pn->size(), an,ab,ae,fa);
		if (hn) {
		    hn->_left = nn->_last;
		    if (nn->_last) nn->_last->_right = hn;
		    else nn->_first= hn;
		    nn->_last = hn; hn->father=nn;
		} else {
		    delete nn;
		    return Empty;
		}
	    }
	} else {
	    const Index count = end - begin;
	    if (!nn->set_gap(0,count)) {
		delete nn;
		return Empty;
	    }
	    if (end <= size1) {
		membwd(nn->p1, p1+begin, p1+end);
	    } else if (size1 <= begin) {
		membwd(nn->p1, p2+begin, p2+end);
	    } else {
		membwd(nn->p1, p1+begin, p1+size1);
		membwd(nn->p1+size1-begin, p2+size1, p2+end);
	    }
	    nn->stencil=MP_False;
	    nn->innr = innr;
	    nn->size1 = count;
	    nn->gap -= count;
	    nn->p2 -= count;
	    for (Node* pn = _first; pn; pn = pn->_right) {
		Index  i = pn->father.pos;
		if (begin<=i && i<end) {
		    Node* hn = pn->fcopy(0,pn->size(),an,ab,ae,fa);
		    if (hn) {
			nn->replace(i-begin,hn);
		    } else {
			delete nn;
			return Empty;
		    }
		}
	    }
	}
    }
    for (Index i = 0; i != nn->size(); i++) {
	if (IsNewline((*nn)[i])) nn->_lines += 1;
    }
    return nn;
}

// match a function to a region of the tree, determine the argument (if any)
Bool Node::match(Node* fn, Node* fa, Node*& an, Index& ab, Index& ae)
{
    if (stencil != fn->stencil ||
	(stencil && which_version(innr) != which_version(fn->innr)))
	return MP_False;
    if (fn == fa->father) {
	if (stencil) {
	    Node *pn;
	    Node *pfn;
	    for (pn=first(),pfn=fn->first();
		 pfn;
		 pn=pn->right(),pfn=pfn->right()) {
		if (pfn!=fa && !pn->match(pfn,fa,an,ab,ae)) return MP_False;
	    }
	    Index i = fa->above().pos;
	    ab = i;
	    ae = i+1;
	} else {
	    if (size() < fn->size()) return MP_False;
	    an = this;
	    Index i;
	    for (i=0, ab=0; i!=fa->father.pos; i+=1, ab+=1) {
		if ((*this)[ab] != (*fn)[i]) return MP_False;
	    }
	    for (i=fn->size(), ae=size(); i-1!=fa->father.pos; i-=1, ae-=1) {
		if ((*this)[ae-1] != (*fn)[i-1]) return MP_False;
	    }
	    Node* pn;
	    Node* pfn;
	    for (pn=_first,pfn=fn->_first;
		 pfn!=fa;
		 pn=pn->_right,pfn=pfn->_right) {
		if (!pn->match(pfn,fa,an,ab,ae)) return MP_False;
	    }
	    for (pn=_last,pfn=fn->_last;
		 pfn!=fa;
		 pn=pn->_left,pfn=pfn->_left) {
		if (!pn->match(pfn,fa,an,ab,ae)) return MP_False;
	    }
	}
	if (ae-ab == 1) {
	    an = under(ab);
	    if (!an) return MP_False;
	    ab = 0;
	    ae = an->size();
	}
    } else {
	if (!stencil) {
	    if (size() != fn->size()) return MP_False;
	    for (Index i = 0; i != size(); i += 1) {
		if ((*this)[i] != (*fn)[i]) return MP_False;
	    }
	}
	Node* pn;
	Node* pfn;
	for (pn=first(),pfn=fn->first();
	     pfn;
	     pn=pn->right(),pfn=pfn->right()) {
	    if (!pn->match(pfn,fa,an,ab,ae)) return MP_False;
	}
    }
    return MP_True;
}

Node* Node::search(Index prec, Opkind k)
{
    Node *pn = this;
    while ((pn->expr() || pn->id()) &&
	   pn->father->expr() && !pn->right() &&
	   ((prec< pn->father->min_preced()) ||
	    (prec==pn->father->min_preced() && k==LeftBinding)) &&
	   (!pn->_parens || !pn->id())) {
	if (pn->_parens) pn->set_parens(MP_False);
	if (pn->_parens) return pn;
	if (prec==pn->father->min_preced()) return (Node*)pn->father;
	pn = pn->father;
    }
    return pn;
}

Node* Node::notation(Index nnr)
{
    Node *rn = Empty;
    Opkind k;

    de_alias_tree();
    if (IsOp(father(0))) {
	if (size()) return Empty;
	insert(nnr);
    } else if (IsDispOrExpr(above()(0))) {
	Bool par = /*id() &&  */ _parens;
	Node* ne = (size() ? cut(0,size()) : Empty);
	Bool b = MP_False;
	if (par)
	    set_parens(MP_True);
	if (ne) ne->set_parens(MP_False);

	k = stencil_kind(nnr);
	if (k == None) {
	    insert(nnr);
	} else {
	    Index i = 0;
	    _kind = PhNum2Char(MP_Expr,Num(_kind));
	    if (k != Prefix) insert(i++,MP_Expr);
	    insert(i,MP_Op);
	    under(i)->insert(nnr);
	    if (k != Postfix) insert(++i,MP_Expr);
	    if (par) set_parens(MP_True);
	    if (father->expr() && !father->stencil  &&
		(k == Infix
		 ||  k == LeftBinding && !left()
		 ||  k == RightBinding && !right())) {
		for (Node* pn = father->first(); pn; pn = pn->right()) {
		    if (pn->stencil && 
			(which_notation(pn->innr) == which_notation(nnr)))
			b = MP_True;
		}
		if (!b) set_parens(parens()); else b = !parens();
	    }
	    if (father->expr() && (k == LeftBinding && left()  ||
		 k == RightBinding && right())) {
		set_parens(MP_False);
	    }
	}
	if (ne) {
	    Bool replaced = MP_False;
	    Node *nnp[7];
	    Node *pn;
	    Index i;
	    
	    if (ne->_kind == MP_Expr && !ne->stencil && ne->size()==1 &&
		(*ne)[0]==MP_Text) {
		Node *tmpn=ne;
		ne=ne->replace(0,Empty);
		delete tmpn;
	    }
	    for (i=0; i<7; nnp[i++]=Empty);
	    for (pn = first(); pn; pn = pn->right()) {
		i=Ph2Num(pn->_kind);
		if (!nnp[i]) nnp[i]=pn;
		else if (Num(pn->_kind) < Num(nnp[i]->_kind))
		    nnp[i]=pn;
	    }
	    i = Ph2Num(ne->_kind);
	    if (i==Ph2Num(MP_Disp)) i=Ph2Num(MP_Expr);
	    if (!nnp[i]) nnp[i]=nnp[Ph2Num(MP_Text)];
	    if (nnp[i]) {
		pn=nnp[i];
		if (Ph(pn->_kind) != Ph(ne->_kind)) {
		    pn->insert(0,ne->_kind);
		    ne=pn->replace(0,ne);
		    replaced=MP_True;
		} else {
		    pn->de_alias_tree();
		    ne->replaces(pn);
		    if (ne->may_be_raised()) {
			Index ia = ne->father.pos;
			ne->unlink();
			remove(ia,ia+1);
			paste(ia,ne);
		    }
		    replaced = MP_True;
		    ne = pn;
		}
	    }
	    if (replaced || !ne->size())
		delete ne;
	    else
		rn = ne;
	}
	if (b) {
	    Mark m(father);
	    unlink();
	    (void) m->remove(m.pos,m.pos+1);
	    m->paste(m.pos,this);
	}
    }
    return rn;
}

Offset Node::search_new_version()
{
    if (!stencil) return -1;
    Index vnr = which_version_nr(innr);
    NOTATION *nota = which_notation(innr);
    Index i = 0;
    if (!nota) return -1;
    if (vnr+1< (Index) nota->versions) i= vnr+1;
    /* searching for a save version is not necessary. Missing options
    ** are stored in the background.
    while (i!=vnr) {
	Node *pn = first();
	Bool save = MP_True;
	while (pn && save) {
	    if (pn->size() > 0) {
		Bool found = MP_False;
		Index j=0;
		for (; !found && nota->vers[i].format[SCREENFORMAT][j];j++)
		    found |= (pn->above()(0) ==
			      nota->vers[i].format[SCREENFORMAT][j]);
		save &= found;
	    }
	    pn = pn->right();
	}
	if (save)
	    return nota->vers[i].ivnr;
	i++;
	if (i==nota->versions) i=0;
    }
    return -1;
    */
    return nota->vers[i].ivnr;
}

Offset  Node::notation_nr(Offset *vnr)
{
    if (!stencil) return -1;
    *vnr = which_version_nr(innr);
    return which_notation(innr)->innr;
}

Offset Node::id_font()
{
    if (id()) return innr; else return -1;
}

void Node::new_id_font(Offset nfnr)
{
    if (id() && nfnr>=0) {
	changed = changed || ((Index)nfnr)!=innr;
	innr=nfnr;
    }
}

void Node::new_version(Offset nnr)
{
    if (!stencil) {
	if (!id()) return;
	if (nnr<0) nnr = next_id_font(innr);
	if (nnr<0) return;
	changed = changed || innr!=((Index)nnr);
	innr=nnr;
    } else {
	if (nnr<0) nnr = search_new_version();
	if (nnr<0) return;
	if (which_notation(nnr)!=which_notation(innr)) {
	    message(MP_ERROR, translate("Selection changed during version switch."));
	    return;
	}
	innr = (Index) nnr;
	changed = MP_True;
	for (Marker* pm = list; pm; pm = pm->next()) {
	    switch (pm->kind()) {
	    case View:
	    case Left:
	    case Line:
	    case LeftLine:
		pm->pos = 0;
		break;
	    case Right:
		pm->pos = size();
		break;
	    case Tree:
		break;
	    }
	}
	_lines = stencil_lines(innr);
    }
}

void Node::save()
{
    Index si;
    Index bc;
    Node* pn;
    changed = MP_False;
    if (stencil) {
	si=0;
	pn = _first;
	while (pn) {
	    si++;
	    pn = pn->_right;
	}
    } else
	si = size1+size2;
    push_hidden();
    put_struct(NODETYPE, si+7);
    bc = ((id() && innr)<<5) |
	 ((_findnr && !stencil && !(size1+size2)) <<4) |
	 ((_display_pos!=0 && 1)                  <<3) |
         ((_opdelta!=0 && 1)                      <<2) |
	 ((stencil>0 && 1)                        <<1) |
	 (_parens>0 && 1);
    put_char((char)bc+'0');
    put_char(Ph2char(_kind));
    put_Char((Char)Num(_kind)+'0');
    if (bc & (1<<5)) put_Char((Char)(innr + 32));
    if (bc & (1<<4)) put_Char((Char)(_findnr + 32));
    if (bc & (1<<3)) put_Char((Char)(_display_pos+64));
    if (bc & (1<<2)) put_Char((Char)(_opdelta+64));
    if (stencil) {
	Index i = save_stencil(innr);
	put_Char((Char)(i+32));
	pn=_first;
	while (pn) {
	    put_Char(pn->_kind);
	    pop_hidden();
	    pn->save();
	    pn=pn->_right;
	    push_hidden();
	}
    } else {
	Index i = 0;
	pn = _first;
	pop_hidden();
	while (i<size1) {
	    if (IsPh(p1[i])) {
		// pn exists && pn->father.pos == i
                push_hidden();
		put_Char(p1[i]);
		pop_hidden();
		pn->save();
		pn=pn->_right;
	    } else
		put_Char(p1[i]);
	    i++;
	}
	while (i<size1+size2) {
	    if (IsPh(p2[i])) {
		// pn exists  && pn->father.pos == i
                push_hidden();
		put_Char(p2[i]);
		pop_hidden();
		pn->save();
		pn=pn->_right;
	    } else
		put_Char(p2[i]);
	    i++;
	}
	push_hidden();
    }
    put_end_struct();
    pop_hidden();
}

static Node* load_stack=Empty;
static Node* end_stack=Empty;

void Node::clean_up(void)
{
    Node *h;
    Node *g;
    h = load_stack;
    while (h) {
	g = h->_left;
	h->_left = Empty;
	delete h;
	h = g;
    }
    if (end_stack) delete end_stack;
    load_stack = Empty;
    end_stack = Empty;
}

void Node::join_stack(void)
{
    if (load_stack && end_stack) {
	load_stack->paste(load_stack->size(),end_stack);
	end_stack=Empty;
    }
}

void * Node::error_node(Char *errtext, int len)
{
    Index j;
    Node *pn=Empty;
    Bool wasempty=MP_False;
    Bool bad_stack=MP_False;
    if (!errtext || !len || !errtext[0]) return NULL;
    if (!end_stack) end_stack = new Node(MP_Text);
    end_stack->set_gap(0,len);
    end_stack->gap=end_stack->gap-len;
    end_stack->size1=end_stack->size1+len;
    end_stack->p2=end_stack->p2-len;
    j=len;
    pn=end_stack->_first;
    while (pn) {
	pn->father.pos+=len;
	pn=pn->_right;
    }
    pn=end_stack->_first;
    if (!end_stack->_first) {
	wasempty=MP_True;
	end_stack->_first=end_stack->_last=load_stack;
    } else {
	end_stack->_first->_left=load_stack;
	if (load_stack) load_stack->_right=end_stack->_first;
	end_stack->_first=load_stack;
    }
    while (j>0) {
	j--;
	end_stack->p1[j] = errtext[j];
	if (IsPh(errtext[j])) {
	    if (end_stack->_first) {
		end_stack->_first->father = end_stack;
		end_stack->_first->father.pos = j;
		pn = end_stack->_first;
		end_stack->_first = pn->_left;
	    } else {
		bad_stack = MP_True;
		end_stack->p1[j]='?';
	    }
	} else if (IsNewline(errtext[j])) end_stack->_lines++;
    }
    if (end_stack->_first == load_stack) {
	if (load_stack) load_stack->_right = Empty;
	end_stack->_first=pn;
	if (wasempty) end_stack->_last = Empty;
    } else {
	load_stack=end_stack->_first;
	if (load_stack) load_stack->_right = Empty;
	end_stack->_first = pn;
    }
    if (pn) pn->_left = Empty;
    if (bad_stack) {
	Node *w = end_stack->cut(0,len);
	delete w;
	return NULL;
    } else
	return end_stack;
}

void Node::parse_error(Char *errtext, int len)
{
    Offset i;
    Node *h;
    Node *g;
    h=load_stack;
    for (i=0; h && i<len; i++)
	if (IsPh(errtext[i])) {
	    g=h->_left;
	    h->_left = Empty;
	    delete h;
	    h = g;
	}
    if (h) h->_right=Empty;
    load_stack = h;
}

void * Node::parsed_node(Char type, Char *partext, int len, int nnr, int popspace)
{
    Offset j;
    Bool bad_stack=MP_False;
    if (!type || !partext) {
	parse_error(partext,len);
	return NULL;
    }
    _kind = type;
    /* this should be calculated from the subexpressions */ 
    _opdelta = (popspace!=0x3fff? popspace:0);
    switch (Ph(_kind)) {
    case MP_Id: innr = nnr; break;
    case MP_Op:
	switch (nnr) {
	case INTERNAL_BRACES:
	    /*  ( E ) is treated as a special template */
	    _kind = (Char)(_kind ^ MP_Op ^ MP_Expr);
	    _parens = 1;
	    partext[0]=MP_Expr;
	    len=1;
	    break;
	case INTERNAL_EXPRESSION:
	    /*  $E$ is treated as a special template */
	    _kind = (Char)(_kind ^ MP_Op ^ MP_Expr);
	    partext[0]=MP_Expr;
	    len=1;
	    break;
	case INTERNAL_DISPLAY:
	    _kind = (Char)(_kind ^MP_Op ^ MP_Disp);
	    partext[0]=MP_Expr;
	    len=1;
	    break;
	case INTERNAL_TEXT:
	    _kind = (Char)(_kind ^ MP_Op ^ MP_Text);
	    partext[0]=MP_Text;
	    len=1;
	    break;
	default:
	    stencil=1;
	    innr = nnr;
	}
	break;
    default: break;
    }
    Node *pn=Empty;
    _first = _last = load_stack;
    if (stencil) {
	lock_stencil(innr);
	j=len;
	while (j>0) {
	    j--;
	    if (IsPh(partext[j])) {
		if (_first) {
		    pn = _first;
		    _first->_kind = partext[j];
		    _first->father = this;
		    _first = _first->_left;
		} else
		    bad_stack = MP_True;
	    }
	}
    } else {
	set_gap(0,len);
	gap=gap-len;
	size1=size1+len;
	p2=p2-len;
	j=len;
	while (j>0) {
	    j--;
	    p1[j] = partext[j];
	    if (IsPh(p1[j])) {
		if (_first) {
		    _first->father = this;
		    _first->father.pos = j;
		    pn = _first;
		    _first = _first->_left;
		} else
		    bad_stack = MP_True;
	    } else if (IsNewline(p1[j])) _lines++;
	}
	
    }
    if (_first != _last) {
	load_stack = _first;
	if (_first) _first->_right = Empty;
	_first = pn;
	_first->_left = Empty;
    } else
	_first = _last = Empty;
    if (nnr==INTERNAL_DISPLAY) parse_error(partext+1,7);
    if (bad_stack) {
	p1 = p2 = 0;
	size1=size2=gap=0;
	Node *pm = _first;
	while (pm) {
	    Node *h = pm;
	    pm = pm->_right;
	    delete h;
	}
	return NULL;
    } else {
	_left = load_stack;
	if (_left) _left->_right = this;
	_right = Empty;
	if (popspace==0x3fff && Ph(_kind)==MP_Expr) {
	  int i;
	  Node *p;
	  for (i=len-1; i>=0; i--) {
	    if (partext[i]==MP_Expr) {
	      p=under(i);
	      p->set_parens(MP_False);
	      if (p->can_be_raised()) {
		p->unlink();
		remove((Index)i,(Index)(i+1));
		paste(i,p);
	      }
	    }
	  }
	}
	load_stack = this;
	return (void*) this;
    }
}


/* apply equal_diff (kind of Leibnitz) on the last element on the stack */
void Node::apply_leibnitz(void)
{
  Bool make_ident=MP_True;
  if (load_stack && load_stack->expr() && load_stack->size()==3 &&
      IsExpr((*load_stack)[0]) && IsOp((*load_stack)[1]) &&
      IsExpr((*load_stack)[2])) {
    Node *oldleft, *oldright;
    oldleft=load_stack->first();
    oldright=load_stack->last();
    if (oldleft && oldright) {
      Node *newleft, *newright;
      if (!oldleft->equal_diff(oldright, &newleft, &newright)) {
	/* construct node by combining newleft and newright with
	** the operator under load_stack[1]
	*/
	Node *opn;
	set_gap(0,3);
	p1[0]=MP_Expr;
	replace(0,newleft->copy(0,newleft->size()));
	p1[1]=MP_Op;
	opn=load_stack->under(1);
	replace(1,opn->copy(0,opn->size()));
	p1[2]=MP_Expr;
	replace(2,newright->copy(0,newright->size()));
	gap -= 3;
	p2 -=3;
	size1=3;
	_kind=MP_Expr;
	make_ident=MP_False;
      }
    }
  }
  if (make_ident) {
    set_gap(0,3);
    p1[0]=p1[1]=p1[2]='?';
    gap -= 3;
    p2 -= 3;
    size1 = 3;
    _kind=MP_Id;
    stencil=0;
    innr=0;
  }
  _left=load_stack;
  if (_left) _left->_right=this;
  _right=Empty;
  load_stack = this;
}

Offset Node::load(Char *str, int *len, int max)
{
    Offset i,j;
    Bool bad_stack=MP_False;
    if (!*len || !str || !max) return FAILURE;
    if (str[0]<'0' || !codedPh(str[1]) || str[2]<'0') return FAILURE;
    i = str[0]-'0';
    j=3;
    _kind = PhNum2Char(char2Ph(str[1]), (Char)str[2]-'0');
    if (i & (1<<5)) innr = str[j++]-32;
    if (i & (1<<4)) _findnr = (char)(str[j++]-32);
    if (i & (1<<3)) _display_pos = str[j++]-64;
    if (i & (1<<2)) _opdelta = str[j++]-64;
    stencil = (i & (1<<1))>0;
    _parens = (i & 1)>0;
    Node *pn=Empty;
    _first = _last = load_stack;
    if (stencil) {
	innr = load_stencil(str[j++]-32);
	lock_stencil(innr);
	while (j< *len) {
	    j++;
	    if (_first) {
		pn = _first;
		_first->father = this;
		_first = _first->_left;
	    } else
		bad_stack = MP_True;
	}
    } else {
	Index k=0;
	p1 = str;
	size1 = *len-j;
	size2 = 0;
	gap = max-size1;
	p2 = p1+gap;
	_lines = 0;
	while (j < *len) {
	    p1[k] = oldtonew(str[j]);
	    if (IsPh(p1[k])) {
		if (_first) {
		    _first->father = this;
		    _first->father.pos = k;
		    pn = _first;
		    _first = _first->_left;
		} else
		    bad_stack = MP_True;
	    } else if (IsNewline(p1[k])) _lines++;
	    k++;j++;
	}
	if (!bad_stack && (_first != _last)) {
	    Node *pm = pn;
	    Node *pl = _last;
	    Index h;
	    while (pm != pl && pm->_left != pl) {
		h = pm->father.pos;
		pm->father.pos = pl->father.pos;
		pl->father.pos = h;
		pm = pm->_right;
		pl = pl->_left;
	    }
	}
    }
    if (_first != _last) {
	load_stack = _first;
	if (_first) _first->_right = Empty;
	_first = pn;
	_first->_left = Empty;
    } else
	_first = _last = Empty;
    if (bad_stack) {
	p1 = p2 = 0;
	size1=size2=gap=0;
	Node *pm = _first;
	while (pm) {
	    Node *h = pm;
	    pm = pm->_right;
	    delete h;
	}
    } else {
	_left = load_stack;
	if (_left) _left->_right = this;
	_right = Empty;
	load_stack = this;
    }
    if (bad_stack) return FAILURE;
    if (stencil) return SUCCESS+FREE_BUFFER;
    return SUCCESS;
}

Bool Node::old_load(FILE* f)
{
    Index ok, insize, ppos,opsv,prc,cbs;
    if (fscanf(f, "\n(%hx;%u,%u,%u,%u,%u,%d;%u:",
	       &_kind, &ppos, &_parens,&ok,&prc,
	       &opsv,&_opdelta,&insize) == 8
    ) {
	if (opsv > 100) opsv = 0;
	if (_opdelta > 100 || _opdelta< -100)
	    _opdelta = 0;
	if (father && IsDisp(father(0))) {
	    _display_pos = _opdelta;
	    _opdelta = 0;
	}
	_kind |= 0xFF00;
	changed = MP_False;
	if (!set_gap(0,insize)) {
	    return MP_False;
	}
	cbs = MP_True;
	for (Index i = 0; i != insize; i += 1) {
	    p1[i] = fgetRune(f);
	    if (IsNewline(p1[i])) {
		if (!father && p1[i]==SoftNewline)
		    p1[i] = Newline;
	    	_lines++;
	    } else if (IsPh(p1[i])) {
	    	Node* nn = new Node(MP_Text); // Any kind would do.
	    	nn->father = this;
	    	nn->father.pos = i;
	    	nn->_left = _last;
	    	if (nn->_left) nn->_left->_right = nn; else _first = nn;
	    	_last = nn;
		cbs = cbs && Num(p1[i]);
	    }
	}
	size1 = insize;
	gap -= insize;
	p2 -= insize;
	// replace it with a stencil if possible
        if (op()) {
	    if (cbs) {
		Offset j= -1;
		if (size1!=1 || p1[0]!=MP_Expr)
		    j = match_format_or_make(p1, size1, ok, prc, opsv);
		if (j>=0) {
		    lock_stencil(j);
		    innr = j;
		    stencil = MP_True;
		    p1 = p2 = NULL;
		    size1 = size2 = gap = 0;
		    _kind = above()(0);
		}
	    } else
		_kind = above()(0);
	}
	if (id()) _kind = above()(0);
	for (Node* pn = _first; pn; pn = pn->_right)
	    if (!pn->old_load(f)) {
		return MP_False;
	    }
	if (fscanf(f,")")) {
	    return MP_False;
	}
	return MP_True;
    }
    return MP_False;
}

Bool Node::old_old_load(FILE* f)
{
    Index ok, opsv, prc,cbs;
    changed = MP_False;
    _kind = fgetChar(f);
    if (!IsPh(_kind)) return MP_False;
    _parens = fgetBool(f);
    if (_parens > 1) return MP_False;
    prc = fgetIndex(f);
    if (prc >MaxPrecedence) return MP_False;
    Index h = fgetIndex(f);
    if ((h & 0xFFFF) > 6) return MP_False;
    ok = (Opkind) (h & 0xFFFF);
    _opdelta = (h >> 24);
    _opdelta = (_opdelta > 128 ? _opdelta-256 : _opdelta);
    if (_opdelta< -100 || _opdelta>100) return MP_False;
    opsv = (h >> 16) & 0xFF;
    if (opsv >10) return MP_False;
    Index insize = fgetIndex(f);
    if (!set_gap(0,insize)) return MP_False;
    cbs=MP_True;
    for (Index i = 0; i != insize; i += 1) {
	p1[i] = fgetChar(f);
	if (IsNewline(p1[i])) {
	    _lines++;
	} else if (IsPh(p1[i])) {
	    Node* nn = new Node(MP_Text); // Any kind would do.
	    nn->father = this;
	    nn->father.pos = i;
	    nn->_left = _last;
	    if (nn->_left) nn->_left->_right = nn; else _first = nn;
	    _last = nn;
	    cbs = cbs && Num(p1[i]);
	}
    }
    size1 = insize;
    gap -= insize;
    p2 -= insize;
    // replace it with a stencil if possible
    if (op()) {
	if (cbs) {
	    Offset j = match_format_or_make(p1, size1, ok, prc, opsv);
	    if (j>=0) {
		lock_stencil(j);
		innr = j;
		stencil = MP_True;
		p1 = p2 = NULL;
		size1 = size2 = gap = 0;
		_kind = above()(0);
	    }
	} else
	    _kind = above()(0);
    }
    if (id()) _kind = above()(0);
    Node* pn = _first;
    while (pn && pn->old_old_load(f))
	pn = pn->_right;
    if (pn) {
	// clean up
	return MP_False;
    }
    return MP_True;
}

Offset Node::load_ascii(Char *str, int *len, int max)
{
    if (p1) free((char*)p1);
    p1 = str;
    size1 =  *len;
    size2 = 0;
    gap = max - *len;
    p2 = p1+gap;
    Offset i=0;
    _lines=0;
    while (i<*len) {
        p1[i] = oldtonew(str[i]);
	_lines += IsNewline(p1[i]);
	i++;
    }
    _left = load_stack;
    if (_left) _left->_right = this;
    _right = Empty;
    load_stack = this;
    return SUCCESS;
}

void Node::get_stack()
{
    Node *pn = load_stack;
    if (pn) {
	Node *qn = pn->_first;
	_first = pn->_first;
	_last = pn->_last;
	p1 = pn->p1;
	p2 = pn->p2;
	size1 = pn->size1;
	size2 = pn->size2;
	gap = pn->gap;
	stencil = pn->stencil;
	innr = pn->innr;
	changed = MP_False;
	_kind = pn->_kind;
	_findnr = pn->_findnr;
	_parens = pn->_parens;
	_opdelta = pn->_opdelta;
	_display_pos = pn->_display_pos;
	_lines = pn->_lines;
	load_stack = pn->_left; if (load_stack) load_stack->_right = Empty;
	pn->_first = pn->_last = pn->_left = Empty;
	while (qn) {
	    qn->father = this;
	    qn = qn->_right;
	}
	pn->p1 = pn->p2 = NULL;
	pn->size1 = pn->size2 = pn->gap = 0;
	delete pn;
    }
}

static int in_display=0;

Mark Node::latex_line(Index &n)
{
    Char c;
    Char *s = 0;
    Index l = size();

    if (!n) {
	in_display=0;
	return Mark(this,0);
    }
    if (stencil) {
	s = stencil_latex(innr);
	if (s) l = stencil_latex_size(innr);
    }
    for (Index i=0; i!=l; i++) {
	c = (s ? s[i] : (*this)[i]);
	if (!IsPh(c)) {
	    if (IsNewline(c)) 
		n--;
	    else if (c==DisplayOpen) {
		n-= !in_display;
		in_display++;
	    } else if (c==DisplayClose) {
		in_display--;
		n-= !in_display;
	    }
	} else {
	    Node *pn;
	    if (s) {
		pn = _first;
		while (pn && pn->_kind != s[i]) pn=pn->_right;
	    } else
		pn = under(i);
	    if (IsDisp(c)) {
		n-= !in_display;
		in_display++;
	    }
	    if (n && pn) {
		Mark m;
		m = pn->latex_line(n);
		if (!n) return m;
	    }
	    if (n && IsDisp(c)) {
		in_display--;
		n-= !in_display;
	    }
	}
	if (!n) {
	    Index j=0,cn=0;
	    in_display=0;
	    if (!s) return Mark(this,i+1);
	    while (j<=i) {
		cn+=IsNewline(s[j]);
		j++;
	    }
	    j=0;
	    while (cn>0 && j<size()) {
		cn-=IsNewline((*this)[j]);
		j++;
	    }
	    return Mark(this, j);
	}
    }
    return Mark(0,0);
}


void Node::latex(Index begin, Index end)
{
    Char c=0,tmode=0;
    Char* s = 0;
    Index l = size();

    if (text_only) {
	if (l>0) {
	    if (stencil) {
		Index i;
		s=stencil_latex(innr);
		if (!s) s=stencil_screen(innr);
		else l=stencil_latex_size(innr);
		for (i=0; i<l; i++) {
		    if (IsPh(s[i])) {
			Node *pn;
			pn=_first;
			while (pn && pn->_kind != s[i]) pn=pn->_right;
			if (pn) {
			    pn->latex();
			    if (pn->text()) out_latex_char(' ');
			}
		    }
		}
	    } else {
		Index i;
		if (!begin && !end) end=l;
		for (i=begin; i<end; i++) {
		    c=(*this)[i];
		    if (IsPh(c)) {
			Node* pn;
			pn=under(i);
			if (text()) out_latex_char(' ');
			pn->latex();
			if (text()) out_latex_char(' ');
		    } else
			if (text() && (c<256 || IsTab(c))) out_latex_char(c);
		}
	    }
	}
	return;
    }
    if (l == 0) {
	out_latex_char(Ph(kind()));
    } else {
	if (stencil) {
	    tmode = (Char) stencil_latexmode(innr);
	    s = stencil_latex(innr);
	    if (s) l = stencil_latex_size(innr);
	    set_default_thinspace(opspace(MP_True));
	    begin = 0;
	    end = l;
	}
	if (op()) {
	    c = (s ? s[0] : (*this)[0]);
	    if (!IsTab(c) && father->expr()
		&&  IsOp(father(0)) && IsExpr(father(-1)))
		out_latex_char(opspace());
	    if (l==1 && !IsExpr(c) && tmode==LMATHMODE)
		tex_code(SOpOpen);
	    else
		tex_code(LOpOpen);
	} else if (expr())
	    tex_code(ExprOpen);
	else if (id()) {
	    if (!innr) {
		if (l>1 && normal_identifier())
		    tex_code(LIdOpen);
		else
		    tex_code(SIdOpen);
	    } else {
		tex_code(ExprOpen);
		out_latex_char(Font2Char(FontFont, innr));
	    }
	} else if (text())
	    tex_code(TextOpen);
	else if (var())
	    tex_code(VarOpen);
	else if (Ph(kind())==MP_Disp) {
	    set_display_delta(display_delta());
	    tex_code(DispOpen);
	}
	if (tmode) tex_to_mode(tmode);
	if (!begin && !end) end = l;
	for (Index i = begin; i != end; i++) {
	    c = (s ? s[i] : (*this)[i]);
	    if (!IsPh(c)) {
		out_latex_char(c);
	    } else {
		Node* pn;
		if (s) {
		    pn = _first;
		    while (pn && pn->_kind != s[i]) pn = pn->_right;
		} else {
		    pn = under(i);
		}
		if (IsDisp(c)) {
		    set_display_delta((pn?pn->display_delta():0));
		    tex_code(DispOpen);
		}
		if (IsDispOrExpr(c) && pn &&
		    (pn->parens() || (all_parens && !op())))
		  out_latex_char('(');
		if (pn) pn->latex();
		if (stencil) set_default_thinspace(opspace(MP_True));
		if (IsDispOrExpr(c) && pn &&
		    (pn->parens() || (all_parens && !op())))
		  out_latex_char(')');
		if (IsDisp(c)) tex_code(DispClose);
	    }
	}
	if (op()) {
	    if (l==1 && !IsExpr(c) && tmode==LMATHMODE)
		tex_code(SOpClose);
	    else
		tex_code(LOpClose);
	    if (!IsTab(c) && father->expr()
		&&  IsOp(father(0)) && IsExpr(father(1))
		) out_latex_char(opspace());
	} else if (expr())
	    tex_code(ExprClose);
	else if (id()) {
	    if (!innr) {
		if (l>1 && normal_identifier())
		    tex_code(LIdClose);
		else
		    tex_code(SIdClose);
	    } else {
		out_latex_char(Font2Char(PopFont, innr));
		tex_code(ExprClose);
	    }
	} else if (var())
	    tex_code(VarClose);
	else if (text())
	    tex_code(TextClose);
	else if (Ph(kind())==MP_Disp)
	    tex_code(DispClose);
    }
}

// ensure that the gap is at least min_size large and at position pos
Bool Node::set_gap(Index pos, Index min_size)
{
    if (gap < min_size) {
	min_size += size1 + size2;
	Index new_size = Nmin;
	while (new_size<min_size && new_size!=Nmod) new_size *= 2;
	while (new_size<min_size && new_size!=Nmax) new_size += Nmod;
	if (new_size < min_size) return MP_False;
	Char* pc = p1 ? (Char*)realloc((char*)p1, new_size*sizeof(Char))
	              : (Char*)malloc(new_size*sizeof(Char));
	if (!pc) return MP_False;
	memfwd(pc+new_size, pc+size1+size2+gap, pc+size1+gap);
	p2 = (p1 = pc) + (gap = new_size-size1-size2);
    }
    if (pos != size1) {
	if (pos < size1) memfwd(p2+size1, p1+size1, p1+pos);
	else             membwd(p1+size1, p2+size1, p2+pos);
	size2 += size1 - pos;
	size1 = pos;
    }
    return MP_True;
}
