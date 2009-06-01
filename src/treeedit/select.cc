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
// select.cc
// Function definitions for class Select

extern "C" {
#include <stddef.h>
#include <stdio.h>
#include "mathpad.h"
#include "message.h"
#include "memman.h"

#include "unistring.h"

}

#include "mathpad.hh"
#include "mark.hh"
#include "marker.hh"
#include "node.hh"
#include "select.hh"
#include "editwindow.hh"

#include "mathpad.icc"

#define CHARSEL 0
#define WORDSEL 1
#define LINESEL 2
#define ALLSEL  3

static int select_mode=0;
static Bool dbl_clck=MP_False;

Select::Select() : begin(Left), end(Right)
{
    window = 0;
}

Select::~Select()
{ }

Mark& Select::start()
{
  static Mark *startval= new Mark();
  return *startval;
}

Bool Select::make_visible()
{
    if (window)
	return window->make_visible(begin);
    else
	return MP_False;
}

Bool Select::next_node_or_text()
{
    Bool ret_val;
    if (!begin) return MP_False;
    
    if (!(aig(ret_val = (begin.pos==end.pos))))
	begin.pos = end.pos;
    if (!!begin.above())
	begin = end = begin->next_text();
    return ret_val;
}

// move to the next empty expression (in a tree) or insert the character
Bool Select::next_node_or_insert(Char c, Index count)
{
    Bool ret_val;
    if (!begin) return MP_False;
    
    if (!(aig(ret_val = (begin.pos==end.pos))))
	begin.pos = end.pos;
    if (!begin->text()) {
	begin = end = begin->next_text();
    } else {
	begin->insert(begin.pos,c,count);
	begin.pos = end.pos;
    }
    return ret_val;
}

// move to the next expression and insert the character in the text.
Bool Select::next_node_insert(Char c, Index count)
{
    Bool ret_val;
    if (!begin) return MP_False;
    if (!(aig(ret_val = (begin.pos==end.pos))))
	begin.pos = end.pos;
    if (!begin->text()) {
	begin = end = begin->next_text();
	if (!begin->text()) return ret_val;
    }
    begin->insert(begin.pos,c,count);
    begin.pos = end.pos;
    return ret_val;
}

// Insert braces around an expression or insert in an identifier or text.
Bool Select::openparen_insert(Char c, Index count)
{
    Index old_pos = 0;
    Bool ret_val;
    if (!begin) return MP_False;
    if (!(aig(ret_val = (begin.pos == end.pos)))) {
	old_pos = begin.pos;
	begin.pos = end.pos;
    }
    if (begin->expr())
	begin->set_parens(MP_True);
    else if (begin->id()) {
	begin->insert(begin.pos, c, 1);
	begin.pos = end.pos;
    } else if (begin->is_stencil() || begin->op()) {
	if (!ret_val) begin.pos = old_pos;
	ret_val = MP_True;
    } else if (begin->text()) {
	begin->insert(begin.pos,c,count);
	begin.pos = end.pos;
    }
    return ret_val;
}

Bool Select::closeparen_insert(Char c, Index count)
{
    Index old_pos = 0;
    Bool ret_val;
    if (!begin) return MP_False;
    if (!(aig(ret_val = (begin.pos == end.pos)))) {
	old_pos = begin.pos;
	begin.pos = end.pos;
    }
    if (begin->expr() || begin->id()) {
	begin.close_parens();
	if (begin->id() || begin->text()) {
	    begin->insert(begin.pos, c, 1);
	    begin.pos = end.pos;
	} else {
	    if (!!begin->above() && begin->above()->expr()) {
		begin=begin->above();
		end=begin;
		end.pos=end.pos+1;
	    } else {
		end = begin;
		begin.pos = 0;
	    }
	}
	ret_val = MP_False;
    } else if (begin->is_stencil() || begin->op()) {
	if (!ret_val) begin.pos = old_pos;
	ret_val = MP_True;
    } else if (begin->text()) {
	begin->insert(begin.pos,c,count);
	begin.pos = end.pos;
    }
    return ret_val;
}

Bool Select::make_list_insert(Char c, Index count)
{
    Index old_pos = 0;
    Bool ret_val;
    if (!begin) return MP_False;
    if (!(aig(ret_val = (begin.pos == end.pos)))) {
	old_pos = begin.pos;
	begin.pos = end.pos;
    }
    if (begin->var()) {
	insert(MP_Id,1);
	to_right();
    }
    if (begin->id() || begin->expr()) {
	Char kd = begin->kind();
	if (IsExpr(begin.above()(0))) kd = MP_Expr;
	if (begin.above()->var() || begin.above()->text()) {
	    begin = begin.above();
	    begin.pos += 1;
	    end = begin;
	    begin->insert(begin.pos,c,1);
	    to_right();
	    insert(kd,1);
	}
	ret_val = MP_False;
    } else if (begin->op()) {
	if (!ret_val) begin.pos = old_pos;
	ret_val = MP_True;
    } else if (begin->text()) {
	begin->insert(begin.pos,c,count);
	begin.pos = end.pos;
    }
    return ret_val;
}

Bool Select::insert(Char c, Index count)
{
    Index old_pos = 0;
    Bool ret_val;
    if (!begin) return MP_False;
    if (!(aig(ret_val = (begin.pos == end.pos)))) {
	old_pos = begin.pos;
	begin.pos = end.pos;
    }
    if (begin->var() && c!=MP_Id) {
	insert(MP_Id,1);
	to_right();
    }
    if ((begin->op() && begin->size()) ||
	(begin->id() && IsPh(c)) ||
	(begin->expr() && begin->left_of(begin->size()) &&
	 !IsOp(c) && !IsExpr(c)) ||
	(begin->expr() && !begin->size() && IsExpr(c))) {
	if (!ret_val) begin.pos = old_pos;
	ret_val = MP_True;
    } else {
	if (begin->op() && !IsExpr(c)) {
	    begin->insert(0, MP_Expr);
	    begin = begin->under(0);
	    end = begin;
	}
	begin->insert(begin.pos,c,count);
	if (IsPh(c) && FRWindow(window)) {
	    Node* pn = begin.base();
	    Index i = 0;
	    while (i<count) {
		Char t = pn->first_unused_find_nr();
		begin->under(begin.pos+i)->set_find_nr(t);
		i++;
	    }
	}
	if (IsPh(c)) {
	    begin = end = begin->under(begin.pos);
	    begin.pos = end.pos =  0;
	}
	begin.pos = end.pos;
    }
    return ret_val;
}

void Select::set_index_nr(Index c)
{
    if (begin) begin->set_find_nr((Char)c);
}

void Select::insert_string(Char* s)
{
    if (begin && begin->text() || begin->id() || begin->expr()) {
	begin.pos = end.pos;
	begin->insert_string(begin.pos, s);
	if (window) window->redraw_full();
    }
}

void Select::include(FILE* f)
{
    if (!begin || begin.pos != end.pos || !begin->text()) return;
    Node* pn = new Node(MP_Text);
    unsigned char c = (unsigned char)getc(f);
    switch (c) {
    case 0xFF:
	ungetc(c,f);
	if (!pn->old_old_load(f)) {
	    pn->remove(0,pn->size());
	    message(MP_ERROR,translate("File is corrupted: include aborted"));
	    return;
	}
	break;
    case 'A':
	if (!pn->old_load(f)) {
	    pn->remove(0,pn->size());
	    message(MP_ERROR,translate("File is corrupted: include aborted"));
	    return;
	}
	break;
    default:
	message(MP_ERROR,translate("File not in a recognized format."));
	return;
    }
    begin->paste(begin.pos, pn);
    if (begin.base().pos <= window->start_pos().base().pos)
	window->recalc_at_line();
    begin.pos = end.pos;
}

void Select::include_ascii()
{
    Node *pn = Empty;
    pn = new Node(MP_Text);
    pn->get_stack();
    if (!begin || (!begin->text() && begin->kind()!=pn->kind())) {
	scratchwindow().backup(pn);
	return;
    }
    if (begin.pos != end.pos) {
        Node *pm;
	pm = begin->cut(begin.pos, end.pos);
	if (window == &(scratchwindow()))
	    delete pm;
	else
	    scratchwindow().backup(pm);
    }
    begin->paste(begin.pos, pn);
    if (begin.base().pos <= window->start_pos().base().pos)
      window->recalc_at_line();
    if (begin->text()) begin.pos = end.pos;
}

void Select::to_right()
{
    begin.pos = end.pos;
}

Bool Select::remove(Offset count)
{
    if (!begin) return MP_False;
    Bool b = MP_False;
    Bool recalc_line = (window->start_pos().base().pos >= begin.base().pos);
    if (begin.pos == end.pos) {
	if (begin->size() == 0) {
	    if (FRWindow(window) && begin->find_nr()) {
		begin->clear_find_nr();
		return MP_False;
	    }
	    if (!begin.above()) return MP_False;
	    if (begin.above()->op()) return MP_False;
	    begin = end = begin.above();
	    end.pos += 1;
	    b = begin->remove(begin.pos,end.pos);
	} else {
	    Index oldsize = begin->size();
	    b = begin->remove(begin.pos,count);
	    if (begin->size()==oldsize) {
		if (count>0) {
		    if (end.pos<end->size()) end.pos++;
		    else forward(1);
		} else {
		    if (begin.pos) begin.pos--;
		    else backward(1);
		}
	    }
	}
    } else {
	if (begin->expr()) {
	    lower();
	}
	if (window==&scratchwindow())
	    b = begin->remove(begin.pos,end.pos);
	else {
	    Node *pn = Empty;
	    pn = begin->cut(begin.pos, end.pos);
	    if (begin->expr() && IsText(begin->above()->kind())) {
		begin=end=begin.above();
		end.pos++;
		begin->remove(begin.pos, end.pos);
	    }
	    scratchwindow().backup(pn);
	    b = MP_True;
	}
    }
    if (FRWindow(window) && !begin->size() && !!begin.above())
	begin->set_find_nr(begin.base()->first_unused_find_nr());
    if (b && recalc_line) window->recalc_at_line();
    return b;
}

Bool Select::remove_double_chars()
{
    Index t = 0;
    if (!begin || begin.pos!=end.pos || !begin->text()) return MP_False;
    if (begin(0) != begin(-1)) return MP_False;
    while (begin(-1)==end(0)) {
	t++;
	begin.pos--;
    }
    while (begin(0)==end(0)) {
	t++;
	end.pos++;
    }
    end.pos = begin.pos;
    t--;
    return remove(t);
}

Bool Select::kill(Bool front)
{
    if (!begin || begin.pos == end.pos) return MP_False;
    Bool b = MP_False;
    if (begin->expr()) {
	lower();
    }
    Node *pn = Empty;
    pn = begin->cut(begin.pos, end.pos);
    b = (pn->lines() || pn->left_of(pn->size()));
    if (!killnode)
	killnode = new Node(begin);
    if (front)
	killnode->paste(0, pn);
    else
	killnode->paste(killnode->size(), pn);
    return b;
}

static Bool move_word(Mark &p, Offset dir)
{
    Char c;
    Offset os = (dir==-1 ? -1 : 0);
    while (aig(c = p(os)) && (!FromWord(c)))
	p.pos += dir;
    while (aig(c = p(os)) && (FromWord(c)))
	p.pos += dir;
    return (c>0);
}

Bool Select::kill_word(Offset count)
{
    if (!begin) return MP_False;
    if (Ph(begin->kind())!=MP_Text) return MP_True;
    if (count<0) {
	count = -count;
	while (count) {
	    if (!move_word(begin,-1))
		count = 0;
	    else
		count -= 1;
	}
	return kill(MP_True);
    } else {
	while (count) {
	    if (!move_word(end,1))
		count = 0;
	    else
		count -= 1;
	}
	return kill(MP_False);
    }
}

Bool Select::kill_line(Offset count)
{
    if (!begin) return MP_False;
    if (Ph(begin->kind())!=MP_Text) return MP_False;
    Bool return_kill = MP_False;
    Char c;
    if (count<0) {
	count = -count;
	while (count) {
	    count += return_kill;
	    return_kill = MP_False;
	    c = begin(-1);
	    if (IsPh(c) || aig(return_kill = IsNewline(c)))
		begin.pos--;
	    else
		while (c && !IsPh(c) && !IsNewline(c)) {
		    begin.pos--;
		    c = begin(-1);
		}
	    if (c)
		count--;
	    else
		count = 0;
	}
	return kill(MP_True);
    } else {
	while (count) {
	    count += return_kill;
	    return_kill = MP_False;
	    c = end(0);
	    if (IsPh(c) || aig(return_kill = IsNewline(c)))
		end.pos++;
	    else
		while (c && !IsPh(c) && !IsNewline(c)) {
		    end.pos++;
		    c = end(0);
		}
	    if (c)
		count--;
	    else
		count = 0;
	}
	return kill(MP_False);
    }
}

void Select::yank()
{
    Bool remove_first;

    if (!begin || !lastkilled) return;
    remove_first =  (begin.pos != end.pos);
    Char tkr = 0;
    if (remove_first && !!begin.above())
	tkr = begin.above()(0);
    if (begin->op() && IsOp(tkr) && !lastkilled->op()) return;
    if (begin->id() && IsId(tkr) && !lastkilled->id()) return;
    if ((begin->op() || begin->id()) && IsExpr(tkr) &&
	!lastkilled->expr() && !lastkilled->op() && !lastkilled->id()) return;
    if (begin->var() && !lastkilled->var() && !lastkilled->id()) return;
    if (begin->expr() && !lastkilled->expr() && !lastkilled->op() && !lastkilled->id()) return;
    if (remove_first) {
	Node *pn = Empty;
	if (begin->expr()) lower();
	pn = begin->cut(begin.pos, end.pos);
	if (pn)
	    if (window == &scratchwindow())
		delete pn;
	    else
		scratchwindow().backup(pn);
    }
    Node* n = lastkilled->copy(0,lastkilled->size());
    if (FRWindow(window)) {
	Char c = (Char)(begin.base()->last_used_find_nr()+1);
	n->set_find_nr_rec(c);
    }
    begin->paste(begin.pos,n);
    if (begin->may_be_raised()) raise();
    if (Ph(begin->kind())==MP_Text) begin.pos = end.pos;
}

void Select::commute()
{
    if (!!begin) {
	begin->commute(begin.pos,end.pos);
    }
}

Bool Select::func_selected(Select &f, Select& a)
{
    return (!!begin && begin.pos != end.pos &&
	    !!f.begin && f.begin.pos != f.end.pos &&
	    !!a.begin && Ph(begin->kind())!=MP_Text &&
	    Ph(f.begin->kind())!=MP_Text);
}

Bool Select::distribute(Select& f, Select& a)
{
    if (!func_selected(f,a)) return MP_False;
    Node* an = a.begin;
    Mark mf;
    Mark ma;
    Index ab = a.begin.pos;
    Index ae = a.end.pos;
    Node* fa = Empty;
    Node* fn = f.begin->fcopy(f.begin.pos,f.end.pos,an,ab,ae,fa);
    if (!fa || !(begin->distribute(begin.pos, end.pos, fn,fa,mf,ma))) {
	delete fn;
	return MP_False;
    }
    if (FRWindow(window) && !FRWindow(f.window)) {
	fn->clear_find_nr_rec();
	Char c = (Char)(begin.base()->last_used_find_nr()+1);
	fn->set_find_nr_rec(c);
    }
    if (&a == this) {
	Node* n;
	n = begin->cut(begin.pos,end.pos);
	n->unlink();
	if (f.begin.pos == 0 && f.end.pos == f.end->size()) {
	    n->replaces(f.begin);
	    delete (Node*)f.begin;
	} else {
	    delete f.begin->cut(f.begin.pos,f.end.pos);
	    f.begin->insert(f.begin.pos,MP_Expr);
	    f.begin->replace(f.begin.pos,n);
	}
	f.begin = f.end = mf;
	a.begin = a.end = ma;
	f.begin.pos = a.begin.pos = 0;
	f.end.pos = f.end->size();
	a.end.pos = a.end->size();
    }
    delete fn;
    return MP_True;
}

Bool Select::factorise(Select& f, Select& a)
{
    if (!func_selected(f,a)) return MP_False;
    Node* an = a.begin;
    Index ab = a.begin.pos;
    Index ae = a.end.pos;
    Node* fa = Empty;
    Node* fn = f.begin->fcopy(f.begin.pos,f.end.pos,an,ab,ae,fa);
    if (!fa || fa==fn) {
	delete fn;
	return MP_False;
    }
    begin->factorise(begin.pos, end.pos, fn, fa);
    if (FRWindow(window) && !FRWindow(f.window)) {
	fn->clear_find_nr_rec();
	Char c = (Char)(begin.base()->last_used_find_nr()+1);
	fn->set_find_nr_rec(c);
    }
    Node* en = Empty;
    if (begin.pos==0 && end.pos==end->size()) {
	en = begin;
	begin = end = end.above();
	end.pos += 1;
	en->unlink();
    } else {
	en = begin->cut(begin.pos,end.pos);
	begin->insert(begin.pos,MP_Expr);
    }
    en->replaces(fa);
    a.begin = a.end = en;
    a.begin.pos = 0;
    a.end.pos = a.end->size();
    begin->replace(begin.pos,fn);
    f.begin = f.end = begin;
    f.end.pos = f.begin.pos+1;
    delete fa;
    return MP_True;
}

void Select::apply(Select& f, Select& a)
{
    if (!func_selected(f,a)) return;
    Node* an = a.begin;
    Index ab = a.begin.pos;
    Index ae = a.end.pos;
    Node* fa = Empty;
    Node* fn = f.begin->fcopy(f.begin.pos,f.end.pos,an,ab,ae,fa);
    if (!fa) {
	delete fn;
	return;
    }
    if (FRWindow(window) && !FRWindow(f.window)) {
	fn->clear_find_nr_rec();
	Char c = (Char)(begin.base()->last_used_find_nr()+1);
	fn->set_find_nr_rec(c);
    }
    Node* en = Empty;
    if (begin.pos==0 && end.pos==end->size() && !!begin.above()) {
	begin = end = begin.above();
	end.pos += 1;
	en = begin->replace(begin.pos,Empty);
    } else {
	en = begin->cut(begin.pos,end.pos);
	begin->insert(begin.pos,MP_Expr);
    }
    if (!en) return;
    en->replaces(fa);
    begin->replace(begin.pos,fn);
}

void Select::rename(Select& n, Select& s)
{
    if (!begin || !n.begin) return;
    if (!begin->id() || !n.begin->id()) return;
    if (!!s.begin && window != s.window) return;
    Bool unset_s = MP_False;
    if (!s.begin) {
	if (!!begin.above() && begin.above()->var()) {
	    s.begin = begin.above().above();
	    s.begin.pos = 0;
	    s.end = end.above().above();
	    s.end.pos = s.end->size();
	} else {
	    s.begin = begin;
	    s.end = end;
	}
	unset_s = MP_True;
    }
    Char *oldname = begin->copy_name();
    Char *newname = n.begin->copy_name();
    if (oldname && newname)
	s.begin->rename(s.begin.pos, s.end.pos,
			oldname, Ustrlen(oldname),
			newname, Ustrlen(newname));
    if (oldname) delete_copied_name(oldname);
    if (newname) delete_copied_name(newname);
    if (unset_s) s.begin = s.end = Empty;
}

void Select::forward(Index n)
{
    if (!begin || !n) return;
    while (n) {
	const Char c = begin(0);
	if (begin->text()) {
	    if (begin.pos != end.pos)
		begin.pos = end.pos;
	    else {
		if (IsPh(c)) {
		    if (begin->under(begin.pos)->op()) {
			Node *m=begin->under(begin.pos)->first();
			while (!!m && !m->text()) m = m->right();
			if (m) {
			    begin = m;
			    begin.pos=0;
			    end=begin;
			} else {
			    begin = begin->under(begin.pos);
			    begin.pos=0;
			    end=begin;
			    end.pos=end->size();
			}
		    } else if (begin->under(begin.pos)->expr() ||
			       begin->under(begin.pos)->id() ||
			       begin->under(begin.pos)->var()) {
			begin = begin->under(begin.pos);
			begin.pos=0;
			end=begin;
			end.pos=end->size();
		    } else if (begin->under(begin.pos)->text()) {
			begin = begin->under(begin.pos);
			begin.pos=0;
			end=begin;
		    }
		} else if (!c) {
		    while (!begin->right() && !!begin->above() &&
			   !begin->above()->text())
			begin=begin->above();
		    if (!!begin->above()) {
			if (begin->above()->text() || !begin->right()) {
			    begin=begin->above();
			    begin.pos++;
			    end=begin;
			} else {
			    begin=begin->right();
			    begin.pos=0;
			    end=begin;
			    if (!begin->text()) end.pos=end->size();
			}
		    } else n=1;
		} else {
		    begin.pos++;
		    end.pos++;
		}
	    }
	} else if (begin->id() && begin.pos==end.pos && c) {
	    begin.pos++;
	    end.pos++;
	} else {
	    Mark m(begin);
	    int d=0;
	    if (begin.pos!=end.pos && (begin.pos!=0 || end.pos !=end->size()))
		m = end->left_of(end.pos);
	    while (!m->right() && !!m->above() && !m->above()->text()) {
		d++;
		m=m->above();
	    }
	    if (!!m->above() && m->above()->text()) {
		m=m->above();
		m.pos++;
		begin=end=m;
	    } else {
		if (m->right()) m=m->right();
		while (m->right() && !IsExpr(m->above()(0)) &&
		       !IsId(m->above()(0)))
		    m=m->right();
		while (d) {
		    if (!m->first()) d=0;
		    else if (m->first()->expr() || m->first()->id()) {
			m=m->first();
			d--;
		    } else d=0;
		}
		if (m->text()) {
		    m.pos=0;
		    begin=end=m;
		} else {
		    m.pos=0;
		    begin=end=m;
		    end.pos=end->size();
		}
	    }
	}
	n--;
    }
}

void Select::backward(Index n)
{
    if (!begin || !n) return;
    while (n) {
	const Char c = begin(-1);
	if (begin->text()) {
	    if (begin.pos != end.pos)
		end.pos = begin.pos;
	    else {
		if (IsPh(c)) {
		    if (begin->under(begin.pos-1)->op()) {
			Node *m=begin->under(begin.pos-1)->last();
			while (!!m && !m->text()) m = m->left();
			if (m) {
			    begin = m;
			    begin.pos=m->size();
			    end=begin;
			} else {
			    begin = begin->under(begin.pos-1);
			    begin.pos=0;
			    end=begin;
			    end.pos=end->size();
			}
		    } else if (begin->under(begin.pos-1)->expr() ||
			       begin->under(begin.pos-1)->id() ||
			       begin->under(begin.pos-1)->var()) {
			begin = begin->under(begin.pos-1);
			begin.pos=0;
			end=begin;
			end.pos=end->size();
		    } else if (begin->under(begin.pos-1)->text()) {
			begin = begin->under(begin.pos-1);
			begin.pos=0;
			end=begin;
		    }
		} else if (!c) {
		    while (!begin->left() && !!begin->above() &&
			   !begin->above()->text())
			begin=begin->above();
		    if (!!begin->above()) {
			if (begin->above()->text() || !begin->left()) {
			    begin=begin->above();
			    end=begin;
			} else {
			    begin=begin->left();
			    begin.pos=begin->size();
			    end=begin;
			    if (!begin->text()) begin.pos=0;
			}
		    } else n=1;
		} else {
		    begin.pos--;
		    end.pos--;
		}
	    }
	} else if (begin->id() && begin.pos==end.pos && c) {
	    begin.pos--;
	    end.pos--;
	} else {
	    Mark m(begin);
	    int d=0;
	    if (begin.pos!=end.pos && (begin.pos!=0 || end.pos !=end->size()))
		m = begin->right_of(begin.pos);
	    while (!m->left() && !!m->above() && !m->above()->text()) {
		d++;
		m=m->above();
	    }
	    if (!!m->above() && m->above()->text()) {
		m=m->above();
		begin=end=m;
	    } else {
		if (m->left()) m=m->left();
		while (m->left() && !IsExpr(m->above()(0)) &&
		       !IsId(m->above()(0)))
		    m=m->left();
		while (d) {
		    if (!m->last()) d=0;
		    else if (m->last()->expr() || m->last()->id()) {
			m=m->last();
			d--;
		    } else d=0;
		}
		if (m->text()) {
		    m.pos=m->size();
		    begin=end=m;
		} else {
		    m.pos=0;
		    begin=end=m;
		    end.pos=end->size();
		}
	    }
	}
	n--;
    }
}

void Select::forward_line(Index n)
{
    if (!begin) return;
    Char c;
    if (begin->text()) {
	while (n) {
	    do {
		end.pos += 1;
		c = end(-1);
	    } while (c && !IsNewline(c));
	    if (!c) {
		n=0;
		end.pos = end->size();
	    } else
		n -= 1;
	}
	begin.pos = end.pos;
    } else down();
}

void Select::backward_line(Index n)
{
    if (!begin) return;
    Char c;
    if (begin->text()) {
	while (n) {
	    do {
		begin.pos -= 1;
		c = begin(0);
	    } while (c && !IsNewline(c));
	    if (!c) {
		begin.pos = 0;
		n=0;
	    } else
		n -= 1;
	}
	end.pos = begin.pos;
    } else
	up();
}

void Select::begin_of_line()
{
    if (!begin) return;
    Char c;
    while (aig(c = begin(-1)) && !IsNewline(c)) begin.pos -= 1;
    end.pos = begin.pos;
}

void Select::end_of_line()
{
    if (!end) return;
    Char c;
    while (aig(c = end(0)) && !IsNewline(c)) end.pos += 1;
    begin.pos = end.pos;
}

void Select::recenter()
{
    window->recenter(begin);
}

void Select::move_to_center()
{
    window->set_center(begin);
    end = begin;
    end.pos = begin.pos;
}

void Select::begin_of_buffer()
{
    if (!begin) return;
    begin = begin.base();
    begin.pos = 0;
    end = begin;
}

void Select::end_of_buffer()
{
    if (!begin) return;
    begin = begin.base();
    begin.pos = begin->size();
    end = begin;
}

void Select::forward_word(Offset n)
{
    if (!begin) return;
    if (begin.pos == 0 && end.pos == end->size() && !!end.above())
	begin = end = end.above();
    int dir;
    if (n<0) {
	n = -n;
	dir = -1;
    } else {
	dir = 1;
	if (begin.pos != end.pos)
	    begin.pos = end.pos;
    }
    while (n) {
	if (!move_word(begin, dir))
	    n = 0;
	else
	    n -= 1;
    }
    end.pos = begin.pos;
}

Bool Select::transpose_chars(Index n)
{
     if (!begin) return MP_False;
     Bool newline_exp_trans = begin->transpose_chars(begin.pos, n);
     end.pos = begin.pos;
     return newline_exp_trans;
}

Bool Select::transpose_words(Index n)
{
    if (!begin) return MP_False;
    Bool retval = MP_False;
    Mark bp = begin;
    move_word(bp,-1);
    Mark ep = bp;
    move_word(ep,1);
    Mark eq = ep;
    Mark bq = eq;
    while (n) {
	if (!move_word(eq,1))
	    n = 0;
	else {
	    bq = eq;
	    move_word(bq, -1);
	    retval |= bp->transpose_words(bp.pos, ep.pos, bq.pos, eq.pos);
	    ep = eq;
	    bp = bq;
	    n -= 1;
	}
    }
    begin.pos = end.pos = eq.pos;
    return retval;
}

void Select::dbl_click()
{
    dbl_clck = MP_True;
    if (!begin) begin = end = start();
    up();
}

void Select::no_dbl_click()
{
    dbl_clck = MP_False;
}

void Select::up()
{
    if (!begin) return;
    if (begin.pos == 0 && end.pos == end->size()) {
	if (!!begin.above()) {
	    begin = end = begin.above();
	    if (!begin->text()) {
		begin.pos = 0;
		end.pos = end->size();
	    } else end.pos++;
	}
	select_mode = CHARSEL;
    } else if (begin->text()) {
	switch (select_mode) {
	case CHARSEL:
	    select_mode = WORDSEL;
	    if (begin(0) && FromWord(begin(0)) &&
		begin(-1) && FromWord(begin(-1))) move_word(begin,-1);
	    if (end(0) && FromWord(end(0)) &&
		((end(-1) && FromWord(end(-1))) || begin.pos==end.pos))
		move_word(end,1);
	    if (begin.pos == end.pos)
		if (end.pos<end->size()) end.pos++;
		else if (begin.pos) begin.pos--;
	    break;
	case WORDSEL:
	    select_mode = LINESEL;
	    while (begin(-1) && !IsNewline(begin(-1))) begin.pos--;
	    while (end(0) && !IsNewline(end(0))) end.pos++;
	    break;
	case LINESEL:
	    select_mode = ALLSEL;
	    begin.pos = 0;
	    end.pos = end->size();
	    break;
	case ALLSEL:
	    select_mode = CHARSEL;
	    // reselect begin and end
	    break;
	default:
	    break;
	}
    } else {
	begin.pos = 0;
	end.pos = end->size();
	select_mode = CHARSEL;
    }
}

void Select::down()
{
    if (!begin) return;
    if (begin.pos == 0 && end.pos == end->size()) {
	if (begin->text() || begin->id()) {
	    end.pos=0;
	} else {
	    if (begin->first()) {
		begin = begin->first();
		begin.pos=0;
		end=begin;
		end.pos=end->size();
	    }
	}
    } else {
	Node* n = begin->right_of(begin.pos);
	if (!n || n->above().pos>end.pos) return;
	begin = end = n;
	begin.pos = 0;
	end.pos = n->size();
    }
}

void Select::into()
{
    if (!begin) return;
    Node* n = begin->under(begin.pos);
    if (!n) return;
    begin = end = n;
    begin.pos = 0;
    end.pos = n->size();
}

Select& Select::operator = (const Select& s)
{
    begin = s.begin;
    end = s.end;
    window = s.window;
    return *this;
}

Select& Select::operator = (const Mark& m)
{
    begin = end = m;
    return *this;
}

void Select::unset()
{
  begin = end = Empty;
}

void Select::set_begin()
{
    if (window) {
	begin = end = window->base();
	begin.pos = end.pos = 0;
    }
}

void Select::restore(const Mark& m)
{
    if (dbl_clck) return;
    select_mode = CHARSEL;
    start() = begin = end = m;
    if (m->op()) {
	if (m->opkind() == None || !m.above()->expr()) {
	    begin.pos = 0;
	    end.pos = m->size();
	} else {
	    begin = end = m.above();
	    end.pos += 1;
	    if (m->opkind() == RightBinding) {
		end.pos = end->size();
	    } else if (m->opkind() != Postfix) {
		Char c;
		while (aig(c = end(0))) {
		    end.pos += 1;
		    if (Char2Ph(c) == MP_Expr) break;
		}
	    }
	    if (m->opkind() == LeftBinding) {
		begin.pos = 0;
	    } else if (m->opkind() != Prefix) {
		Char c;
		while (aig(c = begin(-1))) {
		    begin.pos -= 1;
		    if (Char2Ph(c) == MP_Expr) break;
		}
	    }
	}
    } else if (m->id()) {
	begin.pos = 0;
	end.pos = m->size();
    }
}

void Select::restore(Mark m1, Mark m2)
{
    m1 = start();
    Index d1 = m1->depth();
    Index d2 = m2->depth();
    while (d1 > d2) { m1 = m1.above(); d1 -= 1; }
    while (d2 > d1) { m2 = m2.above(); d2 -= 1; }
    while ((Node*)m1 != (Node*)m2) {
	m1 = m1.above();
	m2 = m2.above();
    }
    if (m1->op() || m1->id()) {
	m1.pos = 0;
	m2.pos = m2->size();
    }
    if (m1.pos < m2.pos) {
	begin = m1;
	end = m2;
    } else {
	begin = m2;
	end = m1;
    }
    // if (!end->text() && IsPh(end(0))) end.pos += 1;
    if (end(0)) end.pos+=1;
    switch (select_mode) {
    case WORDSEL:
	if (begin(0) && FromWord(begin(0)) &&
	    begin(-1) && FromWord(begin(-1))) move_word(begin,-1);
	if (end(-1) && FromWord(end(-1)) &&
	    end(0) && FromWord(end(0))) move_word(end,1);
	break;
    case LINESEL:
	while (begin(-1) && !IsNewline(begin(-1))) begin.pos--;
	while (end(0) && !IsNewline(end(0))) end.pos++;
	break;
    case ALLSEL:
	begin.pos = 0;
	end.pos = end->size();
	break;
    default:
	break;
    }
}

void Select::insert_stencil(Index innr)
{
    Node *pn = Empty;

    if (!begin) return;
    if (begin.pos==end.pos && (begin->id() || 
			       (begin->expr() && begin->size()))) {
	begin = begin->search(stencil_prec(innr), stencil_kind(innr));
	begin.pos = 0;
	end = begin;
	end.pos = end->size();
    }
    if (begin->text()) {
	pn = begin->cut(begin.pos, end.pos);
	insert(MP_Expr);
	into();
	begin->insert(0,MP_Text);
	pn = begin->replace(0,pn);
	delete pn;
    } else {
	lower();
    }
    if (aig(pn = begin->notation(innr)))
	if (window == &scratchwindow())
	    delete pn;
	else
	    scratchwindow().backup(pn);
    if (FRWindow(window)) {
	Mark pt;
	if (begin.pos == end.pos)
	    pt = begin.under();
	else
	    pt = begin;
	pn = begin.base();
	Node *h = pt->under(0);
	Char c;
	if (!h) h = pt->right_of(0);
	while (h) {
	    if (!h->size() && !h->find_nr()) {
		c = pn->first_unused_find_nr();
		h->set_find_nr(c);
	    }
	    h = pt->right_of((h->above()).pos);
	}
    }
    /* if (begin->number_empty()) { */
      begin = end = begin.under()->next_node(); 
      /* } else {
      if (begin.under()->op()) {
	end =begin;
	end.pos=begin.pos+1;
      } else {
	begin = end = begin.under();
	begin.pos=0;
	end.pos=end->size();
      }
    } */
}

int  Select::notation_nr(Offset *vnr)
{
    if (!begin) return -1;
    if (begin->id()) return -1;
    if (begin->op() && begin.pos==0 && end.pos==end->size())
	return begin->notation_nr(vnr);
    else {
	Index i=begin.pos;
	while (i<end.pos && !IsOp(begin(i-begin.pos))) i++;
	if (IsOp(begin(i-begin.pos)))
	    return begin->under(i)->notation_nr(vnr);
	else if (!!begin->above() && begin->above()->op())
	    return begin->above()->notation_nr(vnr);
	return -1;
    }
}

int Select::id_font()
{
    if (!begin) return -1;
    if (begin->id())
	return begin->id_font();
    else return -1;
}

void Select::new_id_font(Index nfnr)
{
    if (!begin) return;
    if (begin->id()) begin->new_id_font(nfnr);
}

void Select::new_version(Index nnnr)
{
    if (!begin) return;
    if (begin->op() || begin->id())
	begin->new_version(nnnr);
    else {
	Index i=begin.pos;
	while (i<end.pos && !IsOp(begin(i-begin.pos))) i++;
	if (IsOp(begin(i-begin.pos))) {
	    Node *pn;
	    pn=begin->under(i);
	    pn->new_version(nnnr);
	} else if (!!begin->above() && begin->above()->op())
	    begin->above()->new_version(nnnr);
    }
}

void Select::set_old()
{
    oldbegin = begin;
    oldend = end;
}

void Select::unset_old()
{
    oldbegin = oldend = Empty;
}

void Select::copy_old(Select &sl)
{
  oldbegin=sl.oldbegin;
  oldend=sl.oldend;
}

void Select::update()
{
    if (!!begin && begin->is_stencil()) {
	begin.pos=0;
	end.pos=end->size();
    }
}

Mark & Select::stackbegin(int n)
{
  static Mark *stackbeginval = new Mark[256];
  return stackbeginval[n];
}

Mark & Select::stackend(int n)
{
  static Mark *stackendval = new Mark[256];
  return stackendval[n];
}

static Index stackdepth=0;

void Select::stack_position()
{
    if (stackdepth<256) {
	stackbegin(stackdepth) = begin;
	stackend(stackdepth++) = end;
    }
}

void Select::use_stack()
{
    if (stackdepth) {
	oldbegin = begin;
	oldend = end;
	begin = stackbegin(--stackdepth);
	end = stackend(stackdepth);
    }
}

void Select::clear_stack()
{
    while (stackdepth>0) {
	stackbegin(--stackdepth) = 0;
	stackend(stackdepth) = 0;
    }
}

void Select::clear_stack_and_use()
{
    if (stackdepth) {
	oldbegin = begin;
	oldend = end;
	begin = stackbegin(0);
	end = stackend(0);
    }
    while (stackdepth>0) {
	stackbegin(--stackdepth) = 0;
	stackend(stackdepth) = 0;
    }
}

void Select::latex_line(Index &n)
{
    Mark m = window->latex_line(n);
    if (m) {
	oldbegin = begin;
	oldend = end;
	begin = m;
	end = m;
    }
}

void Select::latex()
{
    if (begin.pos != end.pos)
	begin->latex(begin.pos,end.pos);
}

Bool Select::find_tree(Node *n)
{
    if (!begin) return MP_False;
    Node *nn = n;
    while (nn->size()==1 && nn->left_of(1)) nn = nn->left_of(1);
    oldbegin = begin;
    oldend = end;
    begin.cond_right(1);
    begin.find_tree(nn);
    if (!begin) {
	begin = oldbegin;
	end = oldend;
	return MP_False;
    } else {
	end = begin;
	if (nn->size()) end.pos+=nn->size(); else end.pos=end->size();
	return MP_True;
    }
}

void Select::clear_replacestop()
{
    begin.base()->clear_find_nr_rec();
}

static int replace_all_call=0;

void Select::replace_tree(Node *oldn, Node *newn)
{
    Node *on = oldn;
    Node *nn = newn;
    while (on->size()==1 && on->left_of(1)) on = on->left_of(1);
    if (!on->size()) return;
    while (nn->size()==1 && nn->left_of(1)) nn = nn->left_of(1);
    while (nn && Ph(nn->kind())!= Ph(on->kind()) &&
	   (!nn->above() || !on->above() ||
	    Ph(nn->above()(0))!=Ph(on->above()(0))))
	nn = nn->above();
    if (!nn) {
	message(MP_ERROR,translate("Sorry, don't know how to replace these constructions."));
	return;
    }
    Node* pn = begin->replace_tree(on, nn, begin.pos);
    if (pn)
	if (window==&scratchwindow())
	    delete pn;
	else
	    scratchwindow().backup(pn);
    if (replace_all_call && nn->text()) {
	if (nn->size()>1)
	    begin.cond_right(nn->size()-1);
	end = begin;
	if (nn->size())
	    forward(1);
    }
}

void Select::replace_tree_all(Node *oldn, Node *newn)
{
    replace_all_call=1;
    replace_tree(oldn, newn);
    replace_all_call=0;
}

Bool Select::find(Char *str)
{
    int n = Ustrlen(str);
    Bool irt;

    if (!begin) return MP_False;
    oldbegin = begin;
    oldend = end;
    irt = begin.find(str,n);
    if (!begin) {
	begin = oldbegin;
	end = oldend;
	return MP_False;
    } else {
	end = begin;
	if (!irt) end.pos+=n;
	return !irt;
    }
}

Bool Select::findnext(Char *str)
{
    int n = Ustrlen(str);
    Bool irt;

    if (!begin) return MP_False;
    oldbegin = begin;
    oldend = end;
    begin.cond_right(n);
    irt = begin.find(str,n);
    if (!begin) {
	begin = oldbegin;
	end = oldend;
	return MP_False;
    } else {
	end = begin;
	if (!irt) end.pos+=n;
	return !irt;
    }
}

Bool Select::findwrap(Char *str)
{
    int n = Ustrlen(str);
    Bool irt;

    if (!begin) return MP_False;
    oldbegin = begin;
    oldend = end;
    begin = begin.base();
    begin.pos = 0;
    irt = begin.find(str,n);
    if (!begin) {
	begin = oldbegin;
	end = oldend;
	return MP_False;
    } else {
	end = begin;
	if (!irt) end.pos+=n;
	return !irt;
    }
}

Bool Select::findback(Char *str)
{
    int n = Ustrlen(str);
    Bool irt;

    if (!begin) return MP_False;
    oldbegin = begin;
    oldend = end;
    irt = begin.findback(str,n);
    if (!begin) {
	begin = oldbegin;
	end = oldend;
	return MP_False;
    } else {
	end = begin;
	if (!irt) end.pos+=n;
	return !irt;
    }
}

Bool Select::findprev(Char *str)
{
    int n = Ustrlen(str);
    Bool irt;

    if (!begin) return MP_False;
    oldbegin = begin;
    oldend = end;
    begin.cond_left(1);
    irt = begin.findback(str,n);
    if (!begin) {
	begin = oldbegin;
	end = oldend;
	return MP_False;
    } else {
	end = begin;
	if (!irt) end.pos+=n;
	return !irt;
    }
}

Bool Select::findwrapback(Char *str)
{
    int n = Ustrlen(str);
    Bool irt;

    if (!begin) return MP_False;
    oldbegin = begin;
    oldend = end;
    begin = begin.base();
    begin.pos = begin->size();
    irt = begin.findback(str,n);
    if (!begin) {
	begin = oldbegin;
	end = oldend;
	return MP_False;
    } else {
	end = begin;
	if (!irt) end.pos+=n;
	return !irt;
    }
}

Bool Select::find_replace(const Select& s, Char *str)
{
    int n = Ustrlen(str);
    Bool irt;

    if (!begin) return MP_False;
    oldbegin = begin;
    oldend = end;
    end = begin;
    if (!!s.begin && s.window==window) {
	begin = end = s.begin;
    }
    irt = begin.find(str,n);
    if (!!begin) {
	end = begin;
	if (!irt) end.pos += n;
    }
    if (!begin || (!!s.begin && !s.contains(*this))) {
	begin = oldbegin;
	end = oldend;
	return MP_False;
    } else {
	return !irt;
    }
}

Bool Select::findnext_replace(const Select& s, Char *str)
{
    int n = Ustrlen(str);
    Bool irt;

    if (!begin) return MP_False;
    oldbegin = begin;
    oldend = end;
    begin.cond_right(n);
    irt = begin.find(str,n);
    if (!!begin) {
	end = begin;
	if (!irt) end.pos +=n;
    }
    if (!begin || (!!s.begin && !s.contains(*this))) {
	begin = oldbegin;
	end = oldend;
	return MP_False;
    } else
	return !irt;
}

void Select::replace(Char *oldstr, Char *newstr)
{
    Index n = Ustrlen(oldstr);
    Index m = Ustrlen(newstr);
    Node *pn = Empty;

    pn = begin->replacestr(oldstr, n, newstr, m, begin.pos);
    if (pn)
	if (window==&scratchwindow())
	    delete pn;
	else
	    scratchwindow().backup(pn);
    if (m>1) begin.cond_right(m-1);
    end = begin;
    if (m) forward(1);
}

Bool Select::find_stencil(Index nnr)
{
    Bool irt;

    if (!begin) return MP_False;
    oldbegin = begin;
    oldend = end;
    irt = begin.find_stencil(nnr);
    if (!begin) {
	begin = oldbegin;
	end = oldend;
	return MP_False;
    } else {
	end = begin;
	if (!irt) end.pos=end->size();
	return !irt;
    }
}

Bool Select::findnext_stencil(Index nnr)
{
    Bool irt;

    if (!begin) return MP_False;
    oldbegin = begin;
    oldend = end;
    begin.cond_right(1);
    irt = begin.find_stencil(nnr);
    if (!begin) {
	begin = oldbegin;
	end = oldend;
	return MP_False;
    } else {
	end = begin;
	if (!irt) end.pos = end->size();
	return !irt;
    }
}

Bool Select::findwrap_stencil(Index nnr)
{
    Bool irt;

    if (!begin) return MP_False;
    oldbegin = begin;
    oldend = end;
    begin = begin.base();
    begin.pos = 0;
    irt = begin.find_stencil(nnr);
    if (!begin) {
	begin = oldbegin;
	end = oldend;
	return MP_False;
    } else {
	end = begin;
	if (!irt) end.pos = end->size();
	return !irt;
    }
}

Bool Select::findback_stencil(Index nnr)
{
    Bool irt;

    if (!begin) return MP_False;
    oldbegin = begin;
    oldend = end;
    irt = begin.findback_stencil(nnr);
    if (!begin) {
	begin = oldbegin;
	end = oldend;
	return MP_False;
    } else {
	end = begin;
	if (!irt) end.pos = end->size();
	return !irt;
    }
}

Bool Select::findprev_stencil(Index nnr)
{
    Bool irt;

    if (!begin) return MP_False;
    oldbegin = begin;
    oldend = end;
    begin.cond_left(1);
    irt = begin.findback_stencil(nnr);
    if (!begin) {
	begin = oldbegin;
	end = oldend;
	return MP_False;
    } else {
	end = begin;
	if (!irt) end.pos= end->size();
	return !irt;
    }
}

Bool Select::findwrapback_stencil(Index nnr)
{
    Bool irt;

    if (!begin) return MP_False;
    oldbegin = begin;
    oldend = end;
    begin = begin.base();
    begin.pos = begin->size();
    irt = begin.findback_stencil(nnr);
    if (!begin) {
	begin = oldbegin;
	end = oldend;
	return MP_False;
    } else {
	end = begin;
	if (!irt) end.pos=end->size();
	return !irt;
    }
}

Bool Select::find_replace_stencil(const Select& s, Index nnr)
{
    Bool irt;

    if (!begin) return MP_False;
    oldbegin = begin;
    oldend = end;
    end = begin;
    if (!!s.begin && s.window==window) {
	begin = end = s.begin;
    }
    irt = begin.find_stencil(nnr);
    if (!!begin) {
	end = begin;
	if (!irt) end.pos = end->size();
    }
    if (!begin || (!!s.begin && !s.contains(*this))) {
	begin = oldbegin;
	end = oldend;
	return MP_False;
    } else {
	return !irt;
    }
}

Bool Select::findnext_replace_stencil(const Select& s, Index nnr)
{
    Bool irt;

    if (!begin) return MP_False;
    oldbegin = begin;
    oldend = end;
    begin.cond_right(1);
    irt = begin.find_stencil(nnr);
    if (!!begin) {
	end = begin;
	if (!irt) end.pos = end->size();
    }
    if (!begin || (!!s.begin && !s.contains(*this))) {
	begin = oldbegin;
	end = oldend;
	return MP_False;
    } else
	return !irt;
}

void Select::replace_notation(Index oldnnr, Index newnnr)
{
    Node *pn = Empty;
    pn = begin->replace_notation(oldnnr, newnnr);
    if (pn)
	if (window==&scratchwindow())
	    delete pn;
	else
	    scratchwindow().backup(pn);
    begin.cond_right(1);
    end = begin;
    forward(1);
}

/* void Select::select_line(const Mark &st, Mark& s, Mark& e, int nr)
{
    if (!begin) return;
    s = begin;
    e = end;
    while (nr-- && s!=st) s.esrevart();
    while (!!s.above() && s!=st && !IsNewline(s.esrevart()));
    e.nextline();
    if (e.above()) {
	e = e.base();
	e.pos += 1;
    }
    if (!e(0))
	e.pos = e->size()+1;	
} */

Bool Select::select_line(Mark& s, Mark& e)
{
    if (!begin && !oldbegin) {
	s = Empty;
	e = Empty;
	return MP_False;
    }
    if (!begin) {
	s = oldbegin;
	e = oldend;
	return MP_True;
    }
    if (!oldbegin) {
	s = begin;
	e = end;
	return MP_True;
    }
    Mark pm = begin;
    Mark pn = oldbegin;
    if (pn!=pm) {
	int k,l,i = pm->depth(), j = pn->depth();
	k=i; l=j;
	while (i>j) { pm = pm.above(); i--; }
	while (j>i) { pn = pn.above(); j--; }
	while (!!pn && !!pm && ((Node*)pm != (Node*)pn)) {
	    pm = pm->above();
	    pn = pn->above();
	}
	if (pm.pos<pn.pos || (pm.pos==pn.pos && l>k)) s = begin;
	else s = oldbegin;
	if (oldend==end)
	    if (pm.pos<pn.pos || (pm.pos==pn.pos && l>k)) e = oldbegin;
	    else e = begin;
    } else
	if (oldend==end) e = begin;
    pm = end;
    pn = oldend;
    if (pn!=pm) {
	int k,l,i = pm->depth(), j = pn->depth();
	k=i;l=j;
	while (i>j) { pm = pm.above(); i--; }
	while (j>i) { pn = pn.above(); j--; }
	while (!!pn && !!pm && ((Node*)pm != (Node*)pn)) {
	    pm = pm->above();
	    pn = pn->above();
	}
	if (pm.pos<pn.pos || (pm.pos==pn.pos && k<l)) e = oldend;
	else e = end;
	if (oldbegin==begin)
	    if (pm.pos<pn.pos || (pm.pos==pn.pos && k<l)) s = end;
	    else s = oldend;
    } else
	if (oldbegin==begin) s = end;
    return MP_True;
}

Bool Select::test_wrap(const Mark& m) const
{
    if (begin == m && end == m) {
	Mark k=begin;
	Char c;
	while ( aig(c=k(-1)) && c!=' ' && !IsNewline(c)) k.pos--;
	if (c==' ') {
	    k->change_to(Newline,k.pos-1);
	    return MP_True;
	}
    }
    return MP_False;
}


static int smaller(const Mark &l, const Mark &r)
{
  Mark pl = l;
  Mark pr = r;
  if (!l) return 0;
  if (!r) return 0;
  if (pl != pr) {
    int kl,kr,i = pl->depth(), j = pr->depth();
    kl=i; kr=j;
    while (i>j) { pl = pl.above(); i--; }
    while (j>i) { pr = pr.above(); j--; }
    while (!!pr && !!pl && ((Node*)pl != (Node*)pr)) {
      pr = pr->above();
      pl = pl->above();
    }
    if (!!pr && !!pl && (pl.pos<pr.pos || (pl.pos==pr.pos && kl<kr))) return 1;
    else return 0;
  } else {
    return 0;
  }
}

void Select::noxor_sel(void)
{
  if (!begin && !oldbegin) return;

  if (smaller(oldend, begin)) {
    Mark swp;
    swp = oldend; oldend=begin; begin=swp;
  }
  if (smaller(oldend, end)) {
    Mark swp;
    swp = oldend; oldend=end; end=swp;
  }
  if (smaller(oldbegin, begin)) {
    Mark swp;
    swp = oldbegin; oldbegin=begin; begin=swp;
  }
  if (smaller(oldbegin, end)) {
    Mark swp;
    swp = oldbegin; oldbegin=end; end=swp;
  }
  if (!!begin) {
    Mark m;
    Char bc;
    int ignore=0;
    m=begin;
    do {
      bc = begin.esrevart();
      if (IsImportantClose(bc)) ignore++;
      else if (IsImportantOpen(bc)) ignore--;
    } while (bc && (ignore>0 || IsOpCode(bc)));
    if (!begin) {
      begin = m.base();
      begin.pos=0;
    }
  }
  if (!!oldend) {
    Mark m;
    Char bc;
    int ignore=0;
    m=oldend;
    do {
      bc = oldend.traverse();
      if (IsImportantClose(bc)) ignore--;
      else if (IsImportantOpen(bc)) ignore++;
    } while (bc && (ignore>0 || IsOpCode(bc)));
    if (!oldend) {
      oldend = m.base();
      oldend.pos=oldend->size();
    }
  }
  if (!!end && (!oldbegin || smaller(end, oldbegin))) {
    Mark m;
    Char bc;
    int ignore=0;
    m=end;
    do {
      bc = end.traverse();
      if (IsImportantClose(bc)) ignore--;
      else if (IsImportantOpen(bc)) ignore++;
    } while (bc && (ignore>0 || IsOpCode(bc)));
    if (!end) {
      end = m.base();
      end.pos=end->size();
    }
  }
  if (!!oldbegin && (!end || smaller(end, oldbegin))) {
    Mark m;
    Char bc;
    int ignore=0;
    m=oldbegin;
    do {
      bc = oldbegin.esrevart();
      if (IsImportantClose(bc)) ignore++;
      else if (IsImportantOpen(bc)) ignore--;
    } while (bc && (ignore>0 || IsOpCode(bc)));
    if (!oldbegin) {
      oldbegin = m.base();
      oldbegin.pos=0;
    }
  }
  if ((end == oldbegin) || smaller(oldbegin,end)) {
    end=oldend;
    oldbegin=Empty;
    oldend=Empty;
  }
}


void Select::test_begin(const Mark& m, Cpfv cb) const
{
    if (begin    == m) (*cb)();
    if (oldbegin == m) (*cb)();
}

void Select::test_begin_before(const Mark& m, Cpfv cb) const
{
    Mark ma = m;
    if (ma.traverse()) {
      if (begin    == ma) (*cb)();
      if (oldbegin == ma) (*cb)();
    }
}

void Select::test_end(const Mark& m, Cpfv cb) const
{
    if (end      == m) (*cb)();
    if (oldend   == m) (*cb)();
}

void Select::test_end_after(const Mark& m, Cpfv cb) const
{
    Mark ma = m;
    if (ma.esrevart()) {
      if (end      == ma) (*cb)();
      if (oldend   == ma) (*cb)();
    }
}

Bool Select::test_noxor(const Mark &m)
{
  // return weither mark m is visible (True) or invisible (False)
  return (!!begin && smaller(begin, m) && !!end && smaller(m,end)) ||
    (!!oldbegin && smaller(oldbegin, m) && !!oldend && smaller(m,oldend));
}

Bool Select::test(const Mark& m)
{
    return (begin==m && end==m && begin.pos==end.pos) ^
	(oldbegin==m && oldend==m && oldbegin.pos==oldend.pos);
}

Bool overlap(const Select& s1, const Select& s2)
{
    Mark b1(s1.begin), e1(s1.end);
    Mark b2(s2.begin), e2(s2.end);
    Index d1 = b1->depth();
    Index d2 = b2->depth();
    while (d1 > d2) { e1 = b1 = b1.above(); e1.pos += 1; d1 -= 1; }
    while (d2 > d1) { e2 = b2 = b2.above(); e2.pos += 1; d2 -= 1; }
    return (Node*)b1==(Node*)b2 && b1.pos<e2.pos && b2.pos<e1.pos;
}

/* Construct a path to indicate where the sub selection is.
** Each integer indicates which sub-expression is chosen
** in each subnode, where binary trees are simulated.
** That is, the rose tree for a+b+c+d is equal to
**    ((a+b)+c)+d    (+ is left associative)
**    a+(b+(c+d))    (+ is right associative)
** If left or right is not specified, left associative is used
** Due to the selection features, it is possible to select b+c
** in the example above, where the binary structure wouldn't
** allow it. In that case, the selection is extend such that
** b+c is part of it, that is a+b+c when left associative and
** b+c+d when right associative.  Some example paths for
** this example:
**    expression      left           right
**        c           [1,2]          [2,2,1]
**        a           [1,1,1]        [1]
**       a+b          [1,1]          []
**       b+c          [1]            [2]
**       c+d          []             [2,2]
** A more complex example:  a + b + c*d!*e + f + g  (+* left, ! postfix)
**    expression      selection
**    c*d!            [1,1,2,1]
**    d               [1,1,2,1,2,1]
*/
int  Select::select_path(const Select &sub, int *list, int max)
{
  if (!sub || !contains(sub))
    return -1;
  Mark m;
  int i,spos, epos,len;
  i=max-1;
  m=sub.begin;
  spos=sub.begin.pos;
  epos=sub.end.pos;
  while (i && !!m) {
    if (m->text()) {
      if ((Node*)m==(Node*)begin) {
	list[i]=m.pos-begin.pos;
      } else {
	list[i]=m.pos;
      }
      i--;
    } else if (m->id()) {
      /* skip */ ;
    } else if (m->op()) {
      if (spos==epos) {
	list[i]=Num(m(0));
	i--;
      }
    } else if (m->expr()) {
      int opleft,opright;
      int leftass, rightass,prefix,postfix;
      int p;
      int sp, ep;
      opleft=opright=leftass=rightass=prefix=postfix=0;
      if ((Node*)m==(Node*)begin) {
	sp=begin.pos;
	ep=end.pos;
      } else {
	sp=0;
	ep=m->size();
      }
      Node *n;
      n=(Node*)m;
      for (p=sp; p<ep; p++) {
	if (IsOp((*n)[p])) {
	  Opkind k;
	  if (p<spos)
	    opleft++;
	  else if (p>=epos)
	    opright++;
	  k=m->under(p)->opkind();
	  switch (k) {
	  case Infix:
	  case LeftBinding: leftass++; break;
	  case RightBinding: rightass++; break;
	  case Prefix: prefix++; break;
	  case Postfix: postfix++; break;
	  default: break;
	  }
	}
      }
      if (prefix || postfix) {
	list[i]=1;
	i--;
      } else if (leftass>rightass) {
	if (spos==epos && spos!=sp) {
	  list[i]=2;
	  i--;
	}
	while (i && opright) {
	  list[i]=1;
	  i--;
	  opright--;
	}
      } else {
	if (spos==epos && epos!=ep-1) {
	  list[i]=1;
	  i--;
	}
	while (i && opleft) {
	  list[i]=2;
	  i--;
	  opleft--;
	}
      }
    }
    if ((Node*)m==(Node*)begin) {
      m=Empty;
    } else {
      m=m.above();
    }
    spos=epos=m.pos;
  }
  len=max-i-1;
  for (i=0; i<len; i++) {
    list[i]=list[max-len+i];
  }
  return len;
}

Bool Select::contains(const Select& s) const
{
    if (window != s.window) return MP_False;
    Mark b(s.begin), e(s.end);
    Index d = b->depth();
    const Index h = begin->depth();
    while (d > h) { e = b = b.above(); e.pos += 1; d -= 1; }
    return (Node*)b==(Node*)begin && begin.pos<=b.pos && e.pos<=end.pos;
}

Bool Select::contains(const Mark& m) const
{
    Bool b = MP_False;
    if (!!begin) {
	int i = m->depth();
	int j = begin->depth();
	int k=i;
	if (i>=j) {
	    Mark pm=m;
	    while (i>j) { pm = pm.above(); i--; }
	    if ((Node*)pm == (Node*)begin)
		b = (begin.pos<pm.pos && k==j && pm.pos<=end.pos) ||
		    (begin.pos<=pm.pos && k>j && pm.pos<end.pos);
	}
    }
    if (!!oldbegin) {
	int i = m->depth();
	int j = oldbegin->depth();
	int k = i;
	if (i>=j) {
	    Mark pm=m;
	    while (i>j) { pm = pm.above(); i--; }
	    if ((Node*)pm == (Node*)oldbegin)
		b ^= (oldbegin.pos<pm.pos && k==j && pm.pos<=oldend.pos) ||
		    (oldbegin.pos<=pm.pos && k>j && pm.pos<oldend.pos);
	}
    }
    return b;
}

void copy(Select& t, Select& s)
{
    Node *pn = Empty;
    Bool remove_first, sct, tcs;

    if (!t.begin || !s.begin) return;
    sct = s.contains(t);
    tcs = t.contains(s);
    if ((!sct && !tcs && overlap(s,t)) || (sct && tcs))	return;
    remove_first =  (t.begin.pos != t.end.pos);
    Char skr = 0;
    if (!!s.begin.above()) skr = s.begin.above()(0);
    Char tkr = 0;
    if (!!t.begin.above()) tkr = t.begin.above()(0);
    if (t.begin->op() && IsOp(tkr) && !IsOp(skr)) return;
    if (t.begin->id() && IsId(tkr) && !s.begin->id()) return;
    if ((t.begin->op() || t.begin->id()) && IsExpr(tkr) &&
	!s.begin->expr() && !s.begin->op() && !s.begin->id() &&
	!t.begin.above()->text()) return;
    if (t.begin->var() && !s.begin->var() && !s.begin->id()) return;
    if (t.begin->expr() && !s.begin->expr() &&
	!s.begin->op() && !s.begin->id()) return;
    Node* n = s.begin->copy(s.begin.pos,s.end.pos);
    if (n->text() && !t.begin->text()) {
	t.begin=t.begin.above();
	t.end=t.begin;
	t.end.pos++;
    }
    if (remove_first) {
	if (t.begin->expr()) t.lower();
	pn = t.begin->cut(t.begin.pos, t.end.pos);
	if (pn)
	    if (t.window == &scratchwindow())
		delete pn;
	    else
		scratchwindow().backup(pn);
    }
    if (FRWindow(t.window) && !FRWindow(s.window)) {
	n->clear_find_nr_rec();
	Char c = (Char)(t.begin.base()->last_used_find_nr()+1);
	n->set_find_nr_rec(c);
    }
    t.begin->paste(t.begin.pos,n);
    if (t.begin->may_be_raised()) t.raise();
    if (t.begin->text() && !s.begin->text()) {
	t.begin = t.begin->under(t.begin.pos);
	t.begin.pos=0;
	t.end=t.begin;
	t.end.pos=t.end->size();
    }
}

void swap(Select& s1, Select& s2)
{
    if (!s1.begin || !s2.begin) return;
    if (overlap(s1,s2)) return;
    if (Ph(s1.begin->kind()) != Ph(s2.begin->kind())) {
	if (IsText(s1.begin->kind())) {
	    if (s1.end.pos==s1.begin.pos+1 &&
		(Ph(s1.begin(0)) == Ph(s2.begin->kind()) ||
		 (!!s2.begin.above() &&
		  Ph(s1.begin(0)) == Ph(s2.begin.above()(0))))) {
		s1.begin=s1.begin.under();
		s1.begin.pos=0;
		s1.end=s1.begin;
		s1.end.pos=s1.end->size();
	    } else
		if (!!s2.begin.above() && IsText(s2.begin.above()->kind())) {
		    s2.begin=s2.begin.above();
		    s2.end=s2.begin;
		    s2.end.pos++;
		}
	} else 	if (IsText(s2.begin->kind())) {
	    if (s2.end.pos==s2.begin.pos+1 &&
		(Ph(s2.begin(0)) == Ph(s1.begin->kind()) ||
		 (!!s1.begin.above() &&
		  Ph(s2.begin(0)) == Ph(s1.begin.above()(0))))) {
		s2.begin=s2.begin.under();
		s2.begin.pos=0;
		s2.end=s2.begin;
		s2.end.pos=s2.end->size();
	    } else
		if (!!s1.begin.above() && IsText(s1.begin.above()->kind())) {
		    s1.begin=s1.begin.above();
		    s1.end=s1.begin;
		    s1.end.pos++;
		}
	}
    }
    if (Ph(s1.begin->kind()) != Ph(s2.begin->kind())) {
	if (!s1.begin.above() || !s2.begin.above()) return;
	if (Ph(s1.begin.above()(0)) != Ph(s2.begin.above()(0))) return;
    }
    if (s1.begin->expr()) s1.lower();
    if (s2.begin->expr()) s2.lower();
    Node* n1 = s1.begin->cut(s1.begin.pos,s1.end.pos);
    Node* n2 = s2.begin->cut(s2.begin.pos,s2.end.pos);
    if (FRWindow(s1.window) && !FRWindow(s2.window)) {
	n2->clear_find_nr_rec();
	Char c = (Char)(s1.begin.base()->last_used_find_nr()+1);
	n2->set_find_nr_rec(c);
    }
    if (FRWindow(s2.window) && !FRWindow(s1.window)) {
	n1->clear_find_nr_rec();
	Char c = (Char)(s2.begin.base()->last_used_find_nr()+1);
	n1->set_find_nr_rec(c);
    }
    s1.begin->paste(s1.begin.pos,n2);
    s2.begin->paste(s2.begin.pos,n1);
    if (s1.begin->may_be_raised()) s1.raise();
    if (s2.begin->may_be_raised()) s2.raise();
}

void Select::lower()
{
    if (!begin) return;
    if (begin.pos == 0 && end.pos == end->size()) return;
    if (!begin->text() && !begin->expr()) return;
    Node* n = begin->cut(begin.pos,end.pos);
    begin->insert(begin.pos,Ph(begin->kind()));
    delete begin->replace(begin.pos,n);
    begin = end = n;
    begin.pos = 0;
    end.pos = n->size();
}

void Select::raise()
{
    if (!begin) return;
    if (begin.pos != 0 || end.pos != end->size()) return;
    Node* n = begin;
    if (!n->can_be_raised()) return;
    begin = end = begin.above();
    end.pos += 1;
    n->unlink();
    begin->remove(begin.pos,end.pos);
    begin->paste(begin.pos,n);
}

void Select::set_parens()
{
    if (!begin) return;
    if (begin.pos == 0 && end.pos == end->size()) {
	begin->set_parens(MP_True);
    } else if (end.pos - begin.pos == 1) {
	Node* n = begin.under();
	if (n) n->set_parens(MP_True);
    } else if (Ph(begin->kind()) == MP_Expr) {
	Node* n = begin->cut(begin.pos, end.pos);
	if (!n) return;
	begin->insert(begin.pos,MP_Expr);
	begin->replace(begin.pos,n);
	n->set_parens(MP_True);
	begin = end = n;
	begin.pos = 0;
	end.pos = n->size();
    }
}

void Select::unset_parens()		
{
    if (!begin) return;
    if (begin.pos == 0 && end.pos == end->size()) {
	begin->set_parens(MP_False);
    } else if (end.pos - begin.pos == 1) {
	Node* n = begin.under();
	if (n) n->set_parens(MP_False);
    }
}

void Select::display_left(Index n)
{
    if (!begin) return;
    begin->display_left(n);
}

void Select::display_right(Index n)
{
    if (!begin) return;
    begin->display_right(n);
}

void Select::increase_spacing(Index count)
{
    if (!begin) return;
    if (begin.pos == 0 && end.pos == end->size() && begin->op()) {
	begin->increase_spacing(count);
    } else {
	Node *n;
	n=begin->first();
	while (n && n->above().pos<end.pos) {
	    if (n->above().pos>=begin.pos && n->op() && IsOp(n->above()(0)))
		n->increase_spacing(count);
	    n=n->right();
	}
    }
}

void Select::reset_spacing()
{
    if (!begin) return;
    if (begin.pos == 0 && end.pos == end->size() && begin->op()) {
	begin->reset_spacing();
    } else {
	Node *n;
	n=begin->first();
	while (n && n->above().pos<end.pos) {
	    if (n->above().pos>=begin.pos && n->op() && IsOp(n->above()(0)))
		n->reset_spacing();
	    n=n->right();
	}
    }
}

void Select::decrease_spacing(Index count)
{
    if (!begin) return;
    if (begin.pos == 0 && end.pos == end->size() && begin->op()) {
	begin->decrease_spacing(count);
    } else {
	Node *n;
	n=begin->first();
	while (n && n->above().pos<end.pos) {
	    if (n->above().pos>=begin.pos && n->op() && IsOp(n->above()(0)))
		n->decrease_spacing(count);
	    n=n->right();
	}
    }
}

void Select::clear_parens()
{
    if (!begin) return;
    if (begin.pos == 0 && end.pos == end->size()) {
	begin->clr_parens();
    } else if (end.pos - begin.pos == 1) {
	Node* n = begin.under();
	if (n) n->clr_parens();
    }
}

void Select::switch_parens()
{
    if (!begin) return;
    if (begin.pos == 0 && end.pos == end->size()) {
	begin->set_parens(!begin->parens());
    } else if (end.pos - begin.pos == 1) {
	Node* n = begin.under();
	if (n) n->set_parens(!n->parens());
    } else if (Ph(begin->kind()) == MP_Expr) {
	Node* n = begin->cut(begin.pos, end.pos);
	if (!n) return;
	begin->insert(begin.pos,MP_Expr);
	begin->replace(begin.pos,n);
	n->set_parens(MP_True);
	begin = end = n;
	begin.pos = 0;
	end.pos = n->size();
    }
}

void Select::select_all(void)
{
    begin = end = window->base();
    begin.pos = 0;
    end.pos = end->size();
}

void Select::combine(const Select& s)
{
    if (window != s.window) return;
    Mark b1(begin), b2(s.begin), e1(end), e2(s.end);
    Index d1 = b1->depth();
    Index d2 = b2->depth();
    while (d1>d2) {
	b1 = e1 = b1.above();
	if (IsPh(e1(0))) e1.pos +=1;
	d1 -= 1;
    }
    while (d2>d1) {
	b2 = e2 = b2.above();
	if (IsPh(e2(0))) e2.pos +=1;
	d2 -= 1;
    }
    while ((Node*)b1 != (Node*)b2) {
	b1 = e1 = b1.above();
	if (IsPh(e1(0))) e1.pos +=1;
	b2 = e2 = b2.above();
	if (IsPh(e2(0))) e2.pos +=1;
    }
    if (b1->op() || b1->id()) {
	b1.pos =0;
	e1.pos = e1->size();
    } else {
	if (b2.pos < b1.pos) b1.pos = b2.pos;
	if (e1.pos < e2.pos) e1.pos = e2.pos;
    }
    begin = b1;
    end = e1;
}
