#  Copyright (C) 1993, 1994, Russell Lang.  All rights reserved.
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

# gvpm.mak
# PM GSview 
# requires emx 0.8h
#
# edit COMPBASE and EMXPATH as required.

# set USE_EMX=0 for BCC
# set USE_EMX=1 for EMX/GCC
USE_EMX=1
# USE_OMF=1 for EMX/GCC with LINK386
USE_OMF=0

!if $(USE_EMX)
# EMX
DRIVE=e:
COMP=gcc
COMPBASE=$(DRIVE)\emx
EMXPATH=$(DRIVE)/emx
!if $(USE_OMF)
FLAGS=-Zomf -Zmts -g
OBJ=obj
!else
FLAGS=-Zmts -g -O
OBJ=o
!endif
!else
# BCC flags
COMP=bcc
COMPBASE=d:\bcos2
EMXPATH=d:/bcos2
FLAGS=-v -I$(INCDIR) -sm
OBJ=obj
!endif

# ICC flags
#COMP=icc
#COMPBASE=c:\ibmc
#EMXPATH=c:/ibmc
#FLAGS=/Gm /Ti /Sm /Ic:\toolkt20\c\os2h;c:\ibmc\include -DNO_MMOS2
#OBJ=obj

COMPDIR=$(COMPBASE)\bin
INCDIR=$(EMXPATH)/include
LIBDIR=$(EMXPATH)/lib


OBJS=gvpm.$(OBJ) gvpdlg.$(OBJ) gvpdisp.$(OBJ) gvpeps.$(OBJ) gvpinit.$(OBJ)\
   gvpmisc.$(OBJ) gvpprn.$(OBJ)\
   gvccmd.$(OBJ) gvcdisp.$(OBJ) ps.$(OBJ) gvceps.$(OBJ) gvcmisc.$(OBJ)\
   gvcprf.$(OBJ) gvcprn.$(OBJ) gvctext.$(OBJ)

all: gvpm.exe gvpm.hlp gvpm.inf gvpm.tex

.c.$(OBJ):
	$(COMP) $(FLAGS) -DOS2 -c $*.c


gvpm.$(OBJ): gvpm.c gvpm.h ps.h gvpm.ipf

gvpdlg.$(OBJ): gvpdlg.c gvpm.h ps.h gvcrc.h

gvpdisp.$(OBJ): gvpdisp.c gvpm.h ps.h

gvpeps.$(OBJ): gvpeps.c gvpm.h gvceps.h ps.h

gvpinit.$(OBJ): gvpinit.c gvpm.h ps.h

gvpmisc.$(OBJ): gvpmisc.c gvpm.h ps.h

gvpprn.$(OBJ): gvpprn.c gvpm.h ps.h

gvccmd.$(OBJ): gvccmd.c gvpm.h ps.h gvcrc.h

gvcdisp.$(OBJ): gvcdisp.c gvpm.h ps.h

ps.$(OBJ): ps.c gvpm.h ps.h

gvceps.$(OBJ): gvceps.c gvpm.h ps.h

gvcmisc.$(OBJ): gvcmisc.c gvpm.h ps.h gvcrc.h

gvcprn.$(OBJ): gvcprn.c gvpm.h ps.h

gvcprf.$(OBJ): gvcprf.c gvpm.h

gvctext.$(OBJ): gvctext.c gvpm.h ps.h

gvpm.res: gvpm.rc gvpm.h binary\gvpm.ico
	rc -i $(COMPBASE)\include -r $*.rc

gvpm.exe: $(OBJS) gvpm.res gvpm.def
!if $(USE_EMX)
!if $(USE_OMF)
	$(COMP) $(FLAGS) -o gvpm $(OBJS) gvpm.def
	rc gvpm.res gvpm.exe
!else
	$(COMP) $(FLAGS) -o gvpm $(OBJS)
	emxbind -p -rgvpm.res -dgvpm.def $(COMPDIR)\emxl.exe gvpm gvpm.exe
	del $*
!endif
!else
	$(COMP) $(FLAGS) -egvpm.exe $(OBJS)
	RC gvpm.res gvpm.exe
!endif

gvdoc.exe: gvdoc.c
!if $(USE_EMX)
	$(COMP) -o $* $*.c
	emxbind $(COMPDIR)\emxl.exe $* $*.exe
	del $*
!else
	$(COMP) $*.c
!endif
	
doc2ipf.exe: doc2ipf.c
!if $(USE_EMX)
	$(COMP) -o $* $*.c
	emxbind $(COMPDIR)\emxl.exe $* $*.exe
	del $*
!else
	$(COMP) $*.c
!endif

doc2html.exe: doc2html.c
!if $(USE_EMX)
	$(COMP) -o $* $*.c
	emxbind $(COMPDIR)\emxl.exe $* $*.exe
	del $*
!else
	$(COMP) $*.c
!endif

gvpm.doc: gvc.doc gvdoc.exe
	gvdoc P gvc.doc gvpm.doc

gvpm.ipf: gvpm.doc doc2ipf.exe 
	doc2ipf gvpm.doc gvpm.ipf gvphelp.h

gvpm.hlp: gvpm.ipf
	ipfc gvpm.ipf
	rename gvpm.HLP gvpm.hlp

gvpm.inf: gvpm.ipf
	ipfc /INF gvpm.ipf
	rename gvpm.INF gvpm.inf

html: gvpm.htm gsview.htm

gvpm.htm: doc2html.exe gvpm.doc
	doc2html gvpm.doc GSview.htm
	-del gvpm.htm
	rename GSview.htm gvpm.htm

gsview.doc: gvc.doc gvdoc.exe
	gvdoc W gvc.doc gsview.doc

gsview.htm: doc2html.exe gsview.doc
	doc2html gsview.doc GSview.htm

gvpm.ps: gvpm.dvi
	dvips gvpm

gvpm.dvi: gvpm.tex titlepag.tex
	-latex gvpm
	-latex gvpm

gvpm.tex: gvpm.doc doc2tex.exe
	doc2tex gvpm.doc gvpm.tex

doc2tex.exe: doc2tex.c
!if $(USE_EMX)
	$(COMP) -o $* $*.c
	emxbind $(COMPDIR)\emxl.exe $* $*.exe
	del $*
!else
	$(COMP) $*.c
!endif


prezip: gvpm.exe gvpm.hlp gvpm.inf README.GV LICENCE
	copy gvpm.exe ..
	emxbind -s ../gvpm.exe
	copy gvpm.hlp ..
	copy gvpm.inf ..
	copy README.GV ..
	copy LICENCE ..
	-del ..\gsview.zip

zip: prezip
	cd ..
	zip -9 -@ gsview.zip < src/gvclist.doc
	cd src

clean:
	-del gvpm.res
	-del gvpm.$(OBJ)
	-del gvpdlg.$(OBJ)
	-del gvpdisp.$(OBJ)
	-del gvpeps.$(OBJ)
	-del gvpinit.$(OBJ)
	-del gvpmisc.$(OBJ)
	-del gvpprn.$(OBJ)
	-del gvccmd.$(OBJ)
	-del gvcdisp.$(OBJ)
	-del ps.$(OBJ)
	-del gvceps.$(OBJ)
	-del gvcmisc.$(OBJ)
	-del gvcprf.$(OBJ)
	-del gvcprn.$(OBJ)
	-del gvctext.$(OBJ)
	-del doc2ipf.$(OBJ)
	-del doc2ipf.exe
	-del doc2html.$(OBJ)
	-del doc2html.exe
	-del doc2tex.$(OBJ)
	-del doc2tex.exe
	-del gvdoc.$(OBJ)
	-del gvdoc.exe
	-del gvpm.ipf
	-del gvpm.doc
	-del gvpm.aux
	-del gvpm.dvi
	-del gvpm.log
	-del gvpm.toc
	-del gvphelp.h
	-del gsview.doc

veryclean: clean
	-del gvpm.exe
	-del gvpm.hlp
	-del gvpm.inf
	-del gvpm.tex
	-del gvpm.htm
	-del gsview.htm
