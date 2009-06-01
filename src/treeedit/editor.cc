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
// editor.cc
// The C heart of the editor, the interface to the rest of the world.
// Declarations introduce names in the appropriate scopes
// Definitions allocate storage or give the body of functions

// Declarations of functions with C linkage
extern "C" {
#include <stdio.h>
#include "mathpad.h"
#include "output.h"
#include "keyboard.h"
#include "editor.h"
#include "fileread.h"
}

#include "mathpad.hh"
#include "mark.hh"
#include "marker.hh"
#include "node.hh"
#include "select.hh"
#include "editwindow.hh"

#include "mathpad.icc"


// Variables

// Variables in dynamic libraries will not get initialized.
// Instead, a function with a static variable is used.

// Primary
Select& ps() { static Select *psval = new Select(); return *psval; }
// Secondary
Select& ss() { static Select *ssval = new Select(); return *ssval; }
// Tertiary
Select& ts() { static Select *tsval = new Select(); return *tsval; }
// Old position Primary
Select& ops() { static Select *opsval = new Select(); return *opsval; }
Select* cs; // Current (in a few contexts)
// Find and replace stack (text with text nodes )
Node& fr_stack() { static Node *frval = new Node(MP_Text); return *frval; }
Index stackpos=0;
static Cpfv ccf; // Cursor function switch_{reverse,thick,thin} for *cs
static Cpfv cscf;
Bool use_xor=MP_False;


EditWindow& miniwindow()
{ static EditWindow *med = new EditWindow(0,0,0); return *med; }
EditWindow& findwindow()
{ static EditWindow *fed = new EditWindow(0,0,0); return *fed; }
EditWindow& replacewindow()
{ static EditWindow *red = new EditWindow(0,0,0); return *red; }
EditWindow& scratchwindow()
{ static EditWindow *sed = new EditWindow(0,0,0); return *sed; }

Node *killnode = Empty;
Node *lastkilled = Empty;

static Mark& m1() { static Mark *m1val = new Mark(); return *m1val; } ;
static Mark& m2() { static Mark *m2val = new Mark(); return *m2val; };

// The C functions

static void end_kill_sequence()
{
    if (killnode) {
	if (lastkilled)
	    scratchwindow().backup(lastkilled);
	lastkilled = killnode;
	killnode = Empty;
    }
}

static void visible_redraw()
{
    if (!ps().make_visible())
	ps().window->redraw_cursor(&(ps()),switch_reverse);
    else
	ps().window->redraw_full();
}

// 1 For keypresses ...

// 1.1 Movement ...
void forward_char(Index n)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().forward(n);
    visible_redraw();
}

void backward_char(Index n)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().backward(n);
    visible_redraw();
}

void up(Index button)
{
    switch (button) {
    case 2: cs = &(ss()); ccf = switch_thick;   break;
    case 3: cs = &(ts()); ccf = switch_thin;    break;
    case 1:
    default: cs = &(ps()); ccf = switch_reverse; break;
    }
    if (!*cs) return;
    end_kill_sequence();
    cs->up();
    cs->window->redraw_cursor(cs,ccf);
}

void up_selection(void *selection)
{
  Select *sel=(Select*) selection;
  if (sel) {
    sel->up();
  }
}

void down(Index)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().down();
    visible_redraw();
}

void forward_line(Index n)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().forward_line(n);
    visible_redraw();
}

void backward_line(Index n)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().backward_line(n);
    visible_redraw();
}

void begin_of_line(void)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().begin_of_line();
    visible_redraw();
}

void end_of_line(void)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().end_of_line();
    visible_redraw();
}

void display_left(Index n)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().display_left(n);
    if (ps().window) ps().window->redraw_full();
}

void display_right(Index n)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().display_right(n);
    if (ps().window) ps().window->redraw_full();
}

void scroll_up(Index n)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().window->scroll_up(n);
}

void scroll_down(Index n)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().window->scroll_down(n);
}

void recenter(void)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().recenter();
    ps().window->redraw_full();
}

void move_to_center(void)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().move_to_center();
    ps().window->redraw_cursor(&(ps()), switch_reverse);
}

void begin_of_buffer(void)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().begin_of_buffer();
    visible_redraw();
}

void end_of_buffer(void)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().end_of_buffer();
    visible_redraw();
}

void forward_word(Index n)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().forward_word(n);
    visible_redraw();
}

void backward_word(Index n)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().forward_word(-(int)n);
    visible_redraw();
}

void transpose_chars(Index n)
{
    if (!(ps())) return;
    end_kill_sequence();
    if (!ps().transpose_chars(n))
	ps().window->redraw_line(n+1);
    else
	ps().window->redraw_full();
}

void transpose_words(Index n)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().transpose_words(n);
    ps().window->redraw_full();

}

void upcase_word(void)
{
}

void downcase_word(void)
{
}

// 1.2 Insertions

extern Char KEY2Char(KeyNum kn, KeyMode km);

static void con_insert(Char c, Index &count)
{
    end_kill_sequence();
    if (!ps().insert(c,count) && ps().window) {
	ps().window->redraw_cursor(&(ps()), switch_reverse);
	count += 10;
    }
}

void next_node_or_insert(int c, Index count)
{
  Char h = (Char)c;
    end_kill_sequence();
    if (!ps().next_node_or_insert(h,count) && ps().window) {
	ps().window->redraw_cursor(&(ps()), switch_reverse);
    }
    if (ps().window)
	if (IsNewline(h)) {
	    ps().make_visible();
	    ps().window->redraw_full();
	} else {
	    ps().window->redraw_cursor(&(ps()), switch_reverse);
	    ps().window->redraw_line(count+saved_chars+1);
	}
}

void next_node_or_text(Index count)
{
    end_kill_sequence();
    if (!ps().next_node_or_text() && ps().window) {
	ps().window->redraw_cursor(&(ps()), switch_reverse);
    }
    if (ps().window) {
	ps().window->redraw_cursor(&(ps()), switch_reverse);
	ps().window->redraw_line(count+saved_chars+1);
    }
}

void next_node_insert(int c, Index count)
{
    Char h = (Char)c;
    end_kill_sequence();
    if (!ps().next_node_insert(h,count) && ps().window) {
	ps().window->redraw_cursor(&(ps()), switch_reverse);
    }
    if (ps().window)
	if (IsNewline(h)) {
	    ps().make_visible();
	    ps().window->redraw_full();
	} else {
	    ps().window->redraw_cursor(&(ps()), switch_reverse);
	    ps().window->redraw_line(count+saved_chars+1);
	}
}

void openparen_insert(int, Index count)
{
    end_kill_sequence();
    if (!ps().openparen_insert('(',count) && ps().window) {
	ps().window->redraw_cursor(&(ps()), switch_reverse);
    }
    if (ps().window)
	ps().window->redraw_line(count+saved_chars+1);
}

void closeparen_insert(int, Index count)
{
    end_kill_sequence();
    if (!ps().closeparen_insert(')',count) && ps().window) {
	ps().window->redraw_cursor(&(ps()), switch_reverse);
    }
    if (ps().window)
	ps().window->redraw_line(count+saved_chars+1);
}

void make_list_insert(int c, Index count)
{
    Char h = (Char)c;
    end_kill_sequence();
    if (!ps().make_list_insert(h,count) && ps().window) {
	ps().window->redraw_cursor(&(ps()), switch_reverse);
    }
    if (ps().window)
	if (IsNewline(h)) {
	    ps().make_visible();
	    ps().window->redraw_full();
	} else
	    ps().window->redraw_line(count+saved_chars+1);
}

void insert_char(int c, Index count)
{
    Char h = (Char)c;
    con_insert(h,count);
    if (ps().window && !more_keys())
	if (IsNewline(h)) {
	    ps().make_visible();
	    ps().window->redraw_full();
	} else
	    ps().window->redraw_line(count+saved_chars+1);
}

void insert_symbol(int c, Index count)
{
    con_insert((Char)c, count);
    if (ps().window && !more_keys())
	ps().window->redraw_line(saved_chars+2);
}

void insert_string(Char *st)
{
    end_kill_sequence();
    ps().insert_string(st);
}

static void insert(Char c, Index count)
{
    con_insert(c,count);
    if (ps().window) ps().window->redraw_end_page(count+saved_chars+1);
}

void insert_text(Index count)
{
    insert(MP_Text,count);
}

void insert_expr(Index count)
{
    insert(MP_Expr,count);
}

void insert_disp(Index count)
{
    insert(MP_Disp,count);
}

void insert_op(Index count)
{
    insert(MP_Op,count);
}

void insert_id(Index count)
{
    insert(MP_Id,count);
}

void insert_var(Index count)
{
    insert(MP_Var,count);
}

void insert_list(Index count)
{
    insert(MP_List,count);
}

void insert_newline(Index count)
{
    next_node_or_text(count);
}

void insert_hard_newline(Index count)
{
    next_node_or_insert(Newline, count);
}

void insert_ltab(Index count)
{
    con_insert(Ltab, count);
    if (ps().window) ps().window->redraw_full();
}

void insert_rtab(Index count)
{
    insert(Rtab,count);
}

void insert_settab(Index count)
{
    insert(Settab,count);
}

void insert_poptabs(Index count)
{
    insert(Poptabs,count);
}

void insert_tabplus(Index count)
{
    insert(Tabplus,count);
}

void insert_tabminus(Index count)
{
    insert(Tabminus,count);
}

void insert_pushtabs(Index count)
{
    insert(Pushtabs,count);
}

// 1.3 Deletions

static void redraw_newlines(Bool removed)
{
    if (ps().window) {
	if (ps().make_visible())
	    ps().window->redraw_full();
	else
	    if (removed)
		ps().window->redraw_end_page(saved_chars+1);
	    else
		ps().window->redraw_line(saved_chars+1);
    }
}

void remove_region(void)
{
    end_kill_sequence();
    redraw_newlines(ps().remove());
}

void forward_remove_char(Index count)
{
    end_kill_sequence();
    if (!more_keys())
	redraw_newlines(ps().remove(count));
    else
	ps().remove(count);
}

void backward_remove_char(Index count)
{
    end_kill_sequence();
    if (!more_keys())
	redraw_newlines(ps().remove(-(int)count));
    else
	ps().remove(-(int)count);
}

void remove_double_chars(void)
{
    end_kill_sequence();
    if (!more_keys())
	redraw_newlines(ps().remove_double_chars());
    else
	ps().remove_double_chars();
} 

void kill_region(void)
{
    redraw_newlines(ps().kill());
}

void kill_word(Index count)
{
    redraw_newlines(ps().kill_word(count));
}

void backward_kill_word(Index count)
{
    redraw_newlines(ps().kill_word(-(int)count));
}

void kill_line(Index count)
{
    redraw_newlines(ps().kill_line(count));
}

void backward_kill_line(Index count)
{
    redraw_newlines(ps().kill_line(-(int)count));
}

void append_next_kill(void)
{
    if (!killnode) {
	killnode = lastkilled;
	lastkilled = Empty;
    }
}

void yank(void)
{
    end_kill_sequence();
    ps().yank();
    if (ps().window) {
	ps().make_visible();
	ps().window->redraw_full();
    }
}

// 1.4 Complex operations

void swap_region(void)
{
    end_kill_sequence();
    swap(ps(),ss());
    if (ps().window) ps().window->redraw_full();
    if (ss().window && ss().window!=ps().window)
	ss().window->redraw_full();
}

void copy_region(void)
{
    end_kill_sequence();
    copy(ps(),ss());
    if (ps().window) ps().window->redraw_full();
}

void distribute(void)
{
    Bool success=MP_False;
    end_kill_sequence();
    if (ts()) success = ps().distribute(ss(),ts());
    if (!success) success = ss().distribute(ps(),ss());
    if (!success) return;
    if (ps().window) {
	ps().window->recalc_at_line();
	ps().window->redraw_full();
    }
    if (ss().window && ss().window != ps().window) {
	ss().window->recalc_at_line();
	ss().window->redraw_full();
    }
}

void factorise(void)
{
    end_kill_sequence();
    if (!ps().factorise(ss(),ts())) return;
    if (ps().window) {
	ps().window->recalc_at_line();
	ps().window->redraw_full();
    }
    if (ss().window && ss().window != ps().window) {
	ss().window->recalc_at_line();
	ss().window->redraw_full();
    }
}

void commute(void)
{
    end_kill_sequence();
    ps().commute();
    if (ps().window) ps().window->redraw_full();
}

void apply(void)
{
    end_kill_sequence();
    ps().apply(ss(),ts());
    if (ps().window) ps().window->redraw_full();
}

void rename_id(void)
{
    end_kill_sequence();
    ps().rename(ss(),ts());
    if (ps().window) ps().window->redraw_full();
}

// 2 For the mpk language

// 2.1 For menus ...

void insert_notation(Index nnr)
{
    end_kill_sequence();
    ps().insert_stencil(nnr);
    if (ps().window) {
	ps().make_visible();
	ps().window->redraw_full();
    }
}

int ps_notation(Offset *vnr)
{
    return ps().notation_nr(vnr);
}

int ss_notation(Offset *vnr)
{
    return ss().notation_nr(vnr);
}

int selected_notation(int selnr, Offset *vnr)
{
  switch (selnr) {
  case 2: return ss().notation_nr(vnr); break;
  case 3: return ts().notation_nr(vnr); break;
  case 1: return ps().notation_nr(vnr); break;
  default: return -1; break;
  }
  return -1;
}

int ps_id_font(void)
{
    return ps().id_font();
}

void new_id_font(Offset nfnr)
{
    end_kill_sequence();
    ps().new_id_font(nfnr);
    if (ps().window) ps().window->redraw_full();
}

void new_version(Offset nnnr)
{
    end_kill_sequence();
    ps().new_version(nnnr);
    if (ps().window) ps().window->redraw_full();
}

void stack_position(void)
{
    end_kill_sequence();
    ps().stack_position();
}

void use_stack(void)
{
    end_kill_sequence();
    ps().use_stack();
    if (ps().window) {
	if (!ps().make_visible())
	    ps().window->redraw_cursor(&(ps()), switch_reverse);
	else
	    ps().window->redraw_full();
    }
}

void clear_stack()
{
    end_kill_sequence();
    ps().clear_stack();
}

void clear_stack_and_use()
{
    end_kill_sequence();
    ps().clear_stack_and_use();
    if (ps().window) {
	if (!ps().make_visible())
	    ps().window->redraw_cursor(&(ps()), switch_reverse);
	else
	    ps().window->redraw_full();
    }
}

Bool check_find()
{
    end_kill_sequence();
    if (!findwindow().base()->check_ph(findwindow().base())) {
	findwindow().redraw_full();
	return MP_False;
    } else
	return MP_True;
}

Bool check_find_replace()
{
    end_kill_sequence();
    if (!findwindow().base()->check_ph(findwindow().base())) {
	if (!replacewindow().base()->check_ph(findwindow().base()))
	    replacewindow().redraw_full();
	findwindow().redraw_full();
	return MP_False;
    } else
	if (!replacewindow().base()->check_ph(findwindow().base())) {
	    replacewindow().redraw_full();
	    return MP_False;
	} else
	    return MP_True;
}

Bool find_tree()
{
    end_kill_sequence();
    if (FRWindow(ps().window) && ops().window) {
	ps().unset();
	ps().window->redraw_cursor(&(ps()),switch_reverse);
	ps() = ops();
	ps().window->redraw_cursor(&(ps()),switch_reverse);
    }
    ps().clear_replacestop();
    Bool found = ps().find_tree(findwindow().base());
    if (found && ps().window)
	visible_redraw();
    return found;
}

void replace_tree()
{
    end_kill_sequence();
    if (FRWindow(ps().window) && ops().window) {
	ps().unset();
	ps().window->redraw_cursor(&(ps()),switch_reverse);
	ps() = ops();
	ps().window->redraw_cursor(&(ps()),switch_reverse);
    }
    ps().clear_replacestop();
    ps().replace_tree(findwindow().base(), replacewindow().base());
    if (ps().window) ps().window->redraw_full();    
}

void replace_all_tree()
{
    Index ir=10;
    Bool irt;
    end_kill_sequence();
    if (FRWindow(ps().window) && ops().window) {
	ps().unset();
	ps().window->redraw_cursor(&(ps()),switch_reverse);
	ps() = ops();
	ps().window->redraw_cursor(&(ps()),switch_reverse);
    }
    ps().clear_replacestop();
    do {
	ps().replace_tree_all(findwindow().base(), replacewindow().base());
	ir--;
	if (!ir) {
	    irt = 0;/* interrupted() */ ;
	    ir = 10;
	} else irt = more_keys();
    } while (!irt && ps().find_tree(findwindow().base()));
    if (ps().window) {
	ps().make_visible();
	ps().window->redraw_full();
    }
}

void remove_find_stack(void)
{
    fr_stack().remove(0,fr_stack().size());
    fr_stack().insert(0,MP_Text,2);
    stackpos=0;
    findwindow().clear();
    replacewindow().clear();
    findwindow().redraw_full();
    replacewindow().redraw_full();
}

static void update_stack(void)
{
    Node *nf = findwindow().base();
    Node *nr = replacewindow().base();
    Node *nfo = fr_stack().under(stackpos);
    Node *nro = fr_stack().under(stackpos+1);
    nfo->remove(0,nfo->size());
    nfo->paste(0,nf->cut(0,nf->size()));
    nro->remove(0,nro->size());
    nro->paste(0,nr->cut(0,nr->size()));
    if (!nro->size() && !nfo->size())
	fr_stack().remove(stackpos, stackpos+2);
    findwindow().clear();
    replacewindow().clear();
}

void find_prev_on_stack(void)
{
    if (stackpos<2) return;
    update_stack();
    stackpos-=2;
    Node *nfo = fr_stack().under(stackpos);
    findwindow().base()->paste(0,nfo->copy(0,nfo->size()));
    nfo = fr_stack().under(stackpos+1);
    replacewindow().base()->paste(0,nfo->copy(0, nfo->size()));
    findwindow().redraw_full();
    replacewindow().redraw_full();
}

void find_next_on_stack(void)
{
    Index oldsize = fr_stack().size();
    update_stack();
    if (stackpos+3>fr_stack().size()) {
	stackpos = fr_stack().size();
	fr_stack().insert(stackpos, MP_Text, 2);
    } else {
	if (oldsize==fr_stack().size()) stackpos+=2;
	Node *nfo = fr_stack().under(stackpos);
	findwindow().base()->paste(0,nfo->copy(0,nfo->size()));
	nfo = fr_stack().under(stackpos+1);
	replacewindow().base()->paste(0,nfo->copy(0, nfo->size()));
    }
    findwindow().redraw_full();
    replacewindow().redraw_full();
}

void find_new_on_stack(void)
{
    update_stack();
    fr_stack().insert(stackpos,MP_Text,2);
    findwindow().redraw_full();
    replacewindow().redraw_full();
}

int get_findrep(Char *, int*, int)
{
    Index i=fr_stack().size();
    fr_stack().insert(i, MP_Text, 2);
    fr_stack().under(i+1)->get_stack();
    fr_stack().under(i)->get_stack();
    return SUCCESS+FREE_BUFFER;
}

int put_findrep(void)
{
    update_stack();
    Node *nfo = fr_stack().under(stackpos);
    findwindow().base()->paste(0, nfo->copy(0, nfo->size()));
    nfo = fr_stack().under(stackpos+1);
    replacewindow().base()->paste(0, nfo->copy(0, nfo->size()));
    Index i=0;
    while (i<fr_stack().size()) {
	if (fr_stack().under(i)->size() || fr_stack().under(i+1)->size()) {
	    put_struct(FINDREPTYPE, 0);
	    fr_stack().under(i)->save();
	    fr_stack().under(i+1)->save();
	    put_end_struct();
	}
	i+=2;
    }
    return 0;
}

Bool find_string(Char *str)
{
    Bool found;
    end_kill_sequence();
    found = ps().find(str);
    if (found && ps().window)
	visible_redraw();
    return found;
}

Bool findnext_string(Char *str)
{
    Bool found;
    found = ps().findnext(str);
    if (found && ps().window)
	visible_redraw();
    return found;
}

Bool findwrap_string(Char *str)
{
    Bool found;
    end_kill_sequence();
    found = ps().findwrap(str);
    if (found && ps().window)
	visible_redraw();
    return found;
}

Bool find_backward_string(Char *str)
{
    Bool found;
    end_kill_sequence();
    found = ps().findback(str);
    if (found && ps().window)
	visible_redraw();
    return found;
}

Bool findprev_string(Char *str)
{
    Bool found;
    found = ps().findprev(str);
    if (found && ps().window)
	visible_redraw();
    return found;
}

Bool findwrap_backward_string(Char *str)
{
    Bool found;
    end_kill_sequence();
    found = ps().findwrapback(str);
    if (found && ps().window)
	visible_redraw();
    return found;
}

Bool find_replace(Char *str)
{
    Bool found;

    end_kill_sequence();
    found = ps().find_replace(ss(), str);
    if (found && ps().window)
	visible_redraw();
    return found;
}

Bool findnext_replace(Char *str)
{
    Bool found;

    end_kill_sequence();
    found = ps().findnext_replace(ss(),str);
    if (found && ps().window)
	visible_redraw();
    return found;
}

void replace_string(Char *oldstr, Char *newstr)
{
    end_kill_sequence();
    ps().replace(oldstr, newstr);
    if (ps().window) ps().window->redraw_full();
}

Bool find_stencil(Index nnr)
{
    Bool found;
    end_kill_sequence();
    found = ps().find_stencil(nnr);
    if (found && ps().window)
	visible_redraw();
    return found;
}

Bool findnext_stencil(Index nnr)
{
    Bool found;
    found = ps().findnext_stencil(nnr);
    if (found && ps().window)
	visible_redraw();
    return found;
}

Bool findwrap_stencil(Index nnr)
{
    Bool found;
    end_kill_sequence();
    found = ps().findwrap_stencil(nnr);
    if (found && ps().window)
	visible_redraw();
    return found;
}

Bool find_backward_stencil(Index nnr)
{
    Bool found;
    end_kill_sequence();
    found = ps().findback_stencil(nnr);
    if (found && ps().window)
	visible_redraw();
    return found;
}

Bool findprev_stencil(Index nnr)
{
    Bool found;
    found = ps().findprev_stencil(nnr);
    if (found && ps().window)
	visible_redraw();
    return found;
}

Bool findwrap_backward_stencil(Index nnr)
{
    Bool found;
    end_kill_sequence();
    found = ps().findwrapback_stencil(nnr);
    if (found && ps().window)
	visible_redraw();
    return found;
}

Bool find_replace_stencil(Index nnr)
{
    Bool found;

    end_kill_sequence();
    found = ps().find_replace_stencil(ss(), nnr);
    if (found && ps().window)
	visible_redraw();
    return found;
}

Bool findnext_replace_stencil(Index nnr)
{
    Bool found;

    end_kill_sequence();
    found = ps().findnext_replace_stencil(ss(),nnr);
    if (found && ps().window)
	visible_redraw();
    return found;
}

void replace_notation(Index onnr, Index nnnr)
{
    end_kill_sequence();
    ps().replace_notation(onnr, nnnr);
    if (ps().window) ps().window->redraw_full();
}

void replace_all(Char *oldstr, Char *newstr)
{
    Index ir=10;
    Bool irt;
    end_kill_sequence();
    do {
	ps().replace(oldstr, newstr);
	ir--;
	if (!ir) {
	    ir = 10;
	    irt = 0; /* interrupted(); */
	} else irt = more_keys();
    } while (!irt && ps().find_replace(ss(), oldstr));
    if (ps().window) {
	ps().make_visible();
	ps().window->redraw_full();
    }
}

void replace_all_notation(Index onnr, Index nnnr)
{
    Index ir=10;
    Bool irt;
    end_kill_sequence();
    do {
	ps().replace_notation(onnr, nnnr);
	ir--;
	if (!ir) {
	    ir = 10;
	    irt = 0; /*interrupted(); */
	} else irt = more_keys();
    } while (!irt && ps().find_replace_stencil(ss(), onnr));
    if (ps().window) {
	ps().make_visible();
	ps().window->redraw_full();
    }
}

void unset_select(Index button)
{
    switch (button) {
    case 1:
	ps().unset();
	if (ps().window) ps().window->redraw_cursor(&(ps()),switch_reverse);
	ps().window = 0;
	break;
    case 2:
	ss().unset();
	if (ss().window) ss().window->redraw_cursor(&(ss()),switch_thick);
	ss().window = 0;
	break;
    case 3:
	ts().unset();
	if (ts().window) ts().window->redraw_cursor(&(ts()),switch_thin);
	ts().window = 0;
	break;
    }
}

// 2.2 For the types.

// Currently no sharing between selections

void construct_selection(void **result)
{
  *result = new Select;
}

void destruct_selection(void *sel)
{
  delete (Select*) sel;
}

void copy_selection(void **result, void *sel)
{
  *result=new Select;
  *((Select *)(*result)) = *((Select*)sel);
}

void *get_selection(int nr)
{
  switch (nr) {
  case 2: return (void*) (&(ss())); break;
  case 3: return (void*) (&(ts())); break;
  default: return (void*) (&(ps())); break;
  }
}




/* Construct a list of numbers that indicates the position of the SUB_SEL
** with respect to the FULL_SEL.  POSITION is an array of MAX integers
** where the result is accumulated.
** The return value is the number positions used.  -1 indicates that
** SUB_SEL is not a subselection.
*/
int get_selection_path(void *full_sel, void *sub_sel, int *list, int max)
{
  Select *fsel, *ssel;
  if (!(full_sel && sub_sel && list && max))
    return -1;
  fsel=(Select*)full_sel;
  ssel=(Select*)sub_sel;
  if (!(*fsel)) return -1;
  return fsel->select_path(*ssel, list,max);
}

void change_selection(void *selection, int *position, int len)
{
  Node *n;
  Index rop;
  int cp;
  Select *sel;
  Mark m;
  sel = (Select*) selection;
  if (!sel) return;
  rop=sel->bpos();
  n = sel->bmark();
  cp=0;
  while (cp<len) {
    if (rop) { n=n->under(rop); rop=0;} else { n=n->first(); }
    for (int i=0; i<position[cp]; i++) n=n->right();
    cp++;
  }
  m=n;
  m.pos=0;
  *sel=m;
}

void filter_template_selection(void *selection, Index *templatenr)
{
  Select *sel = (Select*)selection;
  Mark m;
  int i;
  if (!sel) return;
  i=0;
  m=sel->bmark();
  while (templatenr[i] && Ph(m(0))==MP_Expr) {
    if (m.under()->stencil_nr()==templatenr[i]) {
      sel->remove(1);
      m=sel->bmark();
      i=0;
    } else {
      i++;
    }
  }
}

void insert_template_selection(void *selection, Index nnr)
{
  Select *sel = (Select*) selection;
  if (sel) {
    sel->insert_stencil(nnr);
  }
}

void insert_string_selection(void *selection, Char *string)
{
  Select *sel = (Select*) selection;
  if (sel) {
    sel->insert_string(string);
  }
}

void commute_selection(void *selection)
{
  Select *sel = (Select*) selection;
  if (sel) {
    sel->commute();
  }
}

void next_node_selection(void *selection)
{
  Select *sel = (Select*) selection;
  if (sel) {
    sel->next_node_or_text();
  }
}

void insert_parse_result(void *selection)
{
  Select *sel = (Select*) selection;
  if (sel) {
    sel->include_ascii();
    /* if (sel->window) sel->window->redraw_full(); */
  }
}

void insert_selection(void *selection, Char symbol)
{
  Select *sel = (Select*)selection;
  if (sel) {
    sel->insert(symbol);
  }
}

void filter_selection(void *selection, Char *string)
{
  Select *sel = (Select*)selection;
  Mark m;
  int i;
  if (!sel) return;
  i=0;
  m=sel->bmark();
  while (string[i]) {
    if (m(0)==string[i]) {
      sel->remove(1);
      m=sel->bmark();
      i=0;
    } else {
      i++;
    }
  }
}

void redraw_selection(void *selection)
{
  Select *sel = (Select*) selection;
  if (sel && sel->window) {
    sel->window->redraw_full();
  }
}

// 3 Internal

// 3.1 Update functions

void update_selections(void)
{
    ps().update();
    ss().update();
    ts().update();
    ops().update();
}

void redraw_window(void* w)
{
    ((EditWindow*)w)->redraw_full();
}

void word_wrap_window(void *w)
{
    ((EditWindow*)w)->word_wrap_full();
    ((EditWindow*)w)->redraw_full();
    ((EditWindow*)w)->recalc_lineheight();
}

void word_wrap_selection(int nr)
{
    switch (nr) {
    case 2:
	if (!(ss()) || !ss().window) return;
	ss().window->word_wrap_selection(&(ss()));
	ss().window->redraw_full();
	ss().window->recalc_lineheight();
	break;
    case 3:
	if (!(ts()) || !ts().window) return;
	ts().window->word_wrap_selection(&(ts()));
	ts().window->redraw_full();
	ts().window->recalc_lineheight();
	break;
    default:
	if (!(ps()) || !ps().window) return;
	ps().window->word_wrap_selection(&(ps()));
	ps().window->redraw_full();
	ps().window->recalc_lineheight();
	break;
    }
}

void clear_tab_positions(void)
{
    miniwindow().clear_all_tabs();
}

void resize_window(void* w, unsigned int sx, unsigned int sy)
{
    EditWindow* ew = (EditWindow*)w;
    ew->setwin(sx,sy);
    ew->redraw_full(); /* needed??? expose event should follow */
}

void editwindow_line(void* w, int ln)
{
    EditWindow* ew = (EditWindow*)w;
    ew->start_to_line(ln);
    /*  ew->redraw_full(); */
}

void editwindow_topto(void *w, Char *str)
{
    EditWindow* ew = (EditWindow*) w;
    ew->start_to_str(str);
    ew->redraw_full();
}

int line_number(void* w)
{
    EditWindow* ew = (EditWindow*)w;
    return ew->get_line_number();
}

int number_of_lines(void* w)
{
    return ((EditWindow*)w)->get_number_of_lines();
}

// 3.2 The opening and closing of windows ...

void* open_editwindow(void* w, unsigned int xs, unsigned int ys)
{
    return new EditWindow(w,xs,ys);
}

void *make_node(Char type, Char *txt, int len, int nnr, int spacing)
{
    Node *n = new Node(MP_Text);
    if (!n->parsed_node(type, txt, len,nnr,spacing)) {
	delete n;
	return NULL;
    } else
	return (void*) n;
}

void apply_leibnitz(void)
{
  Node *n = new Node(MP_Expr);
  n->apply_leibnitz();
}

void *add_parse_stack(Char *txt, int len)
{
    Node n(MP_Text);
    void *p;
    p = n.error_node(txt, len);
    if (p)
	return p;
    else
	return NULL;
}

void join_parse_stack(void)
{
    Node n(MP_Text);
    n.join_stack();
}

int get_node(Char* str, int *len, int max)
{
    Node *n = new Node(MP_Text);

    return n->load(str, len, max);
}

int get_ascii_node(Char* str, int *len, int max)
{
    Node *n = new Node(MP_Text);

    return n->load_ascii(str, len, max);
}

void cleanup_nodestack(void)
{
    Node n(MP_Text);
    n.clean_up();
}

void append_editwindow(void* w, Char *s, unsigned int nr)
{
    EditWindow* ew = (EditWindow*)w;
    if (ew) ew->append_string(s,nr);
}

void append_structure(void *w)
{
    EditWindow* ew = (EditWindow*)w;
    Node *n;
    n = new Node(MP_Text);
    n->get_stack();
    ew->append_node(n);
}

void old_load_editwindow(void* w, FILE* f)
{
    end_kill_sequence();
    EditWindow* ew = (EditWindow*)w;
    ps().unset();
    if (ps().window && ps().window != ew)
	ps().window->redraw_cursor(&(ps()), switch_reverse);
    ps().window = ew;
    if (ss().window == ew) ss().unset(), ss().window = 0;
    if (ts().window == ew) ts().unset(), ts().window = 0;
    ew->old_load(f);
    ps().set_begin();
    ew->redraw_full();
}

void load_editwindow(void *w)
{
    end_kill_sequence();
    EditWindow* ew = (EditWindow*)w;
    ps().unset();
    if (ps().window && ps().window != ew)
	ps().window->redraw_cursor(&(ps()), switch_reverse);
    ps().window = ew;
    if (ss().window == ew) ss().unset(), ss().window = 0;
    if (ts().window == ew) ts().unset(), ts().window = 0;
    ew->load_ascii();
    ps().set_begin();
    ew->redraw_full();
    ew->recalc_lineheight();
}

void old_include_editwindow(void* w, FILE* f)
{
    end_kill_sequence();
    EditWindow* ew = (EditWindow*) w;
    ew->old_include(f);
    ew->redraw_full();
}

void include_editwindow(void* w)
{
    end_kill_sequence();
    EditWindow* ew = (EditWindow*)w;
    ew->include_ascii();
    ew->redraw_full();
}

void include_selection(void)
{
    if (!(ps())) return;
    end_kill_sequence();
    ps().include_ascii();
    ps().window->redraw_full();
}

void save_editwindow(void* w)
{
    (*(EditWindow*)w).save();
}

Bool window_changed(void* w)
{
    return ((EditWindow*)w)->poll();
}

Bool window_empty(void* w)
{
    return ((EditWindow*)w)->empty();
}

void latex_editwindow(void* w)
{
    (*(EditWindow*)w).latex();
}

void goto_latex_line(Index n)
{
    if (ps()) {
	end_kill_sequence();
	n--;
	ps().latex_line(n);
	visible_redraw();
    }
}

void latex_text_only(Bool tonly)
{
    text_only=tonly;
}

void latex_all_parens(Bool allparens)
{
  all_parens = allparens;
}

void latex_selection(int selnum)
{
    switch (selnum) {
    case 1:
	if (ps()) {
	    end_kill_sequence();
	    ps().latex();
	}
	break;
    case 2:
	if (ss()) {
	    end_kill_sequence();
	    ss().latex();
	}
	break;
    case 3:
	if (ts()) {
	    end_kill_sequence();
	    ts().latex();
	}
	break;
    default:
	break;
    }
}

void clear_window(void* w)
{
    end_kill_sequence();
    ((EditWindow*)w)->clear();
}

void close_editwindow(void* w)
{
    delete (EditWindow*)w;
}

void* open_scratchwindow(void* w, unsigned int xs, unsigned int ys)
{
    end_kill_sequence();
    scratchwindow().setwin(w,xs,ys);
    return &(scratchwindow());
}

void close_scratchwindow(void)
{
    end_kill_sequence();
    scratchwindow().setwin(0,0,0);
}

void* open_findwindow(void* w, unsigned int xs, unsigned int ys)
{
    end_kill_sequence();
    stackpos=fr_stack().size();
    fr_stack().insert(stackpos, MP_Text,2);
    findwindow().setwin(w,xs,ys);
    return &findwindow();
}

void close_findwindow(void)
{
    end_kill_sequence();
    update_stack();
    Index i=0;
    while (i<fr_stack().size())
	if (!fr_stack().under(i)->size() && !fr_stack().under(i+1)->size())
	    fr_stack().remove(i,i+2);
	else
	    i+=2;
    findwindow().setwin(0,0,0);
}

void* open_replacewindow(void* w, unsigned int xs, unsigned int ys)
{
    end_kill_sequence();
    replacewindow().setwin(w,xs,ys);
    return &replacewindow();
}

void close_replacewindow(void)
{
    end_kill_sequence();
    replacewindow().setwin(0,0,0);
}

void* open_miniwindow(void* w, unsigned int xs, unsigned int ys)
// System Initialisation
{
    miniwindow().setwin(w,xs,ys);
    return &(miniwindow());
}

void close_miniwindow(void)
// System shutdown
{
}


// extra ...

void other_window(void*)
{
}

void dbl_click(void)
{
    if (!cs) return;
    cs->dbl_click();
    cs->window->redraw_cursor(cs,ccf);
}

void mouse_down(void* w, int xpos, int ypos, Index button)
{
    end_kill_sequence();
    if (button==1 && !(FRWindow(ps().window)) && FRWindow((EditWindow*)w))
	ops()=ps();
    switch (button) {
    case 1: cs = &(ps()); ccf = switch_reverse; break;
    case 2: cs = &(ss()); ccf = switch_thick;   break;
    case 3: cs = &(ts()); ccf = switch_thin;    break;
    default: return;
    }
    cs->unset();
    if (cs->window) cs->window->redraw_cursor(cs,ccf);
    cs->unset_old();
    cs->window = (EditWindow*)w;
    m1() = cs->window->findpos(xpos,ypos);
    if (!(m1())) return;
    cs->restore(m1());
    cs->window->redraw_cursor(cs,ccf);
}

void mouse_move(int xpos, int ypos)
{
    m2() = cs->window->findpos(xpos,ypos);
    if (!(m2())) return;
    cs->restore(m1(),m2());
    cs->window->redraw_cursor(cs,ccf);
}

void mouse_up(int, int)
{
    if (cs != &(ps()) && cs->size() == 0 && cs->node_size()!=0)
	cs->unset();
    cs->no_dbl_click();
    m1() = Empty;
    m2() = Empty;
    cs->window->redraw_cursor(cs, ccf);
}

void set_parens(void)
{
    end_kill_sequence();
    ps().set_parens();
    if (ps().window) ps().window->redraw_full();
}

void unset_parens(void)
{
    end_kill_sequence();
    ps().unset_parens();
    if (ps().window) ps().window->redraw_full();
}

void clear_parens(void)
{
    end_kill_sequence();
    ps().clear_parens();
    if (ps().window) ps().window->redraw_full();
}

void increase_spacing(Index count)
{
    end_kill_sequence();
    ps().increase_spacing(count);
    if (ps().window) ps().window->redraw_line(2);
}

void decrease_spacing(Index count)
{
    end_kill_sequence();
    ps().decrease_spacing(count);
    if (ps().window) ps().window->redraw_line(2);
}

void set_index_nr(Index count)
{
    end_kill_sequence();
    ps().set_index_nr(count);
    if (ps().window) ps().window->redraw_line(2);
}

void reset_spacing(void)
{
    end_kill_sequence();
    ps().reset_spacing();
    if (ps().window) ps().window->redraw_full();
}

void switch_parens(void)
{
    end_kill_sequence();
    ps().switch_parens();
    if (ps().window) ps().window->redraw_full();
}

void lower_region(void)
{
    end_kill_sequence();
    ps().lower();
    if (ps().window) ps().window->redraw_full();
}

void selection_use_xor(Bool value)
{
  use_xor=value;
}

void raise_node(void)
{
    end_kill_sequence();
    ps().raise();
    if (ps().window) ps().window->redraw_full();
}

void join_selections(Index sel1, Index sel2)
{
    Select *ct;
    end_kill_sequence();
    switch (sel1) {
    case 2: ct = &(ss()); break;
    case 3: ct = &(ts()); break;
    case 1:
    default: ct = &(ps()); break;
    }
    switch (sel2) {
    case 2: cs=&(ss()); ccf = switch_thick; break;
    case 3: cs=&(ts()); ccf = switch_thin;  break;
    case 1:
    default: cs=&(ps()); ccf = switch_reverse; break;
    }
    if (cs == ct) return;
    if (!!(*cs) && !!(*ct) && cs->window && cs->window==ct->window) {
	cs->combine(*ct);
	cs->window->redraw_cursor(cs, ccf);
    }
}

void copy_selections(Index sel1, Index sel2)
{
  Select *ct;
  end_kill_sequence();
  switch (sel1) {
  case 2: ct = &(ss()); break;
  case 3: ct = &(ts()); break;
  case 1:
  default: ct = &(ps()); break;
  }
  switch (sel2) {
  case 2: cs=&(ss()); ccf = switch_thick; break;
  case 3: cs=&(ts()); ccf = switch_thin;  break;
  case 1:
  default: cs=&(ps()); ccf = switch_reverse; break;
  }
  if (cs == ct) return;
  if (!!(*ct) && ct->window) {
    if (!!(*cs) && cs->window) {
      cs->unset();
      cs->window->redraw_cursor(cs, ccf);
    }
    *cs = *ct;
    cs->window->redraw_cursor(cs, ccf);
  }
}
    

void swap_selections(Index sel1, Index sel2)
{
    Select *ct;
    
    end_kill_sequence();
    switch (sel1) {
    case 2: ct = &(ss()); cscf = switch_thick; break;
    case 3: ct = &(ts()); cscf = switch_thin;   break;
    case 1:
    default: ct = &(ps()); cscf = switch_reverse; break;
    }
    switch (sel2) {
    case 2: cs=&(ss()); ccf = switch_thick; break;
    case 3: cs=&(ts()); ccf = switch_thin;  break;
    case 1:
    default: cs=&(ps()); ccf = switch_reverse; break;
    }
    if (cs == ct) return;
    if (!!(*cs) && !!(*ct) && cs->window && ct->window) {
      Select swp1, swp2;
      swp1 = *cs;swp2=*ct;
      cs->unset();ct->unset();
      cs->window->redraw_cursor(cs, ccf);
      ct->window->redraw_cursor(ct, cscf);
      *cs=swp2;*ct=swp1;
      cs->window->redraw_cursor(cs, ccf);
      ct->window->redraw_cursor(ct, cscf);
    }
}

Char *get_subnode_string(int selnr, int *posi, int len)
{
  Node *n = (Node*) get_subnode(selnr, posi, len);
  return n->content_ref();
}

void *get_subnode(int selnr, int *posi, int len)
{
  Node *n;
  Index rop;
  Select *sel;
  int cp;
  switch (selnr) {
  case 1: sel = &(ps()); break;
  case 2: sel = &(ss()); break;
  case 3: sel = &(ts()); break;
  default: sel = 0;  break;
  }
  if (!sel) return 0;
  rop=sel->bpos();
  n = sel->bmark();
  cp=0;
  while (cp<len) {
    if (rop) { n=n->under(rop); rop=0;} else { n=n->first(); }
    for (int i=0; i<posi[cp]; i++) n=n->right();
    cp++;
  }    
  return n;
}

 
int  node_notation(void *node, int *vnr)
{
  return ((Node*)node)->notation_nr(vnr);
}

void latex_node(void *node)
{
  ((Node*)node)->latex();
}

void *first_node(void *selection)
{
  Select *sel = (Select*) selection;
  if (sel)
    return sel->bmark()->first();
  else
    return 0;
}

void *last_node(void *selection)
{
  Select *sel = (Select*) selection;
  if (sel)
    return sel->bmark()->last();
  else
    return 0;
}
