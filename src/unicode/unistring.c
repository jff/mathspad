/*
 *	Unicode string handling
 */
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>

#include "unitype.h"
#include "unistring.h"
#include "uniconv.h"

#define CHECKBYTE(A) (((A)&0xC0)==0x80)
static char extrabyte[128] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 1000 */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 1001 */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 1010 */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 1011 */
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* 1100 */
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* 1101 */
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  /* 1110 */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; /* 1111 */


int UTFcheck(const unsigned char *c)
{
  int i=0,j,k=0;
  while (c[k]) {
    if (c[k] & 0x80) {
      j=extrabyte[c[k]&0x7f];
      if (!j) return -k;
      k++;
      while (j && CHECKBYTE(c[k])) j--,k++;
      if (j) return -k;
      i++;
    } else
      i++,k++;
  }
  return i;
}

int UTFstrlen(const unsigned char *s)
{
  return abs(UTFcheck(s));
}

int UTFtoUstr(const unsigned char *s, Uchar *t)
{
    int i,j,k,r;
    i=0;j=0;
    while (s[i]) {
	if (s[i] & 0x80) {
	    k=extrabyte[s[i]&0x7f];
	    if (!k) break;
	    r=s[i]&(0x7f>>k);
	    i++;
	    while (k && CHECKBYTE(s[i])) {
		r=(r<<6)|(s[i]&0x3f);
		i++;
		k--;
	    }
	    if (k) break;
	    t[j++]=r;
	} else t[j++]=s[i++];
    }
    t[j]=0;
    return j;
}

#define MAX_BYTES 3
#define THREE_BYTES 0x800
#define TWO_BYTES 0x80
#define BYTE21(A)  (0xC0|((A)>>6))
#define BYTE22(A)  (0x80|((A)&0x3f))
#define BYTE31(A)  (0xE0|((A)>>12))
#define BYTE32(A)  (0x80|(((A)>>6)&0x3f))
#define BYTE33(A)  (0x80|((A)&0x3f))

int UTFneedlen(const Uchar *c)
{
  int n=0;
  while (*c) {
    if (*c<TWO_BYTES) n++;
    else if (*c<THREE_BYTES) n+=2;
    else n+=3;
    c++;
  }
  return n;
}

int UstrtoUTF(const Uchar *s, unsigned char *t)
{
    Uchar c;
    while (*s) {
	c=*s++;
	if (c<TWO_BYTES) { *t++=c; }
        else if (c<THREE_BYTES) {
	    *t++=BYTE21(c);
	    *t++=BYTE22(c);
	} else {
	    *t++=BYTE31(c);
	    *t++=BYTE32(c);
	    *t++=BYTE33(c);
	}
    }
    *t=0;
    return 1;
}


/* Copy SRC to DEST */
Uchar *Ustrcpy(Uchar *dest, const Uchar *src)
{
  int i=0;
  while ((dest[i]=src[i])) i++;
  return dest;
} 
/* Copy no more than N characters of SRC to DEST.  */
Uchar *Ustrncpy(Uchar *dest, const Uchar *src, size_t n)
{
  int i=0;
  while (i<n && (dest[i]=src[i])) i++;
  while (i<n) dest[i++]=0;
  return dest;
}

/* Append SRC onto DEST.  */
Uchar *Ustrcat(Uchar *dest, const Uchar *src)
{
  int i=0;
  Uchar *enddest=dest;
  while (*enddest) enddest++;
  while ((enddest[i]=src[i])) i++;
  return dest;
}
/* Append no more than N characters from SRC onto DEST.  */
Uchar *Ustrncat(Uchar *dest, const Uchar *src, size_t n)
{
  int i=0;
  Uchar *enddest=dest;
  while (*enddest) enddest++;
  while (i<n && (enddest[i]=src[i])) i++;
  return dest;
}

#define  Ucompmap(A) ((int)(A))
/*
** This function should remap everything such that the
** ordering makes more sense (a A a' a` a" a^).
int Ucompmap(const Uchar c)
{
  return (int) c;
}
*/

/* Compare S1 and S2.  */
/* a changable map should be used before comparing
** to enable correct ordering of a a' a^ a` a" A A' A^ A` A" etc.
*/
int Ustrcmp(const Uchar *s1, const Uchar *s2)
{
  int i=0;
  while (s1[i] && Ucompmap(s1[i])==Ucompmap(s2[i])) i++;
  if (Ucompmap(s1[i])==Ucompmap(s2[i])) return 0;
  if (Ucompmap(s1[i])<Ucompmap(s2[i])) return -1;
  return 1;
}

/* Compare N characters of S1 and S2.  */
int Ustrncmp(const Uchar *s1, const Uchar *s2, size_t n)
{
  int i=0;
  while (i<n && s1[i] && Ucompmap(s1[i])==Ucompmap(s2[i])) i++;
  if (i==n || Ucompmap(s1[i])==Ucompmap(s2[i])) return 0;
  if (Ucompmap(s1[i])<Ucompmap(s2[i])) return -1;
  return 1;
}

/* Compare the collated forms of S1 and S2.  */
/* extern int Ustrcoll(const Uchar *s1, const Uchar *s2); */
/* Put a transformation of SRC into no more than N bytes of DEST.  */
/* extern size_t Ustrxfrm(Uchar *dest, const Uchar *src, size_t n); */

/* Duplicate S, returning an identical malloc'd string.  */
Uchar *Ustrdup(const Uchar *s)
{
  int i=0;
  Uchar *c;
  while (s[i++]);
  if ((c = (Uchar*) malloc(i*sizeof(Uchar))))
    memcpy(c,s,i*sizeof(Uchar));
  return c;
}

/* Find the first occurrence of C in S.  */
Uchar *Ustrchr(const Uchar *s, Uchar c)
{
  int i=0;
  while (s[i] && s[i]!=c) i++;
  if (s[i]) return (Uchar*) (s+i);
  return NULL;
}

/* Find the last occurrence of C in S.  */
Uchar *Ustrrchr(const Uchar *s, Uchar c)
{
  int i=0,j=-1;
  
  while (s[i]) {
    if (s[i]==c) j=i;
    i++;
  }
  if (j>=0) return (Uchar*) s+j;
  return NULL;
}

/* These functions should use a table */
/* Return the length of the initial segment of S which
   consists entirely of characters not in REJECT.  */
size_t Ustrcspn(const Uchar *s, const Uchar *reject)
{
  int i=0,j;
  while (s[i]) {
    j=0;
    while (reject[j] && s[i]!=reject[j]) j++;
    if (reject[j]) return i;
    i++;
  }
  return i;
}

/* Return the length of the initial segment of S which
   consists entirely of characters in ACCEPT.  */
size_t Ustrspn(const Uchar *s, const Uchar *accept)
{
  int i=0,j;
  while (s[i]) {
    j=0;
    while (accept[j] && s[i]!=accept[j]) j++;
    if (accept[j]) i++; else return i;
  }
  return i;
}

/* Find the first occurence in S of any character in ACCEPT.  */
Uchar *Ustrpbrk(const Uchar *s, const Uchar *accept)
{
  int i=0,j;
  while (s[i]) {
    j=0;
    while (accept[j] && s[i]!=accept[j]) j++;
    if (accept[j]) return (Uchar*) s+i; else i++;
  }
  return NULL;
}

/* Building a table using NEEDLE could improve performance. */
/* Find the first occurence of NEEDLE in HAYSTACK.  */
Uchar *Ustrstr(const Uchar *haystack, const Uchar *needle)
{
  int i=0,j;
  while (haystack[i]) {
    j=0;
    while (needle[j] && haystack[i+j]==needle[j]) j++;
    if (!needle[j]) return (Uchar*) haystack+i;
    i++;
  }
  return NULL;
}

/* Divide S into tokens separated by characters in DELIM.  */
static Uchar *strtokdefarg=NULL;
Uchar *Ustrtok(Uchar *s, const Uchar *delim)
{
  int i=0,j,k=0;
  if (!s) s=strtokdefarg;
  if (!s) return NULL;
  while (s[i]) {
    j=0;
    while (delim[j] && s[i]!=delim[j]) j++;
    if (delim[j] && k) {
      s[i]=0;
      strtokdefarg=s+i+1;
      return s;
    } else if (delim[j] && !k) {
      s++;
    } else {
      k=1;
      i++;
    }
  }
  if (k) {
    strtokdefarg=NULL;
    return s;
  } else return NULL;
}
  

/* Return the length of S.  */
size_t Ustrlen(const Uchar *s)
{
  int i=0;
  while (s[i]) i++;
  return i;
}

static Uchar iso8859_1_convert(char c)
{
  return (Uchar) c;
}

static Uchar (*loccon)(char) = iso8859_1_convert;

/* convert character string S to a Unicode string, using the default
** encoding */
static Uchar *strtoUstr(const char *s)
{
  int i=strlen(s);
  Uchar *tmp= (Uchar *) malloc(i*sizeof(Uchar));
  i=0;
  while ((tmp[i]=(*loccon)(s[i]))) i++;
  return tmp;
}

/* should be in standard library */
char *strerror(int errnum) { return ""; }
char *strsignal(int signalnum) { return ""; }

/* A language dependent database should be used here, or at least
** a cache to use already converted errors */
/* Return a string describing the meaning of the errno code in ERRNUM.  */
Uchar *Ustrerror(int errnum)
{
  return (strtoUstr(strerror(errnum)));
}

/* Find the first occurrence of C in S (same as strchr).  */
Uchar *Uindex(const Uchar *s, int c)
{
  return Ustrchr(s,c);
}

/* Find the last occurrence of C in S (same as strrchr).  */
Uchar *Urindex(const Uchar *s, int c)
{
  return Ustrrchr(s,c);
}

/* Compare S1 and S2, ignoring case.  */
int Ustrcasecmp(const Uchar *s1, const Uchar *s2)
{
  int i=0;
  while (s1[i] && Utolower(s1[i])==Utolower(s2[i])) i++;
  if (Utolower(s1[i])==Utolower(s2[i])) return 0;
  if (Utolower(s1[i])<Utolower(s2[i])) return -1;
  return 1;
}

/* Return the next DELIM-delimited token from *STRINGP,
   terminating it with a '\0', and update *STRINGP to point past it.  */
Uchar *Ustrsep(Uchar **stringp, const Uchar *delim)
{
  Uchar *tmp=Ustrtok(*stringp, delim);
  *stringp=strtokdefarg;
  return tmp;
}

/* Compare no more than N chars of S1 and S2, ignoring case.  */
int Ustrncasecmp(const Uchar *s1, const Uchar *s2, size_t n)
{
  int i=0;
  while (i<n && s1[i] && Utolower(s1[i])==Utolower(s2[i])) i++;
  if (i==n || Utolower(s1[i])==Utolower(s2[i])) return 0;
  if (Utolower(s1[i])<Utolower(s2[i])) return -1;
  return 1;
}

#define MAX_UNI_SIG 64
static Uchar *Ustrsigcache[MAX_UNI_SIG]={0};
/* Return a string describing the meaning of the signal number in SIG.  */
Uchar *Ustrsignal(int sig)
{
  if (sig<MAX_UNI_SIG) {
    if (!Ustrsigcache[sig]) Ustrsigcache[sig]=strtoUstr(strsignal(sig));
    return Ustrsigcache[sig];
  } else return strtoUstr(strsignal(sig));
}

/* Copy SRC to DEST, returning the address of the terminating '\0' in DEST.  */
Uchar *Ustpcpy(Uchar *dest, const Uchar *src)
{
  int i=0;
  while ((dest[i]=src[i])) i++;
  return dest+i;
}

/* Copy no more then N bytes from SRC to DEST, returning the address
   of the terminating '\0' in DEST.  */
Uchar *Ustpncpy(Uchar *dest, const Uchar *src, size_t n)
{
  int i=0,j=0;
  while (i<n && (dest[i]=src[i])) i++;
  j=i;
  while (i<n) dest[i++]=0;
  return dest+i;
}

/* Change order of bytes in a N characters from Unicode string S */
/* N=0 -> N:=Ustrlen(S) */
void Uswapb(Uchar *s, size_t n)
{
  int i=0;
  while (i<n || (!n && s[i])) {
    s[i]=(s[i]&0xff)*256+(s[i]%256);
    i++;
  }
}


long Ustrtol(const Uchar *s, Uchar **res, int base)
{
    const Uchar *h = s;
    long n=0;
    Uchar c;
    Uchar sn='0', en, sl='A', el='@';

#define isbasenumber(A) ((sn<=(A) && (A)<=en) || (sl<=(A) && (A)<=el))

    if (!s) {
        if (res) *res = NULL;
        return 0;
    }
    while (Uisspace(*h)) h++;
    if (!base) {
        if (*h!='0') {
	  base=10;
	  if (Uisdigit(*h)) {
	    sn = *h-Utovalue(*h);
	  }
        } else {
            h++;
            if (*h=='X' || *h=='x') {
                base=16;
                h++;
            } else
                base=8;
        }
    }
    en = (Uchar)(sn+base-1);
    if (base>10) {
        sn = '0';
        en = '9';
        el = (Uchar)('A'+base-11);
    }
    c = (Uchar)(Utoupper(*h));
    while (isbasenumber(c)) {
      if (sn<=c && c<=en)
	n = n*base+c-sn;
      else
	n = n*base+c-sl+10;
      h++;
      c = (Uchar)(Utoupper(*h));
    }
    if (res)
      *res = (Uchar*)h;
    return n;
}

/* Similar to lltostr on Solaris, that is: 
** * insert the string representation of L before S.
** * return the first position of the resulting string.
** * result points somewhere in S.
** Example of usage:
**    Uchar buffer[BLEN];
**    buffer[BLEN-1]=0;
**    res = Ultostr(l,buffer+BLEN-1);
*/
Uchar *Ultostr(long l, Uchar *s)
{
  Uchar sign=0;
  if (!l) {
    s--;
    *s='0';
    return s;
  }
  if (l<0) { sign='-'; l = -l; }
  while (l) {
    s--;
    *s='0'+l%10;
    l=l/10;
  }
  if (sign) {
    s--;
    *s=sign;
  }
  return s;
}

#ifndef UNISTRING_ROTATE_MAX
#define UNISTRING_ROTATE_MAX 32
#endif

typedef struct {
  void *mem;
  int size;
} RotateBuffer;

static RotateBuffer rotatebuffer[UNISTRING_ROTATE_MAX];
static int currentpos=UNISTRING_ROTATE_MAX-1;
static int rotatecount=0;

void *get_next_buffer(int size)
{
  currentpos = (currentpos+1)%UNISTRING_ROTATE_MAX;
  rotatecount++;
  if (rotatebuffer[currentpos].size<size) {
    if (rotatebuffer[currentpos].size) free(rotatebuffer[currentpos].mem);
    rotatebuffer[currentpos].mem=malloc(size);
    rotatebuffer[currentpos].size=size;
  }
  return rotatebuffer[currentpos].mem;
}

static UConverter *locale_conv;

void get_locale(void)
{
  char *lenc;
  Uchar buf[1024];
  int i;
  UConvID cid;
  
  setlocale(LC_ALL,"");
  lenc = getenv("LC_ENCODING");
  if (!lenc) lenc="UTF8";
  i=0;
  while (i<1024 && (buf[i]=lenc[i])) i++;
  cid = UConvGetID(buf);
  if (cid<0) {
    fprintf(stderr,"Unknown encoding '%s'. Using UTF8.\n",lenc);
    lenc="UTF8";
    i=0;
    while (i<1024 && (buf[i]=lenc[i])) i++;
    cid = UConvGetID(buf);
  }
  locale_conv = UConvGet(cid);
}

/* Convert the string S to the current locale, for export to the
** 8-bit world.  Unencodable characters are filtered out.
** A transliteration table to ASCII could improve this.
*/

unsigned char *UstrtoLocale(const Uchar *s)
{
  unsigned char *res;
  int l;
  l=Ustrlen(s);
  res = get_next_buffer(l*4+1);
  if (!locale_conv) get_locale();
  UConvDecode(s,res,locale_conv);
#ifdef LOCALEDEBUG
  fprintf(stderr,"%2i:-> 8: %s\n", currentpos,res);
#endif
  return res;
}

Uchar *LocaletoUstr(const unsigned char *s)
{
  Uchar *res;
  int l;
  l=strlen(s);
  res = get_next_buffer((l+1)*sizeof(Uchar));
#ifdef LOCALEDEBUG
  fprintf(stderr,"%2i:->16: %s\n", currentpos,s);
#endif
  if (!locale_conv) get_locale();
  UConvEncode(s,res,locale_conv);
  return res;
}

/* Convert the filename contained in S to a 8-bit version
** It returns a temporary string.
** The convertion is done as follows:
** For each directory name in the path, convert the string as follows
** * If the string is encodable in the current locale, use the current
**   locale.
** * If the string is not encodable in the current locale, use the UTF8
**   encoding. Note that the UTF8 encoding can break existing applications,
**   for example, by filtering out the characters which can not be
**   displayed by the system.
** By setting the locale to UTF8, all filenames are stored in the UTF
** encoding. By setting the locale to ISO8859-n, a combination of UTF
** and ISO8859-n is the result. This could cause problems, but might
** allow existing software to handle most names correctly, assuming that
** most filenames are encodable in the current locale.
*/

/* For the moment, just use UstrtoLocale() */
char *UstrtoFilename(const Uchar *s)
{
  return (char*) UstrtoLocale(s);
}

/* For the moment, just use LocaletoUstr() */

Uchar *FilenametoUstr(const char *s)
{
  return LocaletoUstr((const unsigned char*)s);
}

