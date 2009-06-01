/*
**  File:    unifont.h
**  Purpose: provide a general interface to the fonts available on the system
**           to simulate a collection of (incomplete) 16-bit fonts. Fonts are
**           registered which their unique ID, their encoding and possible
**           attributes. A database is build using these attributes and
**           encodings to map a Unicode character and its attributes to a
**           collection of properties for that glyph.
**           An extra database is used to dynamically load the fonts if needed.
**           At the start, only one fallback font is loaded with the ISO8859-1
**           encoding.
*/

#ifndef UNIFONT_H
#define UNIFONT_H

#include "unicode.h"

#define FONT_SUCCESS    1
#define FONT_ERROR      2
#define FONT_NOT_LOADED 0

/* defines Font (fontID) and CharStruct (character information) */
#include "unifontsys.h"

typedef
struct {
    Font font;           /* font number */
    int pos;             /* position in that font */
    CharStruct *sysinfo; /* system specific information about that character */
} CharInfo;

/* The configuration file defines which attributes are available, which
** values they can have, which font encodings are uses, which fonts are
** available and which attribute values these fonts have.  A collection
** of virtual fonts is build using this information.
** The configuration file itself is system independent. How it is loaded
** and parsed might be system specific.
*/
extern int font_load_config(char *filename);

/* change the attribute of the current font */
extern void font_set_attribute(int attrnr, int value);

/* set all attributes at once */
extern void font_set_attributes(int attribcombo);

/* inspect all attributes at once */
extern int font_get_attributes(void);

/* inspect the attributes of the current font */
extern int  font_get_attribute(int attrnr);

/* change one attribute in the given font */
extern int font_change_attribute(int attribcombo,  int attrnr, int value);

/* get the attribute name of the some attribute */
extern char *font_get_name(int attrnr, int attrval);

/* retrieve the information of a character with the current
** attributes.  The returned record is tempory and might be overwritten
** by a next call to character_info.
*/
extern CharInfo *character_info(Uchar c);


/* set the system specific information */
extern void font_set_system_data(void *data);
#endif

