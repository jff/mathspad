#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include "filefind.h"

typedef struct Library Library;
struct Library {
    Library *next;
    char *name;
    void *handle;
};

static Library *liblist=NULL;
static PathInfo libpath = 0;

int load_library(char *libname)
{
    Library *lp=liblist;
    void (*initfunc)();
    char *foundname;
    int ftype;
    if (!libpath) libpath = make_pathinfo("LIBPATH", DEFAULTLIBPATH, ".so");
    foundname = search_file(libpath, libname, &ftype);
    if (!foundname) foundname=strcpy(libname,"");
    while (lp && strcmp(foundname, lp->name)) lp=lp->next;
    if (lp) { free(foundname); return 0; }
    lp=(Library*)malloc(sizeof(Library));
    lp->handle=dlopen(foundname, RTLD_LAZY);
    if (!lp->handle) {
	fprintf(stderr, "%s\n", dlerror());
	return 0;
    }
    initfunc = (void (*)()) dlsym(lp->handle, "init_library");
    if (initfunc) {
	(*initfunc)();
    } else {
	fprintf(stderr, "Library %s does not contain an init function\n",
		foundname);
	/*	dlclose(lp->handle);
	return 0; */
    }
    lp->name=(char*) malloc(sizeof(char)*(strlen(foundname)+1));
    strcpy(lp->name, foundname);
    lp->next=liblist;
    liblist=lp;
    return 1;
}
