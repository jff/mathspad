#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "unimap.h"
#include "unistring.h"
#include "filefind.h"

/*
  A standard empty submap which contains only zeros
  It should be large enough to hold items for any type used in maps
  (Uchar (2), char* (4), char (1), bool (1), ...)
  and should be correctly aligned. Just guess that double is aligned strictly
  enough.  To build in extra security, a union is used for extra alignment
  restrictions and the array is made twice as large as needed.
  According to the ANSI C standard, is it automaticly initialized with
  zeros. Initializing the first element would make the object code
  much larger ( sizeof(emptysubmap) bytes ).
  On PCs running Windows, this could cause a problem due to size restrictions
*/

static union {
    double submap[0x20000/UNI_MAPSIZE];
    long al1;
    void *al2;
    int al3;
} emptysubmap;

int empty_submap(void *ref)
{
    return ref==&(emptysubmap.submap);
}


static PathInfo pinf=0;

static void *get_full_file(char *name)
{
    FILE *f;
    int fsize;
    int rsize=0;
    void *res,*res2;
    if (!pinf) { pinf=make_pathinfo("MAPPATH", DEFAULTMAPPATH, ".map"); }
    fsize=file_size(pinf, name);
    if (fsize<0) fsize=0x20000;
    if (!((res=malloc(fsize)))) return 0;
    f=open_file(pinf, name, "r");
    rsize=fread(res, 1,fsize, f);
    close_file(f);
    if (rsize<fsize) {
        res2=realloc(res,rsize);
        if (res2) {
            res=res2;
            fsize=rsize;
        }
    }
    return res;
}

/*
  Mapping tables are either created with umap_create (and a sequence of
  umap_define's) or umap_load.  The mapping table create is umap_create is
  dynamic, while the table created with umap_load is static.
  The list of submaps contains one extra item which determines if the
  mapping is static or dynamic. A static mapping table can not be changed
  with umap_define's and is destroyed differently. umap_copy can be used
  to create a dynamic copy of a static map. So the sequence:
  
     tmap = umap_copy(size, map, copyfunc);
     umap_destroy(map,  destroyfunc);
     map = tmap;

  converts a static map into a dynamic one.
*/

void *umap_create(int size_of_type)
{
    int i;
    void **mt;
    mt = malloc((UNI_MAPSIZE+1)*sizeof(void*));
    for (i=0; i<UNI_MAPSIZE; i++)
	mt[i]= &emptysubmap;
    mt[i]=0;
    return mt;
}

void  umap_define(int size_of_type, void *map, Uchar pos, void *val)
{
    void **mt=map;
    int i=pos/UNI_MAPSIZE,j=pos%UNI_MAPSIZE;

    if (!mt || mt[UNI_MAPSIZE]) return;
    if (i<0 || i>256 || j<0 || j>256) {
	printf("Error in umap_define (%i,%i)\n",i,j);
	return;
    }
    if (mt[i]== &emptysubmap) {
	mt[i]=malloc(size_of_type*(0x10000/UNI_MAPSIZE));
	memset(mt[i], 0, size_of_type*(0x10000/UNI_MAPSIZE));
    }
    memcpy(((char*)mt[i])+j*size_of_type, val, size_of_type);
}

void  umap_destroy(void *map,
		   int (*destroy_val)(void*))
{
    void **mt=map;
    int i,j;
    if (!mt) return;
    if (mt[UNI_MAPSIZE]) {
	/* static mapping, created with umap_load */
	if (destroy_val) i=0; else i=UNI_MAPSIZE;
	while (i<UNI_MAPSIZE+1) {
	    free(mt[i]);
	    i++;
	}
    } else {
	/* dynamic mapping, created with umap_create or umap_copy */
	for (i=0; i<UNI_MAPSIZE; i++) {
	    if (mt[i]!= &emptysubmap) {
		if (destroy_val) {
		    void **mg=mt[i];
		    for (j=0; j<0x10000/UNI_MAPSIZE; j++)
			if (mg[j]) (*destroy_val)(mg[j]);
		}
		free(mt[i]);
	    }
	}
    }
    free(mt);
}

void *umap_copy(int size_of_type, void *map, void *(*copy_val)(void*))
{
    void **mt=map;
    int i;
    void **mc;
    mc = malloc(sizeof(void*) * (UNI_MAPSIZE+1));
    mc[UNI_MAPSIZE]=0;
    for (i=0; i<UNI_MAPSIZE; i++) {
	if (mt[i]==&emptysubmap)
	    mc[i]=&emptysubmap;
	else {
	    mc[i]= malloc(size_of_type*(0x10000/UNI_MAPSIZE));
	    if (copy_val) {
		void **mcg,**mtg; int j;
		mcg = mc[i];
		mtg = mt[i];
		for (j=0; j<0x10000/UNI_MAPSIZE; j++) {
		    if (!mtg[j]) mcg[j]=mtg[j];
		    else mcg[j]= (*copy_val)(mtg[j]);
		}
	    } else
		memcpy(mc[i], mt[i], size_of_type*(0x10000/UNI_MAPSIZE));
	}
    }
    return mc;
}

static void swap_bytes(unsigned char *data, int size, int per)
{
  int i,j;
  for (i=0; i<size; i++) {
    for (j=0; j<per/2;j++) data[j]^=data[per-1-j]^=data[j]^=data[per-1-j];
    data+=per;
  }
}

void  *umap_load(void *data, int size_of_type, int type_code,
		 void *(*load_val)(void**,int))
{
    int i,j;
    int bswap=0;
    unsigned short *usesubmap = data;
    void **mt;
    void *smp;

    /* automatic conversion should be used in some situations */
    if (usesubmap[0] != 0xFEFF) {
      if (usesubmap[0] == 0xFFFE) {
	/* byte-swapped */
	bswap=1;
	for (i=0; i<UNI_MAPSIZE+4;i++)
	  usesubmap[i]=((usesubmap[i]&0xff)<<8)|(usesubmap[i]>>8);
      } else return 0;
    }
    if (usesubmap[1] != 0x4D50) return 0;  /* not a map file */
    if ((usesubmap[2]>>8) != size_of_type) return 0; /* sizes don't match */
    if ((usesubmap[2]&0xff) != type_code) return 0; /* types don't match */
    if (usesubmap[3] != UNI_MAPSIZE) return 0;  /* mapsizes don't match */
    usesubmap+=4;
    mt = malloc(sizeof(void*)*(UNI_MAPSIZE+1));
    mt[UNI_MAPSIZE]=data;  /* to be able to free it */
    j=0;
    smp = &usesubmap[UNI_MAPSIZE];
    for (i=0; i<UNI_MAPSIZE; i++) {
	if (usesubmap[i]==0xFFFF)
	    /* empty submap -> share it */
	    mt[i]=&emptysubmap;
	else if (usesubmap[i]<j) {
	    /* shared submap -> find reference */
	    int k=0;
	    for (k=0; k<i && usesubmap[k]!=usesubmap[i]; k++);
	    if (k<i) mt[i]=mt[k];
	} else if (usesubmap[i]==j) {
	    /* submap saved at position smp. recover it */
	    if (load_val) {
	        /* a load function: load items save by save function. */
	        /* the data in smp is static */
		void **mg;
		int k;
		mt[i]= malloc(size_of_type*(0x10000/UNI_MAPSIZE));
		mg=mt[i];
		for (k=0; k<0x10000/UNI_MAPSIZE; k++)
		    mg[k]=(*load_val)(&smp,bswap);
	    } else {
	        mt[i]= smp;
		if (bswap && size_of_type>1)
		  /* type is an integer type. Reverse the bytes */
		  swap_bytes(smp, 0x10000/UNI_MAPSIZE, size_of_type);
		smp= ((char*)smp)+size_of_type*(0x10000/UNI_MAPSIZE);
	    }
	    j++;
	} else
	  /* usesubmap[i]>j  -> invalid submap number (should not happen) */
	  mt[i]=&emptysubmap;
    }
    return mt;
}

void *umap_load_file(char *name, int size_of_type, int type_code,
			    void *(*load_val)(void**,int))
{
  void *data;
  data=get_full_file(name);
  if (data) {
    return umap_load(data, size_of_type, type_code, load_val);
  } else {
    return 0;
  }
}

int umap_shared=0;

void  umap_save(int size_of_type, void *map, FILE *f, int type_code,
		       int (*save_val)(void*,FILE*))
{
    unsigned short usesubmap[UNI_MAPSIZE];
    int i,j;
    unsigned short signature;
    void **mt=map;

    /* byte order mark */
    signature=0xFEFF;
    fwrite(&signature, sizeof(unsigned short), 1, f);
    /* just two bytes to check if it is a mapping */
    signature=0x4D50;
    fwrite(&signature, sizeof(unsigned short), 1, f);
    /* some information to check if the types are compatible */
    signature=(size_of_type<<8)+type_code;
    fwrite(&signature, sizeof(unsigned short), 1, f);
    /* to check if the mapsize didn't change */
    signature=UNI_MAPSIZE;
    fwrite(&signature, sizeof(unsigned short), 1, f);
    j=0;
    if (umap_shared && !save_val) {
      /* An algorithm to minimize the disk space by sharing submaps, but it
      ** usually takes too much time and does not save that much. The
      ** variable umap_shared can be used to turn it on.  For strings
      ** (save_val == True), a compare function would be needed and it
      ** would not save much.
      */
      int tablehash[UNI_MAPSIZE];
      int k,l;
      char *h;
      for (i=0; i<UNI_MAPSIZE; i++) {
	if (mt[i]==&emptysubmap) {
	  usesubmap[i]=0xffff;
	  tablehash[i]=0;
	} else {
	  k=0;
	  h=mt[i];
	  for (l=0; l<size_of_type*(0x10000/UNI_MAPSIZE); l++) k+=(*h++)*l;
	  tablehash[i]=k;
	  if (!k &&
	      !memcmp(mt[i],&emptysubmap,size_of_type*(0x10000/UNI_MAPSIZE))) {
	    usesubmap[i]=0xffff;
	  } else {
	    for (l=0; l<i; l++) {
	      if (tablehash[l]==k && usesubmap[l]!=0xffff &&
		  !memcmp(mt[l], mt[i],size_of_type*(0x10000/UNI_MAPSIZE))) {
		usesubmap[i]=usesubmap[l];
		l=i+1;
	      }
	    }
	    if (l==i) usesubmap[i]=j++;
	  }
	}
      }
    } else {
      /* calculate where each submap can be found in the file. 0xffff indicates
      ** the empty submap, which is not written to disk.
      */
      j=0;
      for (i=0; i<UNI_MAPSIZE; i++) {
	if (mt[i]==&emptysubmap)
	  usesubmap[i]=0xffff;
	else
	  usesubmap[i]=j++;
      }
    }
    fwrite(usesubmap, sizeof(usesubmap), 1, f);
    j=0;
    for (i=0; i<UNI_MAPSIZE; i++) {
	if (usesubmap[i]==j) {
	    j++;
	    if (!save_val) {
	        fwrite(mt[i], (0x10000/UNI_MAPSIZE)*size_of_type, 1, f);
	    } else {
	        int k;
		void **mg=mt[i];
		for (k=0; k<0x10000/UNI_MAPSIZE; k++)
		    /* an undefined reference should be saved different from
		    ** a defined empty reference (0 vs. "")
		    */
		    (*save_val)(mg[k],f);
	    }
	}
    }
}

MapUchar umap_uchar_inverse(MapUchar source)
{
    int i,j;
    Uchar s;
    MapUchar res;
    res=umap_create(sizeof(Uchar));
    for (i=0; i<0x10000/UNI_MAPSIZE; i++) {
	if (source[i]!=(Uchar*)&emptysubmap) {
	    for (j=0; j<UNI_MAPSIZE; j++) {
		if (source[i][j]) {
		    s=i*UNI_MAPSIZE+j;
		    umap_define(sizeof(Uchar), res,
				source[i][j], &s);
		}
	    }
	}
    }
    return res;
}

int MapStrDestroyInt(void *value)
{
    if (value) free(value);
    return 0;
}

void *MapStrCopyInt(void *value)
{
    char *c=(char*) value, *h;
    int i=strlen(c)+1;
    h=(char*)malloc(i*sizeof(char));
    memcpy(h,c,i*sizeof(char));
    return (void*) h;
}

void *MapStrLoadInt(void **data,int bswap)
{
    char *c=(char*)(*data);
    if (*c==0x01) {
	*data = (void*) (c+1);
	return 0;
    } else {
	int i=strlen(c)+1;
	*data = (void*) (c+i);
	return (void*) c;
    }
}

int MapStrSaveInt(void *value, FILE *f)
{
    /* 0x01 is not a valid character in a string (for simplicity). Therefore,
    ** it can be used to indicate the undefined string
    */
    char *c=(char*) value;
    if (!c) {
	char h=0x01;
	fwrite(&h,sizeof(char),1,f);
    } else {
	int i=strlen(c)+1;
	fwrite(c,sizeof(char),i,f);
    }
    return 0;
}

int MapUstrDestroyInt(void *value)
{
    if (value) free(value);
    return 0;
}

void *MapUstrCopyInt(void *value)
{
    Uchar *c=(Uchar*) value, *h;
    int i=Ustrlen(c)+1;
    h=(Uchar*)malloc(i*sizeof(Uchar));
    memcpy(h,c,i*sizeof(Uchar));
    return (void*) h;
}

void *MapUstrLoadInt(void **data,int bswap)
{
    Uchar *c=(Uchar*)(*data);
    if ((bswap && (*c==0xFFFE)) || (!bswap && (*c==0xFEFF))) {
	*data = (void*) (c+1);
	return 0;
    } else {
	int i=Ustrlen(c)+1;
	if (bswap) swap_bytes(*data, i-1, sizeof(Uchar));
	*data = (void*) (c+i);
	return (void*) c;
    }
}

int MapUstrSaveInt(void *value, FILE *f)
{
    Uchar *c=(Uchar*) value;
    /* 0xFEFF can not be part of a Unicode string. Therefore, it can be use
    ** to indicate the undefined string
    */
    if (!c) {
	Uchar h=0xFEFF;
	fwrite(&h,sizeof(Uchar), 1, f);
    } else {
	int i=0;
	while (c[i++]); 
	fwrite(c, sizeof(Uchar), i, f);
    }
    return 0;
}

