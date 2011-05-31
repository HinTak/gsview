#  Copyright (C) 2000, Ghostgum Software Pty Ltd.  All rights reserved.
#  
# This file is part of GSview.
#  
# This program is distributed with NO WARRANTY OF ANY KIND.  No author
# or distributor accepts any responsibility for the consequences of using it,
# or for whether it serves any particular purpose or works at all, unless he
# or she says so in writing.  Refer to the GSview Free Public Licence 
# (the "Licence") for full details.
#  
# Every copy of GSview must include a copy of the Licence, normally in a 
# plain ASCII text file named LICENCE.  The Licence grants you the right 
# to copy, modify and redistribute GSview, but only under certain conditions 
# described in the Licence.  Among other things, the Licence requires that 
# the copyright notice and this notice be preserved on all copies.

# gvx.mak
# X11 GSview 
#

ifndef GSVIEW_BASE
GSVIEW_BASE=/usr/local
endif

# binaries placed here
GSVIEW_BINDIR=$(GSVIEW_BASE)/bin
# Man page for pstotext placed here
GSVIEW_MANDIR=$(GSVIEW_BASE)/man/man1
# GSview help files and epstool Readme placed here
GSVIEW_DOCPATH=$(GSVIEW_BASE)/doc
# GSview printer.ini and system wide gsview.ini
GSVIEW_ETCPATH=$(GSVIEW_BASE)/etc

COMP=gcc
OBJ=o
#CDEBUG=
#LDEBUG=
CDEBUG=-g
LDEBUG=

CFLAGS=-O -Wall -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes -Wtraditional -fno-builtin -fno-common -Wcast-qual -Wwrite-strings $(CDEBUG) -DX11 -DUNIX $(RPM_OPT_FLAGS) `gtk-config --cflags`
LFLAGS=$(LDEBUG) `gtk-config --libs`

OBJS=gvx.$(OBJ) gvxdlg.$(OBJ) gvxdisp.$(OBJ) gvxedit.$(OBJ) gvxeps.$(OBJ)\
   gvxgsver.$(OBJ) gvxinit.$(OBJ) gvxmeas.$(OBJ) gvxmisc.$(OBJ) gvxprn.$(OBJ)\
   gvccmd.$(OBJ) gvcdisp.$(OBJ) gvceps.$(OBJ) gvcinit.$(OBJ) gvcbeta.$(OBJ)\
   gvcmeas.$(OBJ) gvcmisc.$(OBJ) gvcprf.$(OBJ) gvcprn.$(OBJ) gvctext.$(OBJ)\
   gvxdll.$(OBJ) gvcdll.$(OBJ) gvcpdf.$(OBJ)\
   dscparse.$(OBJ) dscutil.$(OBJ)\
   gvcreg.$(OBJ) gvxreg.$(OBJ) gvxres.$(OBJ)\
   gvxl_de.$(OBJ) gvxl_en.$(OBJ) gvxl_es.$(OBJ) gvxl_fr.$(OBJ) gvxl_it.$(OBJ)
HDRS=gsvver.h gvx.h dscparse.h gvcfn.h gvcver.h gvxres.h

all: gsview html epstool pstotext

.cpp.$(OBJ):
	$(COMP) $(CFLAGS) -c $*.cpp

ECHOGSV=./echogsv

include gvcver.mak

GSVIEW_DOCDIR=$(GSVIEW_DOCPATH)/gsview-$(GSVIEW_DOT_VERSION)

install: all
	install -m 755 gsview $(GSVIEW_BINDIR)/gsview
	install -m 755 ../pstotext/pstotext $(GSVIEW_BINDIR)/pstotext
	install -m 755 ../epstool/epstool $(GSVIEW_BINDIR)/epstool
	-mkdir $(GSVIEW_MANDIR)
	chmod 755  $(GSVIEW_MANDIR)
	install -m 644 ../pstotext/pstotext.1 $(GSVIEW_MANDIR)/pstotext.1
	-mkdir $(GSVIEW_DOCDIR)
	chmod 755  $(GSVIEW_DOCDIR)
	install -m 644 Readme.htm  $(GSVIEW_DOCDIR)/Readme.htm
	install -m 644 LICENCE $(GSVIEW_DOCDIR)/LICENCE
	install -m 644 gvxde.htm  $(GSVIEW_DOCDIR)/gvxde.htm
	install -m 644 gvxen.htm  $(GSVIEW_DOCDIR)/gvxen.htm
	install -m 644 gvxes.htm  $(GSVIEW_DOCDIR)/gvxes.htm
	install -m 644 gvxfr.htm  $(GSVIEW_DOCDIR)/gvxfr.htm
	install -m 644 gvxit.htm  $(GSVIEW_DOCDIR)/gvxit.htm
	install -m 644 ../epstool/epstool.htm $(GSVIEW_DOCDIR)/epstool.htm
	-mkdir $(GSVIEW_DOCPATH)
	chmod 755  $(GSVIEW_ETCPATH)
	install -m 644 printer.ini  $(GSVIEW_ETCPATH)/gsview/printer.ini

TARNAME=../gsview$(GSVIEW_VERSION)b_linux.tar
tar: all
	-rm ./pstotext
	cp ../pstotext/pstotext ./pstotext
	cp ../pstotext/pstotext.1 ./pstotext.1
	cp ../epstool/epstool ./epstool
	cp ../epstool/epstool.htm ./epstool.htm
	tar -cvf $(TARNAME) gsview Readme.htm LICENCE gvxde.htm gvxen.htm gvxes.htm gvxfr.htm gvxit.htm printer.ini pstotext pstotext.1 epstool epstool.htm
	-rm $(TARNAME).gz
	gzip $(TARNAME)

echogsv: echogsv.c
	$(COMP) -o echogsv echogsv.c

epstool: ../epstool/epstool.cpp ../epstool/epstool.h gvceps.cpp gvceps.h dscparse.cpp
	cd ../epstool
	make -C ../epstool -f makefile.unx
	cd ../src

pstotext: ../pstotext/bundle.c ../pstotext/bundle.h ../pstotext/main.c \
  ../pstotext/mkbundle.c ../pstotext/mkrch.c \
  ../pstotext/ocr.ps ../pstotext/rot270.ps ../pstotext/rot90.ps
	cd ../pstotext
	make -C ../pstotext -f Makefile
	cd ../src

gvx.$(OBJ): gvx.cpp $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvxdlg.$(OBJ): gvxdlg.cpp gvcrc.h $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvxdll.$(OBJ): gvxdll.cpp gvcrc.h gsdll.h $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvxdisp.$(OBJ): gvxdisp.cpp  $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvxedit.$(OBJ): gvxedit.cpp $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvxeps.$(OBJ): gvxeps.cpp gvceps.h $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvxgsver.$(OBJ): gvxgsver.cpp $(HDRS) gvcrc.h
	$(COMP) $(CFLAGS) -c $*.cpp

gvxinit.$(OBJ): gvxinit.cpp $(HDRS) gvcrc.h
	$(COMP) $(CFLAGS) -c $*.cpp

gvxl_de.$(OBJ): de/gvxlang.cpp $(HDRS) gvxlang.h gvxlangh.rc de/gvclang.h de/gvclang.rc  
	$(COMP) $(CFLAGS) -I. -c -o gvxl_de.$(OBJ) de/gvxlang.cpp

gvxl_en.$(OBJ): en/gvxlang.cpp $(HDRS) gvxlang.h gvxlangh.rc en/gvclang.h en/gvclang.rc  
	$(COMP) $(CFLAGS) -I. -c -o gvxl_en.$(OBJ) en/gvxlang.cpp

gvxl_es.$(OBJ): es/gvxlang.cpp $(HDRS) gvxlang.h gvxlangh.rc es/gvclang.h es/gvclang.rc  
	$(COMP) $(CFLAGS) -I. -c -o gvxl_es.$(OBJ) es/gvxlang.cpp

gvxl_fr.$(OBJ): fr/gvxlang.cpp $(HDRS) gvxlang.h gvxlangh.rc fr/gvclang.h fr/gvclang.rc  
	$(COMP) $(CFLAGS) -I. -c -o gvxl_fr.$(OBJ) fr/gvxlang.cpp

gvxl_it.$(OBJ): it/gvxlang.cpp $(HDRS) gvxlang.h gvxlangh.rc it/gvclang.h it/gvclang.rc  
	$(COMP) $(CFLAGS) -I. -c -o gvxl_it.$(OBJ) it/gvxlang.cpp

gvxmeas.$(OBJ): gvxmeas.cpp $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvxmisc.$(OBJ): gvxmisc.cpp $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvxprn.$(OBJ): gvxprn.cpp $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvxreg.$(OBJ): gvxreg.cpp $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvxres.$(OBJ): gvxres.cpp $(HDRS) gvxlang.h
	$(COMP) $(CFLAGS) -c $*.cpp

gvccmd.$(OBJ): gvccmd.cpp gvcrc.h $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvcdisp.$(OBJ): gvcdisp.cpp $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvcdll.$(OBJ): gvcdll.cpp gvcrc.h gsdll.h $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

dscparse.$(OBJ): dscparse.cpp dscparse.h
	$(COMP) $(CFLAGS) -c $*.cpp

dscutil.$(OBJ): dscutil.cpp dscparse.h
	$(COMP) $(CFLAGS) -c $*.cpp

gvcbeta.obj: gvcbeta.cpp gvcbeta.h $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvceps.$(OBJ): gvceps.cpp gvceps.h $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvcinit.$(OBJ): gvcinit.cpp $(HDRS) gvcrc.h
	$(COMP) $(CFLAGS) -c $*.cpp

gvcmeas.$(OBJ): gvcmeas.cpp gvcrc.h $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvcmisc.$(OBJ): gvcmisc.cpp gvcrc.h $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvcpdf.$(OBJ): gvcpdf.cpp $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvcprn.$(OBJ): gvcprn.cpp $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvcprf.$(OBJ): gvcprf.cpp $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvcreg.$(OBJ): gvcreg.cpp $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gvctext.$(OBJ): gvctext.cpp $(HDRS)
	$(COMP) $(CFLAGS) -c $*.cpp

gsview: $(OBJS)
	$(COMP) $(CFLAGS) -o gsview $(OBJS) $(LFLAGS)

gvdoc: gvdoc.cpp
	$(COMP) -o gvdoc gvdoc.cpp
	
doc2html: doc2html.cpp
	$(COMP) -o doc2html doc2html.cpp

html: gvxde.htm gvxen.htm gvxes.htm gvxfr.htm gvxit.htm

gvxde.htm: doc2html gvdoc de/gvclang.txt
	./gvdoc X de/gvclang.txt gvx.txt
	./doc2html gvx.txt GSview.htm
	-rm gvxde.htm
	mv GSview.htm gvxde.htm

gvxen.htm: doc2html gvdoc en/gvclang.txt
	./gvdoc X en/gvclang.txt gvx.txt
	./doc2html gvx.txt GSview.htm
	-rm gvxen.htm
	mv GSview.htm gvxen.htm

gvxes.htm: doc2html gvdoc es/gvclang.txt
	./gvdoc X es/gvclang.txt gvx.txt
	./doc2html gvx.txt GSview.htm
	-rm gvxes.htm
	mv GSview.htm gvxes.htm

gvxfr.htm: doc2html gvdoc fr/gvclang.txt
	./gvdoc X fr/gvclang.txt gvx.txt
	./doc2html gvx.txt GSview.htm
	-rm gvxfr.htm
	mv GSview.htm gvxfr.htm

gvxit.htm: doc2html gvdoc it/gvclang.txt
	./gvdoc X it/gvclang.txt gvx.txt
	./doc2html gvx.txt GSview.htm
	-rm gvxit.htm
	mv GSview.htm gvxit.htm

gvx.ps: gvx.dvi
	dvips gvx

gvx.dvi: gvx.tex titlepag.tex
	-latex gvx
	-latex gvx

gvx.tex: gvx.txt doc2tex
	doc2tex gvx.txt gvx.tex

doc2tex: doc2tex.cpp
	$(COMP) -o doc2tex doc2tex*.cpp

langen: gvxl_en.o gvxres.cpp
	$(COMP) $(CFLAGS) -c -DSTANDALONE gvxres.cpp
	$(COMP) -o langen gvxres.o gvxl_en.o
	-rm gvxres.o


language:
	-rm gvclang.h
	-rm gvclang.rc
	-rm gvc.txt
	-rm *.res

clean: language
	-rm gsvver.h
	-rm gvx.$(OBJ)
	-rm gvxdisp.$(OBJ)
	-rm gvxdlg.$(OBJ)
	-rm gvxdll.$(OBJ)
	-rm gvxedit.$(OBJ)
	-rm gvxeps.$(OBJ)
	-rm gvxgsver.$(OBJ)
	-rm gvxinit.$(OBJ)
	-rm gvxl_de.$(OBJ)
	-rm gvxl_en.$(OBJ)
	-rm gvxl_es.$(OBJ)
	-rm gvxl_fr.$(OBJ)
	-rm gvxl_it.$(OBJ)
	-rm gvxmeas.$(OBJ)
	-rm gvxmisc.$(OBJ)
	-rm gvxprn.$(OBJ)
	-rm gvxreg.$(OBJ)
	-rm gvxres.$(OBJ)
	-rm gvcbeta.$(OBJ)
	-rm gvccmd.$(OBJ)
	-rm dscparse.$(OBJ)
	-rm dscutil.$(OBJ)
	-rm gvcdll.$(OBJ)
	-rm gvcdisp.$(OBJ)
	-rm gvceps.$(OBJ)
	-rm gvcfile.$(OBJ)
	-rm gvcinit.$(OBJ)
	-rm gvcmeas.$(OBJ)
	-rm gvcmisc.$(OBJ)
	-rm gvcpdf.$(OBJ)
	-rm gvcprf.$(OBJ)
	-rm gvcprn.$(OBJ)
	-rm gvcreg.$(OBJ)
	-rm gvctext.$(OBJ)
	-rm doc2ipf.$(OBJ)
	-rm doc2ipf
	-rm doc2html.$(OBJ)
	-rm doc2html
	-rm doc2tex.$(OBJ)
	-rm doc2tex
	-rm gvdoc.$(OBJ)
	-rm gvdoc
	-rm gvx.txt
	-rm gvx.aux
	-rm gvx.dvi
	-rm gvx.log
	-rm gvx.toc
	-rm gsview.txt
	-rm echogsv.$(OBJ)
	-rm echogsv

veryclean: clean
	-rm gsview
	-rm gvxde.htm
	-rm gvxen.htm
	-rm gvxes.htm
	-rm gvxfr.htm
	-rm gvxit.htm
