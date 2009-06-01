#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "variable.h"
#include "leaftree.h"
#include "flexarray.h"
#include "unistring.h"


typedef struct {
    Type pos;
    int conssize;
    char *name;
    void (*construct)(void**);     /* create object */
    void (*destruct)(void*);       /* destroy object */
    void (*copy)(void**, void*);   /* copy(dest,source) */
    int conskind;
    union {
	void *record;
    } cons;
} TypeConstruct;

static int type_cmp(const void *type1, const void *type2)
{
  return strcmp(((TypeConstruct*)type1)->name,
		((TypeConstruct*)type2)->name);
}

static FlexArray typepool = { 0,0,0,sizeof(TypeConstruct), type_cmp };
#define typepool_add(T) fx_add(&typepool, &(T))
#define typepool_item(I) (*((TypeConstruct*) fx_item(&typepool, (I))))
#define typepool_max     fx_max(&typepool)
#define typepool_search(T) fx_contains(&typepool, &(T))


static void typepool_init(void)
{
  TypeConstruct tc;
  /* fill the typepool with the first items */
  if (typepool_max>=FirstFreeType) return;
  tc.conskind=0;
  tc.construct=0;
  tc.destruct=0;
  tc.copy=0;
  tc.pos=0;
  tc.conssize=0;
  tc.name="NoValidType";
  typepool_add(tc);
  tc.pos=1;
  tc.conssize=sizeof(int);
  tc.name="Int";
  typepool_add(tc);
  tc.pos=2;
  tc.conssize=sizeof(Uchar*);
  tc.name="String";
  typepool_add(tc);
  tc.pos=3;
  tc.conssize=sizeof(double);
  tc.name="Real";
  typepool_add(tc);
  tc.pos=4;
  tc.conssize=sizeof(void*);
  tc.name="Lazy";
  typepool_add(tc);
}

Type define_type(char *name, int size,
		 void (*construct)(void**),
		 void (*destruct)(void*),
		 void (*copy)(void**, void*))
{
  TypeConstruct tc;
  int type_exists;
  if (!typepool_max) typepool_init();
  tc.pos=typepool_max;
  tc.conssize=size;
  tc.name=name;
  type_exists = typepool_search(tc);
  if (type_exists) {
    Type t=type_exists-1;
    if (!(typepool_item(t).construct)) {
      typepool_item(t).construct=construct;
    }
    if (!(typepool_item(t).destruct)) {
      typepool_item(t).destruct=destruct;
    }
    if (!(typepool_item(t).copy)) {
      typepool_item(t).copy=copy;
    }
    return t;
  }
  tc.construct=construct;
  tc.destruct=destruct;
  tc.copy=copy;
  tc.conskind=0;
  typepool_add(tc);
  return tc.pos;
}

static char *refext="Ref";
static int refextlen=3;

Type lookup_type(char *name)
{
  TypeConstruct tc;
  char unrefname[1024];
  int isref, len,i;
  if (!typepool_max) typepool_init();
  isref=0;
  strcpy(unrefname, name);
  len=strlen(unrefname);
  if (!strcmp(unrefname+len-refextlen, refext)) {
    unrefname[len-refextlen]=0;
    isref=1;
  }
  tc.name=unrefname;
  i = typepool_search(tc);
  if (i) {
    if (isref) return ToRefType(i-1);
    else return i-1;
  } else
    return 0;
}

char *lookup_typename(Type t)
{
  if (IsRef(t)) t=NoRefType(t);
  if (t<1 || t>=typepool_max) return "???";
  return typepool_item(t).name;
}

void ConstructValue(Value* v)
{
  Type t;
  if (!v) return;
  t=v->type;
  if (IsRef(t)) {
    v->val.stval=0;
  } else {
    if (typepool_item(BaseType(t)).construct) {
      (*(typepool_item(BaseType(t)).construct))(&v->val.stval);
    } else {
      v->val.stval=0;
    }
  }
}

void CopyValue(Value* dest, Value* orig)
{
  if (IsRef(dest->type)) {
    if (IsRef(orig->type)) {
      dest->val.stref=orig->val.stref;
    } else {
      dest->val.stref=(void*) (&(orig->val));
    }
  } else {
    if (IsRef(orig->type)) {
      if (typepool_item(BaseType(dest->type)).copy) {
	(*(typepool_item(BaseType(dest->type)).copy))(&dest->val.stval,
						      *(orig->val.stref));
      } else {
	memcpy(&dest->val, orig->val.stref, sizeof(TVal));
      }
    } else {
      if (typepool_item(BaseType(dest->type)).copy) {
	(*(typepool_item(BaseType(dest->type)).copy))(&dest->val.stval,
						      orig->val.stval);
      } else {
	dest->val=orig->val;
      }
    }
  }
}

void DestructValue(Value* v)
{
  Type t;
  if (!v) return;
  t=v->type;
  if (!IsRef(t)) {
    if (typepool_item(BaseType(t)).destruct) {
      (*(typepool_item(BaseType(t)).destruct))(v->val.stval);
    }
  }    
}

