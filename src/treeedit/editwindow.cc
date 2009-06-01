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
// editwindow.cc

extern "C" {
#include <stddef.h>
#include <stdio.h>
#include "mathpad.h"
#include "output.h"
#include "message.h"
extern void push_attributes(int attributes);
extern void pop_attributes(void);
#define push_fontgroup(A)  push_attributes(A)
#define pop_fontgroup()    pop_attributes()

extern void edit_set_number_of_lines(void*,int);
extern void buffer_set_number_of_lines(void*,int);
extern void find_set_number_of_lines(void*,int);

}
#include "mathpad.hh"
#include "mark.hh"
#include "marker.hh"
#include "node.hh"
#include "select.hh"
#include "editwindow.hh"

#include "mathpad.icc"

// Declarations of the static members
EditWindow* EditWindow::ewlist = 0;
    // The following are used when redrawing a window
int EditWindow::x;
int EditWindow::y;
Bool EditWindow::search;
Bool EditWindow::wrap;
Bool EditWindow::wrapped;
Offset EditWindow::fill_column;
Char EditWindow::wasnewline;
Offset EditWindow::lspp; 
Bool EditWindow::brk;
Bool EditWindow::smart;
Cpfv EditWindow::cf;
Select* EditWindow::cs;

Return::Return() : point(Line)
{
    point = Empty;
    height=0;
    left = NULL;
    right = NULL;
    tabs = NULL;
}

Return::Return(const Mark& m) : point(Line)
{
    point = m;
    height=0;
    left = NULL;
    right = NULL;
    tabs = NULL;
}

Return::Return(const Mark& m, Index) : point(LeftLine)
{
    point = m;
    height=0;
    left = NULL;
    right = NULL;
    tabs = NULL;
}

Return::Return(Node *pn, Index i) : point(Line)
{
    point = Mark(pn,i);
    height=0;
    left=NULL;
    right=NULL;
    tabs=NULL;
}

Return::~Return()
{
    unlink();
    if (tabs) tab_unlock(tabs);
}

void Return::print()
{
    Return* p;
    Mark m;
    char s[1000];
    char t[1000];
    char *g,*h;

    m = point;
    g = s;
    h = t;
    h[0]=0;
    while (m) {
	sprintf(g, "\t%p:%i%s", (Node*)m, m.pos, h);
	if (g==t) {  g=s; h=t; } else { g=t; h=s; }
	m = m->above();
    }
    fprintf(stdout, "H: %i\tT: %p%s\nLeft:\n", height, tabs, h);
    p = left;
    while (p) {
	h[0]=0;
	m = p->point;
	while (m) {
	    sprintf(g, "\t%p:%i%s", (Node*)m, m.pos, h);
	    if (g==t) {  g=s; h=t; } else { g=t; h=s; }
	    m = m->above();
	}
	fprintf(stdout,"H: %i\tT: %p%s\n", p->height, p->tabs, h);
	p=p->left;
    }
    fprintf(stdout, "Right:\n");
    p=right;
    while (p) {
	h[0]=0;
	m = p->point;
	while (m) {
	    sprintf(g, "\t%p:%i%s", (Node*)m, m.pos,h);
	    if (g==t) {  g=s; h=t; } else { g=t; h=s; }
	    m = m->above();
	}
	fprintf(stdout,"H: %i\tT: %p%s\n", p->height,p->tabs, h);
	p=p->right;
    }
}

Bool Return::check()
{
    Return *p, *q;
    Mark mp,mq;
    Index i;

    q= this;
    p= left;
    while (p) {
	mp = p->point;
	mq = q->point;
	i=0;
	if ((Node*)(mp.base()) != (Node*)(mq.base())) return MP_False;
	while (mp->depth()>mq->depth()) mp = mp->above(),i++;
	while (mq->depth()>mp->depth()) mq = mq->above(),i++;
	while ((Node*)mp != (Node*)mq) {
	    mp = mp->above();
	    mq = mq->above();
	    i++;
	}
	if (mp.pos>mq.pos || (mp.pos==mq.pos && i)) return MP_False;
	q=p;
	p=p->left;
    }
    q= right;
    p= this;
    while (q) {
	mp = p->point;
	mq = q->point;
	i=0;
	if ((Node*)(mp.base()) != (Node*)(mq.base())) return MP_False;
	while (mp->depth()>mq->depth()) mp = mp->above(),i++;
	while (mq->depth()>mp->depth()) mq = mq->above(),i++;
	while ((Node*)mp != (Node*)mq) {
	    mp = mp->above();
	    mq = mq->above();
	    i++;
	}
	if (mp.pos>mq.pos || (mp.pos==mq.pos && i)) return MP_False;
	p=q;
	q=q->right;
    }
    return MP_True;
}

Return* Return::link_left(const Mark& m)
{
    Return *p = new Return(m);

    p->left = left;
    p->right = this;
    if (left) left->right = p;
    left = p;
    height = 0;
    return p;
}

Return* Return::link_right(const Mark& m)
{
    Return *p = new Return(m);

    p->right = right;
    p->left = this;
    if (right) right->left = p;
    right = p;
    return p;
}

Return* Return::unlink()
{
    Return *p = (right? right : left);
    if (left) left->right = right;
    if (right) {
	right->left = left;
	right->height = 0;
    }
    left = NULL;
    right = NULL;
    return p;
}

Return* Return::under(const Mark& m)
{
    Mark pm = m.base();
    Mark pp = point.base();

    if ((Node*)pp != (Node*)pm) return NULL;
    if (pm.pos<pp.pos) {
	Return *p = left;
	while (p && p->point != m) p = p->left;
	return p;
    }
    if (pm.pos>pp.pos) {
	Return *p = right;
	while (p && p->point !=m) p = p->right;
	return p;
    }
    Return *p = this;
    while (p && p->point !=m && p->point.base()==pm) p = p->left;
    if (p && p->point==m) return p;
    p = right;
    while (p && p->point !=m && p->point.base()==pm) p = p->right;
    if (p && p->point==m) return p;
    return NULL;
}

Return* Return::before(const Mark& m, int& h)
{
    Return* p = this;
    Return* q = NULL;
    Mark pm, pp;

    h=0;
    for (;;) {
	pm = m;
	pp = p->point;
	int i = pp->depth(), j = pm->depth(),k=-1,l=-1;
	while (i > j) { k=pp.pos; pp = pp->above(); i--; }
	while (j > i) { l=pm.pos; pm = pm->above(); j--; }
	while (pp && pm && (Node*)pp != (Node*)pm) {
	    l = pm.pos; pm = pm->above();
	    k = pp.pos; pp = pp->above();
	}
	if (!pp || !pm) break;
	if (pp.pos<pm.pos || (pp.pos==pm.pos && k<l)) {
	    q=p;
	    p = p->right;
	    if (!p->right) break;
	} else
	    break;
    }
    p=q;
    while (p && p!=this) {
	if (IsNewline(p->point(0)))
	    if (!p->height) h+=line_height();
	    else h+=p->height;
	p=p->left;
    }
    return q;
}

Return* Return::update_left(const Mark& m)
{
    Return *p;
    Return *h;
    do {
	p = left;
	if (p && p->left) {
	    if (!p->point || p->point==point || !IsNewline(p->point(0))) {
		h = p->unlink();
		delete p;
		p=h;
	    }
	}
    }
    while (p==this);
    if (p && p->point == m)
	return p;
    else
	return link_left(m);
}

Return* Return::update_right(const Mark& m)
{
    Return *p;
    Return *h;
    do {
	p = right;
	if (p && p->right) {
	    if (!p->point || p->point==point || !IsNewline(p->point(0))) {
		h = p->unlink();
		delete p;
		p=h->left;
	    }
	}
    } while (p==this);
    if (p && p->point == m)
	return p;
    else
	return link_right(m);
}

int move_selection=MP_False;

Mark& EditWindow::m()
{
  static Mark *mval= new Mark();
  return *mval;
}

Mark& EditWindow::s()
{
  static Mark *sval= new Mark();
  return *sval;
}

Mark& EditWindow::e()
{
  static Mark *eval= new Mark();
  return *eval;
}

Mark& EditWindow::sel_pos()
{
  static Mark *sel_posval= new Mark();
  return *sel_posval;
}

Mark& EditWindow::lsp()
{
  static Mark *lspval= new Mark();
  return *lspval;
}

int EditWindow::height_of_line(int lines, int &oneline, int &sumline)
{
  Return *ret;

  ret=startp;
  oneline=0;
  sumline=0;
  if (lines<0) {
    do {
      oneline=ret->height;
      sumline=sumline+oneline;
      ret=ret->left;
      lines++;
    } while (ret && ret->height && lines<0);
    if (lines<0) return 0;
  } else {
    ret=ret->right;
    while (lines>=0 && ret && ret->height) {
      oneline=ret->height;
      sumline=sumline+oneline;
      ret=ret->right;
      lines--;
    }
    if (lines>=0) return 0;
  }
  return 1;
}

void EditWindow::start_to_line(Index line_number)
{
    int i;
    Index mi = root->lines();
    Bool forward=MP_True;
    Mark pm;
    Bool smartb=MP_True; /* is smart redraw (with content move) possible */
    Bool moveup;
    int height,sumheight;

    if (at_line>mi) { smartb=MP_False;recalc_at_line(); }
    if (at_line==line_number) return;
    if (!height_of_line(line_number-at_line,height,sumheight) ||
	sumheight>ysize)
      smartb=MP_False;
    moveup= (line_number>at_line);
    if (at_line<line_number) {
	if (line_number < mi-(mi-at_line)/2)
	    i = line_number-at_line;
	else {
	    i = mi-line_number+1;
	    start = root;
	    start.pos = root->size();
	    startp = last;
	    forward = MP_False;
	}
    } else if (at_line>line_number && line_number>at_line/2) { 
	i = at_line-line_number+1;
	forward = MP_False;
    } else {
	i = line_number;
	start = root;
	start.pos = 0;
	startp = first;
    }
    if (forward) {
	while (i) {
	    pm = start;
	    Char c = start.traverse();
	    if (c == 0) break;
	    if (IsNewline(c)) {
		startp = startp->update_right(pm);
		i--;
	    }
	}
	at_line = line_number;
    } else {
	while (i) {
	    Char c = start.esrevart();
	    if (c==0) {
		at_line = 0;
		startp = first;
		break;
	    }
	    if (IsNewline(c)) {
		startp = startp->update_left(start);
		i--;
	    }
	}
	if (!i) {
	    start.traverse();
	    at_line = line_number;
	}
    }
    if (smartb && outwin) {
      if (moveup) {
	set_output_window(outwin);
	move_content_up(sumheight-height, ysize);
	unset_output_window();
	redraw_between(ysize-sumheight+height,ysize);
      } else {
	set_output_window(outwin);
	move_content_down(sumheight, ysize);
	unset_output_window();
	redraw_between(0,sumheight);
      }
    } else {
      redraw_full();
    }
}

void EditWindow::start_to_str(Char* str)
{
    Mark sl = root->search_label(str);
    Mark pm;
    Mark qm;
    start = root;
    start.pos=0;
    startp=first;
    at_line=0;
    if (!sl) return;
    qm=start;
    while (qm != sl) {
	pm=qm;
	Char c = qm.traverse();
	if (!c) break;
	if (IsNewline(c)) {
	    start=qm;
	    startp = startp->update_right(pm);
	    at_line++;
	}
    }
}

void EditWindow::recalc_at_line()
{
    Mark rl(root,0);
    int i = 0;
    Mark p;

    startp = first;
    while (rl != start) {
	p = rl;
	Char c = rl.traverse();
	if (!c) break;
	if (IsNewline(c)) {
	    i++;
	    startp = startp->update_right(p);
	}
    }
    at_line = i;
}

void EditWindow::recenter(const Mark& rm)
{
    search = MP_True;
    (void) make_visible(rm);
    recalc_at_line();
    search = MP_False;
}

void EditWindow::set_center(Mark& sm)
{
    Index i = ysize/line_height()/2;
    sm = start;
    while (i) {
	Char c = sm.traverse();
	if (!c) return;
	if (IsNewline(c)) i--;
    }
}

Bool EditWindow::make_visible(const Mark& mv)
{
    Mark p = start;
    int i, n = ysize/line_height();
    Char c;
    int h;

    if (mv.base() != root) return MP_False;
    Return *b = startp->before(mv, h);
    Mark q=mv;
    q.pos = mv.pos-1;
    if (q(0)==Newline && b && q!=b->point) h+=line_height();
    if (b && h<ysize-line_height() && !search)
	if (IsNewline(c = start.esrevart())) {
	    start.traverse();
	    return MP_False;
	} else if (!c)
	    return MP_False;
    Index mpos = mv.base().pos;
    Index startpos = start.base().pos;
    if (mpos<=startpos) {
	while (start != mv && (mpos<startpos||start!=root)) {
	    c = start.esrevart();
	    if (!c) {
		at_line = 0;
		startp = first;
		break;
	    }
	    if (IsNewline(c)) {
		at_line--;
		startp = startp->update_left(start);
	    }
	}
    }
    if (mpos>=startpos) {
	while (start != mv) {
	    p = start;
	    c = start.traverse();
	    if (!c) {
		at_line = root->lines();
		startp = last->left;
		break;
	    }
	    if (IsNewline(c)) {
		at_line++;
		startp = startp->update_right(p);
	    }
	}
    }
    if (start == mv) {
	while (aig(c=start.esrevart()) && !IsNewline(c));
	if (c) start.traverse();
	i=0;
	while (i<n/2+1) {
	    c = start.esrevart();
	    if (!c) {
		at_line = 0;
		startp = first;
		break;
	    }
	    if (IsNewline(c)) {
		at_line--;
		startp = startp->update_left(start);
		i++;
	    }
	}
	if (c) {
	    start.traverse();
	    at_line++;
	}
    }
    return MP_True;
}

EditWindow::EditWindow(void* ow, int xs, int ys) : start(View)
{
    outwin = ow;
    xsize = xs;
    ysize = ys;
    root = new Node(MP_Text);
    start = root;
    start.pos = 0;
    first = new Return(start,1);
    last = new Return(start);
    last->point.pos = 1;
    first->right = last;
    last->left = first;
    startp = first;
    at_line = 0;
    next = ewlist;
    ewlist = this;
    if (move_selection) {
	if (ps().window) {
	    ps().unset();
	    ps().window->redraw_cursor(&(ps()),switch_reverse);
	}
	ps() = start;
	ps().window = this;
    }
    xp = yp = -1;
}

EditWindow::~EditWindow()
{
    Return *h;
    while (first) {
	h = first->unlink();
	delete first;
	first = h;
    }
    delete root;
    EditWindow* ewp = ewlist;
    if (ewp == this) {
	ewlist = next;
    } else {
	while (ewp->next != this) ewp = ewp->next;
	ewp->next = next;
    }
    if (ps().window == this) ps().unset(), ps().window = 0;
    if (ss().window == this) ss().unset(), ss().window = 0;
    if (ts().window == this) ts().unset(), ts().window = 0;
    if (ops().window == this) ops().unset(), ops().window = 0;
}

void EditWindow::setwin(void *ow, int xs, int ys)
{
    outwin = ow;
    xsize = xs;
    ysize = ys;
    if (!ow || !xs || !ys) {
	if (ps().window == this) ps().unset(), ps().window = 0;
	if (ss().window == this) ss().unset(), ss().window = 0;
	if (ts().window == this) ts().unset(), ts().window = 0;
	if (ops().window == this) ops().unset(), ops().window = 0;
    } else {
	if (move_selection) {
	    if (ps().window) {
		ps().unset();
		ps().window->redraw_cursor(&(ps()),switch_reverse);
	    }
	    ps() = start;
	    ps().window = this;
	}
    }
    xp = yp = -1;
}

EditWindow* EditWindow::window_with_base(Node *n)
{
  EditWindow *ew=ewlist;
  while (ew && ew->root!= n) ew=ew->next;
  return ew;
}

void EditWindow::scroll_up(Index n)
{
    Index nr_lines = root->lines();
    if (at_line<nr_lines) {
	Index i = n * ysize/line_height() -1;
	if (at_line+i>nr_lines) i=nr_lines-at_line;
	if (!i) return;
	int h,ch;
	if (height_of_line(i,h,ch)) {
	  if (outwin) {
	    set_output_window(outwin);
	    move_content_up(ch,ysize);
	    unset_output_window();
	  }
	  start_to_line(at_line+i);
	  redraw_between(ysize-ch, ysize);
	} else {
	  start_to_line(at_line+i);
	  redraw_full();
	}
    }
}

void EditWindow::scroll_down(Index n)
{
    if (at_line) {
	Index i = n * ysize/line_height() -1;
	if (i>at_line) i=at_line;
	if (!i) return;
	int h,ch;
	if (height_of_line(-(int)i,h,ch)) {
	  if (outwin) {
	    set_output_window(outwin);
	    move_content_down(ch,ysize);
	    unset_output_window();
	  }
	  start_to_line(at_line -i);
	  redraw_between(0,ch);
	} else {
	  start_to_line(at_line -i);	  
	  redraw_full();
	}
    }
}

void EditWindow::save()
{
    root->save();
}

void EditWindow::old_load(FILE* f)
{
    Return *h;
    startp = first->right;
    while (startp->right) {
	h = startp->unlink();
	delete startp;
	startp = h;
    }
    root->remove(0,root->size());
    unsigned char c = (unsigned char) getc(f);
    switch (c) {
    case 0xFF:
	ungetc(c,f);
	if (!(root->old_old_load(f))) {
	    root->remove(0,root->size());
	    message(MP_ERROR, translate("File is corrupted: load aborted."));
	}
	break;
    case 'A':
	if (!root->old_load(f)) {
	    root->remove(0,root->size());
	    message(MP_ERROR, translate("File is corrupted: load aborted."));
	}
	break;
    default:
	message(MP_ERROR, translate("File not in a recognized format."));
	break;
    }
    start=root;
    start.pos=0;
    startp = first;
    last->point = Mark(root, root->size()+1);
    at_line = 0;
    xp=yp=-1;
}

void EditWindow::load_ascii()
{
    Return *h;
    startp = first->right;
    while (startp->right) {
	h = startp->unlink();
	delete startp;
	startp = h;
    }
    root->remove(0,root->size());
    root->get_stack();
    start=root;
    start.pos=0;
    startp = first;
    last->point = Mark(root, root->size()+1);
    at_line = 0;
    xp=yp=-1;
}

void EditWindow::old_include(FILE* f)
{
    if (!(ps()) || ps().window != this) return;
    ps().include(f);
}

void EditWindow::include_ascii()
{
    if (!(ps()) || ps().window != this) return;
    ps().include_ascii();
}

void EditWindow::clear()
{
    Return *h;
    startp = first->right;
    while (startp->right) {
	h = startp->unlink();
	delete startp;
	startp = h;
    }
    root->remove(0,root->size());
    start = root;
    start.pos = 0;
    startp = first;
    last->point = Mark(root, root->size()+1);
    at_line = 0;
    if (ps() && ps().window==this) ps() = start;
    if (ss() && ss().window==this) ss().unset();
    if (ts() && ts().window==this) ts().unset();
}

void EditWindow::set_fill_column(Index n)
{
    if ((int)n > xsize-16) n=xsize-16;
    fill_column=n;
}

void EditWindow::set_wrap(Bool toggle)
{
    wrap=toggle;
}

void EditWindow::clear_all_tabs()
{
    EditWindow *ew=ewlist;

    while (ew) {
	ew->clear_tabs();
	ew=ew->next;
    }
}

void EditWindow::clear_tabs()
{
    Return *h=first;
    while (h) {
	if (h->tabs) {
	    tab_unlock(h->tabs);
	    h->tabs=NULL;
	}
	h = h->right;
    }
}

Mark EditWindow::adjust_mark(const Mark& init, Bool left)
{
    Mark h = init;
    Mark g = init;
    while (h) {
	Mark pn=g;
	while (h) {
	    Mark pm = h->above();
	    if (h->op()) {
		int i;
		Index j=0;
		for (i=h.pos; i>=0; i--) {
		    if (h(-i)==TabOpen || h(-i)==DisplayOpen)
			j++;
		}
		if (j) g = pm;
	    }
	    if (!!pm)
		if (Ph(pm(0))==MP_Disp) {
		    g = pm;
		}
	    h = pm;
	}
	if (pn!=g) {
	    if (left) {
		Char c;
		while (!IsNewline(c=g.esrevart()) && c);
		if (c) g.traverse();
	    } else {
		Char c;
		g.pos++;
		while (!IsNewline(c=g.traverse()) && c);
		if (c) g.esrevart();
	    }
	    h = g;
	}
    }
    return Mark(g);
}

Mark EditWindow::out_stack(const Mark& init)
{
    Mark h=init;
    Mark g=init;
    while (h) {
	if (h->op()) {
	    int i=-(int)h.pos;
	    int stack = 0;
	    while (i<0) {
		if (Char2Font(h(i))==StackFont || h(i)==StackB || h(i)==StackC)
		    stack++;
		else if (h(i)==CloseStack || h(i)==StackClose)
		    stack--;
		i++;
	    }
	    if (stack) {
		g = h;
		g.pos = 0;
	    }
	}
	h = h->above();
    }
    return Mark(g);
}

const Mark& EditWindow::findpos(int tx, int ty)
{
    Return *p;
    p = startp;
    int h = 0;
    x = tx;
    y = (ty<ysize ? ty:ysize);
    while (p->right && p->right->right && p->right->height &&
	   h+(int)p->right->height < y) {
	h+=p->right->height;
	p = p->right;
    }
    Mark pm = p->point;
    if (!!pm->above() || pm.pos>0  || p->left)
	pm.traverse();
    set_output_window(outwin);
    detect_margin();
    draw(&EditWindow::do_search,
	 &EditWindow::do_nothing,
	 &EditWindow::test_nothing,
	 &EditWindow::test_nothing,
	 &EditWindow::test_nothing,
	 pm, p, h);
    unset_output_window();
    return m();
}

void EditWindow::adjust_scrollbar(void)
{
  if (!outwin) return;
  if (this != &(miniwindow())) {
	if (this == &(scratchwindow()) ) {
	    buffer_set_number_of_lines(outwin,root->lines()+1);
	} else if (this == &(findwindow()) || this == &(replacewindow())) {
	    find_set_number_of_lines(outwin,root->lines()+1);
	} else {
	    edit_set_number_of_lines(outwin, root->lines()+1);
	}
    }
}

void EditWindow::redraw_full()
{
    if (!outwin) return;
    adjust_scrollbar();
    ps().unset_old();
    ss().unset_old();
    ts().unset_old();
    set_output_window(outwin);
    detect_margin();
    draw(&EditWindow::do_visible,
	 &EditWindow::do_full_test,
	 &EditWindow::test_all_begin,
	 &EditWindow::test_all_end,
	 &EditWindow::test_nothing,
	 start, startp, 0);
    if (x != -1) put_mark(x, y);
    unset_output_window();
    xp = x;
    yp = y;
    ps().set_old();
    ss().set_old();
    ts().set_old();
}

void EditWindow::redraw_between(int ystart, int yend)
{
  if (!outwin || ystart>=yend) return;
  adjust_scrollbar();
  Return *ret=startp->right;
  int startheight;
  int heightcal=0;
  while (ret && (heightcal+(int)ret->height)<ystart) {
    heightcal=heightcal+ret->height;
    ret=ret->right;
  }
  Return *b;
  if (ret) {
    b=ret->left;
    startheight=heightcal;
  } else {
    b=startp;
    startheight=0;
  }
  s()=b->point;
  Mark p;
  p = b->point;
  if (!!p->above() || p.pos || b->left) p.traverse();
  s()=p;
  while (ret && heightcal<yend) {
    heightcal=heightcal+ret->height;
    ret=ret->right;
  }
  ps().unset_old();
  ss().unset_old();
  ts().unset_old();
  if (ret) e()=ret->point; else e()=last->point;
  set_output_window(outwin);
  detect_margin();
  draw(&EditWindow::do_visible,
       &EditWindow::do_full_test,
       &EditWindow::test_begin_line,
       &EditWindow::test_end_line,
       &EditWindow::test_end_multiline,
       p, b, startheight);
  if (x!= -1) put_mark(x,y);
  unset_output_window();
  xp = x;
  yp = y;
    ps().set_old();
    ss().set_old();
    ts().set_old();
}

void EditWindow::redraw_cursor(Select* sl, Cpfv pf)
{
  if (use_xor) {
    redraw_cursor_xor(sl, pf);
  } else {
    pf=0;
    redraw_cursor_noxor(sl, switch_visible);
  }
}

void EditWindow::redraw_cursor_xor(Select* sl, Cpfv pf)
{
    if (!outwin) return;
    cs = sl;
    cf = pf;
    x = xp;
    y = yp;
    if (!cs->select_line(s(),e())) {
	s() = start;
	e() = start;
    }
    int h;
    Return *b = startp->before(s(),h);
    if (!b) b = startp;
    s() = b->point;
    if (!!(s()->above()) || s().pos || b->left) s().traverse();
    int i;
    Return *be = b->before(e(),i);
    if (!be) e() = startp->right->point;
    else e() = be->right->point;
    set_output_window(outwin);
    detect_margin();
    if (x != -1) put_mark(x, y);
    draw(&EditWindow::do_shades,
	 &EditWindow::do_test,
	 &EditWindow::test_begin,
	 &EditWindow::test_end,
	 &EditWindow::stop_end,
	 s(), b, h);
    if (x != -1) put_mark(x,y);
    unset_output_window();
    xp = x;
    yp = y;
    cs->set_old();
}

void EditWindow::redraw_cursor_noxor(Select* sl, Cpfv pf)
{
    if (!outwin) return;
    Bool pssel;
    Select tsl;
    tsl=(*sl);
    tsl.copy_old(*sl);
    tsl.noxor_sel();
    pssel= (sl==&(ps()));
    cs = &tsl;
    cf = pf;
    x = xp;
    y = yp;
    if (!cs->select_line(s(),e())) {
	s() = start;
	e() = start;
    }
    int h;
    Return *b = startp->before(s(),h);
    if (!b) b = startp;
    s() = b->point;
    if (!!(s()->above()) || s().pos || b->left) s().traverse();
    int i;
    Return *be = b->before(e(),i);
    if (!be) e() = startp->right->point;
    else e() = be->right->point;
    ps().unset_old();
    ss().unset_old();
    ts().unset_old();
    set_output_window(outwin);
    detect_margin();
    if (x != -1) put_mark(x, y);
    draw(&EditWindow::do_shades_noxor,
	 (pssel? &EditWindow::do_test : &EditWindow::do_test),
	 &EditWindow::test_begin_noxor,
	 &EditWindow::test_end_noxor,
	 &EditWindow::stop_end,
	 s(), b, h);
    if (x != -1) put_mark(x,y);
    unset_output_window();
    xp = x;
    yp = y;
    ps().set_old();
    ss().set_old();
    ts().set_old();
}

void EditWindow::redraw_line(int nr)
{
    if (!outwin) return;
    int h,i;
    Char c=1;
    Return *b, *be;
    Mark p;

    ps().unset_old();
    ss().unset_old();
    ts().unset_old();
    if (!(ps().select_line(s(), e()))) {
	s() = start;
	e() = start;
    }
    while (nr && c ) { c=s().esrevart(); nr--; }
    s() = out_stack(s());
    b = startp->before(s(),h);
    if (!b) b = startp;
    p = b->point;
    if (!!p->above() || p.pos || b->left) p.traverse();
    be = b->before(e(),i);
    if (!be) e() = b->right->point; else e() = be->right->point;
    x = xp;
    y = yp;
    set_output_window(outwin);
    detect_margin();
    wrap=MP_True;
    wrapped=MP_False;
    fill_column= xsize-16;
    if (x!=-1) put_mark(x,y);
    draw(&EditWindow::do_visible,
	 &EditWindow::do_test,
	 &EditWindow::test_begin_line,
	 &EditWindow::test_end_line,
	 &EditWindow::stop_end_line,
	 p, b, h);
    if (x!=-1) put_mark(x,y);
    unset_output_window();
    xp = x;
    yp = y;
    ps().set_old();
    ss().set_old();
    ts().set_old();
    wrap=MP_False;
    if (wrapped) {
	wrapped=MP_False;
	redraw_end_page(2);
    }
}

void EditWindow::redraw_end_page(int nr)
{
    int h,i;
    Return *b,*be;
    Mark p;
    Char c=1;

    if (!outwin) return;
    adjust_scrollbar();
    x = xp;
    y = yp;
    ps().unset_old();
    ss().unset_old();
    ts().unset_old();
    ps().select_line(s(), e());
    while (nr&&c) { c=s().esrevart(); nr--; }
    s() = out_stack(s());
    b = startp->before(s(),h);
    if (!b) b = startp;
    p = b->point;
    if (!!p->above() || p.pos || b->left) p.traverse();
    be = b->before(e(),i);
    if (!be) e() = b->right->point; else e() = be->right->point;
    e() = adjust_mark(e(),MP_False);
    set_output_window(outwin);
    detect_margin();
    if (x!=-1) put_mark(x,y);
    draw(&EditWindow::do_visible,
	 &EditWindow::do_test,
	 &EditWindow::test_begin_line,
	 &EditWindow::test_all_end,
	 &EditWindow::test_nothing,
	 p, b, h);
    if (x!=-1) put_mark(x,y);
    unset_output_window();
    xp = x;
    yp = y;
    ps().set_old();
    ss().set_old();
    ts().set_old();
}

void EditWindow::word_wrap_selection(Select* sl)
{
    if (!outwin || sl->window!=this) return;
    int h;
    sl->unset_old();
    if (!sl->select_line(s(),e())) return;
    Return *b = first->before(s(),h);
    if (!b) b=first;
    e().pos--;
    Mark t=b->point;
    if (!!t->above() || t.pos || b->left) t.traverse();
    set_output_window(outwin);
    detect_margin();
    wrap=MP_False;
    fill_column = xsize-16;
    draw(&EditWindow::do_nothing,
	 &EditWindow::do_nothing,
	 &EditWindow::test_word_wrap,
	 &EditWindow::test_word_wrap_end,
	 &EditWindow::test_word_wrap_end,
	 t, b, 0);
    unset_output_window();
    startp=first->before(start,h);
    if (!startp) startp=first;
    start=startp->point;
    if (!!start->above() || start.pos || startp->left) start.traverse();
    xp = x;
    yp = y;
    sl->set_old();
}

// recalculate the height of all the lines in a document (without
// actually drawing it)

void EditWindow::recalc_lineheight()
{
  set_output_window(outwin);
  Mark pm;
  pm=root;
  pm.pos=0;
  draw(&EditWindow::do_nothing,
       &EditWindow::do_nothing,
       &EditWindow::test_nothing,
       &EditWindow::test_nothing,
       &EditWindow::test_go_back,
       pm, first, -5000);
  unset_output_window();
}

void EditWindow::word_wrap_full()
{
    if (!outwin) return;
    set_output_window(outwin);
    detect_margin();
    Mark pm;
    int h;
    pm=root;pm.pos=0;
    lsp()=Empty;
    lspp=0;
    wasnewline=0;
    wrap=MP_True;
    fill_column = xsize-16;
    draw(&EditWindow::do_nothing,
	 &EditWindow::do_nothing,
	 &EditWindow::test_word_wrap,
	 &EditWindow::test_nothing,
	 &EditWindow::test_nothing,
	 pm, first, 0);
    unset_output_window();
    startp=first->before(start,h);
    if (!startp) startp=first;
    start=startp->point;
    if (!!start->above() || start.pos || startp->left) start.traverse();
    xp = x;
    yp = y;
    ps().set_old();
    ss().set_old();
    ts().set_old();
}

void EditWindow::append_string(Char *c, unsigned int nr)
{
    unsigned int i;
    int h;
    Return *b;
    Mark p;

    p=root;
    for (i=0; i<nr; i++)
	if (!c[i]) c[i]=' ';
    p.pos=root->size();
    s()=p;
    p->insert_string(p.pos, c, nr);
    if (!outwin) return;
    edit_set_number_of_lines(outwin,root->lines()+1);
    x=xp;
    y=yp;
    s()=out_stack(s());
    b=startp->before(s(),h);
    if (!b) b=startp;
    p=b->point;
    if (!!p->above() || p.pos || b->left) p.traverse();
    e()=root;
    e().pos=root->size();
    set_output_window(outwin);
    detect_margin();
    if (x!=-1) put_mark(x,y);
    draw(&EditWindow::do_visible,
	 &EditWindow::do_test,
	 &EditWindow::test_begin_line,
	 &EditWindow::test_all_end,
	 &EditWindow::test_nothing,
	 p, b, h);
    if (x!=-1) put_mark(x,y);
    unset_output_window();
    xp = x;
    yp = y;
}

void EditWindow::append_node(Node *n)
{
    int h;
    Return *b;
    Mark p;

    p=root;
    p.pos=root->size();
    s()=p;
    p->paste(p.pos,n);
    if (!outwin) return;
    edit_set_number_of_lines(outwin,root->lines()+1);
    x=xp;
    y=yp;
    s()=out_stack(s());
    b=startp->before(s(),h);
    if (!b) b=startp;
    p=b->point;
    if (!!p->above() || p.pos || b->left) p.traverse();
    e()=root;
    e().pos=root->size();
    set_output_window(outwin);
    detect_margin();
    if (x!=-1) put_mark(x,y);
    draw(&EditWindow::do_visible,
	 &EditWindow::do_test,
	 &EditWindow::test_begin_line,
	 &EditWindow::test_all_end,
	 &EditWindow::test_nothing,
	 p, b, h);
    if (x!=-1) put_mark(x,y);
    unset_output_window();
    xp = x;
    yp = y;
}

void EditWindow::do_one_line()
{
    set_drawstyle(SMART);
    x = y = -1;
}

void EditWindow::sel_func(void *data)
{
    sel_pos() = (Node*) data;
}

void EditWindow::do_search()
{
    search = MP_True;
    sel_pos() = Empty;
    set_search_func(sel_func, x, y);
}

void EditWindow::do_visible()
{
    set_drawstyle(VISIBLE);
    x = y = -1;
}

void EditWindow::do_shades()
{
    set_drawstyle(SHADES);
    x = y = -1;
}

void EditWindow::do_shades_noxor()
{
  if (cs->test_noxor(m())) {
    set_drawstyle(VISIBLE);
  } else {
    set_drawstyle(INVISIBLE);
  }
  x = y = -1;
}

void EditWindow::do_full_test()
{
    if (ps().test(m())) {
	out_cursor(MARK);
    }
}

void EditWindow::do_test()
{
    if (ps().test(m())) {
	out_cursor(MARK);
    }
}

void EditWindow::do_test_noxor()
{
    if (cs->test(m())) {
	out_cursor(MARK);
    }
}

void EditWindow::do_nothing()
{ }

void EditWindow::test_begin_line(const Mark& mt)
{
    if (s() == mt) set_drawstyle(VISIBLE);
    test_all_begin(mt);
}

void EditWindow::test_end_line(const Mark& mt)
{
    if (e() == mt) set_drawstyle(SMART);
    test_all_end(mt);
}

void EditWindow::test_end_multiline(const Mark& mt)
{
  if (e() == mt) { smart=MP_True;brk=MP_True;set_drawstyle(INVISIBLE); }
  test_all_end(mt);
}

void EditWindow::test_begin(const Mark& mt)
{
    cs->test_begin(mt,cf);
}

void EditWindow::test_end(const Mark& mt)
{
    cs->test_end(mt,cf);
}

void EditWindow::test_begin_noxor(const Mark& mt)
{
    cs->test_begin(mt,cf);
    ps().test_begin(mt, switch_reverse);
    ss().test_begin(mt, switch_thick);
    ts().test_begin(mt, switch_thin);
}

void EditWindow::test_end_noxor(const Mark& mt)
{
    cs->test_end(mt,cf);
    ps().test_end(mt, switch_reverse);
    ss().test_end(mt, switch_thick);
    ts().test_end(mt, switch_thin);
}

void EditWindow::test_all_begin(const Mark& mt)
{
    if (wrap && !wrapped && where_x() > (Offset) fill_column) {
	wrapped= ps().test_wrap(mt);
    }
    ps().test_begin(mt, switch_reverse);
    ss().test_begin(mt, switch_thick);
    ts().test_begin(mt, switch_thin);
}

void EditWindow::test_all_end(const Mark& mt)
{
    ps().test_end(mt, switch_reverse);
    ss().test_end(mt, switch_thick);
    ts().test_end(mt, switch_thin);
}

void EditWindow::stop_end(const Mark& mt)
{
    if (e()==mt) {
	if (smart) {
	    brk = MP_True;
	    set_drawstyle(INVISIBLE);
	} else {
	    brk = MP_False;
	    set_drawstyle(VISIBLE);
	}
    }
}

void EditWindow::stop_end_line(const Mark& mt)
{
    if (e()==mt) {
	if (smart) {
	    brk = MP_True;
	    set_drawstyle(SMART);
	} else {
	    brk = MP_False;
	    set_drawstyle(VISIBLE);
	}
    }
}

void EditWindow::test_word_wrap(const Mark& mt)
{
    if (wrap) {
	Mark pm=mt;
	Char c=pm.traverse();
	if (!!lsp() && where_x() > (Offset)fill_column) {
	    lsp()->change_to(Newline,lsp().pos);
	    lspp=where_x()-lspp;
	    out_char(Newline);
	    set_y(-5000);
	    thinspace(lspp);
	    lspp=0;
	    lsp()=Empty;
	    wasnewline=0;
	}
	if (mt->text() && (IsNewline(c) || c==' ')) {
	    if (IsNewline(wasnewline) && IsNewline(c)) {
		lsp()->change_to(wasnewline, lsp().pos);
		lsp()=Empty;
		lspp=0;
		wasnewline=0;
		set_y(-5000);
	    } else {
		lsp()=mt;
		if (IsNewline(c) && !IsDisp(wasnewline))
		    lsp()->change_to(' ',lsp().pos);
		lspp=where_x();
		wasnewline=c;
	    }
	} else if (IsNewline(c)) {
	    if (IsNewline(wasnewline)) lsp()->change_to(wasnewline, lsp().pos);
	    wasnewline=c;
	    lsp()=mt;
	    lspp=where_x();
	    set_y(-5000);
	} else if (IsDisp(c) || IsTabOpen(c) || IsTabClose(c)) {
	    if (IsNewline(wasnewline)) {
		lsp()->change_to(wasnewline,lsp().pos);
		lspp=where_x()-lspp;
		out_char(Newline);
		set_y(-5000);
		//thinspace(lspp);
		lspp=0;
		lsp()=Empty;
	    }
	    wasnewline=MP_Disp;
	} else if (!IsPh(c) && (c<InDisp || c>AskText))
	    wasnewline=0;
    } else if (mt == s()) wrap=MP_True;
}

void EditWindow::test_word_wrap_end(const Mark& mt)
{
    if (mt == e()) { wrap=MP_False; smart=MP_True;brk=MP_True; }
}

void EditWindow::test_go_back(const Mark&)
{
  set_y(-5000);
}

void EditWindow::test_nothing(const Mark&)
{ }

// The draw function is used for all redrawing procedures.
void EditWindow::draw(Pfv run, Pfv body, Pfm tb, Pfm te, Pfm br,
		      const Mark& st, Return *stp, int height)
{
    int hy = 0;
    search = MP_False;
    m() = st;
    Mark h;
    Index j = m()->depth();
    Index i;
    Index dd = 0;
    Return *mp = NULL;
    Mark sm;
    Bool begun = MP_False;

    smart = MP_True;
    sm = Empty;
    h=m();
    push_fontgroup(0);
    while (!!h->above()) {
	if (IsDisp(h(0))) dd++;
	if (h->op()) {
	    for (int n=-(int)h.pos; n<0; n++)
		if (IsTabOpen(h(n))) dd++;
		else if (IsTabClose(h(n))) dd--;
	}
	h=h->above();
    }
    if (IsDisp(h(0))) dd++;
    if (stp->tabs && !dd) {
	tab_unlock(stp->tabs);
	stp->tabs=0;
    } else if (!stp->tabs && dd) {
	m() = adjust_mark(st,MP_True);
	j = m()->depth();
	dd = 0;
    }
    set_tab_stack(stp->tabs, dd);
    set_drawstyle(INVISIBLE);
    i=0;
    while (i<=j) {
	int n = j-i;
	h = m();
	while (n) {
	    h = h->above();
	    n--;
	}
	open_node((Node*) h);
	if (!!h->above()) {
	    if (h->id() || h->var()) {
		set_italic(MP_True);
	    } else if (h->op()) {
		for (n=-(int)h.pos; n<0; n++) {
		    if (Important(h(n)) || output_important())
			out_char(h(n));
		}
	    }
	}
	i++;
    }
    adjust_lineheight();
    h = m();
    if (ps().window==this && ps().contains(m()))  switch_reverse();
    if (ss().window==this && ss().contains(m()))  switch_thick();
    if (ts().window==this && ts().contains(m()))  switch_thin();
    brk = MP_False;
    for (;!brk;) {
	if (m() == st) {
	    set_y(height);
	    mp = stp;
	    hy = height;
	    begun = MP_True;
	    (*run)();
	    if (search && y<height) break;
	}

	(*tb)(m());
	(*body)();
	(*te)(m());

	Char c = h.traverse();
	if (IsPh(c)) {
	    if (h.pos == 0) {
		if (h->size() == 0) {
		    open_node((Node*) h);
		    if (IsDisp(c)) {
			set_display_delta(h->display_delta());
			open_display();
		    }
		    if (IsOp(c) && m()->expr() && IsExpr(m()(-1))) {
			out_char(h->opspace());
		    }
		    if (IsDispOrExpr(c) && h->parens()) out_char('(');
		    if (IsText(c)) out_text_delim(ON);
		    (*tb)(h);
		    out_char(c);
		    if (this == &(findwindow()) || this == &(replacewindow()))
			out_index(h->find_nr());
		    (*te)(h);
		    if (IsText(c)) out_text_delim(OFF);
		    if (IsDispOrExpr(c) && h->parens()) out_char(')');
		    if (IsOp(c) && m()->expr() && IsExpr(m()(1))) {
			out_char(h->opspace());
		    }
		    if (IsDisp(c)) close_display();
		    close_node();
		    m() = h;
		    if (search && x<where_x() && !sm) sm = m();
		    h.traverse();
		    m() = h;
		} else {
		    open_node((Node*) h);
		    switch (Ph(c)) {
		    case MP_Disp:
			set_display_delta(h->display_delta());
			open_display();
		    case MP_Expr:
			if (h->parens()) out_char('(');
			break;
		    case MP_Op:
			if (m()->expr() && !IsTab(h(0)) && IsExpr(m()(-1)))
			    out_char(h->opspace());
			break;
		    case MP_Var:
			set_italic(MP_True);
			break;
		    case MP_Text:
			out_text_delim(ON);
			break;
		    default:
			break;
		    }
		    if (h->op()) set_default_thinspace(h->opspace(MP_True));
		    if (h->id()) set_italic(MP_True+h->stencil_nr());
		    m() = h;
		}
	    } else {
	        if (m()->id()) set_italic(MP_False);
		switch (Ph(c)) {
		case MP_Disp:
		    if (m()->parens()) out_char(')');
		    close_display();
		    break;
		case MP_Expr:
		    if (m()->parens()) out_char(')');
		    break;
		case MP_Op:
		    if (h->expr() && !IsTab(m()(-1)) && IsExpr(h(0)))
			out_char(m()->opspace());
		    break;
		case MP_Var:
		    set_italic(MP_False);
		    break;
		case MP_Text:
		    out_text_delim(OFF);
		    break;
		default:
		    break;
		}
		close_node();
		if (h->op()) set_default_thinspace(h->opspace(MP_True));
	        if (search && x<where_x() && !sm) sm=m();
	        m() = h;
	    }
	} else if (c == 0) {
	    if (mp && mp->right) {
		Return *mpt = mp->right;
		Return *mph;
		while (mpt->right) {
		    mph = mpt->unlink();
		    delete mpt;
		    mpt = mph;
		}
		set_smart_height(smart?mp->height:0);
		mp = mpt;
	    }
	    if (where_y()>=0) set_drawstyle(VISIBLE);
	    out_char(Newline);
	    if (mp) {
		if ((Index)(where_y()-hy) !=mp->height) smart = MP_False;
		if (!tab_equal(mp->tabs)) {
		    tab_unlock(mp->tabs);
		    mp->tabs = tab_lock();
		    smart = MP_False;
		}
		mp->height = where_y()-hy;
	    }
	    hy = where_y();
	    if (search && y<hy)
		if (!sel_pos()) {
		    if (!!sm) m() = sm;
		} else if (sel_pos()->text()) {
		    if (!!sm)
			if ((Node*)sm == (Node*)sel_pos())
			    m() = sm;
			else {
			    m() = sel_pos();
			    m().pos = m()->size();
			}
		} else {
		    m() = sel_pos();
		    m().pos = 0;
		}
	    (*br)(m());
	    break;
	} else if (IsNewline(c)) {
	    // Must be treated apart for searches
            if (mp) {
		mp = mp->update_right(m());
		set_smart_height(smart?mp->height:0);
	    }
	    out_char(c);
	    if (mp) {
		if ((Index)(where_y()-hy) !=mp->height) smart = MP_False;
		if (!tab_equal(mp->tabs)) {
		    tab_unlock(mp->tabs);
		    mp->tabs = tab_lock();
		    smart = MP_False;
		}
		mp->height = where_y()-hy;
	    }
	    hy = where_y();
	    if (search && y<hy) {
		if (!sel_pos()) {
		    if (!!sm) m() = sm;
		} else if (sel_pos()->text()) {
		    if (!!sm)
			if ((Node*)sm == (Node*)sel_pos())
			    m() = sm;
			else {
			    m() = sel_pos();
			    m().pos = m()->size();
			}
		} else {
		    m() = sel_pos();
		    m().pos = 0;
		}
		break;
	    } else
		sm = Empty;
	    (*br)(m());
	    hy = where_y();
	    m() = h;
	    if (!smart && brk) brk=MP_False;
	    if (begun && ysize < hy) brk=MP_True;
	} else {
	    out_char(c);
	    if (search && x<where_x() && !sm) sm=m();
	    m() = h;
	}
    }
    pop_fontgroup();
    // Clean up
    clear_to_end_of_page();
    search = MP_False;
}

void EditWindow::backup(Node *pn)
{
    root->insert(0, Newline, 1);
    root->paste(0, pn);
    redraw_full();
}
