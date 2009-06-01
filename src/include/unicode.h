/*
**  File:    unicode.h
**  Purpose: define Unicode datatypes.
**  Types:   Uchar     two-byte unsigned integer for Unicode character
**           MapUchar  mapping from Unicode to Unicode or from
**                     ASCII to Unicode
**                     -> Utoupper, Utolower, Utobase, ...
**                        convertion from ISO-8859-*, IBM CP ?.
**           MapBool   mapping from Unicode to boolean
**                     -> Uisalpha, Uisdigit, ...
**           MapStr    mapping from Unicode to strings
**                     -> UtoTeX, UtoSGML, ...
**           MapUstr   mapping from Unicode to Unicode strings
**                     -> substitute for Unicode characters.
**           MapChar   mapping from Unicode to char
**                     -> convertion to ISO-8859-?, IBM CP*
**           MapInt    mapping from Unicode to integer
**                     -> convertion to font/position combination.
**           Since Unicode is too large to store all mappings as plain
**           arrays, the mappings are stored as arrays of arrays to enable
**           shared parts between mappings (most mappings are very empty).
**
**           If memory permits it, we could use plain arrays, although the
**           table for converting a one-byte encoding to Unicode could be
**           stored in a small array.
*/
#ifndef UNICODE_H
#define UNICODE_H

#include "memman.h"

typedef unsigned short Uchar;

#define UNI_MAPSIZE 256

/*
** All these double reference types can be used as arrays of type
**   MapType[UNI_MAPSIZE][0x10000/UNI_MAPSIZE]
** General functions are available in unimap.h to create the needed
** structure.
*/

typedef char **MapBool;
typedef char **MapChar;
typedef Uchar **MapUchar;
typedef Uchar ***MapUstr;
typedef char ***MapStr;
typedef int **MapInt;

/* mapvalue is independ of the type of A.  */
#define MapValue(A,P) ((A)[((Uchar)(P))/UNI_MAPSIZE][((Uchar)(P))%UNI_MAPSIZE])

#endif
