#ifndef FILEFIND_H
#define FILEFIND_H
#include <stdio.h>
/*  Some requirements
**  * the interface to a filefind routine should be simple
**  * no fancy auto-create features (yet)
**  * never changing API interface (additions allowed)
**  * small API, (one include file, one library)
**  * general enough to be useful
**  * automatic compress and decompress where needed
**  * (later: support for sockets and protocols (http,ftp))
*/

/* hide any information contained in PathInfo */
typedef void *PathInfo;

/* create a structure which holds the information needed to find a file
** ENVVAR is the name of the environment variable (the search path)
** DEFPATH contains the default search path
** EXT contians the optional extension
**
** What it should do:
**     *  create a datastructure to be able to find files.
**     *  allow caching.
**     *  allow EXT.index and EXT.alias for index and alias mapping
**     *  allow recursive-decent on directory structures
**     *  allow rehash functions to rebuild the datastructure
**     *  allow optional .gz, .Z and .z extensions
**     *  store extra information for opened files
*/
extern PathInfo make_pathinfo(char *envvar, char *defpath, char *ext);


/* The following functions will also work if no correct PINFO is given.
** The current directory will be used for all access and default database
** for opened files.
*/

/* open a file with name NAME in mode MODE
** if MODE is not write, the file is searched for, otherwise
** the file is written according to the current directory.
** If the used filename (either the found one or NAME) has a
** compress extension (.z, .gz or .Z), the compressor or decompressor
** is automatically started (using popen).
** In write mode, an existing file will first be moved by changing the name
*/
extern FILE *open_file(PathInfo pinfo, char *name, char *mode);

/* close a file opened with open_file. */
extern void close_file(FILE *file);

/* make a backup of a file by adding something to its name */
extern void make_backup(char *name);

/* search for a file in a certain path */
extern char *search_file(PathInfo p, char *name, int *ftype);

/* set if zipped files are allowed */
extern void allow_zips(int yes);

/* determine the size of a file (-1 if size is unknown) */
extern int file_size(PathInfo p, char *name);

/* get the name of a file */
extern char *name_file(FILE *file);

/* rehash the information in the PINFO structure */
extern void rehash_pathinfo(PathInfo pinfo);
/* destroy the PINFO structure */
extern void destroy_pathinfo(PathInfo pinfo);

#endif
