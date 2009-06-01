#ifndef MP_MATHPAD_H
#define MP_MATHPAD_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
/* mathpad.h */
/* Various odds and ends that are useful in most of the system */

typedef unsigned int Index;
typedef int Offset;

typedef enum Opkind { None, Prefix, Postfix,
                      Infix, LeftBinding, RightBinding
                    } Opkind;

/* Char might be defined in an included header file in C++ */


#include "unicode.h"

#define Char Uchar


#define Newline     0xF7FF
#define Settab      0xF7FE
#define Rtab        0xF7FD
#define Ltab        0xF7FC
#define Tabplus     0xF7FB
#define Tabminus    0xF7FA
#define Pushtabs    0xF7F9
#define Poptabs     0xF7F8
#define SoftNewline 0xF7F7
#define AskText     0xF7F6
#define InText      0xF7F5
#define InMath      0xF7F4
#define InDisp      0xF7F3
#define CloseStack  0xF7F2
#define StartHide   0xF7F1
#define PopSize     0xF7F0
#define OpenTop     0xF7EF
#define CloseTop    0xF7EE
#define OpenBottom  0xF7ED
#define CloseBottom 0xF7EC
#define OpenGap     0xF7EB
#define CloseGap    0xF7EA
#define GlueSpace   0xF7E9
#define GlueLine    0xF7E8
#define GlueStipple 0xF7E7
#define StackB      0xF7E6
#define StackC      0xF7E5
#define TopGap      0xF7E4
#define GapBottom   0xF7E3
#define StackClose  0xF7E2
#define EndHide     0xF7E1
#define TabOpen     0xF7E0
#define TabClose    0xF7DF
#define DisplayOpen 0xF7DE
#define DisplayClose 0xF7DD
#define VerLine     0xF7DC
#define ThinSpace   0xF7DB
#define AskMath     0xF7DA
#define AskBoth     0xF7D9
#define PlName      0xF7D8
#define PlNameEnd   0xF7D7
#define ColorStart  0xF7D6
#define ColorSep    0xF7D5
#define ColorEnd    0xF7D4
#define TabCodes    0xF7D0
#define MP_Expr        0xF700
#define MP_Op          0xF710
#define MP_Id          0xF720
#define MP_Var         0xF730
#define MP_Text        0xF740
#define MP_List        0xF750
#define MP_Disp        0xF760
#define NodeCode    0xF700
#define SpaceCode   0xF600

#define SpaceFont   0xF6
#define StackFont   0xF5
#define FontFont    0xF4
#define SizeFont    0xF3
#define PopFont     0xF2
#define AttribFont  0xF1
#define AttrPopFont 0xF0

#define FirstOpCode 0xF000
#define LastOpCode 0xF7FF

#define AttribGroup(A) ((int)(((A)&0xf0)>>4))
#define AttribValue(A) ((int)((A)&0xf))

#define IsTab(c) ((TabCodes <= (c)) && ((c)<=Newline))
#define IsNewline(c) ((c)==Newline || (c)==SoftNewline || \
		      (c)=='\n' || (c)==0xA || (c)==0xC || \
		      (c)==0xD || (c)==0x2028 || (c)==0x2029)
#define Num2Tab(c) (((c)^0xFFFF)-0x800)

typedef enum { Normal, Reverse, Xor } TextMode;
#define SecondSel 0x1
#define ThirdSel  0x2
#define PrimSel   0x4

#define MaxPrecedence 20

#define Ph(c) ((c) & 0xFFF0)
#define Num(c) ((c) & 0xF)
#define PhNum2Char(p,n) ((p) | (n))
#define IsPh(c) (NodeCode <= (c) && (c) < TabCodes)
#define Char2Node(c) (IsPh(c) ? NodeCode : (c))
#define Char2Ph(c) (IsPh(c) ? Ph(c) : (c))

#define Ph2Num(c) (((Char)((c) & 0xF0)) >> 4)
#define Num2Ph(c) (((c) << 4) | NodeCode)

#define Opspace(c) ((c) | SpaceCode)
#define IsOpspace(c) (((c) & SpaceCode) == SpaceCode)

#define Font2Char(f,as) ((Char)(((f) << 8) | ((as) & 0xFF)))
#define Char2Font(c) ((c) >> 8)
#define Char2ASCII(c) ((c) & 0xFF)

#define IsOpCode(c)  (FirstOpCode<=(c) && (c) <=LastOpCode)
#define IsImportantOpen(c) ((c)==ColorStart || (c)==PlName)
#define IsImportantClose(c) ((c)==ColorSep || (c)==PlNameEnd)


#ifndef Bool
#define Bool int
#define MP_True 1
#define MP_False 0
#endif

#define LDEFMODE 0
#define LTEXTMODE 1
#define LMATHMODE 2
#define LBOTHMODE 3

/* for parsing default added "templates" (E), $E$, \mbox{T} */
#define INTERNAL_BRACES 0xffff
#define INTERNAL_EXPRESSION 0xfffe
#define INTERNAL_DISPLAY 0xfffd
#define INTERNAL_TEXT 0xfffc

typedef void (*Cpfv)(void);

/* assignments in guard: disable the warnings with aig()
** GCC/G++:       aig(Assign) ((Assign))
** Borland C++:   aig(Assign) (0!=(Assign))
** Combined:      aig(Assign) (0!=((Assign)))
*/

#define aig(A) ((void*)(0!=((A))))

extern Char *translate(char *string);

#ifndef WIN95

#define DIRSEPCHAR '/'
#define DIRSEPSTR "/"

#else

#define DIRSEPCHAR '\\'
#define DIRSEPSTR "\\"

#endif

#endif
