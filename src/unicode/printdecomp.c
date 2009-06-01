#include <stdio.h>
#include <stdlib.h>

#include "unimap.h"
#include "unitype.h"

char buffer[50000];
static void print_decomp_opentag(int tagval)
{
  switch (tagval) {
  case DEC_CIRCLE:
    printf("<CIRCLE>");
    break;
  case DEC_FINAL:
    printf("<FINAL>");
    break;
  case DEC_FONT:
    printf("<FONT>");
    break;
  case DEC_FRACTION:
    printf("<FRACTION>");
    break;
  case DEC_INITIAL:
    printf("<INITIAL>");
    break;
  case DEC_ISOLATED:
    printf("<ISOLATED>");
    break;
  case DEC_MEDIAL:
    printf("<MEDIAL>");
    break;
  case DEC_NARROW:
    printf("<NARROW>");
    break;
  case DEC_NOBREAK:
    printf("<NOBREAK>");
    break;
  case DEC_SMALL:
    printf("<SMALL>");
    break;
  case DEC_SQUARE:
    printf("<SQUARE>");
    break;
  case DEC_SUB:
    printf("<SUB>");
    break;
  case DEC_SUPER:
    printf("<SUPER>");
    break;
  case DEC_VERTICAL:
    printf("<VERTICAL>");
    break;
  case DEC_WIDE:
    printf("<WIDE>");
    break;
  default:
    break;
  }
}

static void print_decomp_closetag(int tagval)
{
  switch (tagval) {
  case DEC_CIRCLE:
    printf("</CIRCLE>");
    break;
  case DEC_FINAL:
    printf("</FINAL>");
    break;
  case DEC_FONT:
    printf("</FONT>");
    break;
  case DEC_FRACTION:
    printf("</FRACTION>");
    break;
  case DEC_INITIAL:
    printf("</INITIAL>");
    break;
  case DEC_ISOLATED:
    printf("</ISOLATED>");
    break;
  case DEC_MEDIAL:
    printf("</MEDIAL>");
    break;
  case DEC_NARROW:
    printf("</NARROW>");
    break;
  case DEC_NOBREAK:
    printf("</NOBREAK>");
    break;
  case DEC_SMALL:
    printf("</SMALL>");
    break;
  case DEC_SQUARE:
    printf("</SQUARE>");
    break;
  case DEC_SUB:
    printf("</SUB>");
    break;
  case DEC_SUPER:
    printf("</SUPER>");
    break;
  case DEC_VERTICAL:
    printf("</VERTICAL>");
    break;
  case DEC_WIDE:
    printf("</WIDE>");
    break;
  default:
    break;
  }
}

int main(int argc, char **argv)
{
    int i;
    Uchar *list[40];
    char taglist[40];
    int pos;

    unitype_init();

    for (i=0; i<0xffff; i++) {
        pos=0;
	list[pos]=Udecomp(i);
	if (list[pos]) {
	    printf("U%.4x\t@", i);
	    taglist[pos]=Udecomptype(i);
	    print_decomp_opentag(taglist[pos]);
	    while (pos >= 0) {
	      if (!list[pos][0]) {
		print_decomp_closetag(taglist[pos]);
		pos--;
	      } else if ((list[pos+1] = Udecomp(list[pos][0]))) {
		taglist[pos+1]=Udecomptype(list[pos][0]);
		list[pos]++;
		pos++;
		print_decomp_opentag(taglist[pos]);
	      } else {
		printf("%.4x ",list[pos][0]);
		list[pos]++;
	      }
	    }
	    printf("@\n");
	}
    }
    return 0;
}
