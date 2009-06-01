#include <config.h>
#include "uniconv.h"
#include "unistring.h"
#include "unimap.h"
#include "filefind.h"
#include <string.h>
/*
** UConv??Len:  length of TEXT after conversion with CONVERT
**              Save maximum: 3xUstrlen  for Decode
**                            strlen     for Encode
** UConv??Fail: position in TEXT where CONVERT fails to convert
**              that symbol to the other encoding.
** UConv??code: store TEXT converted with CONVERT in TARGET
*/

int    UConvEnLen(const unsigned char *text, UConverter *convert)
{
    return (*convert->needenlen)((unsigned char*)text, convert->dataencode);
}

unsigned char *UConvEnFail(const unsigned char *text, UConverter *convert)
{
    return (*convert->enfail)((unsigned char*)text,convert->dataencode);
}

Uchar *UConvEncode(const unsigned char *text, Uchar *target,
		   UConverter *convert)
{
  if (!target) {
    int n;
    n=(*convert->needenlen)((unsigned char*)text, convert->dataencode);
    target = (Uchar*) malloc(sizeof(Uchar)*(n+1));
    if (!target) return NULL;
  }
  return (*convert->encode)((unsigned char*)text, target, convert->dataencode);
}

int   UConvDeLen(const Uchar *text, UConverter *convert)
{
  return (*convert->needdelen)((Uchar*)text, convert->datadecode);
}

Uchar *UConvDeFail(const Uchar *text, UConverter *convert)
{
    return (*convert->defail)((Uchar*)text,convert->datadecode);
}

unsigned char *UConvDecode(const Uchar *text, unsigned char *target,
			   UConverter *convert)
{
  if (!target) {
    int n;
    n=(*convert->needdelen)((Uchar*)text, convert->datadecode);
    target = (char*) malloc(sizeof(char)*(n+1));
    if (!target) return NULL;
  }
  return (*convert->decode)((Uchar*)text, target, convert->datadecode);
}

Uchar *UConvName(UConverter *convert)
{
  return convert->name;
}


/* an array of pointers to converters */
static UConverter **uconvlist=NULL;
static int max_converter=0;

static UConverter *new_converter(void)
{
  if (!(max_converter % 64)) {
      UConverter **ex;
      if (!uconvlist) {
	  ex= malloc(sizeof(UConverter*)*(max_converter+64));
      } else {
	  ex= realloc(uconvlist, sizeof(UConverter*)*(max_converter+64));
      }
    if (!ex) return NULL;
    uconvlist=ex;
  }
  uconvlist[max_converter]=malloc(sizeof(UConverter));
  if (!uconvlist[max_converter]) return NULL;
  uconvlist[max_converter]->convid=max_converter;
  max_converter++;
  return uconvlist[max_converter-1];
}

static int enlen_8bit(unsigned char *source, void *data)
{
    MapUchar mc=data;
    Uchar c;
    int n=0;
    if (!source) return n;
    while (*source) {
	c=*source++;
	if (MapValue(mc,c)) n++;
    }
    return n;
}

static unsigned char *enfail_8bit(unsigned char *source, void *data)
{
    MapUchar mc=data;
    Uchar c;
    if (!source) return NULL;
    while (*source) {
	c=*source++;
	if (!MapValue(mc,c)) return source-1;
    }
    return NULL;
}

static Uchar *encode_8bit(unsigned char *source, Uchar *target, void *data)
{
    /* data contains a MapUchar for an 8-bit encoding
    ** Thus: characters are mapped individually
    ** unknown characters are filtered out.
    */
    MapUchar mc=data;
    Uchar c;
    Uchar *res;
    res=target;
    if (!target) return NULL;
    if (!source) {
	*target=0;
	return target;
    }
    while (*source) {
	c=*source++;
	if ((*target=MapValue(mc,c))) target++;
    }
    *target=0;
    return res;
}

static int enlen_16bit(unsigned char *source, void *data)
{
    MapUchar mc=data;
    Uchar c;
    int n=0;
    if (!source) return n;
    while (*source) {
	c=*source++;
	c=(c<<8)|(*source++);
	if (MapValue(mc,c)) n++;
    }
    return n;
}

static unsigned char *enfail_16bit(unsigned char *source, void *data)
{
    MapUchar mc=data;
    Uchar c;
    if (!source) return NULL;
    while (*source) {
	c=*source++;
	c=(c<<8)|(*source++);
	if (!MapValue(mc,c)) return source-2;
    }
    return NULL;
}

static Uchar *encode_16bit(unsigned char *source, Uchar *target, void *data)
{
    /* data contains a MapUchar for an 16-bit encoding
    ** Thus: characters are mapped in pairs
    ** unknown pairs are filtered out.
    */
    MapUchar mc=data;
    Uchar c;
    Uchar *res;
    res=target;
    if (!target) return NULL;
    if (!source) {
	*target=0;
	return target;
    }
    while (*source) {
	c=*source++;
	c=(c<<8)|(*source++);
	if ((*target=MapValue(mc,c))) target++;
    }
    *target=0;
    return res;
}

static int enlen_12bit(unsigned char *source, void *data)
{
    MapUchar mc=data;
    Uchar c;
    int n=0;
    if (!source) return n;
    while (*source) {
	c=((source[0])<<8)|(source[1]);
	if (MapValue(mc,c)) {
	    source+=2;
	    n++;
	} else {
	    c=*source++;
	    if (MapValue(mc,c)) n++;
	}
    }
    return n;
}

static unsigned char *enfail_12bit(unsigned char *source, void *data)
{
    MapUchar mc=data;
    Uchar c;
    int n=0;
    if (!source) return NULL;
    while (*source) {
	c=((source[0])<<8)|(source[1]);
	if (MapValue(mc,c)) {
	    source+=2;
	    n++;
	} else {
	    c=*source++;
	    if (!MapValue(mc,c)) return source-1;
	}
    }
    return NULL;
}

static Uchar *encode_12bit(unsigned char *source, Uchar *target, void *data)
{
    /* data contains a MapUchar for an 12-bit (mix 8&16 bit) encoding.
    ** Thus: a pair of characters is mapped to a single unicode character
    ** if that pair is defined, otherwise a single character character is
    ** is mapped.
    ** unknown characters are filtered out.
    */
    MapUchar mc=data;
    Uchar c;
    Uchar *res;
    res=target;
    if (!target) return NULL;
    if (!source) {
	*target=0;
	return target;
    }
    while (*source) {
	c=((source[0])<<8)|(source[1]);
	if ((*target=MapValue(mc,c))) {
	    source+=2;
	    target++;
	} else {
	    c=*source++;
	    if ((*target=MapValue(mc,c))) target++;
	}
    }
    *target=0;
    return res;
}

static int delen_8bit(Uchar *source, void *data)
{
    MapUchar mc=data;
    Uchar c;
    int n=0;
    if (!source) return n;
    while (*source) {
	c=*source++;
	c=MapValue(mc,c);
	if (c && c<256) n++;
    }
    return n;
}

static Uchar *defail_8bit(Uchar *source, void *data)
{
    MapUchar mc=data;
    Uchar c;
    if (!source) return NULL;
    while (*source) {
	c=*source++;
	c=MapValue(mc,c);
	if (!(c && c<256)) return source-1;
    }
    return NULL;
}

static unsigned char *decode_8bit(Uchar *source, unsigned char *target,
				  void *data)
{
    /* data contains a MapUchar for an 8-bit encoding
    ** Thus: characters are mapped individually
    ** unknown characters are filtered out.
    */
    MapUchar mc=data;
    Uchar c;
    unsigned char *res;
    res=target;
    if (!target) return NULL;
    if (!source) {
	*target=0;
	return target;
    }
    while (*source) {
	c=*source++;
	c=MapValue(mc,c);
	if (c && c<256) *target++=c;
    }
    *target=0;
    return res;
}

static int delen_16bit(Uchar *source, void *data)
{
    MapUchar mc=data;
    Uchar c;
    int n=0;
    if (!source) return n;
    while (*source) {
	c=*source++;
	c=MapValue(mc,c);
	/* only add if both bytes are not zero */
	if ((c&0xff00) && (c&0x00ff)) n=n+2;
    }
    return n;
}

static Uchar *defail_16bit(Uchar *source, void *data)
{
    MapUchar mc=data;
    Uchar c;
    if (!source) return NULL;
    while (*source) {
	c=*source++;
	c=MapValue(mc,c);
	if (!((c&0xff00) && (c&0x00ff))) return source-1;
    }
    return NULL;
}

static unsigned char *decode_16bit(Uchar *source, unsigned char *target,
				   void *data)
{
    /* data contains a MapUchar for an 16-bit encoding
    ** Thus: characters are mapped in pairs
    ** unknown pairs are filtered out.
    */
    MapUchar mc=data;
    Uchar c;
    unsigned char *res;
    res=target;
    if (!target) return NULL;
    if (!source) {
	*target=0;
	return target;
    }
    while (*source) {
	c=*source++;
	c=MapValue(mc,c);
	if ((c&0xff00) && (c&0x00ff)) {
	    *target++=(c>>8);
	    *target++=(c&0xff);
	}
    }
    *target=0;
    return res;
}

static int delen_12bit(Uchar *source, void *data)
{
    MapUchar mc=data;
    Uchar c;
    int n=0;
    if (!source) return n;
    while (*source) {
	c=*source++;
	c=MapValue(mc,c);
	if (c>256 && c&0xff) {
	    n=n+2;
	} else if (c && c<256) {
	    n++;
	}
    }
    return n;
}

static Uchar *defail_12bit(Uchar *source, void *data)
{
    MapUchar mc=data;
    Uchar c;
    if (!source) return NULL;
    while (*source) {
	c=*source++;
	c=MapValue(mc,c);
	if (!(c&0xff)) return source-1;
    }
    return NULL;
}

static unsigned char *decode_12bit(Uchar *source, unsigned char *target,
				   void *data)
{
    /* data contains a MapUchar for an 12-bit (mix 8&16 bit) encoding.
    ** Thus: a pair of characters is mapped to a single unicode character
    ** if that pair is defined, otherwise a single character is mapped.
    ** unknown characters are filtered out.
    */
    MapUchar mc=data;
    Uchar c;
    unsigned char *res;
    res=target;
    if (!target) return NULL;
    if (!source) {
	*target=0;
	return target;
    }
    while (*source) {
	c=*source++;
	c=MapValue(mc,c);
	if (c>256 && (c&0xff)) {
	    *target++=(c>>8);
	    *target++=(c&0xff);
	} else if (c && c<256) {
	    *target++=c;
	}
    }
    *target=0;
    return res;
}

Uchar utf8name[]= { 'U', 'T', 'F', '8',0};

static int utf8enlen(unsigned char *text, void *data)
{
    return UTFstrlen(text);
}
static int utf8delen(Uchar *text, void *data)
{
    return UTFneedlen(text);
}

static Uchar *utf8encode(unsigned char *source, Uchar *target, void *data)
{
    if (!source) return target;
    UTFtoUstr(source, target);
    return target;
}

static unsigned char *utf8decode(Uchar *source, unsigned char *target,
				 void *data)
{
    if (!source) return target;
    UstrtoUTF(source,target);
    return target;
}
static unsigned char *utf8enfail(unsigned char *text, void *data)
{
    int i=UTFcheck(text);
    if (i<0) return text+(-i);
    else return NULL;
}

static Uchar *utf8defail(Uchar *text, void *data)
{
    /* utf conversion should never fail */
    return NULL;
}

static void add_builtin_conv(void)
{
    UConverter *uc;
    uc=new_converter();
    if (!uc) return;
    uc->initialised=1;
    uc->name=utf8name;
    uc->encode=utf8encode;
    uc->decode=utf8decode;
    uc->needenlen=utf8enlen;
    uc->needdelen=utf8delen;
    uc->enfail=utf8enfail;
    uc->defail=utf8defail;
    uc->dataencode=0;
    uc->datadecode=0;
}

static UConverter conv8bit = { 0, 0, 0,
			       encode_8bit, decode_8bit,
			       enlen_8bit, delen_8bit,
			       enfail_8bit, defail_8bit,
			       0, 0};
static UConverter conv16bit = { 0, 0, 0,
				encode_16bit, decode_16bit,
				enlen_16bit, delen_16bit,
				enfail_16bit, defail_16bit,
				0, 0};
static UConverter conv12bit = { 0, 0, 0,
				encode_12bit, decode_12bit,
				enlen_12bit, delen_12bit,
				enfail_12bit, defail_12bit,
				0, 0};

static void add_8bit_conv(Uchar *name, void *dataenc, void *datadec)
{
    UConverter *uc;
    UConvID ci;
    uc=new_converter();
    if (!uc) return;
    ci=uc->convid;
    *uc=conv8bit;
    uc->name=name;
    uc->dataencode=dataenc;
    uc->datadecode=datadec;
}    

static void add_16bit_conv(Uchar *name, void *dataenc, void *datadec)
{
    UConverter *uc;
    UConvID ci;
    uc=new_converter();
    if (!uc) return;
    ci=uc->convid;
    *uc=conv16bit;
    uc->name=name;
    uc->dataencode=dataenc;
    uc->datadecode=datadec;
}    

static void add_12bit_conv(Uchar *name, void *dataenc, void *datadec)
{
    UConverter *uc;
    UConvID ci;
    uc=new_converter();
    if (!uc) return;
    ci=uc->convid;
    *uc=conv12bit;
    uc->name=name;
    uc->dataencode=dataenc;
    uc->datadecode=datadec;
}    

/* Load a converter database. The database contains lists of
** converter names and mapping tables.
** The standaard mappings are loaded if NAME is NULL
** Internal mappings are added at the first call to LoadDatabase.
*/

void UConvLoadDatabase(char *name)
{
    char buffer[256];
    PathInfo pinf;
    UConverter *utfconv;
    FILE *f;
    /* add builtin converters */
    if (!max_converter) add_builtin_conv();
    utfconv=UConvGet(UConvGetID(utf8name));
    pinf=make_pathinfo("MAPPATH", DEFAULTMAPPATH, ".conv");
    if ((f=open_file(pinf, name, "r"))) {
	while (fgets(buffer, 255,f)) {
	    char *c,*h;
	    char *enname, *dename;
	    char convtype;
	    Uchar *convname;
	    h=buffer;
	    c=strchr(buffer,'@');
	    if (!c) continue;
	    *c=0;
	    convname=UConvEncode(h, NULL, utfconv);
	    h=c+1;
	    convtype=*h++;
	    if (convtype=='1') {
		convtype=*h++;
	    }
	    if (*h!='@') { free(convname); continue; }
	    h++;
	    c=strchr(h,'@');
	    if (c) {
		*c=0;
		enname=malloc((c-h+1)*sizeof(char));
		strcpy(enname, h);
		h=c+1;
	    } else {
		enname=0;
	    }
	    c=strchr(h,'@');
	    if (c) {
		*c=0;
		dename=malloc((c-h+1)*sizeof(char));
		strcpy(dename, h);
		h=c+1;
	    } else {
		dename=0;
	    }
	    switch (convtype) {
	    case '8':
		add_8bit_conv(convname, enname, dename);
		break;
	    case '2':
		add_12bit_conv(convname, enname, dename);
		break;
	    case '6':
		add_16bit_conv(convname, enname, dename);
		break;
	    default:
		fprintf(stderr, "Unknown encoding type '%c'\n",
			convtype);
		if (convname) free(convname);
		if (enname) free(enname);
		if (dename) free(dename);
		break;
	    }
	}
	close_file(f);
    }
    destroy_pathinfo(pinf);
  /* open file with name NAME */
  /* LOOP:  read item, parse item, add convertor */
}

static PathInfo pinf=0;

/* load a Uchar to Uchar map */
static void *load_map(void *name)
{
    FILE *f;
    int fsize;
    int rsize=0;
    void *res,*res2;
    if (!pinf) { pinf=make_pathinfo("MAPPATH", DEFAULTMAPPATH, ".map"); }
    fsize=file_size(pinf, name);
    if (fsize<0) fsize=0x20000;
    f=open_file(pinf, name, "r");
    res=malloc(fsize);
    rsize=fread(res, 1,fsize, f);
    close_file(f);
    if (rsize<fsize) {
	res2=realloc(res,rsize);
	if (res2) {
	    res=res2;
	    fsize=rsize;
	}
    }
    MapUcharLoad(res2,res);
    if (!res2) free(res);
    return res2;
}

/* Load the converter with a certain ID */
static void UConvLoad(UConvID convid)
{
    UConverter *uc;
    if (convid<0) return;
    uc = uconvlist[convid];
    if (uc->initialised) return;
    if (uc->dataencode) {
	void *de=load_map((Uchar*)uc->dataencode);
	free(uc->dataencode);
	uc->dataencode=de;
    }
    if (uc->datadecode) {
	void *dd=load_map((Uchar*)uc->datadecode);
	free(uc->datadecode);
	uc->datadecode=dd;
    } else {
	MapUcharInverse(uc->datadecode, uc->dataencode);
    }
    uc->initialised=1;
}

/* lookup a converter or encoding name */
UConvID UConvGetID(Uchar *name)
{
  int i;
  for (i=0; i<max_converter; i++) {
    if (!Ustrcmp(name, uconvlist[i]->name)) return i;
  }
  return -1;
}    

/* get the converter with a certain ID */
UConverter *UConvGet(UConvID convid)
{
    if (convid>=0 && convid<max_converter) {
	if (!uconvlist[convid]->initialised) UConvLoad(convid);
	return uconvlist[convid];
    }
    return NULL;
}
