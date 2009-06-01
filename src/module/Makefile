
TOPSRCDIR = ../..
TOPOBJ    = .
SRCDIR    = .


SOURCE = SEJ.c hugs.c hugsparse.c ispell.c maple.c parser.c parsertemp.c \
	pvs.c MAG.c pvsparse.c

LIBRARIES = libpvs.la libMAG.la

# Global rules shared by all makefiles     -*-Makefile-*-
#
# (Adjusted from the Wine distribution)
#
# Each individual makefile should define the following variables:
# TOPSRCDIR    : top-level source directory
# TOPOBJDIR    : top-level object directory
# SRCDIR       : source directory for this module
# SOURCE       : C sources for the module (optional)
# SOURCEXX     : C++ sources for the module (optional)
# PROGRAMS     : programs in this directory (optional)
# First some useful definitions

alltarget: all

SHELL     = /bin/sh
CC        = gcc
CXX       = g++
CPP       = gcc -E
CFLAGS    = -g -O2
CXXFLAGS  = -g -O2
X_CFLAGS  = 
X_LIBS    = 

XLIB      = $(X_LIBS)  -lSM -lICE -lX11 
LIBS      = -ldl 
YACC      = bison -y
LEX       = flex
LEXLIB    = -lfl
top_builddir = $(TOPSRCDIR)
LIBTOOL   = $(SHELL) $(top_builddir)/libtool
RANLIB    = ranlib
MAKEDEP   = :
LN_S      = ln -s
INCLUDES   = -I$(TOPSRCDIR)/include -I$(TOPOBJDIR)/include -I$(SRCDIR) -I. $(X_CFLAGS)
ALLCFLAGS = $(CFLAGS) $(DEFS) $(INCLUDES) $(X_CFLAGS)
ALLCXXFLAGS = $(CXXFLAGS) $(DEFS) $(INCLUDES) $(X_CFLAGS)
SUBMAKE   = $(MAKE) 'CC=$(CC)' 'CFLAGS=$(CFLAGS)' 'CXXFLAGS=$(CXXFLAGS)'



COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(X_CFLAGS)
LTCOMPILE = $(LIBTOOL) --mode=compile $(COMPILE)
LINK = $(LIBTOOL) --mode=link $(CC) $(CFLAGS) $(LDFLAGS)

CXXFLAGS = -g -O2
COMPILEXX = $(CXX) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CXXFLAGS) $(X_CFLAGS)
LTCOMPILEXX = $(LIBTOOL) --mode=compile $(COMPILEXX)
LINKXX = $(LIBTOOL) --mode=link $(CXX) $(CXXFLAGS) $(LDFLAGS)


# Installation infos
# Fix to convert ./install-sh into $(TOPSRCDIR)/install-sh
FIXINSTALL        = /bin/install -c
INSTALL           = $(FIXINSTALL:./install-sh=$(TOPSRCDIR)/install-sh)

INSTALL_PROGRAM   = ${INSTALL}
INSTALL_DATA      = ${INSTALL} -m 644
LTINSTALL         = $(LIBTOOL) --mode=install $(INSTALL)
LTINSTALL_PROGRAM = $(LIBTOOL) --mode=install $(INSTALL_PROGRAM)
LTINSTALL_DATA    = $(LIBTOOL) --mode=install $(INSTALL_DATA)


prefix          = /home/jff/code/mathpad-0.80-jff/build
exec_prefix     = ${prefix}
bindir          = ${exec_prefix}/bin
libdir          = ${exec_prefix}/lib/MathSpad
datadir         = ${prefix}/share/MathSpad
infodir         = ${prefix}/share/info
mandir          = ${prefix}/share/man/man1
manext          = .1
includedir      = ${prefix}/include/mathpad

OBJECTS = $(SOURCE:.c=.o) $(SOURCEXX:.cc=.o)

LTOBJECTS = $(SOURCE:.c=.lo) $(SOURCEXX:.cc=.lo)

MISCOBJECTS = $(SOURCEMISC:.c=.o) $(SOURCEXXMISC:.cc=.o)
LTMISCOBJECTS = $(SOURCEMISC:.c=.lo) $(SOURCEXXMISC:.cc=.lo)

PROGOBJECTS = $(SOURCEPROG:.c=.o) $(SOURCEXXPROG:.cc=.o)
LTPROGOBJECTS = $(SOURCEPROG:.c=.lo) $(SOURCEXXPROG:.cc=.lo)

# PROGRAMS = $(PROGRAMS) $(SOURCEPROG:.c=) $(SOURCEXXPROG:.cc=)



# Implicit rules

.SUFFIXES: .c .cc .C .lo .o
.c.o:
	$(COMPILE) -c $<
.c.lo:
	$(LTCOMPILE) -c $<
.cc.o:
	$(COMPILEXX) -c $<
.cc.lo:
	$(LTCOMPILEXX) -c $<
.C.o:
	$(COMPILEXX) -c $<
.C.lo:
	$(LTCOMPILEXX) -c $<

# Rule for main module

depend:: $(SOURCE) $(SOURCEXX) $(SOURCEPROG) $(SOURCEXXPROG) $(SOURCEMISC)
	$(MAKEDEP) $(INCLUDES) $(SOURCE) $(SOURCEXX) $(SOURCEPROG) $(SOURCEXXPROG) $(SOURCEMISC)

clean::
	$(RM) $(OBJECTS) $(LTOBJECTS) $(PROGOBJECTS) $(LTPROGOBJECT) \
		$(MISCOBJECTS) $(LTMISCOBJECTS) *.la *.a *.o *.lo \#*\# \
		*~ *% .#* *.bak *.orig *.rej *.flc *-tmp.c y.tab.c \
		y.tab.h lex.yy.c core $(PROGRAMS)
	$(RM) -r .libs

install:: $(LIBRARIES) $(PROGRAMS) $(DATAFILES)
	if [ "$(LIBRARIES)" != "" ] ; then \
		mkdir -p $(libdir)$(libsubdir) ;\
		for library in $(LIBRARIES)"" ; do $(LTINSTALL_DATA) $(LIBRARYMODE) $$library $(libdir)$(libsubdir)/; done ;\
	fi
	if [ "$(PROGRAMS)" != "" ] ; then \
		mkdir -p $(bindir)$(binsubdir) ; \
		for program in $(PROGRAMS)"" ; do $(LTINSTALL_PROGRAM) $(PROGRAMMODE) $$program $(bindir)$(binsubdir)/; done ;\
	fi
	if [ "$(DATAFILES)" != "" ] ; then \
		mkdir -p $(datadir)$(datasubdir) ;\
		for datafile in $(DATAFILES)"" ; do $(INSTALL_DATA) $(DATAMODE) $$datafile $(datadir)$(datasubdir)/; done ;\
	fi

# End of global rules

all:  $(LIBRARIES)

libmodule.la: $(LTOBJECTS)
	$(LINK) -o libmodule.la -rpath $(libdir) $(LTOBJECTS)

libpvs.la: pvs.lo pvsparse.lo
	$(LINK) -o libpvs.la -rpath $(libdir) pvs.lo pvsparse.lo

libMAG.la: MAG.lo
	$(LINK) -o libMAG.la -rpath $(libdir) MAG.lo

pvsparse.c: pvsparse.y pvsparselex.c
	$(YACC) pvsparse.y
	$(CPP) $(INCLUDES)  y.tab.c | \
	 sed -e 's#\([^a-zA-Z_]\)yy#\1pvs_yy#g' -e 's#^yy#pvs_yy#g' > pvsparse.c

pvsparselex.c:  pvsparse.lex
	$(LEX) pvsparse.lex
	mv lex.yy.c pvsparselex.c
