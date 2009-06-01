
#include <stdlib.h>
#include <stdio.h>
#include "translate.h"
#include "unicode.h"
#include "unistring.h"

typedef struct Translation Translation;

struct Translation {
  char *original;    /* original string */
  Uchar *translate;  /* translated string */
  int hashvalue;     /* hash value of the original string */
  int autoconv;      /* Is the string converted automatically?
		     ** (To find untranslated program strings and
		     ** generate dummy translate files)
		     */
  Translation *next; /* Next translation in the list */
};

typedef struct {
  Uchar *langname;         /* name of the language */
  int languagecode;        /* codenumber for this language */
  int entries;             /* how many entries are there */
  int hashsize;            /* who many lists are there? */
  Translation **translist; /* hashsize pointers to translation strings */
} TransLang;

#define CYCLE_NEXT(A) (A) = ((A)+5)%23
#define MAX_CYCLE 15

#ifndef REHASHLIMIT
#define REHASHLIMIT 6 
#endif
#define DEFAULTHASHSIZE 7

static int hash_string(char *str)
{
  int i,k,j;
  i=0; k=0;j=0;
  if (!str) return 0;
  while (*str && j<MAX_CYCLE) {
    i=i^((*str)<<k);
    CYCLE_NEXT(k);
    str++;
    j++;
  }
  return i;
}

static TransLang *currentlang=0;

static void rehash_translang(TransLang *transl)
{
  if (transl->entries/transl->hashsize > REHASHLIMIT) {
    int i;
    int nhs;
    Translation **tlist;
    nhs = transl->hashsize*2+1;
    tlist = malloc(sizeof(Translation*)*nhs);
    for (i=0; i<nhs; i++) tlist[i]=0;
    for (i=0; i<transl->hashsize; i++) {
      Translation *tl, *tlh, *tlg;
      tl = transl->translist[i];
      tlh=0;
      /* reverse the list:
      ** tl: the part that still has to be reversed
      ** tlh: the part that already reversed.
      ** tlg: help variable
      */
      while (tl) {
	tlg=tl;
	tl=tl->next;
	tlg->next=tlh;
	tlh=tlg;
      }
      /* divide the list over the new lists,
      ** which will reverse the lists again.
      */
      tl = tlh;
      while (tl) {
	tlh=tl->next;
	tl->next=tlist[tl->hashvalue%nhs];
	tlist[tl->hashvalue%nhs]=tl;
	tl=tlh;
      }
    }
    free(transl->translist);
    transl->translist=tlist;
    transl->hashsize=nhs;
  }
}

static TransLang *new_language(void)
{
  TransLang *tlang;
  int i;
  tlang = malloc(sizeof(TransLang));
  tlang->languagecode=0;
  tlang->langname=0;
  tlang->entries=0;
  tlang->hashsize=DEFAULTHASHSIZE;
  tlang->translist = malloc(sizeof(Translation*)*DEFAULTHASHSIZE);
  for (i=0; i<DEFAULTHASHSIZE; i++) {
    tlang->translist[i]=0;
  }
  return tlang;
}


Uchar *translate(char *orig)
{
  int i;
  Translation *tl;
  if (!orig) return 0;
  i=hash_string(orig);
  if (!currentlang) currentlang=new_language();
  tl = currentlang->translist[i%currentlang->hashsize];
  while (tl && (tl->hashvalue!=i || strcmp(tl->original, orig))) tl=tl->next;
  if (tl) {
    return tl->translate;
  } else {
    /* Convert orig to a multibyte version.
    ** Assumption: ISO8859-1 encoding in program strings.
    */
    int l,n;
    Uchar *trans;
    l=strlen(orig)+1;
    trans = malloc(sizeof(Uchar)*l);
    for (n=0; n<l; n++) trans[n]=(unsigned)(orig[n]);
    tl= malloc(sizeof(Translation));
    tl->hashvalue=i;
    tl->original=orig;
    tl->translate=trans;
    tl->autoconv=1;
    tl->next=currentlang->translist[i%currentlang->hashsize];
    currentlang->translist[i%currentlang->hashsize]=tl;
    currentlang->entries++;
    rehash_translang(currentlang);
    return trans;
  }
}

void set_translation8(char *orig,  Uchar *transl)
{
  int i;
  Translation *tl;
  if (!currentlang) currentlang=new_language();
  i=hash_string(orig);
  tl = malloc(sizeof(Translation));
  tl->hashvalue=i;
  tl->original=orig;
  tl->translate=transl;
  tl->autoconv=0;
  tl->next=currentlang->translist[i%currentlang->hashsize];
  currentlang->translist[i%currentlang->hashsize]=tl;
  currentlang->entries++;
  rehash_translang(currentlang);
}

/* Convert the ORIG string into a 8bit string and call set_translation8. */
void set_translation(Uchar *orig,  Uchar *transl)
{
  int i;
  char *h, *orig8;
  Translation *tl;
  orig8 = h = (char*) orig;
  while (*orig) {
    *h = *orig;
    h++;
    orig++;
  }
  *h=0;
  /* could reallocate orig, but ORIG is usually not that large (<80) */
  if (!currentlang) currentlang=new_language();
  i=hash_string(orig8);
  tl = malloc(sizeof(Translation));
  tl->hashvalue=i;
  tl->original=orig8;
  tl->translate=transl;
  tl->autoconv=0;
  tl->next=currentlang->translist[i%currentlang->hashsize];
  currentlang->translist[i%currentlang->hashsize]=tl;
  currentlang->entries++;
  rehash_translang(currentlang);
}

void dump_translation(char *name, TransLang *lang)
{
  FILE *f;
  Translation *tl;
  int i;

  f=fopen(name, "w");
  if (!f) return;
  if (lang->langname) {
    fprintf(f,"Translation %s {\n\n", UstrtoLocale(lang->langname));
  } else {
    fprintf(f, "Translation Unknown {\n\n");
  }
  for (i=0; i<lang->hashsize; i++) {
    tl=lang->translist[i];
    while (tl) {
      if (tl->autoconv) {	
	fprintf(f, "    \"%s\"\t: \"%s\\0(TODO)\";\n", tl->original,
		tl->original);
      } else {
	fprintf(f, "    \"%s\"\t: \"%s\";\n", tl->original,
		UstrtoLocale(tl->translate));
      }
      tl=tl->next;
    }
  }
  fprintf(f, "}\n");
  fclose(f);
}

  
