/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
** Permission to use, copy, modify and distribute this software
** and its documentation for any purpose is hereby granted
** without fee, provided that the above copyright notice appear
** in all copies and that both that copyright notice and this
** permission notice appear in supporting documentation, and
** that the name of EUT not be used in advertising or publicity
** pertaining to distribution of the software without specific,
** written prior permission.  EUT makes no representations about
** the suitability of this software for any purpose. It is provided
** "as is" without express or implied warranty.
** 
** EUT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
** SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL EUT
** BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
** DAMAGES OR ANY DAMAGE WHATSOEVER RESULTING FROM
** LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
** CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
** OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
** OF THIS SOFTWARE.
** 
** 
** Roland Backhouse & Richard Verhoeven.
** Department of Mathematics and Computing Science.
** Eindhoven University of Technology.
**
********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include "mathpad.h"
#include "system.h"
#include "funcs.h"

#include "unistring.h"
#include "unitype.h"

static char *currentwd=NULL;
static int currentwdlen=0;
int user_id=0, group_id=0;
Bool failure=MP_False;

int mystrtoi(char *s, char **endp, int base)
{
    char *h = s;
    int n=0;
    char c;
    char sn='0', en, sl='A', el='@';

#define isbasenumber(A) (sn<=(A) && ((A)<=en || (sl<=(A) && (A)<=el)))

    if (!s) {
        *endp = NULL;
        return 0;
    }
    while (isspace(*h)) h++;
    if (!base) {
        if (*h!='0') base=10;
        else {
            h++;
            if (*h=='X' || *h=='x') {
                base=16;
                h++;
            } else
                base=8;
        }
    }
    en = (char)('0'+base-1);
    if (base>10) {
        en = '9';
        el = (char)('A'+base-1);
    }
    c = (char)(toupper(*h));
    while (isbasenumber(c)) {
        if (c<='9')
            n = n*base+c-'0';
        else
            n = n*base+c-'A'+10;
        h++;
        c = (char)(toupper(*h));
    }
    if (endp)
        *endp = h;
    return n;
}

char *begins_with(char *s, char *t)
{
    while (s && *s)
        if (!(toupper(*s++) == toupper(*t++)))
            return NULL;
    while (isspace(*t)) t++;
    return t;
}

Char *strip_name(Char *name)
{
    Char *c;

    c = Ustrrchr(name, '/');
    if (c)
        return c+1;
    else
        return name;
}

void concat_in(Char *dest, Char *s1, Char *s2)
{
    if (s1) {
        while (aig(*dest++ = *s1++));
        dest--;
    }
    if (s2)
        while (aig(*dest++ = *s2++));
    else *dest = '\0';
}

Char *concat(Char *s1, Char *s2)
{
    Char *temp;
    temp = (Char *) malloc(sizeof(Char)*((s1?Ustrlen(s1):0) +
                           (s2?Ustrlen(s2):0) + 1));
    concat_in(temp, s1, s2);
    return temp;
}

FILE *open_dirfile(Char *dir, Char *filename, char *mode)
{
    Char tchar[5000];
    FILE *tfile;

    concat_in(tchar, dir, translate(DIRSEPSTR));
    concat_in(tchar, tchar, filename);
    tfile = fopen(UstrtoFilename(tchar), mode);
    return tfile;
}

static Bool can_access_file(struct stat *stbuf)
{
    if (!user_id || !group_id) return MP_True;
    if (stbuf->st_uid==user_id) {
        if (!(stbuf->st_mode & S_IRUSR)) return MP_False;
        if (((stbuf->st_mode & S_IFMT) == S_IFDIR) &&
            !(stbuf->st_mode & S_IXUSR)) return MP_False;
        return MP_True;
    }
    if (stbuf->st_gid==group_id) {
        if (!(stbuf->st_mode & S_IRGRP)) return MP_False;
        if (((stbuf->st_mode & S_IFMT) == S_IFDIR) &&
            !(stbuf->st_mode & S_IXGRP)) return MP_False;
        return MP_True;
    }
    if (!(stbuf->st_mode & S_IROTH)) return MP_False;
    if (((stbuf->st_mode & S_IFMT) == S_IFDIR) &&
        !(stbuf->st_mode & S_IXOTH)) return MP_False;
    return MP_True;
}

Bool is_directory(Char *name)
{
    struct stat stbuf;
    if (stat(UstrtoFilename(name), &stbuf) != -1 && can_access_file(&stbuf))
        return ((stbuf.st_mode & S_IFMT)==S_IFDIR);
    else
        return 0;
}

static Bool is_file(Char *name)
{
    struct stat stbuf;
    if (stat(UstrtoFilename(name), &stbuf) != -1 && can_access_file(&stbuf))
        return ((stbuf.st_mode & S_IFMT)!=S_IFDIR);
    else
        return 0;
}

static int file_size(Char *name)
{
    struct stat stbuf;
    if (stat(UstrtoFilename(name), &stbuf) != -1 && can_access_file(&stbuf))
        return (stbuf.st_size);
    else
        return 0;
}

static Char search_result[1000];

Char *search_through_dirs(Char **dirs, int nr, Char *filename)
{
    int i,j,k,n;
    Bool fname;
    Bool f = MP_False;
    Char *buffer=search_result;

    for (i=0; !f && i<nr; i++) {
        fname=MP_False;
        k=0;
        for (j=0; dirs[i][j]; j++) {
            if (dirs[i][j]=='%' && !fname) {
                for (n=0; filename[n]; n++,k++)
                    buffer[k]=filename[n];
                fname=MP_True;
            } else
                buffer[k++]=dirs[i][j];
        }
        if (!fname) {
            if (buffer[k-1]!=DIRSEPCHAR) buffer[k++]=DIRSEPCHAR;
            for (j=0; filename[j]; buffer[k++]=filename[j++]);
        }
        buffer[k]='\0';
        f = is_file(buffer);
    }
    if (f)
        return buffer;
    else
        return NULL;
}

static Char *buffer2;

static int qsortintstrcmp(const void *e1, const void *e2)
{
    return Ustrcmp(buffer2+ *((int*) e1), buffer2+ *((int*) e2)); 
}

static char checkname[1024];

int read_dir_contents(Char *dirname, Bool only_files,
		      Bool (*mask_check)(Char*,Char*),
		      Char *mask,
		      Char ***dirs, int *nr_dirs, Char ***files, int *nr_files)
{
    DIR *dr;
    struct dirent *de;
    struct stat stbuf;
    char *ldp;
    int i,n,ld=0,lf=0;
    int idx;
    int *found_files;
    int *found_dirs;
    int fdlen, fflen, buflen;

    *nr_dirs = 0;
    *nr_files = 0;
    strcpy(checkname, UstrtoFilename(dirname));
    if (!(aig(dr = opendir(checkname)))) return 0;
    ldp = checkname+(strlen(checkname)-1);
    if (*ldp!=DIRSEPCHAR) {
        ldp[1] = DIRSEPCHAR;
        ldp[2] = '\0';
        ldp += 2;
    } else
        ldp++;
    n=file_size(dirname);
    /* problems with an automounter on Solaris:
    ** n=6 -> 6 subdirectories (Solaris 2.5.1)
    **     -> 6 mounted subdirectories of N visible directories. (Solaris 2.6)
    */
    if (!n) n=63;
    if (n<64) n=n*MAXNAMLEN;
    buffer2 = (Char *) malloc(n*sizeof(Char));
    found_files = (int*) malloc(n*sizeof(int));
    found_dirs = (int*) malloc(n*sizeof(int));
    buflen=fflen=fdlen=n;
    idx = 0;
    while (aig(de = readdir(dr))) {
        strcpy(ldp,de->d_name);
        if (stat(checkname, &stbuf) != -1 &&
	    can_access_file(&stbuf)) {
	    Char *ufn;
	    ufn = FilenametoUstr(de->d_name);
	    i=Ustrlen(ufn);
	    if ((stbuf.st_mode & S_IFMT)==S_IFDIR) {
                if (strcmp(de->d_name,".") && !only_files) {
		  if (*nr_dirs==fdlen) {
		    found_dirs=realloc(found_dirs, (fdlen*2)*sizeof(int));
		    fdlen=fdlen*2;
		  }
                  found_dirs[*nr_dirs] = idx;
		  (*nr_dirs)++;
		  if (idx+i+1>=buflen) {
		    buffer2=realloc(buffer2,((buflen+i)*2)*sizeof(Char));
		    buflen=(buflen+i)*2;
		  }
		  Ustrcpy(buffer2+idx, ufn);
		  idx += i+1;
		  ld+=i+1;
                }
            } else {
                if (mask_check(mask,ufn)) {
		  if (*nr_files==fflen) {
		    found_files=realloc(found_files, (fflen*2)*sizeof(int));
		    fflen=fflen*2;
		  }
		  found_files[*nr_files] = idx;
		  (*nr_files)++;
		  if (idx+i+1>=buflen) {
		    buffer2=realloc(buffer2,((buflen+i+1)*2)*sizeof(Char));
		    buflen=(buflen+i+1)*2;
		  }
		  Ustrcpy(buffer2+idx, ufn);
		  idx += i+1;
		  lf+=i+1;
                }
            }
        }
    }
    closedir(dr);
    if (*nr_dirs) {
        Char *dres;
        *dirs = (Char**)malloc(sizeof(Char*)*(*nr_dirs));
        qsort(found_dirs, *nr_dirs, sizeof(int), qsortintstrcmp);
        dres = (Char *) malloc(ld*sizeof(Char));
        i = 0;
        while (i<*nr_dirs) {
            Ustrcpy(dres, buffer2+found_dirs[i]);
	    (*dirs)[i] = dres;
            dres += Ustrlen(dres)+1;
            i++;
        }
    } else {
      *dirs=0;
    }
    if (*nr_files) {
        Char *fres;
        *files = (Char**)malloc(sizeof(Char*)*(*nr_files));
        qsort(found_files, *nr_files, sizeof(int), qsortintstrcmp);
        fres = (Char*) malloc(lf*sizeof(Char));
        i=0;
        while (i<*nr_files) {
            Ustrcpy(fres, buffer2+found_files[i]);
	    (*files)[i]=fres;
            fres += Ustrlen(fres)+1;
            i++;
        }
    } else {
      *files=0;
    }
    free(buffer2);
    free(found_files);
    free(found_dirs);
    return *nr_files+ *nr_dirs;
}

static void expand_user(Char *name, Char *result)
{
    struct passwd *pinfo=NULL;

    if (!name || !result) return;
    setpwent();
    if (*name)
        pinfo = getpwnam((char*)UstrtoLocale(name));
    else
        pinfo = getpwuid(user_id);
    endpwent();
    if (pinfo)
        Ustrcpy(result, FilenametoUstr(pinfo->pw_dir));
    else if (*name) {
	char *c=getenv((char*)UstrtoLocale(name));
	if (c) Ustrcpy(result, FilenametoUstr(c));
    }
}

void get_currentwd(void)
{
    char *p;
    char buf[2048];
    struct stat fs1,fs2;

    p=getenv("PWD");
    if (!p || stat(p,&fs1) ||
        (!stat(".",&fs2) && fs1.st_ino != fs2.st_ino)) {
        p=NULL;
        if (getcwd((char *)buf, 2048))
            p=buf;
    }
    user_id=getuid();
    group_id=getgid();
    currentwdlen=strlen(p);
    currentwd = (char*) malloc((currentwdlen+1)*sizeof(char));
    strcpy(currentwd,p);
}

static Char *expand_environment(Char *name)
{
    int i=0,j=0,expanded=0;
    Char buffer[2000];
    if (!name) return NULL;
    do {
        if (name[j]=='$' && Uisalpha(name[j+1])) {
            int k=j+1;
            Char *en;
            Char c;
            j++;
            expanded=1;
            while (Uisalnum(name[j]) || name[j]=='_') j++;
            c=name[j];
            name[j]='\0';
            en = LocaletoUstr(getenv((char*)UstrtoLocale(name+k)));
            name[j]=c;
            if (en) while (*en) buffer[i++]=*en++;
        }
        buffer[i++]=name[j];
    } while (name[j++]);
    if (expanded) {
        if (j<i) {
            free(name);
            name = concat(buffer,NULL);
        } else
            Ustrcpy(name, buffer);
    }
    return name;
}
            
static int pointstate[7] = { 0, 2, 3, 0, 5, 5, 0 };
static int tildestate[7] = { 0, 4, 0, 0, 5, 5, 4 };
static int defaultstate[7] = {0, 0, 0, 0, 5, 5, 0 };

Char *standard_dir(Char *dirname)
{
    int i = 0, state = 6, j = 0;
    Bool ok;
    Char buffer[2000];
    Char *c = expand_environment(dirname);

    if (!c) return NULL;
    dirname=c;
    /* meaning of state:
    ** 1:  ending in /
    ** 2:  ending in /.
    ** 3:  ending in /..
    ** 4:  ending in /~
    ** 5:  ending in /~[^/]*
    ** 0:  everything else.
    */
    while (*c) {
        switch (*c) {
        case '.':
            state = pointstate[state];
            buffer[i++]='.';
            break;
        case DIRSEPCHAR:
            switch (state) {
            case 1:
                break;
            case 2:
                i--;
                state = 1;
                break;
            case 3:
                buffer[i]='/';
                i -= 2;
                ok = (i>=2 && buffer[i-2]!='.') ||
                     (i>=3 && buffer[i-3]!='.') ||
                     (i>=4 && buffer[i-4]!='/');
                if (ok) {
                    i-=2;
                    while (i>0 && buffer[i]!='/') i--;
                    if (buffer[i]=='/') i++;
                } else
                    i+=3;
                state = i>0;
                break;
            case 4:
            case 5:
                buffer[i]='\0';
                while (i>0 && buffer[i]!='~') i--;
                if (buffer[i]=='~') {
                    i++;
                    expand_user(buffer+i,buffer);
                    i = Ustrlen(buffer);
                    if (!i || buffer[i-1]!='/') buffer[i++]='/';
                } else {
                    i = Ustrlen(buffer);
                    buffer[i++]='/';
                }
                state = 1;
                break;
            default:
                buffer[i++]='/';
                state = 1;
                break;
            }
            break;
        case '~':
            state = tildestate[state];
            buffer[i++]='~';
            break;
        default:
            state = defaultstate[state];
            buffer[i++]= *c;
            break;
        }
        c++;
        j++;
    }
    switch (state) {
    case 2:
        i--;
        break;
    case 3:
        i -= 2;
        ok = (i>=2 && buffer[i-2]!='.') ||
            (i>=3 && buffer[i-3]!='.') ||
            (i>=4 && buffer[i-4]!='/');
        if (ok) {
            i-=2;
            while (i>0 && buffer[i]!='/') i--;
            if (buffer[i]=='/') i++;
        } else
            i+=2;
        break;
    case 4:
    case 5:
        buffer[i]='\0';
        while (i>0 && buffer[i]!='~') i--;
        if (buffer[i]=='~') {
            i++;
            expand_user(buffer+i,buffer);
            i = Ustrlen(buffer);
            if (!i || buffer[i-1]!='/') buffer[i++]='/';
        } else {
            i = Ustrlen(buffer);
            buffer[i++]='/';
        }
        break;
    default:
        break;
    }
    if (!i) {
        Ustrcpy(buffer, FilenametoUstr(currentwd));
        i=currentwdlen;
    }   
    buffer[i]='\0';
    if (j<i) {
        free(dirname);
        dirname = concat(buffer,NULL);
    } else
        Ustrcpy(dirname, buffer);
    return dirname;
}

void remove_file(Char *name)
{
    unlink(UstrtoFilename(name));
}


int skip_fontpart(FILE *f)
{
    
#define get_new_string (not_finished = ((fgets(buffer, 2000, f)!=NULL) && \
					!begins_with("STOP", buffer)))

    int nr;
    Bool not_finished;
    char *ind;
    char buffer[2000];

    nr=0;
    get_new_string;
    while (not_finished && nr<MAXFONTS) {
	if ((aig(ind = begins_with("FONT:", buffer))) ||
	    (aig(ind = begins_with("FONTGROUP:", buffer))))
	    nr++;
	else
	    if (aig(ind = begins_with("BUTTONFONT:", buffer)))
		nr++;
	get_new_string;
    }
    if (begins_with("STOP", buffer))
	return nr;
    else return 0;
}
