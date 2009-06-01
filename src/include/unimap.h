/*
** File:      unimap.h
** Purpose:   provide functions and macros to create, destroy, copy, load,
**            save, define and use maps to and from Unicode.
** Types:     provided by unicode.h
** Functions: general type-independent functions to handle maps. Macros are
**            used to simulate type-dependent versions.
*/

#ifndef UNIMAP_H
#define UNIMAP_H

#include <stdio.h>
#include "unicode.h"



#define UM_BOOLTYPE 1
#define UM_CHARTYPE 2
#define UM_UCHARTYPE 3
#define UM_CHARREFTYPE 4
#define UM_UCHARREFTYPE 5
#define UM_INTTYPE 6
/*
** These functions are type independent and SIZE_OF_TYPE is used to make
** them aware of the size of items. Some functions receive functions as
** an argument to perform an action on an item if that item is a reference.
** The default action is skip.
**
** A mapping table can be either static or dynamic. A static mapping table
** is created with umap_load, while a dynamic table is created with
** umap_create. umap_copy creates a copy of a mapping table and automatically
** converts it to dynamic.
*/

/* Saved maps can share subtables and the variable umap_shared can be used
** to share non-empty submaps where possible. It is only used when maps are
** saved, since the load routine detects shared submaps. This should only
** be used for optimalisation.
*/
extern int umap_shared;

extern int empty_submap(void *b);
#define EmptySubmap(A,I)  empty_submap((A)[(I)/UNI_MAPSIZE])

/* create a new map */
extern void *umap_create(int size_of_type);

/* define position POS in map MAP to have value VAL */
extern void  umap_define(int size_of_type, void *map, Uchar pos, void *val);

/* destroy map MAP. The function DESTROY_VAL should destroy a single value. */
extern void  umap_destroy(void *map,
			  int (*destroy_val)(void*));

/* make a copy of map MAP. COPY_VAL should copy a single value. */
extern void *umap_copy(int size_of_type, void *map, void *(*copy_val)(void*));

/* load map MAP from DATA. LOAD_VAL should load one value from a reference
** if the integer argument is set, it should perform a byte swap where needed
** inside DATA. MAP will be static (t.i. can not be changed with umap_define).
*/
extern void *umap_load(void *data, int size_of_type, int type_code,
		       void *(*load_val)(void**,int));

/* load map MAP from a file named NAME. NAME is searched for in a path set by MAPPATH
** and the default extension ".map" is added when needed. It justs reads the file into
** memory and calls umap_load.
*/
extern void *umap_load_file(char *name, int size_of_type, int type_code,
			    void *(*load_val)(void**,int));

/* save map MAP, so it can be loaded faster */
extern void  umap_save(int size_of_type, void *map, FILE *f, int type_code,
		       int (*save_val)(void*,FILE*));
/*
#define MapValue(A,P)  ((A)[(P)/UNI_MAPSIZE][(P)%UNI_MAPSIZE])
*/
#define MapBoolCreate()       (MapBool) umap_create(sizeof(char))
#define MapBoolDefine(A,P,V)  umap_define(sizeof(char), (A), (P), &(V))
#define MapBoolDestroy(A)     umap_destroy((A), 0)
#define MapBoolCopy(A)        (MapBool) umap_copy(sizeof(char), (A), 0)
#define MapBoolLoad(A,D)      (A)=(MapBool) umap_load((D), sizeof(char), UM_BOOLTYPE, 0)
#define MapBoolLoadFile(A,D)  (A)=(MapBool) umap_load_file((D), sizeof(char), UM_BOOLTYPE, 0)
#define MapBoolSave(A,F)      umap_save(sizeof(char), (A), (F), UM_BOOLTYPE, 0)

#define MapIntCreate()       (MapInt) umap_create(sizeof(int))
#define MapIntDefine(A,P,V)  umap_define(sizeof(int), (A), (P), &(V))
#define MapIntDestroy(A)     umap_destroy((A), 0)
#define MapIntCopy(A)        (MapInt) umap_copy(sizeof(int), (A), 0)
#define MapIntLoad(A,D)      (A)=(MapInt) umap_load((D), sizeof(int), UM_INTTYPE, 0)
#define MapIntLoadFile(A,D)  (A)=(MapInt) umap_load_file((D), sizeof(int), UM_INTTYPE, 0)
#define MapIntSave(A,F)      umap_save(sizeof(int), (A), (F), UM_INTTYPE, 0)

#define MapCharCreate()       (MapChar) umap_create(sizeof(char))
#define MapCharDefine(A,P,V)  umap_define(sizeof(char), (A), (P), &(V))
#define MapCharDestroy(A)     umap_destroy((A), 0)
#define MapCharCopy(A)        (MapChar) umap_copy(sizeof(char), (A), 0)
#define MapCharLoad(A,D)      (A)=(MapChar) umap_load((D), sizeof(char), UM_CHARTYPE, 0)
#define MapCharLoadFile(A,D)      (A)=(MapChar) umap_load_file((D), sizeof(char), UM_CHARTYPE, 0)
#define MapCharSave(A,F)      umap_save(sizeof(char), (A), (F), UM_CHARTYPE, 0)

#define MapUcharCreate()       (MapUchar) umap_create(sizeof(Uchar))
#define MapUcharDefine(A,P,V)  umap_define(sizeof(Uchar), (A), (P), &(V))
#define MapUcharDestroy(A)     umap_destroy((A), 0)
#define MapUcharCopy(A)        (MapUchar) umap_copy(sizeof(Uchar), (A), 0)
#define MapUcharLoad(A,D)      (A)=(MapUchar) umap_load((D), sizeof(Uchar), UM_UCHARTYPE, 0)
#define MapUcharLoadFile(A,D)  (A)=(MapUchar) umap_load_file((D), sizeof(Uchar), UM_UCHARTYPE, 0)
#define MapUcharSave(A,F)      umap_save(sizeof(Uchar), (A), (F), UM_UCHARTYPE, 0)

extern Uchar **umap_uchar_inverse(Uchar **original);

#define MapUcharInverse(A,B)   (A)=umap_uchar_inverse(B)

extern  int   MapStrDestroyInt(void*);
extern  void *MapStrCopyInt(void*);
extern  void *MapStrLoadInt(void**,int);
extern  int   MapStrSaveInt(void*, FILE*);

#define MapStrCreate()       (MapStr) umap_create(sizeof(char*))
#define MapStrDefine(A,P,V)  umap_define(sizeof(char*), (A), (P), &(V))
#define MapStrDestroy(A)     umap_destroy((A), MapStrDestroyInt)
#define MapStrCopy(A)        (MapStr) umap_copy(sizeof(char*), (A), MapStrCopyInt)
#define MapStrLoad(A,D)      (A)=(MapStr) umap_load((D), sizeof(char*), UM_CHARREFTYPE, MapStrLoadInt)
#define MapStrLoadFile(A,D)  (A)=(MapStr) umap_load_file((D), sizeof(char*), UM_CHARREFTYPE, MapStrLoadInt)
#define MapStrSave(A,F)      umap_save(sizeof(char*), (A), (F), UM_CHARREFTYPE, MapStrSaveInt)

extern  int   MapUstrDestroyInt(void*);
extern  void *MapUstrCopyInt(void*);
extern  void *MapUstrLoadInt(void**,int);
extern  int   MapUstrSaveInt(void*, FILE*);

#define MapUstrCreate()       (MapUstr) umap_create(sizeof(Uchar*))
#define MapUstrDefine(A,P,V)  umap_define(sizeof(Uchar*), (A), (P), &(V))
#define MapUstrDestroy(A)     umap_destroy((A), MapUstrDestroyInt)
#define MapUstrCopy(A)        (MapUstr) umap_copy(sizeof(Uchar*), (A), MapUstrCopyInt)
#define MapUstrLoad(A,D)      (A)=(MapUstr) umap_load((D), sizeof(Uchar*), UM_UCHARREFTYPE, MapUstrLoadInt)
#define MapUstrLoadFile(A,D)      (A)=(MapUstr) umap_load_file((D), sizeof(Uchar*), UM_UCHARREFTYPE, MapUstrLoadInt)
#define MapUstrSave(A,F)    umap_save(sizeof(Uchar*), (A), (F), UM_UCHARREFTYPE, MapUstrSaveInt)


#endif
