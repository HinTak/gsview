#  Copyright (C) 1993-2000, Ghostgum Software Pty Ltd.  All rights reserved.
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

!if $(USE_EMX)
# EMX
DRIVE=e:
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


OBJS=gvpm.$(OBJ) gvpdlg.$(OBJ) gvpdisp.$(OBJ) gvpedit.$(OBJ) gvpeps.$(OBJ)\
   gvpgsver.$(OBJ) gvpinit.$(OBJ) gvpmeas.$(OBJ) gvpmisc.$(OBJ) gvpprn.$(OBJ)\
   gvccmd.$(OBJ) gvcdisp.$(OBJ) gvceps.$(OBJ) gvcinit.$(OBJ) gvcbeta.$(OBJ)\
   gvcmeas.$(OBJ) gvcmisc.$(OBJ) gvcprf.$(OBJ) gvcprn.$(OBJ) gvctext.$(OBJ)\
   gvpdll.$(OBJ) gvcdll.$(OBJ)  gvcpdf.$(OBJ) ps.$(OBJ) 
HDRS=ver.h gvpm.h ps.h gvcfn.h gvcver.h


all: gvpm.exe\
 gvpmen.hlp\
 gvpmde.hlp gvpmde.dll setup2de.dll\
 gvpmfr.hlp gvpmfr.dll setup2fr.dll\
 gvpmit.hlp gvpmit.dll setup2it.dll\
 gvpgs.exe\
 os2setup.exe

.c.$(OBJ):
	$(COMP) $(FLAGS) -DOS2 -c $*.c

!include "gvcver.mak"

gvpm.$(OBJ): gvpm.c $(HDRS)

gvpdlg.$(OBJ): gvpdlg.c gvcrc.h $(HDRS)

gvpdll.$(OBJ): gvpdll.c gvcrc.h gsdll.h $(HDRS)

gvpdisp.$(OBJ): gvpdisp.c  $(HDRS)

gvpedit.$(OBJ): gvpedit.c $(HDRS)

gvpeps.$(OBJ): gvpeps.c gvceps.h $(HDRS)

gvpgsver.$(OBJ): gvpgsver.c $(HDRS) gvcrc.h

gvpinit.$(OBJ): gvpinit.c $(HDRS) gvcrc.h

gvpmeas.$(OBJ): gvpmeas.c $(HDRS)

gvpmisc.$(OBJ): gvpmisc.c $(HDRS)

gvpprn.$(OBJ): gvpprn.c $(HDRS)

gvccmd.$(OBJ): gvccmd.c gvcrc.h $(HDRS)

gvcdisp.$(OBJ): gvcdisp.c $(HDRS)

gvcdll.$(OBJ): gvcdll.c gvcrc.h gsdll.h $(HDRS)

ps.$(OBJ): ps.c

gvcbeta.obj: gvcbeta.c gvcbeta.h $(HDRS)

gvceps.$(OBJ): gvceps.c gvceps.h $(HDRS)

gvcmeas.$(OBJ): gvcmeas.c gvcrc.h $(HDRS)

gvcmisc.$(OBJ): gvcmisc.c gvcrc.h $(HDRS)

gvcpdf.$(OBJ): gvcpdf.c $(HDRS)

gvcprn.$(OBJ): gvcprn.c $(HDRS)

gvcprf.$(OBJ): gvcprf.c $(HDRS)

gvctext.$(OBJ): gvctext.c $(HDRS)

ansi2oem.exe: ansi2oem.c
	$(COMP) ansi2oem.c

gvpmde.res: gvpmde.hlp gvcrc.h de\gvclang.h gvpm2.rc de\gvclang.rc de\gvplang.rc gvpm3.rc binary\gvpm1.ico ansi2oem.exe $(HDRS)
	ansi2oem < de\gvclang.h > gvclang.h
	ansi2oem < de\gvclang.rc > gvclang.rc
	ansi2oem < de\gvplang.rc > gvplang.rc
	copy gvpm2.rc+gvphlpde.rc+gvclang.rc+gvplang.rc+gvpm3.rc gvpmde.rc
	rc -i $(COMPBASE)\include -r $*.rc
	-del gvclang.rc
	-del gvplang.rc
	-del gvclang.h
	ansi2oem < $(LANGUAGE)\gvclang.h > gvclang.h

gvpmde.dll: gvpmde.res de\gvplang.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, gvpmde.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, de\gvplang.def
	rc gvpmde.res gvpmde.dll
!endif

gvpmfr.res: gvpmfr.hlp gvcrc.h fr\gvclang.h gvpm2.rc fr\gvclang.rc fr\gvplang.rc gvpm3.rc binary\gvpm1.ico ansi2oem.exe $(HDRS)
	ansi2oem < fr\gvclang.h > gvclang.h
	ansi2oem < fr\gvclang.rc > gvclang.rc
	ansi2oem < fr\gvplang.rc > gvplang.rc
	copy gvpm2.rc+gvphlpfr.rc+gvclang.rc+gvplang.rc+gvpm3.rc gvpmfr.rc
	rc -i $(COMPBASE)\include -r $*.rc
	-del gvclang.rc
	-del gvplang.rc
	-del gvclang.h
	ansi2oem < $(LANGUAGE)\gvclang.h > gvclang.h

gvpmfr.dll: gvpmfr.res fr\gvplang.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, gvpmfr.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, fr\gvplang.def
	rc gvpmfr.res gvpmfr.dll
!endif

gvpmit.res: gvpmit.hlp gvcrc.h gvpm2.rc it\gvclang.h it\gvclang.rc it\gvplang.rc gvpm3.rc binary\gvpm1.ico ansi2oem.exe $(HDRS)
	ansi2oem < it\gvclang.h > gvclang.h
	ansi2oem < it\gvclang.rc > gvclang.rc
	ansi2oem < it\gvplang.rc > gvplang.rc
	copy gvpm2.rc+gvphlpit.rc+gvclang.rc+gvplang.rc+gvpm3.rc gvpmit.rc
	rc -i $(COMPBASE)\include -r $*.rc
	-del gvclang.rc
	-del gvplang.rc
	-del gvclang.h
	ansi2oem < $(LANGUAGE)\gvclang.h > gvclang.h

gvpmit.dll: gvpmit.res it\gvplang.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, gvpmit.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, it\gvplang.def
	rc gvpmit.res gvpmit.dll
!endif

gvpm.res: gvpm1.rc gvpm.h binary\gvpm1.ico gvpmen.hlp gvcrc.h en\gvclang.h gvpm1.rc en\gvclang.rc en\gvplang.rc gvpm3.rc ansi2oem.exe $(HDRS)
	ansi2oem < en\gvclang.h > gvclang.h
	ansi2oem < en\gvclang.rc > gvclang.rc
	ansi2oem < en\gvplang.rc > gvplang.rc
	copy gvpm1.rc+gvphlpen.rc+gvclang.rc+gvplang.rc+gvpm3.rc gvpm.rc
	rc -i $(COMPBASE)\include -r $*.rc
	-del gvclang.rc
	-del gvplang.rc
	-del gvclang.h
	ansi2oem < $(LANGUAGE)\gvclang.h > gvclang.h

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

os2setup.res: os2setup.rc setup.h $(LANGUAGE)\gvclang.h ansi2oem.exe
	ansi2oem < $(LANGUAGE)\gvclang.h > gvclang.h
	rc -i $(COMPBASE)\include -r $*.rc

os2beta.obj: gvcbeta.c gvcbeta.h gvcrc.h
!if $(USE_EMX)
	$(COMP) -Zomf -Zsys -c -o os2beta.obj gvcbeta.c
!else
	$(COMP) -c /Foos2beta.obj gvcbeta.c
!endif

os2prf.obj: gvcprf.c
!if $(USE_EMX)
	$(COMP) -Zomf -Zsys -c -o os2prf.obj gvcprf.c
!else
	$(COMP) -c /Foos2prf.obj gvcprf.c
!endif

os2unzip.obj: unzip2.h os2unzip.c
!if $(USE_EMX)
	$(COMP) -Zomf -Zsys -DOS2 -c os2unzip.c
!else
	$(COMP) -DOS2 -c os2unzip.c
!endif

setupc.obj: setup.h setupc.c
!if $(USE_EMX)
	$(COMP) -Zomf -Zsys -DOS2 -c setupc.c
!else
	$(COMP) -DOS2 -c setupc.c
!endif

os2setup.obj: os2setup.c setup.h os2setup.def gvcrc.h gvcbeta.h ansi2oem.exe
	ansi2oem < $(LANGUAGE)\gvclang.h > gvclang.h
!if $(USE_EMX)
	$(COMP) -Zomf -Zsys -c $(DEBUGFLAG) $*.c
!else
	$(COMP) -c $*.c
!endif

os2setup.exe: os2setup.obj os2setup.res os2setup.def gvcrc.h gvcbeta.h os2unzip.obj os2beta.obj os2prf.obj setupc.obj
!if $(USE_EMX)
	$(COMP) -Zomf -Zsys $(DEBUGFLAG) os2setup.obj os2unzip.obj setupc.obj os2prf.obj os2beta.obj os2setup.def
!else
	$(COMP) os2setup.obj os2unzip.obj setupc.obj os2prf.obj os2beta.obj os2setup.def
!endif
	rc os2setup.res os2setup.exe
	
setup2de.res: os2setup.rc setup.h gvcrc.h gvcver.h de\gvclang.h ansi2oem.exe
	ansi2oem < de\gvclang.h > gvclang.h
	rc -i $(COMPBASE)\include -r os2setup.rc setup2de.res

setup2de.dll: setup2de.res de\setup2.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, setup2de.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, de\setup2.def
	rc setup2de.res setup2de.dll
!endif

setup2fr.res: os2setup.rc setup.h gvcrc.h gvcver.h fr\gvclang.h ansi2oem.exe
	ansi2oem < fr\gvclang.h > gvclang.h
	rc -i $(COMPBASE)\include -r os2setup.rc setup2fr.res

setup2fr.dll: setup2fr.res fr\setup2.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, setup2fr.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, fr\setup2.def
	rc setup2fr.res setup2fr.dll
!endif


setup2it.res: os2setup.rc setup.h gvcrc.h gvcver.h it\gvclang.h ansi2oem.exe
	ansi2oem < it\gvclang.h > gvclang.h
	rc -i $(COMPBASE)\include -r os2setup.rc setup2it.res

setup2it.dll: setup2it.res it\setup2.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, setup2it.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, it\setup2.def
	rc setup2it.res setup2it.dll
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

gvc.txt: ansi2oem.exe $(LANGUAGE)\gvclang.txt
	ansi2oem < $(LANGUAGE)\gvclang.txt > gvc.txt

gvpmen.hlp: en\gvclang.txt  ansi2oem.exe gvdoc.exe doc2ipf.exe
	ansi2oem < en\gvclang.txt > gvc.txt
	gvdoc P gvc.txt gvpm.txt
	doc2ipf gvpm.txt gvpmen.ipf gvphlpen.rc
	ipfc gvpmen.ipf
	rename gvpmen.hlp gvpmen.hlp
	-del gvc.txt
	-del gvpm.txt

gvpmde.hlp: de\gvclang.txt  ansi2oem.exe gvdoc.exe doc2ipf.exe
	ansi2oem < de\gvclang.txt > gvc.txt
	gvdoc P gvc.txt gvpm.txt
	doc2ipf gvpm.txt gvpmde.ipf gvphlpde.rc
	ipfc gvpmde.ipf
	rename gvpmde.hlp gvpmde.hlp
	-del gvc.txt
	-del gvpm.txt

gvpmfr.hlp: fr\gvclang.txt  ansi2oem.exe gvdoc.exe doc2ipf.exe
	ansi2oem < fr\gvclang.txt > gvc.txt
	gvdoc P gvc.txt gvpm.txt
	doc2ipf gvpm.txt gvpmfr.ipf gvphlpfr.rc
	ipfc gvpmfr.ipf
	rename gvpmfr.hlp gvpmfr.hlp
	-del gvc.txt
	-del gvpm.txt

gvpmit.hlp: it\gvclang.txt  ansi2oem.exe gvdoc.exe doc2ipf.exe
	ansi2oem < it\gvclang.txt > gvc.txt
	gvdoc P gvc.txt gvpm.txt
	doc2ipf gvpm.txt gvpmit.ipf gvphlpit.rc
	ipfc gvpmit.ipf
	rename gvpmit.hlp gvpmit.hlp
	-del gvc.txt
	-del gvpm.txt

gvpmen.inf: gvpmen.hlp
	ipfc /INF gvpmen.ipf
	rename gvpmen.INF gvpmen.inf

html: gvpm.htm gsview.htm

gvpm.htm: doc2html.exe en\gvclang.txt
	gvdoc P en\gvclang.txt gvpm.txt
	doc2html gvpm.txt GSview.htm
	-del gvpm.htm
	rename GSview.htm gvpm.htm

gsview.txt: gvc.txt gvdoc.exe
	gvdoc W en\gvclang.txt gsview.txt

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


gvpgs.res: gvpgs.rc gvpgs.h gvcrc.h $(LANGUAGE)\gvclang.h
	ansi2oem < $(LANGUAGE)\gvclang.h > gvclang.h
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


gsv$(GSVIEW_VERSION)os2.zip:
	copy Readme.htm ..
	copy cdorder.txt ..
	copy LICENCE ..
	copy FILE_ID.DIZ ..
	copy gvpm.exe ..
!if $(USE_EMX) && !$(USE_OMF)
	emxbind -s ../gvpm.exe
!endif
	copy binary\gvpm1.ico ..\gvpm.ico
	copy gvpmen.hlp ..
	copy gvpmde.hlp ..
	copy gvpmfr.hlp ..
	copy gvpmit.hlp ..
	copy gvpmde.dll ..
	copy gvpmfr.dll ..
	copy gvpmit.dll ..
	copy gvpgs.exe ..
	copy os2setup.exe ..
	copy setup2de.dll ..
	copy setup2fr.dll ..
	copy setup2it.dll ..
	copy printer.ini ..\printer.ini
	cd ..
	-del os2.zip
	zip -9 -@ os2.zip < src\gvclist2.txt
	echo Redistribution of this OS/2 GSview MUST be accompanied by the> README2.TXT
	echo sources in gsv$(GSVIEW_VERSION)src.zip to meet the licence requirements. >> README2.TXT
	-del gsv$(GSVIEW_VERSION)os2.zip
	zip -9 gsv$(GSVIEW_VERSION)os2.zip os2.zip os2setup.exe unzip2.dll setup2de.dll setup2fr.dll setup2it.dll
	zip -9 gsv$(GSVIEW_VERSION)os2.zip README2.TXT Readme.htm cdorder.txt FILE_ID.DIZ LICENCE
	-del README2.TXT
	-del Readme.htm
	-del cdorder.txt
	-del LICENCE
	-del FILE_ID.DIZ
	-del gvpm.exe
	-del gvpm.ico
	-del gvpmen.hlp
	-del gvpmde.hlp
	-del gvpmfr.hlp
	-del gvpmit.hlp
	-del gvpmde.dll
	-del gvpmfr.dll
	-del gvpmit.dll
	-del gvpgs.exe
	-del os2setup.exe
	-del setup2de.dll
	-del setup2fr.dll
	-del setup2it.dll
	-del printer.ini
	cd src

prezip: gsv$(GSVIEW_VERSION)os2.zip

zip: prezip
	echo Doesn't work from here
	echo Use the Windows makefile

language:
	-del gvclang.h
	-del gvclang.rc
	-del gvplang.rc
	-del gvc.txt
	-del *.res

clean: language
	-del ver.h
	-del gvpm.res
	-del gvpm.$(OBJ)
	-del gvpdisp.$(OBJ)
	-del gvpdlg.$(OBJ)
	-del gvpdll.$(OBJ)
	-del gvpedit.$(OBJ)
	-del gvpeps.$(OBJ)
	-del gvpgsver.$(OBJ)
	-del gvpinit.$(OBJ)
	-del gvpmeas.$(OBJ)
	-del gvpmisc.$(OBJ)
	-del gvpprn.$(OBJ)
	-del ps.$(OBJ)
	-del gvcbeta.$(OBJ)
	-del gvccmd.$(OBJ)
	-del gvcdll.$(OBJ)
	-del gvcdisp.$(OBJ)
	-del gvceps.$(OBJ)
	-del gvcinit.$(OBJ)
	-del gvcmeas.$(OBJ)
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
	-del gsview.txt
	-del gvpgs.res
	-del gvpgs.$(OBJ)
	-del os2setup.obj
	-del os2setup.res
	-del os2unzip.obj
	-del os2beta.obj
	-del os2prf.obj
	-del setupc.obj
	-del gvplang.obj
	-del setup2de.map
	-del setup2fr.map
	-del setup2it.map
	-del gvpmen.map
	-del gvpmde.map
	-del gvpmfr.map
	-del gvpmit.map
	-del gvpmen.ipf
	-del gvpmde.ipf
	-del gvpmfr.ipf
	-del gvpmit.ipf
	-del gvpmen.rc
	-del gvpmde.rc
	-del gvpmfr.rc
	-del gvpmit.rc
	-del gvphlpen.rc
	-del gvphlpde.rc
	-del gvphlpfr.rc
	-del gvphlpit.rc
	-del ansi2oem.exe

veryclean: clean
	-del gvpm.exe
	-del gvpmde.dll
	-del gvpmfr.dll
	-del gvpmit.dll
	-del gvpmen.hlp
	-del gvpmde.hlp
	-del gvpmfr.hlp
	-del gvpmit.hlp
	-del gvpm.inf
	-del gvpm.tex
	-del gvpm.htm
	-del gsview.htm
	-del gvpgs.exe
	-del os2setup.exe
	-del setup2de.dll
	-del setup2fr.dll
	-del setup2it.dll
