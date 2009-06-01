
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "filefind.h"

static char *concat(char *str1, char *str2)
{
    int i;
    char *res,*c;
    i= (str1?strlen(str1):0)+(str2?strlen(str2):0)+1;
    c=res=malloc(sizeof(char)*i);
    if (str1) while ((*c=*str1)) c++,str1++;
    if (str2) while ((*c++=*str2++));
    else *c=0;
    return res;
}

/*  Some requirements
**  * the interface to a filefind routine should be simple
**  * no fancy auto-create options
**  * never changing API interface (additions allowed)
**  * general enough to be useful
**  * automatic compress and decompress where needed
**  * (later: support for sockets and protocols (http,ftp))
*/

typedef enum { FileNormal, FileCompressed,
	       FileGzipped, FileMaxType } FileType;

static int normal_filesize(char *name)
{
    struct stat stbuf;
    if (stat(name, &stbuf)!=-1) {
	return stbuf.st_size;
    }
    return -1;
}

static int compress_filesize(char *name)
{
    return -1;
}

static int gzip_filesize(char *name)
{
    /* not 100% full-proof:  appended gzip files are wrong */
    FILE *f;
    struct stat stbuf;
    int i,j,n,k;
    if (stat(name, &stbuf)==-1) return -1;
    if (!(f=fopen(name,"rb"))) return -1;
    fseek(f,stbuf.st_size-4,0);
    n=0;k=0;
    for (i=0;i<4;i++) {
	j=fgetc(f);
	if (j==EOF) { n=-1; break; }
	n=n+((j&0xff)<<k);
	k=k+8;
    }
    fclose(f);
    return n;
}

/* possible extension: search/read within zip or tar files. */
static struct {
  char *ext;      /* extension */
  int extlen;     /* length of extension */
  char *readcom;  /* read command for pipes (%s indicates full filename) */
  char *writecom; /* write command for pipes (%s indicates full filename) */
  char *appendcom;/* append command for pipes (%s indicates full filename) */
  int (*fsize)(); /* detect size of file */
} filecom[FileMaxType] = {
  { NULL,  0, NULL,               NULL,         NULL,   normal_filesize  },
  { ".Z",  2, "uncompress -c %s", "compress > %s",
    "(uncompress -c %1$s; cat) | compress > %1$s.append;mv %1$s.append %1$s",
    compress_filesize },
  { ".gz", 3, "gunzip -c %s",     "gzip > %s",  "gzip >> %1", gzip_filesize },
};

/* Note that the append commands for compressed files are very inefficient.
** Adding something to the end involves decompressing the file and
** compressing it again.  It should only be done for small files, but for
** an orthogonal behaviour of the open_file and close_file, it is needed.
** (My guess is that append is not used very often, certainly not on
** on compressed files).
** A program that tries to do special things (as in "r+" mode), should not
** use the compressed extensions.  If it is needed, support for automatic
** compression an decompression could be added, so actual files are
** uses instead of pipes.
*/
 
typedef struct ExtFileInfo ExtFileInfo;
struct ExtFileInfo {
  FILE *f;
  char *fullname;
  /* time_t lastmod; */
  FileType ftype;
  ExtFileInfo *next;
};

static ExtFileInfo *globfilelist=0;

typedef struct DirInfo DirInfo;
struct DirInfo {
  char *fullname;
  int len;
  DirInfo *next;
};

static DirInfo rootdir= { "", 0, 0 };


typedef struct CacheInfo CacheInfo;
struct CacheInfo {
  int hashval;     /* for basename */
  char *fullname;  /* full name (including extension) */
  char *basename;  /* point in fullname */
  FileType ftype;  /* file type */
  time_t lastmod;
  CacheInfo *next;
};

typedef struct ExtenInfo ExtenInfo;
struct ExtenInfo {
  char *extension;
  int extlen;
  ExtenInfo *next;
};


/* hide any information contained in PathInfo */
typedef struct {
  /* ExtFileInfo *filelist; */
  DirInfo *dirlist;
  ExtenInfo *extlist;
  char *envname;   /* environment variable  */
  char *defpath;   /* default/built-in path */
  char *ext;
} InPathInfo;

static void free_dirlist(DirInfo *dlist)
{
    DirInfo *h;
    while (dlist) {
	h=dlist;
	dlist=h->next;
	free(h->fullname);
	free(h);
    }
}

static DirInfo *get_dirs(char *path, char *defpath)
{
    DirInfo *res=0;
    DirInfo **add=&res;
    char *reserve=0;
    int defused=0;
    char *pos;
    int l;
#define PATHSEP ':'
#define DIRSEP '/'
    pos=path;
    while (*pos || (((pos=reserve)) && *pos)) {
        if (pos==reserve) reserve=0;
	l=0;
	while (pos[l] && pos[l]!=PATHSEP) l++;
	if (!l) {
	    if (!defused) {
		reserve=pos+1;
		pos=defpath;
		defused=1;
	    }
	} else {
	    *add=malloc(sizeof(DirInfo));
	    (*add)->next=0;
	    (*add)->fullname=malloc(sizeof(char)*(l+2));
	    strncpy((*add)->fullname, pos, l);
	    pos=pos+l;
	    if (pos[-1]!=DIRSEP) {
		(*add)->fullname[l++]=DIRSEP;
	    }
	    (*add)->fullname[l]='\0';
	    (*add)->len=l;
	    add=&(*add)->next;
	    /* special care for at the end */
	    if (pos[0] && (pos[1] || defused)) {
		pos++;
	    } /* else: at the end or empty field at end follows */
	}
    }
    return res;
}

static ExtenInfo *get_extensions(char *elist)
{
    ExtenInfo *res=0;
    ExtenInfo **add=&res;
    char *pos;
    int l;
#define EXTSEP ','
    pos=elist;
    while (*pos) {
	l=0;
	while (pos[l] && pos[l]!=EXTSEP) l++;
	if (l) {
	    *add=malloc(sizeof(ExtenInfo));
	    (*add)->next=0;
	    (*add)->extension=malloc(sizeof(char)*(l+1));
	    strncpy((*add)->extension, pos, l);
	    (*add)->extension[l]='\0';
	    (*add)->extlen=l;
	    add=&(*add)->next;
	    pos=pos+l+(pos[l]==EXTSEP?1:0);
	} else {
	  pos=pos+(*pos==EXTSEP?1:0);
	}
    }
    return res;
}

static void free_extlist(ExtenInfo *elist)
{
    ExtenInfo *h;
    while (elist) {
	h=elist;
	elist=h->next;
	free(h->extension);
	free(h);
    }
}

/* rehash the information in the PINFO structure */
void rehash_pathinfo(PathInfo pinfo)
{
  InPathInfo *ipi=(InPathInfo*)pinfo;
  char *envval;
  if (ipi->dirlist) free_dirlist(ipi->dirlist);
  envval=getenv(ipi->envname); /* alternative: "echo $ipi->envname" */
  if (!envval) envval=":";
  ipi->dirlist=get_dirs(envval, ipi->defpath);
  if (ipi->extlist) free_extlist(ipi->extlist);
  ipi->extlist=get_extensions(ipi->ext);
}

/* create a structure which holds the information needed to find a file
** ENVVAR is the name of the environment variable (the search path)
** DEFPATH contains the default search path
** EXTS contains the a list of possible extensions
**
** What is should do:
**     *  create a datastructure to be able to find files.
**     *  allow caching.
**     *  allow EXT.index and EXT.alias for index and alias mapping
**     *  allow recursive-decent on directory structures
**     *  allow rehash functions to rebuild the datastructure
**     *  allow optional .gz, .Z and .z extensions
**     *  store extra information for opened files
*/
PathInfo make_pathinfo(char *envvar, char *defpath, char *exts)
{
  InPathInfo *ipi;

  ipi=malloc(sizeof(InPathInfo));
  ipi->envname=concat(envvar,"");
  ipi->defpath=concat(defpath,"");
  ipi->ext=concat(exts,"");
  ipi->dirlist=0;
  ipi->extlist=0;
  rehash_pathinfo(ipi);
  return ipi;
}

/* destroy the PINFO structure */
void destroy_pathinfo(PathInfo pinfo)
{
  InPathInfo *ipi=(InPathInfo*)pinfo;
  free_dirlist(ipi->dirlist);
  free_extlist(ipi->extlist);
  free(ipi->envname);
  free(ipi->defpath);
  free(ipi->ext);
  ipi->dirlist=0;
  ipi->extlist=0;
  ipi->envname=ipi->defpath=ipi->ext=0;
  free(ipi);
}

/* The following functions will also work if no correct PINFO is given.
** The current directory will be used for all access and default database
** for opened files.
*/

static FileType detect_ftype(char *name)
{
    int i,j;
    i=strlen(name);
    j=FileMaxType;
    while (j) {
	j--;
	if (filecom[j].extlen && i>filecom[j].extlen &&
	    !strcmp(name+i-filecom[j].extlen, filecom[j].ext)) break;
    }
    return j;
}

static ExtFileInfo *make_fileinfo(char *name, FileType ftype)
{
    ExtFileInfo *efi;
    efi=malloc(sizeof(ExtFileInfo));
    if (name[0]!='/') {
	int i;
	char workdir[1024];
	getcwd(workdir, 1024);
	i=strlen(workdir);
	if (workdir[i-1]!='/') {
	    workdir[i++]='/';
	    workdir[i]='\0';
	}
	efi->fullname=concat(workdir, name);
    } else {
	efi->fullname=concat(name,"");
    }
    if (ftype==FileMaxType) ftype=detect_ftype(name);
    efi->ftype=ftype;
    return efi;
}

/* are zipped files allowed in a search ? */
static int zips_allowed=1;

void allow_zips(int yes)
{
    zips_allowed=yes; /*?*/
}

char *search_file(PathInfo pinfo, char *name, int *ftype)
{
    struct stat stbuf;
    DirInfo *di;
    ExtenInfo *eli=0;
    ExtenInfo noext;
    ExtenInfo *ei;
    InPathInfo *ipi=(InPathInfo*)pinfo;
    FileType ft=FileNormal;
    int found=0, i,j;
    char buffer[1024];
    char *bpos;
    /* Test if name ends with a default (.Z, .gz) extension. If
    ** so, we do not need to add any extension, since it should
    ** be there already.
    */
    ft=detect_ftype(name);
    if (!ipi) {
	*ftype=ft;
	return concat(name,"");
    }
    /* Test if name ends with a correct extension. If not, we need
    ** to add one during the search.
    */
    if (!filecom[ft].ext) {
	i=strlen(name);
	ei=ipi->extlist;
	while (ei) {
	    if (i>ei->extlen &&
		!strcmp(name+i- ei->extlen, ei->extension)) break;
	    ei=ei->next;
	}
	if (!ei) {
	    eli=ipi->extlist;
	} else {
	    noext.extension="";
	    noext.extlen=0;
	    noext.next=0;
	    eli=&noext;
	}
    }
    /* if name starts with DIRSEP ('/'), then rootdir is used and
    ** the search path is ignored.
    ** Zip extensions are added if the file itself is not available.
    */ 
    if (name[0]==DIRSEP) {
      di = &rootdir;
    } else {
      di=ipi->dirlist;
    }
    j=(zips_allowed?FileMaxType:FileNormal+1);
    while (di && !found) {
	strcpy(buffer,di->fullname);
	bpos=buffer+strlen(buffer);
	strcpy(bpos,name);
	bpos=bpos+strlen(bpos);
	if (!eli) {
	    /* name already contains a (zip) extension. */
	    if (stat(buffer, &stbuf)!=-1) {
		found=1;
	    }
	} else {
	    /* try all combinations of extensions and methods (.Z,.gz) */
	    ei=eli;
	    while (ei && !found) {
		strcpy(bpos, ei->extension);
		i=FileNormal;
		while (i<j && !found) {
		    if (filecom[i].ext) {
			strcpy(bpos+ei->extlen, filecom[i].ext);
		    } else bpos[ei->extlen]='\0';
		    if (stat(buffer, &stbuf)!=-1) {
			found=1;
			ft=i;
		    }
		    i++;
		}
		ei=ei->next;
	    }
	}
	di=di->next;
    }
    if (found) {
	*ftype=ft;
	return concat(buffer,"");
    } else {
	*ftype=FileMaxType;
	return 0;
    }
}

time_t backuptime=900; /* 15 minuten */

static time_t current_time(void)
{
    return time(NULL);
}

void make_backup(char *name)
{
    /* emacs style backups */
    char newname[1024];
    char newnameextra[1024];
    struct stat stbuf;
    int i,n;
    strcpy(newname, name);
    strcpy(newnameextra, name);
    n=i=strlen(newname);
    newnameextra[i]='~';
    newnameextra[i+1]='\0';
    /* add '~' until file is not used */
    while (stat(newnameextra, &stbuf)!=-1) {
	newname[i]=newnameextra[i+1]='~';
	i++;
	newname[i]=newnameextra[i+1]='\0';
    }
    /* add '~' to all files */
    while (i>=n) {
	link(newname, newnameextra);
	unlink(newname);
	i--;
	newname[i]=newnameextra[i+1]='\0';
    }
}

/* open a file with name NAME in mode MODE
** if MODE is not write, the file is searched for, otherwise
** the file is written according to the current directory.
** If the used filename (either the found one or NAME) has a
** compress extension (.z, .gz or .Z), the compressor or decompressor
** is automatically started (using popen).
** In write mode, an existing file will first be moved by changing the name
*/
FILE *open_file(PathInfo pinfo, char *name, char *mode)
{
  char amode=mode[0]; /* 'r' 'a' or 'w' */
  ExtFileInfo *efi=NULL;
  if (amode=='w') {
      /* no search needed */
      struct stat stbuf;
      if (stat(name,&stbuf) != -1) {
	  /* file exists, make backup if file is more then backuptime seconds
	  ** old.
	  */
	  if (stbuf.st_mtime + backuptime < current_time()) {
	      /* make a backup */
	      make_backup(name);
	  }
      }
      efi=make_fileinfo(name,FileMaxType);
      if (efi) {
	  if (filecom[efi->ftype].writecom) {
	      char command[2048];
	      sprintf(command, filecom[efi->ftype].writecom, efi->fullname);
	      efi->f=popen(command, mode);
	  } else {
	      efi->f=fopen(efi->fullname, mode);
	  }
      }
  } else {
      char *foundname;
      int i;
      FileType ftype;
      /* search file */
      foundname=search_file(pinfo, name, &i);
      ftype=i;
      /* open file FOUNDNAME as type FTYPE */
      /* add file to filelist */
      if (foundname) {
	  efi=make_fileinfo(foundname, ftype);
	  free(foundname);
      }
      if (efi) {
	  if (amode=='r' && filecom[efi->ftype].readcom) {
	      char command[2048];
	      sprintf(command, filecom[efi->ftype].readcom, efi->fullname);
	      efi->f=popen(command, mode);
	  } else if (amode=='a' && filecom[efi->ftype].appendcom) {
	      char command[2048];
	      char newmode[256];
	      /* append command should the problem of getting the original
	      ** file in front of the rest
	      */
	      sprintf(command, filecom[efi->ftype].appendcom, efi->fullname);
	      /* append on pipes is not allowed, change to write. */
	      strncpy(newmode, mode,254);
	      mode[0]='w';
	      efi->f=popen(command, mode);
	  } else { /* write is handled somewhere else */
	      efi->f=fopen(efi->fullname, mode);
	  }
      }
  }
  if (efi) {
      if (efi->f) {
	  efi->next=globfilelist;
	  globfilelist=efi;
	  return efi->f;
      } else {
	  fprintf(stderr, "Unable to open file %s in mode '%s'\n",
		  efi->fullname, mode);
	  free(efi->fullname);
	  free(efi);
      }
  } else {
      fprintf(stderr, "File `%s' not found.\n", name);
  }
  return 0;
}

/* close a file opened with open_file. */
extern void close_file(FILE *file)
{
  ExtFileInfo *efi;
  ExtFileInfo **pefi;

  efi=globfilelist;
  pefi=&globfilelist;
  while (efi && efi->f != file) { pefi=&(efi->next); efi=efi->next; }
  if (efi) {
    switch (efi->ftype) {
    case FileNormal:     fclose(file); break;
    case FileCompressed: pclose(file); break;
    case FileGzipped:    pclose(file); break;
    default:  fclose(file);
    }
    free(efi->fullname);
    *pefi=efi->next;
    free(efi);
  } else {
    /* no file found */
    fprintf(stderr, "Unable to close unknown file. Use fclose or pclose.\n");
  }
}

int file_size(PathInfo p, char *name)
{
    int i,j=-1;
    char *fn;
    fn=search_file(p, name, &i);
    if (fn) {
	j=(*filecom[i].fsize)(fn);
	free(fn);
    }
    return j;
}

/* get the name of a file */
char *name_file(FILE *file)
{
  ExtFileInfo *efi=globfilelist;
  while (efi && efi->f!=file) efi=efi->next;
  if (efi) return efi->fullname;
  else return 0;
}
