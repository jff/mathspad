/*
** File:    uniconv.h
** Purpose: conversion between Unicode and a different mapping. A converter
**          consists of two functions and two references to data, needed
**          by these functions. Most of these converters use standard functions
**          and a table to map single characters to Unicode characters.
**          Since there are too many existing mappings, only some of the
**          tables are standard available, while the others are loaded on
**          demand.
**          Some encodings are more difficult and need special functions
**          (for example: ISO 2022, UTF8, UTF7, EUC, ...) 
**          These functions are provided if these encodings are used.
**          At the moment, only UTF8 conversion routines are provided.
** Type:    UConverter  Combined mapping to and from Unicode.
** File-
** Format:  A converter is saved in a file with its name and two mappings.
**          All the mappings are located in the same directory and the
**          filename is equal to the UTF8 version of the name.
*/

#ifndef UNICONV_H
#define UNICONV_H

#include "unicode.h"

typedef short UConvID;

/*
**  encode:  unsigned char * -> Uchar*  (to Unicode)
**  decode:  Uchar* -> unsigned char *  (from Unicode)
*/
typedef struct {
    UConvID convid;                            /* converter ID */
    char initialised;                          /* is it loaded ? */
    Uchar *name;                               /* name of encoding */
    Uchar *(*encode)(unsigned char *, Uchar *, void *);
                                               /* encoding function */
    unsigned char *(*decode)(Uchar *, unsigned char *, void *);
                                               /* decoding function */
    int (*needenlen)(unsigned char *, void *); /* encoding result length */
    int (*needdelen)(Uchar *, void *);         /* decoding result length */
    unsigned char *(*enfail)(unsigned char *, void*);
                                               /* where does encoding fail */
    Uchar *(*defail)(Uchar *, void *);         /* where does decoding fail */
    void *dataencode;                          /* data for encoding */
    void *datadecode;                          /* data for decoding */
} UConverter;

/*
** UConv??Len:  length of TEXT after conversion with CONVERT
**              Save maximum: 3xUstrlen  for Decode
**                            strlen     for Encode
** UConv??Fail: where does the conversion fail (if it does).
** UConv??code: store TEXT converted with CONVERT in TARGET
*/
extern int    UConvEnLen(const unsigned char *text, UConverter *convert);
extern unsigned char *UConvEnFail(const unsigned char *text,
				 UConverter *convert);
extern Uchar *UConvEncode(const unsigned char *text, Uchar *target,
			  UConverter *convert);

extern int   UConvDeLen(const Uchar *text, UConverter *convert);
extern Uchar *UConvDeFail(const Uchar *text, UConverter *convert);
extern unsigned char *UConvDecode(const Uchar *text, unsigned char *target,
				  UConverter *convert);

extern Uchar *UConvName(UConverter *convert);

/* Load the converter database. The database contains lists of
** converter names and mapping tables (or algorithms)
*/
extern void UConvLoadDatabase(char *name);

/* lookup a converter or encoding name */
extern UConvID UConvGetID(Uchar *name);

/* get the converter with a certain ID */
extern UConverter *UConvGet(UConvID convid);

#endif
