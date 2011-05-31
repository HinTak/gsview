#  Copyright (C) 2000-2002, Ghostgum Software Pty Ltd.  All rights reserved.
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

prefix=
GSVIEW_BASE=$(prefix)/usr

# binaries placed here
GSVIEW_BINDIR=$(GSVIEW_BASE)/bin
# Man page for pstotext placed here
GSVIEW_MANDIR=$(GSVIEW_BASE)/man
# GSview help files and epstool Readme placed here
GSVIEW_DOCPATH=$(GSVIEW_BASE)/share/doc
# GSview printer.ini and system wide gsview.ini
GSVIEW_ETCPATH=$(prefix)/etc

MAKE=make
COMP=gcc
OBJ=o
INSTALL=install -m 644
INSTALL_EXE=install -m 755
#CDEBUG=
#LDEBUG=
CDEBUG=-g
LDEBUG=

# Linux
XINCLUDE=
PFLAGS=-DMULTITHREAD
PLINK=-lpthread -lrt

# SunOS 5.7
#XINCLUDE=-I/usr/openwin/share/lib
#PFLAGS=-DMULTITHREAD
#PLINK=-lpthread -lrt

# SGI Irix 6.2
# without MULTITHREAD
#XINCLUDE=-I/usr/freeware/include
#PFLAGS=
#PLINK=-L/usr/freeware/lib32 -rpath /usr/freeware/lib32 -L/usr/lib32

# SGI Irix 6.5
# mutlithreaded, but can't debug with gdb
#XINCLUDE=-I/usr/freeware/include
#PFLAGS=-DMULTITHREAD
#PLINK=-L/usr/freeware/lib32 -rpath /usr/freeware/lib32 -L/usr/lib32 -lpthread

# Other possible options are -Wtraditional
# Compiler flags for C and C++ files.
CFLAGS=-O -Wall -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes -fno-builtin -fno-common -Wcast-qual -Wwrite-strings $(CDEBUG) -DX11 -DUNIX -DNONAG $(RPM_OPT_FLAGS) `gtk-config --cflags` $(XINCLUDE) $(PFLAGS)

# Linker flags
LFLAGS=$(LDEBUG) $(PLINK) `gtk-config --libs`

OBJS=gvx.$(OBJ) gvxdlg.$(OBJ) gvxdisp.$(OBJ) gvxedit.$(OBJ) gvxeps.$(OBJ)\
   gvxgsver.$(OBJ) gvxinit.$(OBJ) gvxmeas.$(OBJ) gvxmisc.$(OBJ) gvxprn.$(OBJ)\
   gvccmd.$(OBJ) gvcdisp.$(OBJ) gvcedit.$(OBJ) gvceps.$(OBJ) gvcfile.$(OBJ) \
   gvcinit.$(OBJ) gvcbeta.$(OBJ) gvcmeas.$(OBJ) gvcmisc.$(OBJ) gvcprf.$(OBJ) \
   gvcprn.$(OBJ) gvctext.$(OBJ) gvxdll.$(OBJ) gvcdll.$(OBJ) gvcpdf.$(OBJ)\
   dscparse.$(OBJ) dscutil.$(OBJ)\
   gvcreg.$(OBJ) gvxreg.$(OBJ) gvxres.$(OBJ)\
   gvxl_de.$(OBJ) gvxl_en.$(OBJ) gvxl_es.$(OBJ) gvxl_fr.$(OBJ)\
   gvxl_gr.$(OBJ) gvxl_it.$(OBJ) gvxl_nl.$(OBJ) gvxl_se.$(OBJ)\
   cdll.$(OBJ) cimg.$(OBJ) cview.$(OBJ)
HDRS=gsvver.h gvx.h dscparse.h gvcfn.h gvcver.h gvxres.h gvcfile.h gvctype.h gvcedit.h

all: gsview html epstool pstotext

#.c.$(OBJ):
#	$(COMP) $(CFLAGS) -c $*.c

ECHOGSV=./echogsv

include gvcver.mak

GSVIEW_DOCDIR=$(GSVIEW_DOCPATH)/gsview-$(GSVIEW_DOT_VERSION)

install: all
	-mkdir -p $(GSVIEW_BASE)
	chmod 755 $(GSVIEW_BASE)
	-mkdir -p $(GSVIEW_BINDIR)
	chmod 755 $(GSVIEW_BINDIR)
	$(INSTALL_EXE) gsview $(GSVIEW_BINDIR)/gsview
	$(INSTALL_EXE) gvxhelp.txt $(GSVIEW_BINDIR)/gsview-help
	$(INSTALL_EXE) ../pstotext/pstotext $(GSVIEW_BINDIR)/pstotext
	$(INSTALL_EXE) ../epstool/epstool $(GSVIEW_BINDIR)/epstool
	-mkdir -p $(GSVIEW_MANDIR)
	chmod 755  $(GSVIEW_MANDIR)
	-mkdir -p $(GSVIEW_MANDIR)/man1
	chmod 755  $(GSVIEW_MANDIR)/man1
	$(INSTALL) ../pstotext/pstotext.1 $(GSVIEW_MANDIR)/man1/pstotext.1
	$(INSTALL) gsview.1 $(GSVIEW_MANDIR)/man1/gsview.1
	-mkdir -p $(GSVIEW_DOCPATH)
	chmod 755 $(GSVIEW_DOCPATH)
	-mkdir -p $(GSVIEW_DOCDIR)
	chmod 755  $(GSVIEW_DOCDIR)
	$(INSTALL) gsview.css $(GSVIEW_DOCDIR)/gsview.css
	$(INSTALL) cdorder.txt $(GSVIEW_DOCDIR)/cdorder.txt
	$(INSTALL) regorder.txt $(GSVIEW_DOCDIR)/regorder.txt
	$(INSTALL) Readme.htm  $(GSVIEW_DOCDIR)/Readme.htm
	$(INSTALL) LICENCE $(GSVIEW_DOCDIR)/LICENCE
	$(INSTALL) gvxde.htm  $(GSVIEW_DOCDIR)/gvxde.htm
	$(INSTALL) gvxen.htm  $(GSVIEW_DOCDIR)/gvxen.htm
	$(INSTALL) gvxes.htm  $(GSVIEW_DOCDIR)/gvxes.htm
	$(INSTALL) gvxfr.htm  $(GSVIEW_DOCDIR)/gvxfr.htm
	$(INSTALL) gvxgr.htm  $(GSVIEW_DOCDIR)/gvxgr.htm
	$(INSTALL) gvxit.htm  $(GSVIEW_DOCDIR)/gvxit.htm
	$(INSTALL) gvxnl.htm  $(GSVIEW_DOCDIR)/gvxnl.htm
	$(INSTALL) gvxse.htm  $(GSVIEW_DOCDIR)/gvxse.htm
	$(INSTALL) ../epstool/epstool.htm $(GSVIEW_DOCDIR)/epstool.htm
	-mkdir -p $(GSVIEW_ETCPATH)
	chmod 755  $(GSVIEW_ETCPATH)
	-mkdir -p $(GSVIEW_ETCPATH)/gsview
	chmod 755  $(GSVIEW_ETCPATH)/gsview
	$(INSTALL) printer.ini  $(GSVIEW_ETCPATH)/gsview/printer.ini

tar:
	rm -rf /var/tmp/gsview
	$(MAKE)
	$(MAKE) prefix=/var/tmp/gsview install
	cd /var/tmp/gsview; tar -cvf ~/gsview.tar *

./echogsv: echogsv.c
	$(COMP) -o echogsv echogsv.c

./codepage: codepage.c
	$(COMP) -o codepage codepage.c

epstool: ../epstool/epstool.c ../epstool/epstool.h gvceps.c gvceps.h dscparse.c
	cd ../epstool
	$(MAKE) -C ../epstool -f makefile.unx
	cd ../src

pstotext: ../pstotext/bundle.c ../pstotext/bundle.h ../pstotext/main.c \
  ../pstotext/mkbundle.c ../pstotext/mkrch.c \
  ../pstotext/ocr.ps ../pstotext/rot270.ps ../pstotext/rot90.ps
	cd ../pstotext
	$(MAKE) -C ../pstotext -f Makefile
	cd ../src

gvx.$(OBJ): gvx.c $(HDRS)
	$(COMP) $(CFLAGS) -c gvx.c

gvxdlg.$(OBJ): gvxdlg.c gvcrc.h $(HDRS)
	$(COMP) $(CFLAGS) -c gvxdlg.c

gvxdll.$(OBJ): gvxdll.c gvcrc.h $(HDRS)
	$(COMP) $(CFLAGS) -c gvxdll.c

gvxdisp.$(OBJ): gvxdisp.c  $(HDRS)
	$(COMP) $(CFLAGS) -c gvxdisp.c

gvxedit.$(OBJ): gvxedit.c $(HDRS)
	$(COMP) $(CFLAGS) -c gvxedit.c

gvxeps.$(OBJ): gvxeps.c gvceps.h $(HDRS)
	$(COMP) $(CFLAGS) -c gvxeps.c

gvxgsver.$(OBJ): gvxgsver.c $(HDRS) gvcrc.h
	$(COMP) $(CFLAGS) -c gvxgsver.c

gvxinit.$(OBJ): gvxinit.c $(HDRS) gvcrc.h
	$(COMP) $(CFLAGS) -c gvxinit.c

gvxl_de.$(OBJ): de/gvxlang.c $(HDRS) gvxlang.h gvxlangh.rc de/gvclang.h de/gvclang.rc  
	$(COMP) $(CFLAGS) -I. -c -o gvxl_de.$(OBJ) de/gvxlang.c

gvxl_en.$(OBJ): en/gvxlang.c $(HDRS) gvxlang.h gvxlangh.rc en/gvclang.h en/gvclang.rc  
	$(COMP) $(CFLAGS) -I. -c -o gvxl_en.$(OBJ) en/gvxlang.c

gvxl_es.$(OBJ): es/gvxlang.c $(HDRS) gvxlang.h gvxlangh.rc es/gvclang.h es/gvclang.rc  
	$(COMP) $(CFLAGS) -I. -c -o gvxl_es.$(OBJ) es/gvxlang.c

gvxl_fr.$(OBJ): fr/gvxlang.c $(HDRS) gvxlang.h gvxlangh.rc fr/gvclang.h fr/gvclang.rc  
	$(COMP) $(CFLAGS) -I. -c -o gvxl_fr.$(OBJ) fr/gvxlang.c

gvxl_gr.$(OBJ): gr/gvxlang.c $(HDRS) gvxlang.h gvxlangh.rc gr/gvclang.h gr/gvclang.rc ./codepage
	./codepage 1253_8859-7 gr/gvxlang.c gvxlang.c
	./codepage 1253_8859-7 gr/gvclang.h gvclang.h
	./codepage 1253_8859-7 gr/gvclang.rc gvclang.rc
	$(COMP) $(CFLAGS) -I. -c -o gvxl_gr.$(OBJ) gvxlang.c
	-rm gvxlang.c
	-rm gvclang.h
	-rm gvclang.rc

gvxl_it.$(OBJ): it/gvxlang.c $(HDRS) gvxlang.h gvxlangh.rc it/gvclang.h it/gvclang.rc  
	$(COMP) $(CFLAGS) -I. -c -o gvxl_it.$(OBJ) it/gvxlang.c

gvxl_nl.$(OBJ): nl/gvxlang.c $(HDRS) gvxlang.h gvxlangh.rc nl/gvclang.h nl/gvclang.rc  
	$(COMP) $(CFLAGS) -I. -c -o gvxl_nl.$(OBJ) nl/gvxlang.c

gvxl_se.$(OBJ): se/gvxlang.c $(HDRS) gvxlang.h gvxlangh.rc se/gvclang.h se/gvclang.rc  
	$(COMP) $(CFLAGS) -I. -c -o gvxl_se.$(OBJ) se/gvxlang.c

gvxmeas.$(OBJ): gvxmeas.c $(HDRS)
	$(COMP) $(CFLAGS) -c gvxmeas.c

gvxmisc.$(OBJ): gvxmisc.c $(HDRS)
	$(COMP) $(CFLAGS) -c gvxmisc.c

gvxprn.$(OBJ): gvxprn.c $(HDRS)
	$(COMP) $(CFLAGS) -c gvxprn.c

gvxreg.$(OBJ): gvxreg.c $(HDRS)
	$(COMP) $(CFLAGS) -c gvxreg.c

gvxres.$(OBJ): gvxres.c $(HDRS) gvxlang.h
	$(COMP) $(CFLAGS) -c gvxres.c

gvccmd.$(OBJ): gvccmd.c gvcrc.h $(HDRS)
	$(COMP) $(CFLAGS) -c gvccmd.c

gvcdisp.$(OBJ): gvcdisp.c $(HDRS)
	$(COMP) $(CFLAGS) -c gvcdisp.c

gvcdll.$(OBJ): gvcdll.c gvcrc.h $(HDRS)
	$(COMP) $(CFLAGS) -c gvcdll.c

dscparse.$(OBJ): dscparse.c dscparse.h
	$(COMP) $(CFLAGS) -c dscparse.c

dscutil.$(OBJ): dscutil.c dscparse.h
	$(COMP) $(CFLAGS) -c dscutil.c

gvcbeta.$(OBJ): gvcbeta.c gvcbeta.h $(HDRS)
	$(COMP) $(CFLAGS) -c gvcbeta.c

gvcedit.$(OBJ): gvcedit.c $(HDRS)
	$(COMP) $(CFLAGS) -c gvcedit.c

gvceps.$(OBJ): gvceps.c gvceps.h $(HDRS)
	$(COMP) $(CFLAGS) -c gvceps.c

gvcfile.$(OBJ): gvcfile.c $(HDRS)
	$(COMP) $(CFLAGS) -c gvcfile.c

gvcinit.$(OBJ): gvcinit.c $(HDRS) gvcrc.h
	$(COMP) $(CFLAGS) -c gvcinit.c

gvcmeas.$(OBJ): gvcmeas.c gvcrc.h $(HDRS)
	$(COMP) $(CFLAGS) -c gvcmeas.c

gvcmisc.$(OBJ): gvcmisc.c gvcrc.h $(HDRS)
	$(COMP) $(CFLAGS) -c gvcmisc.c

gvcpdf.$(OBJ): gvcpdf.c $(HDRS)
	$(COMP) $(CFLAGS) -c gvcpdf.c

gvcprn.$(OBJ): gvcprn.c $(HDRS)
	$(COMP) $(CFLAGS) -c gvcprn.c

gvcprf.$(OBJ): gvcprf.c $(HDRS)
	$(COMP) $(CFLAGS) -c gvcprf.c

gvcreg.$(OBJ): gvcreg.c $(HDRS)
	$(COMP) $(CFLAGS) -c gvcreg.c

gvctext.$(OBJ): gvctext.c $(HDRS)
	$(COMP) $(CFLAGS) -c gvctext.c

cdll.$(OBJ): cdll.c $(HDRS)
	$(COMP) $(CFLAGS) -c cdll.c

cimg.$(OBJ): cimg.c $(HDRS)
	$(COMP) $(CFLAGS) -c cimg.c

cview.$(OBJ): cview.c $(HDRS)
	$(COMP) $(CFLAGS) -c cview.c

gsview: $(OBJS)
	$(COMP) $(CFLAGS) -o gsview $(OBJS) $(LFLAGS)

gvdoc: gvdoc.c
	$(COMP) -o gvdoc gvdoc.c
	
doc2html: doc2html.c
	$(COMP) -o doc2html doc2html.c

html: gvxde.htm gvxen.htm gvxes.htm gvxfr.htm gvxgr.htm gvxit.htm gvxnl.htm gvxse.htm

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

gvxgr.htm: doc2html gvdoc gr/gvclang.txt
	./gvdoc X gr/gvclang.txt temp.txt
	./codepage 1253_8859-7 temp.txt gvx.txt
	./doc2html gvx.txt GSview.htm ISO-8859-7
	-rm temp.txt
	-rm gvxgr.htm
	mv GSview.htm gvxgr.htm

gvxit.htm: doc2html gvdoc it/gvclang.txt
	./gvdoc X it/gvclang.txt gvx.txt
	./doc2html gvx.txt GSview.htm
	-rm gvxit.htm
	mv GSview.htm gvxit.htm

gvxnl.htm: doc2html gvdoc nl/gvclang.txt
	./gvdoc X nl/gvclang.txt gvx.txt
	./doc2html gvx.txt GSview.htm
	-rm gvxnl.htm
	mv GSview.htm gvxnl.htm

gvxse.htm: doc2html gvdoc se/gvclang.txt
	./gvdoc X se/gvclang.txt gvx.txt
	./doc2html gvx.txt GSview.htm
	-rm gvxse.htm
	mv GSview.htm gvxse.htm

gvx.ps: gvx.dvi
	dvips gvx

gvx.dvi: gvx.tex titlepag.tex
	-latex gvx
	-latex gvx

gvx.tex: gvx.txt doc2tex
	doc2tex gvx.txt gvx.tex

doc2tex: doc2tex.c
	$(COMP) -o doc2tex doc2tex*.c

langen: gvxl_en.o gvxres.c
	$(COMP) $(CFLAGS) -c -DSTANDALONE gvxres.c
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
	-rm gvxl_gr.$(OBJ)
	-rm gvxl_it.$(OBJ)
	-rm gvxl_nl.$(OBJ)
	-rm gvxl_se.$(OBJ)
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
	-rm gvcedit.$(OBJ)
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
	-rm cdll.$(OBJ)
	-rm cimg.$(OBJ)
	-rm cview.$(OBJ)
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
	-rm codepage.$(OBJ)
	-rm codepage

veryclean: clean
	-rm gsview
	-rm gvxde.htm
	-rm gvxen.htm
	-rm gvxes.htm
	-rm gvxfr.htm
	-rm gvxgr.htm
	-rm gvxit.htm
	-rm gvxnl.htm
	-rm gvxse.htm
