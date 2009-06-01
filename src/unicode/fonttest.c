#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#define EXIT_FAILURE 1
void print_property(Display *display, XFontProp *fp)
{
    printf("\t\t%s:%li\n", XGetAtomName(display, fp->name), fp->card32);
}

void print_charstruct(XCharStruct *cs, int i)
{
    printf("%8x%8i%8i%8i%8i%8i  %8x\n", i,
	   cs->lbearing, cs->rbearing,
	   cs->width, cs->ascent, cs->descent, cs->attributes);
}

int main(int argc, char **argv)
{
    Display *display;
    XFontStruct *fs;
    int i;
    unsigned long prop;

    if (!(display=XOpenDisplay(""))) {
	fprintf(stderr, "Unable to connect to server\n");
	exit(EXIT_FAILURE);
    }
    fs = XLoadQueryFont(display, argv[1]);
    if (!fs) {
	fprintf(stderr, "Unable to load font %s\n", argv[1]);
	exit(EXIT_FAILURE);
    }
    printf("FontID\t\t%lu\n"
	  "Direction\t%i\n"
	  "MinChar\t\t%i\n"
	  "MaxChar\t\t%i\n"
	  "MinByte\t\t%i\n"
	  "MaxByte\t\t%i\n"
	  "All Exist\t%s\n"
	  "Default Char\t%i\n"
	  "Ascent\t\t%i\n"
	  "Descent\t\t%i\n"
	  "Extra Data\t%s\n"
	  "Properties\t%i\n",
	  fs->fid, fs->direction, fs->min_char_or_byte2, fs->max_char_or_byte2,
	  fs->min_byte1, fs->max_byte1, (fs->all_chars_exist?"Yes":"No"),
	  fs->default_char, fs->ascent, fs->descent, (fs->ext_data?"Yes":"No"),
	  fs->n_properties);
    for (i=0; i<fs->n_properties; i++)
	print_property(display, fs->properties+i);
    if (XGetFontProperty(fs, XA_POINT_SIZE, &prop))
	printf("PointSize\t%li\n", prop);
    if (XGetFontProperty(fs, XA_COPYRIGHT, &prop))
	printf("Copyright\t%li\n", prop);
    printf("%8s%8s%8s%8s%8s%8s   %8s\n", "Pos","Left", "Right", "Width",
	   "Ascent", "Descent", "Attrib");
    print_charstruct(&fs->min_bounds, -1);
    print_charstruct(&fs->max_bounds, -2);
    if (!fs->per_char) printf("All glyphs have equal size\n");
    else {
	if (!fs->min_byte1 && !fs->max_byte1) {
	    /* single byte fonts */
	    int i,j,n;
	    j=fs->min_char_or_byte2;
	    n=fs->max_char_or_byte2-j+1;
	    for (i=0; i<n; i++)
		print_charstruct(fs->per_char+i, j+i);
	} else {
	    /* double byte fonts */
	    int i,j,n;
	    j=(fs->max_char_or_byte2-fs->min_char_or_byte2+1);
	    n=j*(fs->max_byte1-fs->min_byte1+1);
	    printf("Total %i, Row: %i\n", n,j);
	    for (i=0; i<n; i++) {
		print_charstruct(fs->per_char+i,
				 ((i/j+fs->min_byte1)<<8)+i%j+
				 fs->min_char_or_byte2);
	    }
	}
    }
    XCloseDisplay(display);
    return 0;
}
