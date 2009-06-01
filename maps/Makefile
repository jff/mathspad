
TOPSRCDIR = ..
TOPOBJ    = .
SRCDIR    = .



EBCDICMAP = EBCDIC-AT-DE-A.map EBCDIC-AT-DE.map EBCDIC-CA-FR.map \
		EBCDIC-DK-NO-A.map EBCDIC-DK-NO.map EBCDIC-ES-A.map \
		EBCDIC-ES-S.map EBCDIC-ES.map EBCDIC-FI-SE-A.map \
		EBCDIC-FI-SE.map EBCDIC-FR.map EBCDIC-IS-FRISS.map \
		EBCDIC-IT.map EBCDIC-PT.map EBCDIC-UK.map EBCDIC-US.map

CODEPAGEMAP = CP1250.map CP1251.map CP1252.map CP1253.map CP1254.map \
		CP1255.map CP1256.map CP1257.map

TEXMAP = CMMathEncoding.map CMRomanEncoding.map CMSYEncoding.map \
		CMSmallCapsEncoding.map CMTEXEncoding.map MSAMEncoding.map \
		MSBMEncoding.map WNCYREncoding.map

IBMMAP = IBM037.map IBM038.map IBM1004.map IBM1026.map IBM1047.map \
		IBM256.map IBM273.map IBM274.map IBM275.map IBM277.map \
		IBM278.map IBM280.map IBM281.map IBM284.map IBM285.map \
		IBM290.map IBM297.map IBM420.map IBM423.map IBM424.map \
		IBM437.map IBM500.map IBM850.map IBM851.map IBM852.map \
		IBM855.map IBM857.map IBM860.map IBM861.map IBM862.map \
		IBM863.map IBM864.map IBM865.map IBM868.map IBM869.map \
		IBM870.map IBM871.map IBM875.map IBM880.map IBM891.map \
		IBM903.map IBM904.map IBM905.map IBM918.map

ISOMAP = ISO-IR-90.map ISO_10367-BOX.map ISO_2033-1983.map ISO_5426-1980.map \
		ISO_5427.map ISO_5427-1981.map ISO_5428-1980.map \
		ISO_646.BASIC-1983.map ISO_646.IRV-1983.map \
		ISO_6937-2-25.map ISO_6937-2-ADD.map ISO_6937-1992.map \
		ISO_8859-1-1987.map ISO_8859-2-1987.map ISO_8859-3-1988.map \
		ISO_8859-4-1988.map ISO_8859-5-1988.map ISO_8859-6-1987.map \
		ISO_8859-7-1987.map ISO_8859-8-1988.map ISO_8859-9-1989.map \
		ISO_8859-10-1993.map ISO_8859-SUPP.map

JAPANESEMAP = JIS0201.map JIS0208-shift.map JIS0208.map JIS_C6220-1969-JP.map \
		JIS_C6220-1969-RO.map JIS_C6226-1978.map JIS_C6226-1983.map \
		JIS_C6229-1984-A.map JIS_C6229-1984-B-ADD.map \
		JIS_C6229-1984-B.map JIS_C6229-1984-HAND-ADD.map \
		JIS_C6229-1984-HAND.map JIS_C6229-1984-KANA.map \
		JIS_X0201.map JIS_X0212-1990.map SHIFT-JIS.map \
		JAPANESE-EUC.map

KOREANMAP = KOREAN-EUC.map KSC5601-1.1-old.map KSC5601-2.0.map KSC5636.map \
		KS_C_5601-1987.map KS_C_5601-1992.map

CHINESEMAP = SHIFT-GB.map GB_1988-80.map GB_2312-80.map Big5.map \
		CHINESE-EUC.map

POSTSCRIPTMAP = StandardEncoding.map SymbolEncoding.map \
		ZapfDingbats.map

CYRILLICMAP = INIS-CYRILLIC.map ECMA-CYRILLIC.map KOI-8R.map 

GREEKMAP = GREEK-CCITT.map GREEK7-OLD.map GREEK7.map LATIN-GREEK-1.map \
		LATIN-GREEK.map

MATHPADMAP = MathSymbolEncoding.map


MISCMAP = ANSI_X3.110-1983.map ANSI_X3.4-1968.map ASMO_449.map BALTIC.map \
		BS_4730.map BS_VIEWDATA.map CSA_Z243.4-1985-1.map \
		CSA_Z243.4-1985-2.map CSA_Z243.4-1985-GR.map \
		CSN_369103.map CWI.map DEC-MCS.map DIN_66003.map \
		DS_2089.map ES.map ES2.map GOST_19768-74.map \
		HP-ROMAN8.map IEC_P27-1.map INIS-8.map \
		INIS.map INVARIANT.map IT.map \
		JUS_I.B1.002.map JUS_I.B1.003-MAC.map JUS_I.B1.003-SERB.map \
		MAC-IS.map MACINTOSH.map MSZ_7795.3.map NATS-DANO-ADD.map \
		NATS-DANO.map NATS-SEFI-ADD.map NATS-SEFI.map \
		NC_NC00-10-81.map NEXTSTEP.map NF_Z_62-010.map \
		NF_Z_62-010_1973.map NS_4551-1.map NS_4551-2.map \
		PT.map PT2.map SAMI.map SEN_850200_B.map SEN_850200_C.map \
		T.101-G2.map T.61-7BIT.map T.61-8BIT.map VIDEOTEX-SUPPL.map


DATAFILES = $(TEXMAP) $(CODEPAGEMAP) $(EBCDICMAP) $(IBMMAP) \
		$(ISOMAP) $(JAPANESEMAP) $(KOREANMAP) $(CHINESEMAP) \
		$(POSTSCRIPTMAP) $(CYRILLICMAP) $(GREEKMAP) \
		$(MATHPADMAP) $(MISCMAP) map.alias

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


all: 

