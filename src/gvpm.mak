#  Copyright (C) 1993-1996, Russell Lang.  All rights reserved.
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
# requires emx 0.9b or later
#
# edit COMPBASE and EMXPATH as required.

# set one of the following to non-zero
USE_EMX=1
USE_BCC=0
USE_IBM=0
# USE_OMF=1 for EMX/GCC with LINK386
USE_OMF=1

# DEBUG=1 for debugging
DEBUG=0

!if $(USE_EMX)
# EMX
DRIVE=c:
COMP=gcc
COMPBASE=$(DRIVE)\emx
EMXPATH=$(DRIVE)/emx
!if $(USE_OMF)
OBJ=obj
!if $(DEBUG)
CDEBUG=-g
LDEBUG=/DEBUG
!endif
FLAGS=-O -Zomf -Zmts $(CDEBUG)
!else
OBJ=o
!if $(DEBUG)
CDEBUG=-g
LDEBUG=-g
!endif
FLAGS=-O -Zmts $(CDEBUG)
!endif
!endif

!if $(USE_IBM)
# ICC flags
COMP=icc
COMPBASE=d:\ibmcpp
EMXPATH=d:/ibmcpp
!if $(DEBUG)
CDEBUG=/Ti /Gm
LDEBUG=/DEBUG
!endif
FLAGS=/Q $(CDEBUG) /Sm /Id:\toolkit\h;d:\ibmcpp\include -DNO_MMOS2
OBJ=obj
!endif

!if $(USE_BCC)
# BCC flags
COMP=bcc
COMPBASE=d:\bcos2
EMXPATH=d:/bcos2
FLAGS=-v -I$(INCDIR) -sm
OBJ=obj
!endif


COMPDIR=$(COMPBASE)\bin
INCDIR=$(EMXPATH)/include
LIBDIR=$(EMXPATH)/lib


OBJS=gvpm.$(OBJ) gvpdlg.$(OBJ) gvpdisp.$(OBJ) gvpeps.$(OBJ) gvpinit.$(OBJ)\
   gvpmisc.$(OBJ) gvpprn.$(OBJ)\
   gvccmd.$(OBJ) gvcdisp.$(OBJ) gvceps.$(OBJ) gvcinit.$(OBJ) gvcbeta.$(OBJ)\
   gvcmisc.$(OBJ) gvcprf.$(OBJ) gvcprn.$(OBJ) gvctext.$(OBJ)\
   gvpdll.$(OBJ) gvcdll.$(OBJ)  gvcpdf.$(OBJ) ps.$(OBJ) 
HDRS=gvpm.h ps.h gvcfn.h 

all: gvpm.exe gvpm.hlp gvpm.inf gvpm.tex gvpgs.exe os2setup.exe

.c.$(OBJ):
	$(COMP) $(FLAGS) -DOS2 -c $*.c



gvpm.$(OBJ): gvpm.c gvpm.ipf $(HDRS)

gvpdlg.$(OBJ): gvpdlg.c gvcrc.h $(HDRS)

gvpdll.$(OBJ): gvpdll.c gvcrc.h gsdll.h $(HDRS)

gvpdisp.$(OBJ): gvpdisp.c  $(HDRS)

gvpeps.$(OBJ): gvpeps.c gvceps.h $(HDRS)

gvpinit.$(OBJ): gvpinit.c $(HDRS)

gvpmisc.$(OBJ): gvpmisc.c $(HDRS)

gvpprn.$(OBJ): gvpprn.c $(HDRS)

gvccmd.$(OBJ): gvccmd.c gvcrc.h $(HDRS)

gvcdisp.$(OBJ): gvcdisp.c $(HDRS)

gvcdll.$(OBJ): gvcdll.c gvcrc.h gsdll.h $(HDRS)

ps.$(OBJ): ps.c

gvcbeta.obj: gvcbeta.c gvcbeta.h $(HDRS)

gvceps.$(OBJ): gvceps.c gvceps.h $(HDRS)

gvcmisc.$(OBJ): gvcmisc.c gvcrc.h $(HDRS)

gvcpdf.$(OBJ): gvcpdf.c $(HDRS)

gvcprn.$(OBJ): gvcprn.c $(HDRS)

gvcprf.$(OBJ): gvcprf.c $(HDRS)

gvctext.$(OBJ): gvctext.c $(HDRS)

gvpm.res: gvpm.rc gvpm.h binary\gvpm1.ico
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
!endif
!if $(USE_IBM)
#	$(COMP) $(FLAGS) /Fe gvpm.exe $(OBJS) gvpm.def
	LINK386 /NOE /nologo $(LDEBUG) /noi /align:16 /exepack /base:65536 $(OBJS), gvpm.exe, , ,gvpm.def
	RC gvpm.res gvpm.exe
!endif
!if $(USE_BCC)
	$(COMP) $(FLAGS) -egvpm.exe $(OBJS) gvpm.def
	RC gvpm.res gvpm.exe
!endif

os2setup.res: os2setup.rc setup.h
	rc -i $(COMPBASE)\include -r $*.rc

os2setup.exe: os2setup.c setup.h os2setup.res os2setup.def
!if $(USE_EMX)
	$(COMP) -Zomf -Zsys -c -o os2prf.obj gvcprf.c
	$(COMP) -Zomf -Zsys -c -o os2beta.obj gvcbeta.c
	$(COMP) -Zomf -Zsys $(DEBUGFLAG) $*.c os2prf.obj os2beta.obj os2setup.def
!else
	$(COMP) -c /Foos2prf.obj gvcprf.c
	$(COMP) -c /Foos2beta.obj gvcbeta.c
	$(COMP) $*.c os2prf.obj os2beta.obj os2setup.def
!endif
	rc os2setup.res os2setup.exe
	

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


gvpgs.res: gvpgs.rc gvpgs.h gvcrc.h
	rc -i $(COMPBASE)\include -r $*.rc

gvpgs.$(OBJ): gvpgs.c gvpgs.h gvcrc.h

gvpgs.exe: gvpgs.$(OBJ) gvpgs.res gvpgs.def
!if $(USE_EMX)
!if $(USE_OMF)
	$(COMP) $(FLAGS) -o gvpgs gvpgs.$(OBJ) gvpgs.def
	rc gvpgs.res gvpgs.exe
!else
	$(COMP) $(FLAGS) -o gvpgs gvpgs.$(OBJ)
	emxbind -p -rgvpgs.res -dgvpgs.def $(COMPDIR)\emxl.exe gvpgs gvpgs.exe
	del $*
!endif
!endif
!if $(USE_IBM)
	LINK386 /NOE /nologo $(LDEBUG) /noi /align:16 /exepack /base:65536 gvpgs.obj, gvpgs.exe, , ,gvpgs.def
	RC gvpgs.res gvpgs.exe
!endif


prezip: gvpm.exe gvpm.hlp gvpm.inf README.TXT FILE_ID.DIZ LICENCE
	copy gvpm.exe ..
!if $(USE_EMX) && !$(USE_OMF)
	emxbind -s ../gvpm.exe
!endif
	-del ..\gvpm.eas
	eautil ..\gvpm.exe ..\gvpm.eas /s
	copy gvpm.hlp ..
	copy gvpm.inf ..
	copy gvpgs.exe ..
	copy os2setup.exe ..
	copy printer.ini ..\printer.ini
	copy README.TXT ..\README.TXT
	copy FILE_ID.DIZ ..\FILE_ID.DIZ
	copy LICENCE ..\LICENCE
	-del ..\epstool.zip
	-del ..\gsview.zip
	-del ..\pstotext.zip
	-del ..\src.zip
	-del gsviewXX.zip

zip: prezip
	cd ..
	zip -9 -@ epstool.zip < src\gvcliste.txt
	zip -9 -@ pstotext.zip     < src\gvclistp.txt
	zip -9 -@ src.zip     < src\gvclists.txt
	cd ..
	zip -9 -@ gsview\gsview.zip  < gsview\src\gvclist.txt
	cd gsview
	zip -9 gsviewXX.zip gsview.zip README.TXT FILE_ID.DIZ LICENCE os2setup.exe os2unzip.exe setup.exe wizunz32.dll
	cd src

clean:
	-del gvpm.res
	-del gvpm.$(OBJ)
	-del gvpdisp.$(OBJ)
	-del gvpdlg.$(OBJ)
	-del gvpdll.$(OBJ)
	-del gvpeps.$(OBJ)
	-del gvpinit.$(OBJ)
	-del gvpmisc.$(OBJ)
	-del gvpprn.$(OBJ)
	-del ps.$(OBJ)
	-del gvcbeta.$(OBJ)
	-del gvccmd.$(OBJ)
	-del gvcdll.$(OBJ)
	-del gvcdisp.$(OBJ)
	-del gvceps.$(OBJ)
	-del gvcinit.$(OBJ)
	-del gvcmisc.$(OBJ)
	-del gvcpdf.$(OBJ)
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
	-del gvpgs.res
	-del gvpgs.$(OBJ)
	-del os2setup.obj
	-del os2setup.res
	-del os2beta.obj
	-del os2prf.obj

veryclean: clean
	-del gvpm.exe
	-del gvpm.hlp
	-del gvpm.inf
	-del gvpm.tex
	-del gvpm.htm
	-del gsview.htm
	-del gvpgs.exe
	-del os2setup.exe
