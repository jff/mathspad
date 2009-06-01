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
// mathpad.hh

#if !defined MATHPAD_HH
#define MATHPAD_HH

class Node;
class Mark;
class Marker;
class Select;
class EditWindow;

#define Empty ((Node*)0)

extern Select& ps();
extern Select& ss();
extern Select& ts();
extern Select& ops();

extern EditWindow& miniwindow();
extern EditWindow& scratchwindow();
extern EditWindow& findwindow();
extern EditWindow& replacewindow();

extern Node *killnode;
extern Node *lastkilled;

extern Bool use_xor;

#define FRWindow(A) ((A)==&(findwindow()) || (A)==&(replacewindow()))

#define IsExpr(A) (Ph(A)==MP_Expr)
#define IsOp(A)   (Ph(A)==MP_Op)
#define IsText(A) (Ph(A)==MP_Text)
#define IsVar(A)  (Ph(A)==MP_Var)
#define IsId(A)   (Ph(A)==MP_Id)
#define IsDisp(A) (Ph(A)==MP_Disp)
#define IsDispOrExpr(A) (IsExpr(A) || IsDisp(A))

/*
extern Bool IsExpr(Char c);
extern Bool IsText(Char c);
extern Bool IsOp(Char c);
extern Bool IsId(Char c);
extern Bool IsVar(Char c);
extern Bool IsDisp(Char c);
extern Bool IsDispOrExpr(Char c);
*/
typedef void (*Pfv)();
typedef void (*Pfm)(const Mark&);

#endif
