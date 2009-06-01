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
// select.hh
// Definition of class Select

#if !defined SELECT_HH
#define SELECT_HH

class Select {
    Marker begin;
    Marker end;
    Mark oldbegin;
    Mark oldend;
public:
    EditWindow* window;

public:

    Index size() { return end.pos - begin.pos; }
    Index node_size() { return begin->size(); }
    Char kind() { return begin->kind(); }

    Index bpos() { return begin.pos; }
  Mark  bmark() { return begin; }   

    Bool make_visible();
    void forward(Index n);
    void backward(Index n);
    void forward_line(Index n);
    void backward_line(Index n);
    void begin_of_line();
    void end_of_line();
    void recenter();
    void move_to_center();
    void begin_of_buffer();
    void end_of_buffer();
    void forward_word(Offset n);
    Bool transpose_chars(Index n);
    Bool transpose_words(Index n);
    void up();
    void down();
    void into();
    void to_right();

    Select();
    ~Select();

         operator Bool () const { return begin != Empty; }
    Bool operator !    () const { return begin == Empty; }

    Select& operator = (const Select& s);
    Select& operator = (const Mark& m);

    void unset();  // re-initialise the selection
    void set_begin();

    Bool next_node_or_text();
    Bool next_node_or_insert(Char c, Index count);
    Bool next_node_insert(Char c, Index count);
    Bool openparen_insert(Char c, Index count);
    Bool closeparen_insert(Char c, Index count);
    Bool make_list_insert(Char c, Index count);
    Bool insert(Char c, Index count = 1);
    void set_index_nr(Index c);
    void insert_stencil(Index nnr);
    void insert_string(Char *s);
    void include(FILE* f);
    void include_ascii();
    int  notation_nr(Offset *vnr);
    int  id_font();
    void new_id_font(Index nfnr);
    void new_version(Index nnnr);
    Bool remove(Offset count = 0);
    Bool remove_double_chars();

    Bool kill(Bool front = MP_False);
    Bool kill_word(Offset count);
    Bool kill_line(Offset count);
    void yank();

    void raise();
    void lower();
    void set_parens();
    void unset_parens();
    void display_left(Index count);
    void display_right(Index count);
    void increase_spacing(Index count);
    void decrease_spacing(Index count);
    void reset_spacing();
    void switch_parens();
    void clear_parens();

    void commute();
    Bool func_selected(Select& f, Select& a);
    Bool distribute(Select& f, Select& a);
    Bool factorise(Select& f, Select& a);
    void apply(Select& f, Select& a);
    void rename(Select& n, Select& s);

    void restore(const Mark&);
    void restore(Mark, Mark);
    void dbl_click(void);
    void no_dbl_click(void);
    void combine(const Select&);
    void select_all(void);

    // void select_line(const Mark &start, Mark & s, Mark& e, int nr);
    Bool select_line(Mark & s, Mark& e);
    Bool test_wrap(const Mark& m) const;
    void test_begin(const Mark& m, Cpfv cb) const;
    void test_end(const Mark& m, Cpfv cb) const;
    void test_begin_before(const Mark& m, Cpfv cb) const;
    void test_end_after(const Mark& m, Cpfv cb) const;
    void noxor_sel(void);
    Bool test_noxor(const Mark& m);
    Bool test(const Mark& m);

    void set_old();
    void unset_old();
    void copy_old(Select &sl);
    void update();

    void stack_position();
    void use_stack();
    void clear_stack();
    void clear_stack_and_use();
    void latex_line(Index &n);
    void latex();
    void clear_replacestop();
    Bool find_tree(Node *);
    void replace_tree(Node *, Node *);
    void replace_tree_all(Node *, Node *);
    Bool find(Char *);
    Bool findnext(Char *);
    Bool findwrap(Char *);
    Bool findback(Char *);
    Bool findprev(Char *);
    Bool findwrapback(Char *);
    Bool find_replace(const Select& s, Char *);
    Bool findnext_replace(const Select& s, Char *);
    void replace(Char *, Char *);
    Bool find_stencil(Index);
    Bool findnext_stencil(Index);
    Bool findwrap_stencil(Index);
    Bool findback_stencil(Index);
    Bool findprev_stencil(Index);
    Bool findwrapback_stencil(Index);
    Bool find_replace_stencil(const Select& s, Index);
    Bool findnext_replace_stencil(const Select& s, Index);
    void replace_notation(Index, Index);

    int  select_path(const Select& sub, int *list, int max);
    Bool contains(const Select& s) const;
    Bool contains(const Mark& m) const;
    friend Bool overlap(const Select&, const Select&);

    friend void swap(Select&,Select&);
    friend void copy(Select& t, Select& s);
private:
    static Mark &stackbegin(int);
    static Mark &stackend(int);
    static Mark &start();
};

#endif
