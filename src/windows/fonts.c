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
/*
**  fonts.c:   to handle general font loading and usage.
*/

#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "message.h"
#include "intstack.h"
#include "output.h"
#include "fonts.h"
#include <ctype.h>

#include "unitype.h"
#include "unistring.h"
#include "unimap.h"

#define GENERALINFOFILE "fonts.mpt"

int font_height_max, font_ascent_max, font_descent_max;
short font_width_max;

int font_height_ar[NR_SIZE], font_ascent_ar[NR_SIZE], font_descent_ar[NR_SIZE];
short font_width_ar[NR_SIZE];
Char * generalinfofile=NULL;

#define BUFSIZE 1000
static char buffer[BUFSIZE];

typedef
struct {
    short fnr, lnr, fsnr, number, page[2], nrml, max;
    short *size;
    char *loaded;
    char *name;
} FONTINFO;

static INTSTACK *font_stack = NULL;


static void remove_return(char *str)
{
    int i;

    if (str && (aig(i=strlen(str))) && (str[i-1] == '\n')) {
	str[i-1] = '\0';
    }
}

static int read_integer_short(char **txt)
{
    int n=0;
    Bool neg=MP_False;

    if (**txt=='-') { neg=MP_True; (*txt)++;}
    while (isdigit(**txt)) {
	n=n*10+ (**txt)-'0';
	(*txt)++;
    }
    return (neg? -n:n);
}

static int read_integer(char **txt)
{
    int n=read_integer_short(txt);

    while (isspace(**txt)) (*txt)++;
    return n;
}

static int read_fontpart(FILE *f, FONTINFO finf[MAXFONTS])
{

#define get_new_string (not_finished = ((fgets(buffer, BUFSIZE, f)!=NULL) && \
					!begins_with("STOP", buffer)))

    int nr, i;
    Bool not_finished;
    char *ind;
    unsigned char last_fontnr = 0;

    nr=0;
    get_new_string;
    while (not_finished && nr<MAXFONTS) {
	if (aig(ind = begins_with("FONT:", buffer)) ||
	    aig(ind = begins_with("FONTGROUP:", buffer))) {
	    remove_return(buffer);
	    finf[nr].name = (char *) malloc(sizeof(char)*(strlen(ind)+1) );
	    strcpy(finf[nr].name,ind);
	    if (buffer[4]!=':') finf[nr].nrml = 1;
	    else finf[nr].nrml = 0;
	    finf[nr].page[0]= -1;
	    finf[nr].page[1]= -2;
	    finf[nr].lnr=0;
	    finf[nr].fnr=0;
	    finf[nr].max = 0;
	    finf[nr].size = NULL;
	    finf[nr].number = last_fontnr;
	    if ((int)(last_fontnr+1)< BUTTONFONT) last_fontnr++;
	    if (aig(get_new_string) && aig(ind = begins_with("NUMBER:", buffer))) {
		remove_return(buffer);
		if (*ind) {
		    i = read_integer(&ind);
		    if (0<=i && i<BUTTONFONT)
			finf[nr].number = (short)i;
		}
		get_new_string;
	    }
	    if (not_finished && aig(ind = begins_with("PAGE:", buffer))) {
		remove_return(buffer);
		if (begins_with("NONE", ind))
		    finf[nr].page[0]= -2;
		else {
		    for (i=0; i<2 && ind; i++) {
			while (*ind && !isdigit(*ind)) ind++;
			if (*ind)
			    finf[nr].page[i]=(short)read_integer(&ind);
		    }
		}
		get_new_string;
	    }
	    nr++;
	} else
	    if (aig(ind = begins_with("BUTTONFONT:", buffer))) {
		remove_return(buffer);
		finf[nr].name= (char *) malloc(strlen(ind)+1);
		strcpy(finf[nr].name,ind);
		finf[nr].page[0] = finf[nr].page[1] = -2;
		finf[nr].number = BUTTONFONT;
		nr++;
		get_new_string;
	    } else 
		get_new_string;
    }
    return nr;
}

static int usednums[10] = { 0,1,2,3,4,5,6,7,8,50 };
#define USEDMAX 10

int missing_font(FILE *f)
{
  int n,i;
  int wrong1,good;
  FONTINFO tfi[MAXFONTS];
  wrong1=good=0;
  n = read_fontpart(f,tfi);
  for (i=0; i<n; i++) {
    if (tfi[i].number!=BUTTONFONT) {
      int j;
      for (j=0; j<USEDMAX && tfi[i].number!=usednums[j]; j++);
      if (tfi[i].number==TEXTFONT) good++;
      if (j<USEDMAX) {
	wrong1++;
      }
    }
  }
  for (i=0; i<n; i++) free(tfi[i].name);
  if (!good) {
    return MP_EXIT;
  } else if (wrong1) {
    message(MP_CLICKREMARK, translate("Missing fonts for this file."));
    return MP_EXIT-1;
  } else {
    return 0;
  }
}


Bool new_fontfile(int nr, Char *newname)
{
  return (nr>100 && !newname);
}

int fontattributes[NR_SIZE];

void push_attributes(int nr)
{
    int i;

    i = (nr<0||nr>NR_SIZE ? font_get_attributes() : fontattributes[nr]);
    push_int(&font_stack, font_get_attributes());
    font_set_attributes(i);
    setsimpletabsize(8*char_width('n'));
}

void pop_attributes(void)
{
    int i;
    i = head_int(font_stack);
    pop_int(&font_stack);
    font_set_attributes(i);
    setsimpletabsize(8*char_width('n'));
}

#define FNR(A,B) fnr2anr[A][B]
#define FINFO(A,B) finfo[A][FNR(A,B)]

int font_ascent(void)
{
    return (char_ascent('H')*12+5)/10;
}

int font_descent(void)
{
    return (char_descent('g')*12+5)/10;
}

int font_height(void)
{
  return font_ascent()+font_descent();
}

short font_width(void)
{
    return char_width('W');
}

int char_width(Char data)
{
  CharInfo *ci;
  ci = character_info(data);
  if (ci) return CharWidth(ci->sysinfo);
  else return 0;
}

int char_ascent(Char data)
{
  CharInfo *ci;
  ci = character_info(data);
  if (ci) return CharAscent(ci->sysinfo);
  else return 0;
}

int char_descent(Char data)
{
  CharInfo *ci;
  ci = character_info(data);
  if (ci) return CharDescent(ci->sysinfo);
  else return 0;
}

int char_left(Char data)
{
  CharInfo *ci;
  ci = character_info(data);
  if (ci) return CharLeft(ci->sysinfo);
  else return 0;
}

int char_right(Char data)
{
  CharInfo *ci;
  ci = character_info(data);
  if (ci) return CharRight(ci->sysinfo);
  else return 0;
}

int string_width(Char *string, int nr)
{
    int w;
    w=0;
    while (nr && *string) {
      w=w+char_width(*string);
      string++;
      nr--;
    }
    return w;
}

