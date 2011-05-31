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
# only USE_EMX currently works
USE_EMX=1
USE_BCC=0
USE_IBM=0
# USE_OMF=1 for EMX/GCC with LINK386
USE_OMF=1

# DEBUG=1 for debugging
DEBUG=0

# Language is English (en) or Deutsch (de)
LANGUAGE=en

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
FLAGS=-O -Wall -Zomf -Zmts $(CDEBUG)
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
HDRS=gvpm.h ps.h gvcfn.h gvcver.h

all: gvpm.exe\
 gvpmen.hlp gvpmen.dll\
 gvpmde.hlp gvpmde.dll\
 gvpgs.exe os2setup.exe

.c.$(OBJ):
	$(COMP) $(FLAGS) -DOS2 -c $*.c


gvpm.$(OBJ): gvpm.c $(HDRS)

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

ansi2oem.exe: ansi2oem.c
	$(COMP) ansi2oem.c

gvpmen.res: gvpmen.hlp gvcrc.h gvpm1.rc gvcen.rc gvpen.rc gvpm2.rc binary\gvpm1.ico ansi2oem.exe $(HDRS)
	ansi2oem < gvcen.h > gvclang.h
	ansi2oem < gvcen.rc > gvclang.rc
	ansi2oem < gvpen.rc > gvplang.rc
	copy gvpm1.rc+gvphlpen.rc+gvclang.rc+gvplang.rc+gvpm2.rc gvpmen.rc
	rc -i $(COMPBASE)\include -r $*.rc
	-del gvclang.rc
	-del gvplang.rc
	-del gvclang.h
	ansi2oem < gvc$(LANGUAGE).h > gvclang.h

gvpmen.dll: gvpmen.res gvpmen.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 /DEBUG $(COMPBASE)\lib\dll0.obj gvplang.obj, gvpmen.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\os2.lib, gvpmen.def
	rc gvpmen.res gvpmen.dll
!endif

gvpmde.res: gvpmde.hlp gvcrc.h gvpm1.rc gvcde.rc gvpde.rc gvpm2.rc binary\gvpm1.ico ansi2oem.exe $(HDRS)
	ansi2oem < gvcde.h > gvclang.h
	ansi2oem < gvcde.rc > gvclang.rc
	ansi2oem < gvpde.rc > gvplang.rc
	copy gvpm1.rc+gvphlpde.rc+gvclang.rc+gvplang.rc+gvpm2.rc gvpmde.rc
	rc -i $(COMPBASE)\include -r $*.rc
	-del gvclang.rc
	-del gvplang.rc
	-del gvclang.h
	ansi2oem < gvc$(LANGUAGE).h > gvclang.h

gvpmde.dll: gvpmde.res gvpmde.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 /DEBUG $(COMPBASE)\lib\dll0.obj gvplang.obj, gvpmde.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\os2.lib, gvpmde.def
	rc gvpmde.res gvpmde.dll
!endif

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

os2setup.res: os2setup.rc setup.h gvc$(LANGUAGE).h ansi2oem.exe
	ansi2oem < gvc$(LANGUAGE).h > gvclang.h
	rc -i $(COMPBASE)\include -r $*.rc

os2setup.exe: os2setup.c setup.h os2setup.res os2setup.def gvcrc.h gvcbeta.h
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

gvc.txt: ansi2oem.exe gvc$(LANGUAGE).txt
	ansi2oem < gvc$(LANGUAGE).txt > gvc.txt

gvpmen.hlp: gvcen.txt  ansi2oem.exe gvdoc.exe doc2ipf.exe
	ansi2oem < gvcen.txt > gvc.txt
	gvdoc P gvc.txt gvpm.txt
	doc2ipf gvpm.txt gvpmen.ipf gvphlpen.rc
	ipfc gvpmen.ipf
	rename gvpmen.hlp gvpmen.hlp
	-del gvc.txt
	-del gvpm.txt

gvpmde.hlp: gvcde.txt  ansi2oem.exe gvdoc.exe doc2ipf.exe
	ansi2oem < gvcde.txt > gvc.txt
	gvdoc P gvc.txt gvpm.txt
	doc2ipf gvpm.txt gvpmde.ipf gvphlpde.rc
	ipfc gvpmde.ipf
	rename gvpmde.hlp gvpmde.hlp
	-del gvc.txt
	-del gvpm.txt

gvpmen.inf: gvpmen.hlp
	ipfc /INF gvpmen.ipf
	rename gvpmen.INF gvpmen.inf

html: gvpm.htm gsview.htm

gvpm.htm: doc2html.exe gvpm.txt
	doc2html gvpm.txt GSview.htm
	-del gvpm.htm
	rename GSview.htm gvpm.htm

gsview.txt: gvc.txt gvdoc.exe
	gvdoc W gvc.txt gsview.txt

gsview.htm: doc2html.exe gsview.txt
	doc2html gsview.txt GSview.htm

gvpm.ps: gvpm.dvi
	dvips gvpm

gvpm.dvi: gvpm.tex titlepag.tex
	-latex gvpm
	-latex gvpm

gvpm.tex: gvpm.txt doc2tex.exe
	doc2tex gvpm.txt gvpm.tex

doc2tex.exe: doc2tex.c
!if $(USE_EMX)
	$(COMP) -o $* $*.c
	emxbind $(COMPDIR)\emxl.exe $* $*.exe
	del $*
!else
	$(COMP) $*.c
!endif


gvpgs.res: gvpgs.rc gvpgs.h gvcrc.h gvc$(LANGUAGE).h
	ansi2oem < gvc$(LANGUAGE).h > gvclang.h
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


prezip: gvpm.exe gvpmen.dll gvpmen.hlp gvpmde.dll gvpmde.hlp README.TXT FILE_ID.DIZ LICENCE
	copy gvpm.exe ..
!if $(USE_EMX) && !$(USE_OMF)
	emxbind -s ../gvpm.exe
!endif
	-del ..\gvpm.eas
	eautil ..\gvpm.exe ..\gvpm.eas /s
	copy gvpmen.hlp ..
	copy gvpmde.hlp ..
	copy gvpmen.dll ..
	copy gvpmde.dll ..
	copy binary\gvpm1.ico ..\gvpm.ico
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
	zip -9 -@ epstool.zip  < src\gvcliste.txt
	zip -9 -@ pstotext.zip < src\gvclistp.txt
	zip -9 -@ src.zip      < src\gvclists.txt
	zip -9 -@ gsview.zip   < src\gvclist.txt
	zip -9 gsviewXX.zip gsview.zip README.TXT FILE_ID.DIZ LICENCE os2setup.exe os2unzip.exe setup.exe wizunz32.dll
	cd src

language:
	-del gvclang.h
	-del gvclang.rc
	-del gvplang.rc
	-del gvc.txt
	-del *.res

clean: language
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
	-del gvpm.txt
	-del gvpm.aux
	-del gvpm.dvi
	-del gvpm.log
	-del gvpm.toc
	-del gvphelp.h
	-del gsview.txt
	-del gvpgs.res
	-del gvpgs.$(OBJ)
	-del os2setup.obj
	-del os2setup.res
	-del os2beta.obj
	-del os2prf.obj
	-del gvplang.obj
	-del gvpmen.map
	-del gvpmde.map
	-del gvpmen.ipf
	-del gvpmde.ipf
	-del gvpmen.rc
	-del gvpmde.rc
	-del gvphlpen.rc
	-del gvphlpde.rc
	-del ansi2oem.exe

veryclean: clean
	-del gvpm.exe
	-del gvpmen.dll
	-del gvpmde.dll
	-del gvpmen.hlp
	-del gvpmde.hlp
	-del gvpm.inf
	-del gvpm.tex
	-del gvpm.htm
	-del gsview.htm
	-del gvpgs.exe
	-del os2setup.exe
