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
// mark.hh

#if !defined MARK_HH
#define MARK_HH

class Mark {

public:

    Index pos;

          operator Node* () const { return  node; }
    Bool  operator !     () const { return !node; }
    Node* operator ->    () const { return  node; }
    Node& operator *     () const { return *node; }

    Char  operator ()    (Offset i) const;

    Mark();
    Mark(const Mark& m);
    Mark(Node* pn, Index i);

    virtual ~Mark();
    virtual Mark& operator = (const Mark& m);
    virtual Node* operator = (Node* pn);

    const Mark& base()  const;
    const Mark& above() const;
          Node* under() const;
          Node* left()  const;
          Node* right() const;

    Char traverse();
    Char esrevart();
    Bool find(Char *str, int n);
    Bool findback(Char *str, int n);
    Bool find_stencil(Index nnr);
    Bool findback_stencil(Index nnr);
    Bool find_tree(Node *n);
    void cond_right(int n);
    void cond_left(int n);
    void close_parens();

    void nextline();
    void prevline();

    friend Bool operator == (const Mark& m1, const Mark& m2);
    friend Bool operator != (const Mark& m1, const Mark& m2);

protected:

    Node* node;

};

#endif
