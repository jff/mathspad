#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/stat.h>
#include "unimap.h"
#include "unifont.h"
#include "unitype.h"
#include "unistring.h"
#include "uniconv.h"
#include "markup.h"
#include "filefind.h"

Display *display;
static Window window;
static GC gc;
static GC acgc;
static int screen;
static PathInfo mappath=0;
static int accents=0;

Uchar standard_string[] =
   { 'R', 'i', 'c', 'h', 'a', 'r', 'd', ' ', 0x3a3, 0x3bb,
     0x41a, 0x419, 0x2021, 0x2194, 0x21d4, 0x2227, 0x2229,
     0x2469, 0x25c6, 0x266c, 0x3066, 0x3bb8, 0 };
Uchar *list = standard_string;
Uchar convbuf[40]={'U','T','F','8',0};
Uchar *convname=convbuf;

void read_string(char *fname)
{
    FILE *f;
    char *buffer;
    int i;
    UConvID cid;
    UConverter *ucv;
    if (!(f=open_file(0,fname, "rb"))) return;
    i = file_size(0,fname);
    buffer=malloc((i+1));
    i=fread(buffer,1, i, f);
    close_file(f);
    buffer[i]=0;
    UConvLoadDatabase("UniConvert");
    cid=UConvGetID(convname);
    ucv=UConvGet(cid);
    list=UConvEncode(buffer, NULL, ucv);
    free(buffer);
    /* i=UTFstrlen(buffer);
    list = (Uchar*) malloc(sizeof(Uchar)*(i+1));
    UTFtoUstr(buffer, list); */
}

void print_charstruct(XCharStruct *cs, int i)
{
    printf("%8x%8i%8i%8i%8i%8i  %8x\n", i,
           cs->lbearing, cs->rbearing,
           cs->width, cs->ascent, cs->descent, cs->attributes);
}

static int yline=0;
int draw_text(void);
extern void draw_string(Uchar *string, short fontattrib,
			int x, int y, int dry, int *detectpos, int *left,
			int *right, int *ascent, int *descent);

static XTextProperty winname;
static unsigned char asciiname[] = {
  'T', 'e', 's', 't', ' ',
  224, 225, 226,  /* some characters in the high region */
  27, 45, 72,     /* switch to iso8859-8 (hebrew) */
  224, 225, 226,  /* some characters in the high region */
  0 };
static unsigned char *namelist[] = { asciiname,0};


int main(int argc, char **argv)
{
    int n,i;
    XGCValues gcvalues;
    XEvent report;
    int accentcolor=9;
    XSizeHints size_hints;
    XWMHints wm_hints;
    XClassHint class_hints;

    unitype_init();
    markup_init();
    mappath=make_pathinfo("MAPPATH", ".", ".map");
    n=0;
    if (argc>3) UTFtoUstr(argv[3],convbuf);
    if (argc>1) read_string(argv[1]);
    if (argc>2) accentcolor=atoi(argv[2]);
    display = XOpenDisplay("");
    if (!display) {
	fprintf(stderr,"Unable to connect to Xserver.\n");
	exit(1);
    }
    font_set_system_data(display);
    font_load_config("helvetica");
    screen = DefaultScreen(display);
    XmbTextListToTextProperty(display, namelist, 1, XCompoundTextStyle, &winname);
    window = XCreateSimpleWindow(display, RootWindow(display, screen), 75, 75,
				 400, 400, 1, BlackPixel(display, screen),
				 WhitePixel(display, screen));
    gc = XCreateGC(display, window, 0, &gcvalues);
    acgc=XCreateGC(display, window, 0, &gcvalues);
    size_hints.flags = PPosition | PSize | PMinSize;
    size_hints.min_width = 40;
    size_hints.min_height = 40;
    wm_hints.flags=0;
    class_hints.res_name="brows";
    class_hints.res_class="demo";
    XSetWMProperties(display, window, &winname, &winname,
                     NULL, 0, &size_hints, &wm_hints, &class_hints);
    
    XSetForeground(display, gc, BlackPixel(display, screen));
    XSetForeground(display, acgc, accentcolor);
    font_set_attribute(0,0);
    font_set_attribute(1,0);
    font_set_attribute(2,4);
    font_set_attribute(3,0);
    for (i=0; i<4; i++) {
	char *name;
	int k;
	name=font_get_name(i,-1);
	fprintf(stderr, "%s : ", name);
	k= font_get_attribute(i);
	name=font_get_name(i,k);
	fprintf(stderr, "%s (%i)\n", name, k);
    }
    XSelectInput(display, window, ExposureMask|ButtonPressMask);
    XMapWindow(display, window);
    while (1) {
	XNextEvent(display,&report);
	switch (report.type) {
	case Expose:
	    if (!report.xexpose.count) draw_text();
	    break;
	case ButtonPress:
	  { int i=report.xbutton.button*15;
	  while (i>0) {
	    while (list[yline]) {
	      if (list[yline]=='\n' || list[yline]==0x2028) {
		yline++;
		break;
	      } else if (list[yline]==0x2029) {
		i=i-1;yline++;
		break;
	      }
	      yline++;
	    }
	    i--;
	  }
	  if (!list[yline]) yline=0;
	  }
	XClearWindow(display, window);
	draw_text();
	break;
	default:
	    break;
	}
    }
    return (0);
}

int draw_words(int x, int y, XTextItem16 *txt, int len)
{
    XDrawText16(display, window, (accents?acgc:gc), x, y, txt, len);
    accents=1;
    return 0;
}

int draw_text(void)
{
    int xpos,ypos;
    Uchar *c;
    Uchar buffer[2000];
    int bpos=0;
    int left, right, ascent, descent, dpos;
    short last_font_attrib;
    short fattr=0;

    fattr=last_font_attrib=font_get_attributes();
    ypos=10;
    xpos=10;
    c=list+yline;
    while (*c && ypos<2000) {
	if (*c=='\n' || *c==0x2028 || *c==0x2029) {
	    buffer[bpos]=0;
	    accents=0;
	    draw_string(buffer,fattr, xpos, ypos, 0, &dpos,
			&left, &right, &ascent, &descent);
	    bpos=0;
	    ypos+=20;
	    xpos=10;
	} else if (Uissurrogatehigh(*c)) {
	    /* surrogate, handler might change font attributes */
	    buffer[bpos]=0;
	    accents=0;
	    draw_string(buffer, fattr, xpos, ypos, 0, &dpos,
			&left, &right, &ascent, &descent);
	    xpos+=right;
	    bpos=0;
	    set_handle(*c);
	} else if (Uissurrogatelow(*c)) {
	    Uchar *hans;
	    Attribute fat;
	    fat.font=fattr;
	    hans=handle_surro(*c,&fat,0);
	    fattr=fat.font;
	    if (hans) {
		while (*hans) {
		    buffer[bpos++]=*hans;
		    hans++;
		}
	    }
	} else if (*c=='\t') {
	    /* flush. move forward */
	    buffer[bpos]=0;
	    accents=0;
	    draw_string(buffer, fattr, xpos, ypos, 0, &dpos,
			&left, &right, &ascent, &descent);
	    xpos+=right;
	    xpos+=(60-xpos%60);
	    bpos=0;
	} else {
	    buffer[bpos++]=*c;
	}
	c++;
    }
    accents=0;
    draw_string(buffer, fattr, xpos, ypos, 0, &dpos,
		&left, &right, &ascent, &descent);
    font_set_attributes(last_font_attrib);
    return 0;
}
