/*
**  File:    unistring.h
**  Purpose: provide simular functions for standard C functions to handle
**           Unicode. Most of the functions from string.h are available,
**           where the name is prefixed with "U" and the meaning remains
**           the same (str*  -> Ustr*)
*/
#ifndef UNISTRING_H
#define UNISTRING_H

#include "unistd.h"
#include "unicode.h"

/* using the GNU string.h include file as an example */

/* Copy SRC to DEST.  */
extern Uchar *   Ustrcpy(Uchar *dest, const Uchar *src);
/* Copy no more than N characters of SRC to DEST.  */
extern Uchar *   Ustrncpy(Uchar *src, const Uchar *dest, size_t n);

/* Append SRC onto DEST.  */
extern Uchar *   Ustrcat(Uchar *dest, const Uchar *src);
/* Append no more than N characters from SRC onto DEST.  */
extern Uchar *   Ustrncat(Uchar *dest, const Uchar *src, size_t n);

/* Compare S1 and S2.  */
extern int       Ustrcmp(const Uchar *s1, const Uchar *s2);
/* Compare N characters of S1 and S2.  */
extern int       Ustrncmp(const Uchar *s1, const Uchar *s2, size_t n);

/* Compare S1 and S2, ignoring case.  */
extern int       Ustrcasecmp(const Uchar *s1, const Uchar *s2);
/* Compare no more than N chars of S1 and S2, ignoring case.  */
extern int       Ustrncasecmp(const Uchar *s1, const Uchar *s2, size_t n);

/* Compare the collated forms of S1 and S2.  */
extern int       Ustrcoll(const Uchar *s1, const Uchar *s2);
/* Put a transformation of SRC into no more than N bytes of DEST.  */
extern size_t    Ustrxfrm(char *dest, const char *src, size_t n);

/* Duplicate S, returning an identical malloc'd string.  */
extern Uchar *   Ustrdup(const Uchar *s);
/* Duplicate S, returning a malloc'd string of length at most N. */ 
extern Uchar *   Ustrndup(const Uchar *s, size_t n);

/* Return the length of S.  */
extern size_t    Ustrlen(const Uchar *s);

/* Find the first occurrence of C in S.  */
extern Uchar *   Ustrchr(const Uchar *s, Uchar c);
/* Find the last occurrence of C in S.  */
extern Uchar *   Ustrrchr(const Uchar *s, Uchar c);

/* Find the first occurrence of NEEDLE in HAYSTACK.  */
extern Uchar *   Ustrstr(const Uchar *haystack, const Uchar *needle);
/* Find the first occurrence in S of any character in ACCEPT.  */
extern Uchar *   Ustrpbrk(const Uchar *s, const Uchar *accept);
/* Divide S into tokens separated by characters in DELIM.  */
extern Uchar *   Ustrtok(Uchar *s, const Uchar *delim);
/* Return the length of the initial segment of S which
   consists entirely of characters not in REJECT.  */
extern size_t    Ustrcspn(const Uchar *s, const Uchar *reject);
/* Return the length of the initial segment of S which
   consists entirely of characters in ACCEPT.  */
extern size_t    Ustrspn(const Uchar *s, const Uchar *accept);

extern int UTFneedlen(const Uchar *s);
extern int UstrtoUTF(const Uchar *s, unsigned char *t);
extern int UTFtoUstr(const unsigned char *s, Uchar *t);
extern int UTFstrlen(const unsigned char *s);
extern int UTFcheck(const unsigned char *s);

extern char *UstrtoFilename(const Uchar *s);
extern unsigned char *UstrtoLocale(const Uchar *s);
extern Uchar *LocaletoUstr(const unsigned char *s);
extern Uchar *FilenametoUstr(const char *s);

extern long Ustrtol(const Uchar *s, Uchar **res, int base);
extern Uchar *Ultostr(long l, Uchar *s);


#endif
