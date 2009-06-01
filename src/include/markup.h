#ifndef MP_MARKUP_H
#define MP_MARKUP_H
#include "unicode.h"


typedef struct _attribs {
    int font;     /* font selection */
    int fgcolor;  /* foreground color (0x00rrggbb) */
    int bgcolor;  /* background color (0x00rrggbb) */
    int language; /* language code (might influence other attributes) */
    int country;  /* country code (might influence other attributes) */
    int direction;/* direction (left-to-right, right-to-left, level) */
    int text;     /* underline/stroke/upcase/large (bitvector) */
    int plane;    /* ISO10646 plane, needed for correct surrogates */
    void *object; /* object which caused it */
    int pos;      /* position within object */
    int posdelta; /* in a string, how relates pos to a pos in the original
		  ** string.
		  ** 0: string as a result of expansion
		  ** 1: normal 2byte string.
		  ** 2: surrogate/markup string.
		  */
    /* more to come */
} Attribute;

/* the private zone surrogates are used for markup codes. These values
** are used in the high surrogate part to indicate what kind of markup
** follows. The low surrogate part provides a specific argument value.
** There are 128 high surrogate parts available and 1024 possible arguments.
** Each of these groups has its own handler function, such that the
** processing the low surrogate parts can be done by selecting handler
** functions.  Other non-private surrogates are processed the same way,
** where font changes might be used for rendering them.
*/
typedef enum {   /* Meaning of argument */
    FontAttrib = 0xDB81,
                 /* 00000xxxxx: general codes (push/pop/reset)
                 ** xxxxxyyyyy: set font attribute xxxxx to yyyyy.
		 ** Allows 32 attributes with 32 values (32^32 combinations)
		 */
    BoxAttrib,   /* sizes, positions, alignments, types. */
    ColorAttrib, /* 11xxxyyzzz: color cube (allocated on demand)
		 ** 00*, 01*, 10*, red, green or blue setting
		 */
    LangAttrib,  /* xxxxxxxxxx: index in a language table.
		 **
		 ** 0 -> N: the ISO 639 language tags
		 ** M -> 1023: unofficial languages.
		 ** new languages are added in between.
		 ** a lookup table is used to find full names or 2/3-character
		 ** codes.
		 */
    ArgAttrib,   /* xxxxxxxxxx: number of argument to be insert. */
    LocaleAttrib,/* date, value, currency, time (might depend on language) */
    ProgAttrib,  /* programming constructs, conditional output. */
    StackAttrib, /* for refering to stack information with additional data */
    /* more to come */
    /* some of these are processed by the main program, some are processed by
    ** the drawing routine. Especially the programming constructs and the
    ** arguments to be inserted should be handled by the main program.
    */
} MarkupGroup;

/* a handle function receives 3 arguments:
** an integer   (in)     -> the low surrogate
** an Attribute (in/out) -> the current attributes
** a  function  (out)    -> a temporary redirect function.
**
** The integer contains the low surrogate. The handle function can obtain
** the high surrogate with the function high_surro().
** The Attribute structure contains the current structure and the handle
** function might change them when needed.
** The function can be set, so input is temporary redirected, which might
** be handy to construct arguments for functions.
*/
typedef int (*RedirectFunc)(Uchar);
typedef Uchar *(*HandleFunc)(int, Attribute *, RedirectFunc *);

/* print is used to print that value without interpretation.
** handle is called to interpret the markup code
** The Attribute argument to the functions is used to report changes
** in attributes, but it also contains the current attributes, which
** might influence the interpretation of the markup (for example,
** language or country might influence the output of date or time markup.
*/
int add_markup_handler(MarkupGroup mg,
		       HandleFunc print, HandleFunc handle);


/* A surrogate handler is set for remapped high surrogate values (0-1023)
** The handler should handle all the possible values for the lower surrogate
** part. To draw characters constructed with surrogates, it might
** temporary change other parts of the drawing engine (fonts/size/filters)
** to achieve that.
*/
int add_surrogate_handler(Uchar highvalue,
			  HandleFunc print, HandleFunc handle);

#define IsPrefix(A) (((A)>=0xD800) && ((A)<=0xDBFF))
#define IsFormatPrefix(A)  (((A)>=0xDB80) && ((A)<=0xDBFF))

extern void set_handle(Uchar highvalue);
extern Uchar *handle_surro(Uchar lowvalue, Attribute *attr, RedirectFunc *func);
extern Uchar *print_surro(Uchar lowvalue, Attribute *attr, RedirectFunc *func);
extern int high_surro(void);

/* determine if string starts with valid characters, that is,
** not a second part of a surrogate pair, a diacritic or any
** other character that should be combined with a previous
** character.
** This function can be used to check if a cursor position is valid.
*/
extern int valid_start_position(Uchar *string);


extern void markup_init(void);

#endif
