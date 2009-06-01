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
// editwindow.hh

#if !defined EDITWINDOW_HH
#define EDITWINDOW_HH

class Return {

public:
    Marker point;
    Index height;
    Return *left;
    Return *right;
    void *tabs;

    Return();
    Return(const Mark& m);
    Return(const Mark& m, Index i);
    Return(Node *pn, Index i);

    ~Return();

    Return* link_left(const Mark& m);
    Return* link_right(const Mark& m);
    Return* unlink();
    Return* under(const Mark& m);
    Return* before(const Mark& m, int &x);
    void print();
    Bool check();

    Return* update_right(const Mark& m);
    Return* update_left(const Mark& m);
};

extern Bool move_selection;

class EditWindow {
    Node* root;
    Marker start;
    Index at_line;
    void* outwin;
    EditWindow* next;
    Return* first;
    Return* last;
    Return* startp;
    int xp, yp;
    int xsize, ysize;
public:
    EditWindow(void* w, int xs, int ys);
    ~EditWindow();

    Mark start_pos();
    Bool poll() { return root->poll(); }
    Bool empty() { return (root->size()==0); }
    void save();
    void old_load(FILE* f);
    void load_ascii();
    void old_include(FILE* f);
    void include_ascii();
    void latex() { root->latex(); }
    Mark latex_line(Index &n);
    void clear();
    void set_fill_column(Index n);
    void set_wrap(Bool toggle);
    void clear_tabs();
    void clear_all_tabs();
    void cleanup() { root->clean_up(); }
    void backup(Node *);

  // calculate the height of a line, relative to the current topline.
  // oneline will return the height of exactly that line.
  // sumline will return the height of all the lines up to that line.
  // it returns 0 if the lineheight information is not available.
    int height_of_line(int line, int &oneline, int &sumline);

    void redraw_cursor(Select* ps, Cpfv f);
    void redraw_cursor_xor(Select* ps, Cpfv f);
    void redraw_cursor_noxor(Select* ps, Cpfv f);
    void redraw_full();
    void redraw_between(int ystart, int yend);
    void redraw_line(int nr);
    void redraw_end_page(int nr);
    const Mark& findpos(int x, int y);
    void recalc_lineheight();
    void word_wrap_full();
    void word_wrap_selection(Select* ps);
    void append_string(Char *s, unsigned int nr);
    void append_node(Node *n);

    void setwin(int xs, int ys)
        { xsize = xs; ysize = ys; } 
    void setwin(void* w, int xs, int ys);

    Node* base() { return root; }
    EditWindow* window_with_base(Node *n);
    void set_base(Node *n) { root=n; }
    void scroll_up(Index n);
    void scroll_down(Index n);
    void start_to_line(Index line_number);
    void start_to_str(Char *c);
    void recalc_at_line();
    Bool make_visible(const Mark& m);
    void recenter(const Mark& m);
    void set_center(Mark& m);
    Index get_number_of_lines() { return root->lines(); }
    Index get_line_number() { return at_line; }

	// Windows '95
    int get_page_lines();
    int get_line_height(Index i);
    int get_x() {return xp;}
    int get_y() {return yp;}


private:

    static EditWindow* ewlist;

    void draw(Pfv run, Pfv body, Pfm tb, Pfm te, Pfm br,
	      const Mark& st, Return *stp, int height);
    void adjust_scrollbar();
        // Helper variables for draw
    static int x;
    static int y;
    static Mark& m();
    static Mark& s();
    static Mark& e();
    static Mark& sel_pos();
    static Bool brk;
    static Bool wrap;
    static Bool wrapped;
    static Offset fill_column;
    static Char wasnewline;
    static Mark& lsp();
    static Offset lspp; 
    static Bool smart;
    static Bool search;
    static Cpfv cf;
    static Select* cs;
        // Helper functions for draw
    static void sel_func(void* data);
    static void do_search();
    static void do_visible();
    static void do_one_line();
    static void do_shades();
    static void do_shades_noxor();
    static void do_full_test();
    static void do_test();
    static void do_test_noxor();
    static void do_nothing();
    static Mark adjust_mark(const Mark& m, Bool left);
    static Mark out_stack(const Mark& m);
    static void test_begin_line(const Mark& m);
    static void test_end_line(const Mark& m);
    static void test_end_multiline(const Mark& m);
    static void test_begin(const Mark& m);
    static void test_end(const Mark& m);
    static void test_begin_noxor(const Mark& m);
    static void test_end_noxor(const Mark& m);
    static void test_all_begin(const Mark& m);
    static void test_all_end(const Mark& m);
    static void test_word_wrap(const Mark& m);
    static void test_word_wrap_end(const Mark& m);
    static void test_nothing(const Mark& m);
    static void test_go_back(const Mark& m);
    static void stop_end(const Mark& m);
    static void stop_end_line(const Mark& m);
};

#endif

