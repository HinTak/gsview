#  Copyright (C) 2000-2004, Ghostgum Software Pty Ltd.  All rights reserved.
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

# unx.mak
# Unix/gtk+/X11 GSview 
#
# WARNING: Doesn't install pstotext

prefix=
GSVIEW_ROOT=/usr/local
GSVIEW_BASE=$(prefix)$(GSVIEW_ROOT)


BINDIR=./bin
OBJDIR=./obj
SRCDIR=./src
SRCUNXDIR=./srcunx

# binaries placed here
GSVIEW_BINDIR=$(GSVIEW_BASE)/bin
# Man page for pstotext placed here
GSVIEW_MANDIR=$(GSVIEW_BASE)/man
# GSview help files placed here
GSVIEW_DOCPATH=$(GSVIEW_BASE)/share/doc
# GSview printer.ini and system wide gsview.ini
GSVIEW_ETCPATH=$(prefix)/etc

MAKE=make
CC=gcc
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

# Solaris 8
#XINCLUDE=-I/usr/openwin/share/lib
#PFLAGS=-DMULTITHREAD
#PLINK=-lpthread -lposix4 -lrt

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
CFLAGS=-O -Wall -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes -fno-builtin -fno-common -Wcast-qual -Wwrite-strings $(CDEBUG) -DX11 -DUNIX -DNONAG $(RPM_OPT_FLAGS) `pkg-config gtk+-2.0 --cflags` $(XINCLUDE) $(PFLAGS)

# Linker flags
LFLAGS=$(LDEBUG) $(PLINK) `pkg-config gtk+-2.0 --libs` -ldl

COMP=$(CC) $(CFLAGS) -I. -I$(SRCDIR) -I$(SRCUNXDIR) -I$(OBJDIR)
CCAUX=$(CC) $(CFLAGS) -I$(SRCDIR)


NUL=
SRC=$(SRCDIR)/$(NUL)
SRCUNX=$(SRCUNXDIR)/$(NUL)
OD=$(OBJDIR)/$(NUL)
BD=$(BINDIR)/$(NUL)
OBJ=.o
EXE=
CO=-c

FE=-o $(NUL)
FO=-o $(NUL)
FEO=-o $(OD)
FOO=-o $(OD)

HDRSPLAT=$(SRCUNX)gvx.h $(SRCUNX)gvxres.h 


CP=cp
RM=rm

# if you have a strict ANSI compiler, add -D__STDC__
EPSDEF=-I$(SRC) -I/usr/openwin/include -DUNIX -DEPSTOOL -DHAVE_UNISTD_H
EPSLIBS=
EPSOBJ2=


target: all


#################################################################
# Common

include $(SRC)common.mak
include $(SRC)gvcver.mak
DISTDIR=gsview-$(GSVIEW_DOT_VERSION)


#################################################################
# Platform files


VIEWONLY=0

OBJPLAT1=$(OD)gvx$(OBJ) $(OD)gvxdlg$(OBJ) $(OD)gvxdisp$(OBJ) \
 $(OD)gvxedit$(OBJ) $(OD)gvxeps$(OBJ) $(OD)gvxgsver$(OBJ) \
 $(OD)gvxinit$(OBJ) $(OD)gvxmeas$(OBJ) $(OD)gvxmisc$(OBJ) \
 $(OD)gvxprn$(OBJ) $(OD)gvxdll$(OBJ) \
 $(OD)gvxres$(OBJ) $(OD)gvcfile$(OBJ) \
 $(OD)gvxl_en$(OBJ) \
 $(OD)gvxl_ct$(OBJ) \
 $(OD)gvxl_de$(OBJ) \
 $(OD)gvxl_es$(OBJ) \
 $(OD)gvxl_fr$(OBJ) \
 $(OD)gvxl_gr$(OBJ) \
 $(OD)gvxl_it$(OBJ) \
 $(OD)gvxl_nl$(OBJ) \
 $(OD)gvxl_ru$(OBJ) \
 $(OD)gvxl_se$(OBJ) \
 $(OD)gvxl_sk$(OBJ)

OBJS=$(OBJCOM1) $(OBJCOM2) $(OBJPLAT1)


#all: gsview html pstotext
all: $(BD)gsview$(EXE) html

GSVIEW_DOCDIR=$(GSVIEW_DOCPATH)/gsview-$(GSVIEW_DOT_VERSION)

$(BD)gsview$(EXE): $(OBJS)
	$(COMP) $(CFLAGS) $(FO)$(BD)gsview$(EXE) $(OBJS) $(LFLAGS)

install: all
	-mkdir -p $(GSVIEW_BASE)
	chmod 755 $(GSVIEW_BASE)
	-mkdir -p $(GSVIEW_BINDIR)
	chmod 755 $(GSVIEW_BINDIR)
	$(INSTALL_EXE) $(BD)gsview$(EXE) $(GSVIEW_BINDIR)/gsview
	$(INSTALL_EXE) $(SRCUNX)gvxhelp.txt $(GSVIEW_BINDIR)/gsview-help
	-mkdir -p $(GSVIEW_MANDIR)
	chmod 755  $(GSVIEW_MANDIR)
	-mkdir -p $(GSVIEW_MANDIR)/man1
	chmod 755  $(GSVIEW_MANDIR)/man1
	$(INSTALL) $(SRCUNX)gsview.1 $(GSVIEW_MANDIR)/man1/gsview.1
	-mkdir -p $(GSVIEW_DOCPATH)
	chmod 755 $(GSVIEW_DOCPATH)
	-mkdir -p $(GSVIEW_DOCDIR)
	chmod 755  $(GSVIEW_DOCDIR)
	$(INSTALL) gsview.css $(GSVIEW_DOCDIR)/gsview.css
	$(INSTALL) Readme.htm  $(GSVIEW_DOCDIR)/Readme.htm
	$(INSTALL) LICENCE $(GSVIEW_DOCDIR)/LICENCE
	$(INSTALL) $(BD)gvxct.htm  $(GSVIEW_DOCDIR)/gvxct.htm
	$(INSTALL) $(BD)gvxde.htm  $(GSVIEW_DOCDIR)/gvxde.htm
	$(INSTALL) $(BD)gvxen.htm  $(GSVIEW_DOCDIR)/gvxen.htm
	$(INSTALL) $(BD)gvxes.htm  $(GSVIEW_DOCDIR)/gvxes.htm
	$(INSTALL) $(BD)gvxfr.htm  $(GSVIEW_DOCDIR)/gvxfr.htm
	$(INSTALL) $(BD)gvxgr.htm  $(GSVIEW_DOCDIR)/gvxgr.htm
	$(INSTALL) $(BD)gvxit.htm  $(GSVIEW_DOCDIR)/gvxit.htm
	$(INSTALL) $(BD)gvxnl.htm  $(GSVIEW_DOCDIR)/gvxnl.htm
	$(INSTALL) $(BD)gvxru.htm  $(GSVIEW_DOCDIR)/gvxru.htm
	$(INSTALL) $(BD)gvxse.htm  $(GSVIEW_DOCDIR)/gvxse.htm
	$(INSTALL) $(BD)gvxsk.htm  $(GSVIEW_DOCDIR)/gvxsk.htm
	-mkdir -p $(GSVIEW_ETCPATH)
	chmod 755  $(GSVIEW_ETCPATH)
	-mkdir -p $(GSVIEW_ETCPATH)/gsview
	chmod 755  $(GSVIEW_ETCPATH)/gsview
	$(INSTALL) $(SRC)printer.ini  $(GSVIEW_ETCPATH)/gsview/printer.ini

tar:
	rm -rf /var/tmp/gsview
	$(MAKE)
	$(MAKE) prefix=/var/tmp/gsview install
	cd /var/tmp/gsview; tar -cvf ~/gsview.tar *

$(OD)gvx$(OBJ): $(SRCUNX)gvx.c $(HDRS)
	$(COMP) $(FOO)gvx$(OBJ) $(CO) $(SRCUNX)gvx.c

$(OD)gvxdlg$(OBJ): $(SRCUNX)gvxdlg.c $(SRC)gvcrc.h $(HDRS)
	$(COMP) $(FOO)gvxdlg$(OBJ) $(CO) $(SRCUNX)gvxdlg.c

$(OD)gvxdll$(OBJ): $(SRCUNX)gvxdll.c $(SRC)gvcrc.h $(HDRS)
	$(COMP) $(FOO)gvxdll$(OBJ) $(CO) $(SRCUNX)gvxdll.c

$(OD)gvxdisp$(OBJ): $(SRCUNX)gvxdisp.c  $(HDRS)
	$(COMP) $(FOO)gvxdisp$(OBJ) $(CO) $(SRCUNX)gvxdisp.c

$(OD)gvxedit$(OBJ): $(SRCUNX)gvxedit.c $(HDRS)
	$(COMP) $(FOO)gvxedit$(OBJ) $(CO) $(SRCUNX)gvxedit.c

$(OD)gvxeps$(OBJ): $(SRCUNX)gvxeps.c $(SRC)gvceps.h $(HDRS)
	$(COMP) $(FOO)gvxeps$(OBJ) $(CO) $(SRCUNX)gvxeps.c

$(OD)gvxgsver$(OBJ): $(SRCUNX)gvxgsver.c $(HDRS) $(SRC)gvcrc.h
	$(COMP) $(FOO)gvxgsver$(OBJ) $(CO) $(SRCUNX)gvxgsver.c

$(OD)gvxinit$(OBJ): $(SRCUNX)gvxinit.c $(HDRS) $(SRC)gvcrc.h
	$(COMP) $(FOO)gvxinit$(OBJ) $(CO) $(SRCUNX)gvxinit.c

$(OD)gvxl_ct$(OBJ): ct/gvxlang.c $(HDRS) $(SRCUNX)gvxlang.h $(SRCUNX)gvxlangh.rc ct/gvclang.h ct/gvclang.rc  $(CODEPAGE)
	$(CODEPAGE) 1252 1252 ct/gvxlang.c gvxlang.c
	$(CODEPAGE) 1252 1252 ct/gvclang.h gvclang.h
	$(CODEPAGE) 1252 1252 ct/gvclang.rc gvclang.rc
	$(COMP) $(FOO)gvxl_ct$(OBJ) $(CO) gvxlang.c
	-rm gvxlang.c
	-rm gvclang.h
	-rm gvclang.rc

$(OD)gvxl_de$(OBJ): de/gvxlang.c $(HDRS) $(SRCUNX)gvxlang.h $(SRCUNX)gvxlangh.rc de/gvclang.h de/gvclang.rc  $(CODEPAGE)
	$(CODEPAGE) 1252 1252 de/gvxlang.c gvxlang.c
	$(CODEPAGE) 1252 1252 de/gvclang.h gvclang.h
	$(CODEPAGE) 1252 1252 de/gvclang.rc gvclang.rc
	$(COMP) $(FOO)gvxl_de$(OBJ) $(CO) gvxlang.c
	-rm gvxlang.c
	-rm gvclang.h
	-rm gvclang.rc

$(OD)gvxl_en$(OBJ): en/gvxlang.c $(HDRS) $(SRCUNX)gvxlang.h $(SRCUNX)gvxlangh.rc en/gvclang.h en/gvclang.rc  $(CODEPAGE)
	$(CODEPAGE) 1252 1252 en/gvxlang.c gvxlang.c
	$(CODEPAGE) 1252 1252 en/gvclang.h gvclang.h
	$(CODEPAGE) 1252 1252 en/gvclang.rc gvclang.rc
	$(COMP) $(FOO)gvxl_en$(OBJ) $(CO) gvxlang.c
	-rm gvxlang.c
	-rm gvclang.h
	-rm gvclang.rc

$(OD)gvxl_es$(OBJ): es/gvxlang.c $(HDRS) $(SRCUNX)gvxlang.h $(SRCUNX)gvxlangh.rc es/gvclang.h es/gvclang.rc  $(CODEPAGE)
	$(CODEPAGE) 1252 1252 es/gvxlang.c gvxlang.c
	$(CODEPAGE) 1252 1252 es/gvclang.h gvclang.h
	$(CODEPAGE) 1252 1252 es/gvclang.rc gvclang.rc
	$(COMP) $(FOO)gvxl_es$(OBJ) $(CO) gvxlang.c
	-rm gvxlang.c
	-rm gvclang.h
	-rm gvclang.rc

$(OD)gvxl_fr$(OBJ): fr/gvxlang.c $(HDRS) $(SRCUNX)gvxlang.h $(SRCUNX)gvxlangh.rc fr/gvclang.h fr/gvclang.rc  $(CODEPAGE)
	$(CODEPAGE) 1252 1252 fr/gvxlang.c gvxlang.c
	$(CODEPAGE) 1252 1252 fr/gvclang.h gvclang.h
	$(CODEPAGE) 1252 1252 fr/gvclang.rc gvclang.rc
	$(COMP) $(FOO)gvxl_fr$(OBJ) $(CO) gvxlang.c
	-rm gvxlang.c
	-rm gvclang.h
	-rm gvclang.rc

$(OD)gvxl_gr$(OBJ): gr/gvxlang.c $(HDRS) $(SRCUNX)gvxlang.h $(SRCUNX)gvxlangh.rc gr/gvclang.h gr/gvclang.rc $(CODEPAGE)
	$(CODEPAGE) 1253 8859-7 gr/gvxlang.c gvxlang.c
	$(CODEPAGE) 1253 8859-7 gr/gvclang.h gvclang.h
	$(CODEPAGE) 1253 8859-7 gr/gvclang.rc gvclang.rc
	$(COMP) $(FOO)gvxl_gr$(OBJ) $(CO) gvxlang.c
	-rm gvxlang.c
	-rm gvclang.h
	-rm gvclang.rc

$(OD)gvxl_it$(OBJ): it/gvxlang.c $(HDRS) $(SRCUNX)gvxlang.h $(SRCUNX)gvxlangh.rc it/gvclang.h it/gvclang.rc  $(CODEPAGE)
	$(CODEPAGE) 1252 1252 it/gvxlang.c gvxlang.c
	$(CODEPAGE) 1252 1252 it/gvclang.h gvclang.h
	$(CODEPAGE) 1252 1252 it/gvclang.rc gvclang.rc
	$(COMP) $(FOO)gvxl_it$(OBJ) $(CO) gvxlang.c
	-rm gvxlang.c
	-rm gvclang.h
	-rm gvclang.rc

$(OD)gvxl_nl$(OBJ): nl/gvxlang.c $(HDRS) $(SRCUNX)gvxlang.h $(SRCUNX)gvxlangh.rc nl/gvclang.h nl/gvclang.rc  $(CODEPAGE)
	$(CODEPAGE) 1252 1252 nl/gvxlang.c gvxlang.c
	$(CODEPAGE) 1252 1252 nl/gvclang.h gvclang.h
	$(CODEPAGE) 1252 1252 nl/gvclang.rc gvclang.rc
	$(COMP) $(FOO)gvxl_nl$(OBJ) $(CO) gvxlang.c
	-rm gvxlang.c
	-rm gvclang.h
	-rm gvclang.rc

$(OD)gvxl_ru$(OBJ): ru/gvxlang.c $(HDRS) $(SRCUNX)gvxlang.h $(SRCUNX)gvxlangh.rc ru/gvclang.h ru/gvclang.rc $(CODEPAGE)
	$(CODEPAGE) 1251 KOI8-R ru/gvxlang.c gvxlang.c
	$(CODEPAGE) 1251 KOI8-R ru/gvclang.h gvclang.h
	$(CODEPAGE) 1251 KOI8-R ru/gvclang.rc gvclang.rc
	$(COMP) $(FOO)gvxl_ru$(OBJ) $(CO) gvxlang.c
	-rm gvxlang.c
	-rm gvclang.h
	-rm gvclang.rc

$(OD)gvxl_se$(OBJ): se/gvxlang.c $(HDRS) $(SRCUNX)gvxlang.h $(SRCUNX)gvxlangh.rc se/gvclang.h se/gvclang.rc  $(CODEPAGE)
	$(CODEPAGE) 1252 1252 se/gvxlang.c gvxlang.c
	$(CODEPAGE) 1252 1252 se/gvclang.h gvclang.h
	$(CODEPAGE) 1252 1252 se/gvclang.rc gvclang.rc
	$(COMP) $(FOO)gvxl_se$(OBJ) $(CO) gvxlang.c
	-rm gvxlang.c
	-rm gvclang.h
	-rm gvclang.rc

$(OD)gvxl_sk$(OBJ): sk/gvxlang.c $(HDRS) $(SRCUNX)gvxlang.h $(SRCUNX)gvxlangh.rc sk/gvclang.h sk/gvclang.rc  $(CODEPAGE)
	$(CODEPAGE) 1250 1250 sk/gvxlang.c gvxlang.c
	$(CODEPAGE) 1250 1250 sk/gvclang.h gvclang.h
	$(CODEPAGE) 1250 1250 sk/gvclang.rc gvclang.rc
	$(COMP) $(FOO)gvxl_sk$(OBJ) $(CO) gvxlang.c
	-rm gvxlang.c
	-rm gvclang.h
	-rm gvclang.rc

$(OD)gvxmeas$(OBJ): $(SRCUNX)gvxmeas.c $(HDRS)
	$(COMP) $(FOO)gvxmeas$(OBJ) $(CO) $(SRCUNX)gvxmeas.c

$(OD)gvxmisc$(OBJ): $(SRCUNX)gvxmisc.c $(HDRS)
	$(COMP) $(FOO)gvxmisc$(OBJ) $(CO) $(SRCUNX)gvxmisc.c

$(OD)gvxprn$(OBJ): $(SRCUNX)gvxprn.c $(HDRS)
	$(COMP) $(FOO)gvxprn$(OBJ) $(CO) $(SRCUNX)gvxprn.c

$(OD)gvxres$(OBJ): $(SRCUNX)gvxres.c $(HDRS) $(SRCUNX)gvxlang.h
	$(COMP) $(FOO)gvxres$(OBJ) $(CO) $(SRCUNX)gvxres.c



html: $(BD)gvxct.htm $(BD)gvxde.htm $(BD)gvxen.htm $(BD)gvxes.htm $(BD)gvxfr.htm $(BD)gvxgr.htm $(BD)gvxit.htm $(BD)gvxnl.htm $(BD)gvxru.htm $(BD)gvxse.htm $(BD)gvxsk.htm

$(BD)gvxen.htm: $(DOC2HTML) $(GVDOC) en/gvclang.txt
	$(GVDOC) X en/gvclang.txt $(OD)gvx.txt
	$(DOC2HTML) $(OD)gvx.txt $(BD)gvxen.htm

$(BD)gvxct.htm: $(DOC2HTML) $(GVDOC) ct/gvclang.txt
	$(GVDOC) X ct/gvclang.txt $(OD)gvx.txt
	$(DOC2HTML) $(OD)gvx.txt $(BD)gvxct.htm

$(BD)gvxde.htm: $(DOC2HTML) $(GVDOC) de/gvclang.txt
	$(GVDOC) X de/gvclang.txt $(OD)gvx.txt
	$(DOC2HTML) $(OD)gvx.txt $(BD)gvxde.htm

$(BD)gvxes.htm: $(DOC2HTML) $(GVDOC) es/gvclang.txt
	$(GVDOC) X es/gvclang.txt $(OD)gvx.txt
	$(DOC2HTML) $(OD)gvx.txt $(BD)gvxes.htm

$(BD)gvxfr.htm: $(DOC2HTML) $(GVDOC) fr/gvclang.txt
	$(GVDOC) X fr/gvclang.txt $(OD)gvx.txt
	$(DOC2HTML) $(OD)gvx.txt $(BD)gvxfr.htm

$(BD)gvxgr.htm: $(DOC2HTML) $(GVDOC) gr/gvclang.txt
	$(GVDOC) X gr/gvclang.txt $(OD)temp.txt
	$(CODEPAGE) 1253 UTF-8 $(OD)temp.txt $(OD)gvx.txt
	$(DOC2HTML) $(OD)gvx.txt $(BD)gvxgr.htm utf-8

$(BD)gvxit.htm: $(DOC2HTML) $(GVDOC) it/gvclang.txt
	$(GVDOC) X it/gvclang.txt $(OD)gvx.txt
	$(DOC2HTML) $(OD)gvx.txt $(BD)gvxit.htm

$(BD)gvxnl.htm: $(DOC2HTML) $(GVDOC) nl/gvclang.txt
	$(GVDOC) X nl/gvclang.txt $(OD)gvx.txt
	$(DOC2HTML) $(OD)gvx.txt $(BD)gvxnl.htm

$(BD)gvxru.htm: $(DOC2HTML) $(GVDOC) ru/gvclang.txt
	$(GVDOC) X ru/gvclang.txt $(OD)temp.txt
	$(CODEPAGE) 1251 UTF-8 $(OD)temp.txt $(OD)gvx.txt
	$(DOC2HTML) $(OD)gvx.txt $(BD)gvxru.htm utf-8

$(BD)gvxse.htm: $(DOC2HTML) $(GVDOC) se/gvclang.txt
	$(GVDOC) X se/gvclang.txt $(OD)gvx.txt
	$(DOC2HTML) $(OD)gvx.txt $(BD)gvxse.htm

$(BD)gvxsk.htm: $(DOC2HTML) $(GVDOC) sk/gvclang.txt
	$(GVDOC) X sk/gvclang.txt $(OD)temp.txt
	$(CODEPAGE) 1250 UTF-8 $(OD)temp.txt $(OD)gvx.txt
	$(DOC2HTML) $(OD)gvx.txt $(BD)gvxsk.htm utf-8


clean: commonclean

veryclean: clean
	-$(RM) $(BD)gsview
	-$(RM) $(BD)gvxct.htm
	-$(RM) $(BD)gvxde.htm
	-$(RM) $(BD)gvxen.htm
	-$(RM) $(BD)gvxes.htm
	-$(RM) $(BD)gvxfr.htm
	-$(RM) $(BD)gvxgr.htm
	-$(RM) $(BD)gvxit.htm
	-$(RM) $(BD)gvxnl.htm
	-$(RM) $(BD)gvxru.htm
	-$(RM) $(BD)gvxse.htm
	-$(RM) $(BD)gvxsk.htm

