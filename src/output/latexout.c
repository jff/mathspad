#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "mathpad.h"
#include "system.h"
#include "sources.h"
#include "fonts.h"
#include "output.h"
#include "latexout.h"
#include "memman.h"

#include "unitype.h"
#include "unistring.h"
#include "unimap.h"

#define LATEX_TAB(c)       (latex_tab[Num2Tab(c)])
#define ASCII_TAB(c)       (ascii_tab[Num2Tab(c)])
#define LATEX_PH(c)        (latex_ph[Ph2Num(c)])
#define ASCII_PH(c)        (ascii_ph[Ph2Num(c)])
#define LATEX_PHNUM(c)     (latex_phnum[Num(c)])
#define ASCII_PHNUM(c)     (ascii_phnum[Num(c)])

#define PLACENAMESIZE 40
static char placename[PLACENAMESIZE+1];
static int placepos=0;
static int nameset=0;
static int inplacename=0;

static
char *latex_tab[44] = { "\\\\\n\t", "\\=",  "\t\\>",    "\\<",
			      "\\+",      "\\-",  "\\push",   "\\pop",
			      "\n\t",     "",     "",         "",
			      "",         "}",    "",         "}",
			      "{",        "}",    "{",        "}",
			      "{",        "}",    "\\hfill",  "\\hrulefill",
			      "\\dotfill", "\\stackrel{", "\\stackrel{",
			      "}{\\stackrel{", "}{", "}}", "",
			      "\\begin{mptabbing}", "\\end{mptabbing}",
			      "\\begin{mpdisplay}", "\\end{mpdisplay}", "|",
			      "\\ms{%i}", "", "", "", "",
                              "\\textcolor[X]{", "}{", "}" };

static
char *ascii_tab[44] = { "\n", "", "\t", "",   "", "", "", "",
			      "\n", "", "",   "",   "", "", "", "",
			      "",   "", "",   "",   "", "", "", "",
			      "",   "", "",   "",   "", "", "", "",
			      "",   "", "",   "",   "", "", "", "", "",
                              "",   "", "" };

static
char *latex_ph[7] = { "\\mpE ", "\\mpO ", "\\mpI ", "\\mpV ", 
			    "\\mpT ", "\\mpL ", "\\mpD " };

static
char *ascii_ph[7] = { "Expr", "Op", "Id", "Var",
			    "Text", "List", "Disp" };

static
char *latex_phnum[16] = { "", "_1", "_2", "_3", "_4", "_5", "_6", "_7",
				"_8", "_9", "_{10}", "_{11}",
				"_{12}", "_{13}", "_{14}", "_{15}"};


#define NAMEDHEAD    "\\mpN{"
#define NAMEDTAIL    "}"

static
char *size_latex[10] = {
    "{\\tiny ",   "{\\scriptsize ", "{\\footnotesize ",
    "{\\small ",  "{\\normalsize ", "{\\large ",
    "{\\Large ",  "{\\LARGE ",      "{\\huge ",
    "{\\Huge " };

#define FONTLATEX(A) ((A)>3? font_latex[0]:font_latex[A])
#define SIZELATEX(A) ((A)>128? ((A)>197? size_latex[9]: \
				((A)<188? size_latex[0]: \
				 size_latex[(A)-188]))\
		      :  ((A)>69? size_latex[9]: \
			 ((A)<60?size_latex[0]: size_latex[(A)-60])))

/*
**  Some functions for generating LaTeX output and checking mode changes
*/

static Bool in_math, in_tabbing, tex_plhl, after_slash, after_macro,
            secure, setms;
static FILE *texf = NULL;
static char **texstring = NULL;       /* LaTeX -> string */
static int stringlen=0, stringmax=0;  /* length of string, size allocated */
static int pref_math[50] = {0};
static int pref_count[50] = {1};
static int total_count = 1;
static int soft_push[50] = {0};
static int soft_count = 0;  
static int nrpref = 0;
static int last_close = 0;
static int nrdisp = 0;
static int texmode = 0;
static int texoutput=0; /* 0=FILE 1=SCHERM 2=STRING */


static MapStr latextext=0;
static MapStr latexmath=0;
static int latexloaded=0;
static void latex_autoload(void)
{
  if (!latexloaded) {
    if (!latextext) MapStrLoadFile(latextext, "latextext");
    if (!latexmath) MapStrLoadFile(latexmath, "latexmath");
    latexloaded=1;
  }
}

char *char_latex(Char data, Bool math)
{
  if (!latexloaded) latex_autoload();
  if (data && math<5) {
    if (math) return MapValue(latexmath, data);
    return MapValue(latextext, data);
  } else return 0;
}

char *font_opentexttex(int attribute)
{
  if (!latexloaded) latex_autoload();
  if (attribute) return MapValue(latextext, Font2Char(FontFont, attribute));
  else return 0;
}

char *font_openmathtex(int attribute)
{
  if (!latexloaded) latex_autoload();
  if (attribute) return MapValue(latexmath, Font2Char(FontFont, attribute));
  else return 0;
}

char *font_closetex(int attribute)
{
  if (!latexloaded) latex_autoload();
  if (attribute) return MapValue(latextext, Font2Char(PopFont,attribute));
  else return 0;
}

/* for parsing: loop through all the available symbols to add them
** to the parser. linfoused indicates which (latex)maptables are already used
** and fonts using that table are skipped. Undefined fonts are also
** skipped. (otherwise it would result in 256*256*2=131072 loops, now the
** maximum is 256*30*2=15360 loops, but usually much less (standard 2048).
*/
char *char_latex_next(Char *data, int *math)
{
#ifdef MATH_OLD_WAY
    int i,j,h,m,id;
    char *lh;
    if (!*data)	for (i=0; i<30; linfoused[i++]=0);
    if (!*math) (*math)++; else { *math=0; (*data)++; }
    i=Char2Font(*data);
    j=Char2ASCII(*data);
    h = FINFO(grnr, i).lnr;
    lh=linfo[h].code;
    m=*math;
    while (1) { /* end if i>=256 */
	if (!h || linfoused[h]) {
	    /* font is not available or uses same coding: skip it */
	    i++; j=0;m=0;
	    if (i==256) break;
	    h = FINFO(grnr, i).lnr;
	    lh=linfo[h].code;
	} else if (((m  && aig(id=linfo[h].math[j])) ||
		    (!m && aig(id=linfo[h].normal[j]))) &&
		   ((lh[id]!=j) || (lh[id+1]!=0))) {
	    /* found something of interest */
	    *data=(Char)Font2Char(i,j);
	    *math=m;
	    return lh+id;
	} else {
	    /* increase (i,j,m) */
	    if (!m) { /* increase math tag (m) */
		m=1;
	    } else {
		m=0;
		if (j<255) {  /* increase character position (j) */
		    j++;
		} else { /* increase font position  (i) */
		    j=0;
		    linfoused[h]=1;
		    i++;
		    if (i==256) break;
		    h = FINFO(grnr,i).lnr;
		    lh=linfo[h].code;
		}
	    }
	}
    }
#endif
    *data=0;
    *math=0;
    return 0;

}

#define TrueTrue  2
#define prefmath     pref_math[nrpref]

static void add_string(char *str)
{
    int i;
    char *c;

    if (!str || !(i = strlen(str))) return;
    if (stringlen+i>=stringmax) {
	stringmax = (((stringlen+i)>>11)+1)<<11;
	c = (char*) malloc(stringmax);
	if (stringlen) {
	    strcpy(c, *texstring);
	    free(*texstring);
	} else
	    *c=0;
	*texstring = c;
    }
    strcpy(*texstring+stringlen, str);
    stringlen+=i;
}

static void printlatex(Bool bld, char *str)
{
    switch (texoutput) {
    case 0:
	if (str) fputs(str,texf);
	break;
    case 1:
	switch (bld) {
	case MP_False:
	    out_char_string(str);
	    break;
	default:
	    out_char_bold(str);
	    break;
	}
	break;
    case 2:
	add_string(str);
	break;
    default:
	break;
    }
}

static void close_slash(void)
{
    if (after_slash) {
	printlatex(0," ");
	after_slash = after_macro = MP_False;
    }
}

static void close_macro(void)
{
    if (after_macro) {
	if (after_slash)
	    printlatex(MP_False," ");
	else
	    printlatex(MP_False,"{}");
	after_slash=after_macro=MP_False;
    }
}

static void close_math(void)
{
    if (after_slash) {
	printlatex(MP_False," ");
	after_macro = after_slash = MP_False;
    }
    if (in_math == MP_True) {
	printlatex(MP_True,"$");
	last_close = MP_False;
    } else if (in_math == TrueTrue) {
	printlatex(MP_True,"\\mbox{");
	last_close = MP_True;
    }
    after_macro = after_macro && !in_math;
    in_math = MP_False;
}

static void open_math(void)
{
    if (after_slash) {
	printlatex(MP_False," ");
	after_slash = after_macro = MP_False;
    }
    after_macro = after_macro && in_math;
    if (!in_math)
	if (prefmath == TrueTrue || last_close) {
	    printlatex(MP_True,"}");
	    in_math = TrueTrue;
	    last_close = MP_False;
	} else {
	    printlatex(MP_True,"$");
	    in_math = MP_True;
	}
}

void push_math_pref(Bool premath)
{
    if (pref_math[nrpref]==premath)
	pref_count[nrpref]++;
    else {
	pref_math[++nrpref]=premath;
	pref_count[nrpref]=1;
    }
    total_count++;
}

static void soft_math_pref(Bool premath)
{
    push_math_pref(premath);
    soft_push[++soft_count] = total_count;
} 

void pop_math_pref(void)
{
    while (soft_push[soft_count]==total_count) {
	pref_count[nrpref]--;
	if (pref_count[nrpref]<1 && nrpref>0) nrpref--;
	in_math = pref_math[nrpref];
	total_count--;
	soft_count--;
    }
    pref_count[nrpref]--;
    if (pref_count[nrpref]<1 && nrpref>0) nrpref--;
    total_count--;
}

static void clear_state(void)
{
    pref_math[0] =
	in_tabbing = 
	in_math =
	tex_plhl =
	after_slash =
	after_macro =
	secure =
	last_close = MP_False;
    pref_count[0]= 1;
    soft_count = 0;
    set_default_thinspace(0);
    total_count = 1;
    texmode = output_mode;
    setms = (output_mode!=MPTEX);
    nrdisp = 0;
    nrpref = 0;
}

void tex_set_file(FILE *f)
{
    texf=f;
    clear_state();
    texoutput = (!f);
}

void tex_set_string(char **str)
{
    texstring = str;
    stringlen = stringmax = 0;
    if (*str) free(*str);
    *str = NULL;
    clear_state();
    texoutput = 2;
    setms=MP_True;
}

int tex_current_pos(void)
{
    return stringlen;
}

void tex_mode(int mode)
{
    texmode = mode;
    setms = setms || mode!=MPTEX;
}

void tex_open_proof(void)
{
    int d = (get_display_delta()+latex_side<0 ? 0 : get_display_delta()+latex_side); 
    if (!in_tabbing) {
	char buf[500];
	close_math();
	if (texoutput!=1) {
	    sprintf(buf, "\\begin{mpdisplay}{%s}{%s}{%s}{%i}\n\t",
		    UstrtoLocale(latex_space_unit), UstrtoLocale(latex_tab_unit),
		    UstrtoLocale(latex_line_unit), d);
	    printlatex(MP_True, buf);
	} else
	    open_tabbing();
	in_tabbing = MP_True;
	after_slash = MP_False;
	after_macro = MP_False;
	secure = MP_False;
	in_math = MP_False;
	nrdisp = 1;
    } else nrdisp++;
    set_display_delta(0);
}

void tex_close_proof(void)
{
    if (in_tabbing && nrdisp==1) {
	if (last_close) {
	    close_slash();
	    printlatex(MP_True,"}");
	    last_close = MP_False;
	}
	close_math();
	if (texoutput!=1)
	    printlatex(MP_True,"\n\\end{mpdisplay}");
	else
	    close_tabbing();
	in_tabbing = 
	    after_slash =
	    after_macro =
	    secure =
	    in_math = MP_False;
	nrdisp = 0;
    } else if (nrdisp) nrdisp--;
}

void tex_unset(void)
{
    close_math();
    if (in_tabbing) 
	if (texoutput!=1) 
	    printlatex(MP_True,"\\end{mpdisplay}");
	else
	    close_tabbing();
    if (!texstring) printlatex(MP_False,"\n");
    clear_state();
    texf = NULL;
    texstring=0;
    texoutput=0;
    stringlen=0;
    stringmax=0;    
}

void tex_placeholders(Bool texthem)
{
    tex_plhl = texthem;
}

static void update_after(char *str)
{
    while (*str) {
	if (*str=='\\')
	    after_slash= after_macro=!after_slash;
	else {
	    after_macro=after_macro && isalpha(*str);
	    after_slash = MP_False;
	}
	str++;
    }
}

void out_latex_char(Char c)
{
    int newmode;
    char *lc;

    if (!c) return;
    if (inplacename) {
	if (placepos<PLACENAMESIZE && !(c&0xff00))
	    placename[placepos++]=(char)(c&0xff);
	else if (c==PlNameEnd) inplacename=0;
	return;
    }
    if (IsTab(c)) {
	if (c==PlName) { placepos=0; inplacename=1; } else
	if (texmode==ASCII) {
	    printlatex(MP_True,ASCII_TAB(c));
	} else 	if (in_tabbing && (c>=SoftNewline)) {
	    if (c!=SoftNewline)	close_math();
	    printlatex(MP_True,LATEX_TAB(c));
	} else {
	    close_slash();
	    if (!prefmath) close_math();
	    switch (c) {
	    case Rtab:
		printlatex(MP_False,"\t");
		break;
	    case Newline:
	    case SoftNewline:
		printlatex(MP_False,"\n");
		break;
	    case AskText:
	    case AskMath:
	    case AskBoth:
		break;
	    case InText:
		if (!in_math && prefmath==TrueTrue) open_math();
		in_math = MP_False;
		soft_math_pref(MP_False);
		if (texoutput==1) out_char(c);
		break;
	    case InMath:
		in_math = MP_True;
		soft_math_pref(MP_True);
		if (texoutput==1) out_char(c);
		break;
	    case InDisp:
		in_math = TrueTrue;
		soft_math_pref(TrueTrue);
		if (texoutput==1) out_char(c);
		break;
	    case ThinSpace:
		if (get_default_thinspace() || texoutput==1) {
		    char oss[10];
		    sprintf(oss, LATEX_TAB(ThinSpace), get_default_thinspace());
		    close_slash();
		    printlatex(MP_True,oss);
		}
		break;
	    case PopSize:
	    case CloseStack:
	    case OpenTop:
	    case CloseTop:
	    case OpenBottom:
	    case CloseBottom:
	    case OpenGap:
	    case CloseGap:
	    case StackClose:
	    case StackB:
	    case StackC:
	    case TopGap:
	    case GapBottom:
		if (prefmath) open_math();
		printlatex(MP_True,LATEX_TAB(c));
		break;
	    case TabOpen:
		if (!in_tabbing) {
		    in_tabbing = MP_True;
		    nrdisp=1;
		    close_math();
		    printlatex(MP_True, LATEX_TAB(TabOpen));
		} else if (texoutput==1) {
		    nrdisp++;
		    close_math();
		    printlatex(MP_True, LATEX_TAB(TabOpen));
		} else nrdisp++;
		break;
	    case TabClose:
		nrdisp--;
		if (!nrdisp) {
		    in_tabbing = MP_False;
		    close_math();
		    printlatex(MP_True, LATEX_TAB(TabClose));
		} else if (texoutput==1) {
		    close_math();
		    printlatex(MP_True, LATEX_TAB(TabClose));
		}
		break;
	    case DisplayOpen:
		tex_open_proof();
		if (texoutput==1)
		    printlatex(MP_True, LATEX_TAB(DisplayOpen));
		break;
	    case DisplayClose:
		tex_close_proof();
		if (texoutput==1)
		    printlatex(MP_True, LATEX_TAB(DisplayClose));
		break;
	    default:
		printlatex(MP_True,LATEX_TAB(c));
		break;
	    }
	}
    } else if (IsPh(c)) {
	if (texmode==ASCII) {
	    if (texoutput==1)
		out_char(c);
	    else if (placepos || nameset) {
		if (!nameset) placename[placepos]='\0';
		printlatex(MP_True,placename);
		placepos=nameset=0;
	    } else
		printlatex(MP_True,ASCII_PH(c));
	} else {
	    if (tex_plhl) {
		if (Ph(c)!=MP_Text) open_math();
		if (placepos || nameset) {
		    if (!nameset) placename[placepos]='\0';
		    printlatex(MP_True,NAMEDHEAD);
		    printlatex(MP_True,placename);
		    printlatex(MP_True,NAMEDTAIL);
		    placepos=nameset=0;
		} else printlatex(MP_True,LATEX_PH(c));
		if (Ph(c)!=MP_Text) printlatex(MP_True,LATEX_PHNUM(c));
	    }
	    if (texoutput==1) {
		if (Ph(c) == MP_Text)
		    close_math();
		else
		    open_math();
		if (placepos || nameset) {
		  if (!nameset)  placename[placepos]='\0';
		  printlatex(MP_True,placename);
		  placepos=nameset=0;
		  out_index(Num(c));
		} else {
		  out_char(c);
		}
	    }
	}
    } else if (IsOpspace(c)) {
	if (texmode!=ASCII) {
	    char oss[10];
	    int i = Char2ASCII(c);

	    if (i>0) {
		switch (texmode) {
		case PROOFTEX:
		    close_slash();
		    while (i) {
			printlatex(MP_False, "\\,");
			i--;
		    }
		    break;
		case PLAINTEX:
		    break;
		case MPTEX:
		default:
		    sprintf(oss, "\\ms{%i}", i);
		    close_slash();
		    printlatex(MP_True,oss);
		    break;
		}
	    }
	}
    } else if (Char2Font(c)==StackFont) {
	if (texmode!=ASCII) {
	    open_math();
	    close_slash();
	    printlatex(MP_False,LATEX_TAB(StackC));
	}
    } else if (Char2Font(c)==FontFont) {
	if (texmode!=ASCII) {
	    char *h;
	    int npm=prefmath;
	    if (npm) {
		h = font_openmathtex(Char2ASCII(c));
		if (!h && aig(h= font_opentexttex(Char2ASCII(c))))
		    npm=0;
	    } else {
		h = font_opentexttex(Char2ASCII(c));
		if (!h && aig(h= font_openmathtex(Char2ASCII(c))))
		    npm=1;
	    }
	    push_math_pref(npm);
	    if (h) {
		if (!prefmath)
		    close_math();
		else
		    open_math();
		secure=MP_True;
		printlatex(MP_True,h);
		update_after(h);
	    }
	}
    } else if (Char2Font(c)==PopFont) {
	if (texmode!=ASCII) {
	    char *h=font_closetex(Char2ASCII(c));
	    if (h) {
		if (!prefmath)
		    close_math();
		else
		    open_math();
		secure=MP_True;
		printlatex(MP_True,h);
		update_after(h);
	    }
	    pop_math_pref();
	}
    } else if (Char2Font(c)==SizeFont) {
	if (texmode!=ASCII) {
	    if (!prefmath)
		close_math();
	    else
		open_math();
	    printlatex(MP_True,SIZELATEX((int)Char2ASCII(c)));
	}
    } else if (c<(Char)(BUTTONFONT*256)) {
	if (!aig(lc = char_latex(c, pref_math[nrpref])))
	    if (!aig(lc = char_latex(c, !pref_math[nrpref])))
		newmode = pref_math[nrpref];
	    else
		newmode = !pref_math[nrpref];
	else
	    newmode = pref_math[nrpref];
	if (texmode!=ASCII && newmode && !in_math) open_math();
	if (texmode!=ASCII && !newmode && in_math) close_math();
	if (!lc) {
	    char hex[20];
	    sprintf(hex, "\\MPU{%04X}", c);
	    printlatex(MP_True,hex);
	    after_macro = MP_False;
	} else {
	    if (texmode!=ASCII) {
		if (strlen(lc)>1) {
		    if (isalpha(*lc))
			close_macro();
		    else
			close_slash();
		    secure=MP_True;
		    printlatex(MP_True,lc);
		} else {
		    if (secure) {
			if (isalpha(*lc))
			    close_macro();
			else
			    close_slash();
			secure=MP_False;
		    }
		    printlatex(MP_False,lc);
		}
		update_after(lc);
	    } else
		if (strlen(lc)>1)
		    printlatex(MP_True,lc);
		else
		    printlatex(MP_False,lc);
	}
    }
}


void tex_to_mode(int tm)
{
    switch (tm) {
    case LTEXTMODE:
	if (prefmath!=MP_False) {
	    pop_math_pref();
	    push_math_pref(MP_False);
	}
	break;
    case LMATHMODE:
	if (prefmath!=MP_True) {
	    pop_math_pref();
	    push_math_pref(MP_True);
	}
	break;
    case LBOTHMODE:
	pop_math_pref();
	push_math_pref(prefmath);
	break;
    default: break;
    }
}

void tex_code(TexCode c)
{
    placepos=nameset=0;
    if (texmode!=ASCII) {
	secure = MP_True;
	switch (c) {
	case ExprOpen:
	    if (!setms && texf) {
		close_slash();
		fprintf(texf, "\\setms{%s}",UstrtoLocale(latex_space_unit));
		setms=MP_True;
	    }
	    push_math_pref(MP_True);
	    break;
	case ExprClose:
	    pop_math_pref();
	    break;
	case SOpOpen:
	    switch (texmode) {
	    case PLAINTEX:
		break;
	    case PROOFTEX:
	    case MPTEX:
	    default:
		open_math();
		printlatex(MP_False,"{");
		after_macro = MP_False;
		break;
	    }
	    push_math_pref(MP_True);
	    break;
	case SOpClose:
	    switch (texmode) {
	    case PLAINTEX:
		break;
	    case PROOFTEX:
	    case MPTEX:
	    default:
		open_math();
		printlatex(MP_False,"}");
		after_macro = MP_False;
		break;
	    }
	    pop_math_pref();
	    break;
	case LOpOpen:
	    push_math_pref(MP_True);
	    break;
	case LOpClose:
	    pop_math_pref();
	    break;
	case SIdOpen:
	    open_math();
	    push_math_pref(MP_True);
	    break;
	case SIdClose:
	    pop_math_pref();
	    break;
	case LIdOpen:
	    open_math();
	    printlatex(MP_False,"\\mathit{");
	    after_macro = MP_False;
	    push_math_pref(in_math);
	    break;
	case LIdClose:
	    if (prefmath != in_math) {
		if (in_math)
		    close_math();
		else
		    open_math();
	    } else close_slash();
	    in_math = prefmath;
	    pop_math_pref();
	    printlatex(MP_False,"\\/}");
	    after_macro = MP_False;
	    break;
	case VarOpen:
	    push_math_pref(MP_True);
	    break;
	case VarClose:
	    pop_math_pref();
	    break;
	case TextOpen:
	    secure = MP_False;
	    if (prefmath == TrueTrue && in_math) {
		close_slash();
		printlatex(MP_False,"\\mbox{");
		after_macro = MP_False;
		in_math = MP_False;
	    }
	    push_math_pref(MP_False);
	    break;
	case TextClose:
	    secure = MP_False;
	    pop_math_pref();
	    if (prefmath == TrueTrue) {
		close_math();
		printlatex(MP_False,"}");
		after_macro = MP_False;
		in_math = TrueTrue;
	    }
	    break;
	case DispOpen:
	    if (!setms && texf) {
		close_slash();
		fprintf(texf, "\\setms{%s}",UstrtoLocale(latex_space_unit));
		setms=MP_True;
	    }
	    tex_open_proof();
	    push_math_pref(MP_True);
	    set_display_delta(0);
	    break;
	case DispClose:
	    tex_close_proof();
	    pop_math_pref();
	    break;
	default:
	    break;
	}
    }
}
