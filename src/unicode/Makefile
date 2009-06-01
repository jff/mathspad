
TOPSRCDIR = ../..
TOPOBJ    = .
SRCDIR    = .


SOURCE = uniconv.c unifont.c unimap.c unistring.c unitype.c 

LIBRARIES = libunicode.la
TESTPROGS = brows convert datatotable decomp fonttest printdecomp \
		typetest unicode1to2 virtread

PROGRAMS = strasciitomap maptoascii asciitomap

DATAFILES = Baghdad.map CanComClass.map DeCompTag.map Decomposition.map \
	GenClass.map ToDenum.map ToFinal.map ToHexValue.map ToInitial.map \
	ToIsolated.map ToLower.map ToMedial.map ToMirror.map ToNoAccent.map \
	ToNum.map ToSpacing.map ToTitle.map ToUpper.map Unicode-2.0.map \
	X11keysym.map latexmath.map latextext.map mptouchar.map unicode1to2.map \
	helvetica.ufont UniConvert.conv

datasubdir = /map

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

all: $(LIBRARIES) $(PROGRAMS)

libunicode.la: $(LTOBJECTS)
	$(LINK) -o libunicode.la -rpath $(libdir) $(LTOBJECTS) $(XLIB)

asciitomap: libunicode.la asciitomap.o
	$(LINK) -o asciitomap asciitomap.o libunicode.la ../util/libutil.la

strasciitomap: libunicode.la strasciitomap.o
	$(LINK) -o strasciitomap strasciitomap.o libunicode.la ../util/libutil.la

maptoascii: libunicode.la maptoascii.o
	$(LINK) -o maptoascii maptoascii.o libunicode.la ../util/libutil.la

brows: libunicode.la brows.o drawstring.o
	$(LINK) -o brows brows.o drawstring.o libunicode.la ../markup/libmarkup.la ../util/libutil.la $(XLIB)

convert: libunicode.la convert.o
	$(LINK) -o convert convert.o libunicode.la ../util/libutil.la

datatotable: libunicode.la datatotable.o
	$(LINK) -o datatotable datatotable.o libunicode.la ../util/libutil.la

decomp: libunicode.la decomp.o
	$(LINK) -o decomp decomp.o libunicode.la ../util/libutil.la

fonttest: libunicode.la fonttest.o
	$(LINK) -o fonttest fonttest.o libunicode.la ../util/libutil.la $(XLIB)

printdecomp: libunicode.la printdecomp.o
	$(LINK) -o printdecomp printdecomp.o libunicode.la ../util/libutil.la

typetest: libunicode.la typetest.o
	$(LINK) -o typetest typetest.o libunicode.la ../util/libutil.la

unicode1to2: libunicode.la unicode1to2.o
	$(LINK) -o unicode1to2 unicode1to2.o libunicode.la ../util/libutil.la

virtread: libunicode.la virtread.o
	$(LINK) -o virtread virtread.o libunicode.la ../util/libutil.la
