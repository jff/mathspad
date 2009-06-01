TOPSRCDIR = ..
TOPOBJ    = .
SRCDIR    = .



SUBDIRS = include util markup unicode keyboard language templates \
	output module documents windows treeedit tools

all: build

build:
	for dir in $(SUBDIRS); do (cd $$dir; $(MAKE) -k); done

install:
	for dir in $(SUBDIRS); do (cd $$dir; $(MAKE) -k install); done

clean:
	for dir in $(SUBDIRS); do (cd $$dir; $(MAKE) clean); done

depend:
	(cd include; $(MAKE) -k)
	for dir in $(SUBDIRS); do (cd $$dir; $(MAKE) depend); done
