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
// node.hh

#if !defined NODE_HH
#define NODE_HH

void  delete_copied_name(Char* n);
#if !defined stencil_screen
extern "C" {
#include "notatype.h"
}
#endif

extern Bool text_only;
extern Bool all_parens;
extern void cleanup_nodestack(void);

class Node {

public:

    Char& operator [] (Index pos) const {
	return *((stencil ? stencil_screen(innr) : (pos<size1?p1:p2))+pos);}
    Char  operator () (Index pos) const;

    Bool operator == (const Char* s) const;
//    int  cmp(const Char* s) const;

    Node(const Node* pn);
    Node(Char nk);
    ~Node();

    void clean_up();
    void parse_error(Char *txt, int len);
    Node* next();
    Mark  next_text();
    Mark  next_node();
    Node* tree_walk(Node *stop);
    Node* tree_walk_skip(Node *stop);

    Bool  check_ph(Node *n);
    Index number_empty();
    Bool  expr() const { return IsDispOrExpr(_kind) && !stencil &&
			 (!(size1+size2) || _first); }
    Bool  id() const { return IsId(_kind)||(IsDispOrExpr(_kind) && !stencil &&
					    (size1+size2) && !_first); }
    Bool  op() const { return IsOp(_kind) || (IsDispOrExpr(_kind)&&stencil); }
    Bool  text() const { return IsText(_kind); }
    Bool  var() const { return IsVar(_kind); }
    Bool  normal_identifier() const;

    void  change_to(Char newval, Index pos);
    Bool  transpose_chars(Index &pos, Index n);
    Bool  transpose_words(Index &bp, Index &ep, Index &bq, Index &eq);
    Bool  insert(Index pos, Char c, Index count = 1);
    Bool  insert(Index innr);
    void  insert_string(Index pos, Char* s, Index count = 0);
    Bool  remove(Index begin, Index end);
    Bool  remove(Index begin, Offset count);
    Node* cut(Index begin, Index end);
    Bool  paste(Index pos, Node* pn);
    Node* copy(Index begin, Index end) const;
    Char* copy_name();
    Char* content_ref();

    Bool  can_rename(Char *name);
    void  rename(Index begin, Index end, Char *oldn, int ol, Char *newn, int nl);
    Node* replace(Index pos, Node* n);
    void  replaces(Node* n);
    Node* replace_tree(Node *oldn, Node *newn, Index pos);
    Node* replace_notation(Index oldnnr, Index newnnr);
    Node* replacestr(Char *oldstr, const Index n, Char *newstr, const Index m,
		     Index pos);
    Bool  matches(Char *str, Index n, Index pos);
    Bool  match_tree(Node *n, Index pos, Node **nlist = 0, Char *tlist = 0);
    Mark  search_label(Char *str);

    void  unlink();

    Node* search(Index prec, Opkind k);
    Node* notation(Index innr);
    Offset search_new_version();
    Offset notation_nr(Offset *vnr);
    void  new_version(Offset nnr);
    void  new_id_font(Offset nfnr);
    Offset id_font();

    void  commute(Index begin, Index end);
    Bool  distribute(Index begin, Index end, Node* fn, Node* fa,
		     Mark &mf, Mark &ma);
    Bool  factorise(Index begin, Index end, Node* fn, Node* fa);

    Node* fcopy(Index begin,Index end,Node* an,Index ab,Index ae,Node*& fa);
    Bool  match(Node* fn, Node* fa, Node*& an, Index& ab, Index& ae);

    Node* under(Index pos);
    Node* left_of(Index pos);
    Node* right_of(Index pos);
    const Mark& above();
    Node* right() const;
    Node* left() const;
    Node* first();
    Node* last();

    Index size() const { return (stencil? stencil_size(innr): size1 + size2); }
    Index depth() const;
    Index height();
    Bool  is_stencil() const { return stencil; }
    Index stencil_nr() const { return innr; }

    Char kind() const { return _kind; }
    int  display_delta() const { return _display_pos; }

    Char find_nr() const { return _findnr; }
    void set_find_nr(Char c) { _findnr = c; }
    void set_find_nr_rec(Char& c);
    void clear_find_nr() {_findnr = 0; }
    void clear_find_nr_rec();
    void fill_find_nr(Char* nlist, Char &n);
    Char first_unused_find_nr();
    Char last_used_find_nr();

    Bool equal_diff(Node *compare_with, Node **leftres, Node **rightres);
    void apply_leibnitz();

    Bool parens() const { return _parens; }
    void set_parens(Bool b);
    void clr_parens() { changed=changed||_parens; _parens = MP_False; }

    Bool can_be_raised();  // Reality check for Select::raise()
    Bool may_be_raised();  // Sanity check for Select::raise()

    void display_left(Index Count);
    void display_right(Index Count);
    void increase_spacing(Index Count);
    void decrease_spacing(Index Count);
    void reset_spacing();
    Char opspace(Bool ins = 0);

    Index preced() const { return (stencil? stencil_prec(innr):0); }
    Index min_preced();
    Index max_preced();

    Opkind opkind() const { return (stencil? stencil_kind(innr):None); }

    Bool poll();

    Index lines();

    void de_alias_node();
    void de_alias_tree();

    friend void Marker::link();
    friend void Marker::unlink();

    void * parsed_node(Char type, Char *text, int len, int nnr, int opspace);
    void * error_node(Char *text, int len);
    void join_stack(void);

    Offset load(Char *str, int *len, int max);
    Bool old_load(FILE* f);
    Bool old_old_load(FILE* f);
    Offset load_ascii(Char *str, int *len, int max);
    void get_stack();
    void save();
    void latex(Index begin = 0, Index end = 0);
    Mark latex_line(Index &n);

private:

    Bool set_gap(Index pos, Index min_size = 0);
    void var_comma_adjust(Index& begin, Index& end);

        // Not defined.
    Node& operator = (const Node&);

        // Structure

    Marker father;   // Pointer to father + position in father's string.
    Node* _left;     // Pointer to left brother.
    Node* _right;    // Same right.
    Node* _first;    // Same first of the sons.
    Node* _last;     // Same last of the sons.

        // Representation (if it s not a stencil)

    Char* p1;       // Pointer to block of Chars in memory.
    Char* p2;       // Pointer into the block p1 points to.
    Index size1;    // Number of Chars used before the gap.
    Index size2;    // Same after the gap.
    Index gap;      // Size of the gap in Chars.
    // Note: (p2 == p1 + gap) | ((p1 == 0) & (p2 == 0)) Holds.

        // Other

    Bool stencil;   // Node contains a stencil.
    Index innr;     // Number of the used stencil and version
    Marker* list;   // List of markers pointing into the node.
    Bool changed;   // True: node has changed since last poll().
    Char _kind;     // Kind of node: placeholder + number.
    Char _findnr;   // Number of the empty placeholder in a find/replace.
    Bool _parens;   // If Ph(_kind)==MP_Expr: parentheses?
    Offset _opdelta; // user added modification to _opspace.
    Offset _display_pos; // user added change to display indentation
    Index _lines;   // Number of lines in a node.

};

// Sizes for the string part of a Node
// Needed: (Offset)Nmax > 0
//         Nmod == Nmin << k, for some k >= 0
//         Nmax == Nmod * n, for some n > 0
#define Nmin 0x4
#define Nmod 0x800
#define Nmax 0x10000000

#define NewNodes 200
#endif
