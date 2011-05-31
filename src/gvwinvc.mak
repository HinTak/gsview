#  Copyright (C) 1993-1997, Russell Lang.  All rights reserved.
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

# Makefile for GSview for Windows - GSVIEW32.EXE
# using Microsoft Visual C++ 5.0
# 'make -fgvwinvc.mak'
#
# Path to Microsoft Visual C++ must NOT include spaces

# Edit COMPBASE and WIN32 as required
DEVBASE = e:\devstudio
# DEBUG=1 for Debugging options
DEBUG=1
# WIN32 is the default
WIN32=1
# ALPHA=1 for DEC Alpha
ALPHA=0
# Language is English (en), Deutsch (de) or French (fr)
# This only applies to the utilties, not GSview itself.
LANGUAGE=en
# GSview version
GSVIEW_VERSION=24

# Shouldn't need editing below here
COMPBASE = $(DEVBASE)\vc
COMPDIR = $(COMPBASE)\bin
INCDIR = $(COMPBASE)\include
LIBDIR = $(COMPBASE)\lib
RCOMP32=$(DEVBASE)\sharedide\bin\rc -D_MSC_VER
RCOMP=$(RCOMP32)
HC=hcw /C /E
!if $(WIN32)
!if $(ALPHA)
WINEXT=da
CFLAGS=-D_Windows -D__WIN32__ -DDECALPHA -I$(INCDIR)
!else
WINEXT=32
CFLAGS=-D_Windows -D__WIN32__ -I$(INCDIR)
!endif
CCAUX = cl
MODEL=32
CC = cl
!if $(DEBUG)
DEBUGLINK=/DEBUG
!endif
!else
    echo Win16 not supported with MSVC++
!endif

OBJ1=gvwin.obj gvwdll.obj gvwdisp.obj gvwdlg.obj
OBJ2= gvwclip.obj gvweps.obj gvwmisc.obj gvwprf.obj gvwprn.obj
OBJ3=gvcmisc.obj gvcdisp.obj ps.obj gvccmd.obj gvcprn.obj
OBJ4=gvceps.obj gvcinit.obj gvctext.obj
OBJ5=gvcdll.obj gvcpdf.obj gvwinit.obj gvcbeta.obj
OBJS=$(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5)

HDRS=gvwin.h ps.h gvcfn.h gvcver.h

all: gsview$(WINEXT).exe\
  gsviewen.hlp\
  gsvw$(WINEXT)de.dll gsviewde.hlp\
  gsvw$(WINEXT)fr.dll gsviewfr.hlp\
  gvwgs$(WINEXT).exe\
  winsetup.exe setp$(WINEXT)fr.dll setp$(WINEXT)de.dll

.c.obj:
	$(COMPDIR)\$(CC) -c $(CFLAGS) $< 

lib.rsp: makefile
        echo $(LIBDIR)\shell32.lib > lib.rsp
        echo $(LIBDIR)\comdlg32.lib >> lib.rsp
        echo $(LIBDIR)\gdi32.lib >> lib.rsp
        echo $(LIBDIR)\user32.lib >> lib.rsp
        echo $(LIBDIR)\winspool.lib >> lib.rsp
        echo $(LIBDIR)\advapi32.lib >> lib.rsp
	echo /NODEFAULTLIB:LIBC.lib >> lib.rsp
        echo $(LIBDIR)\libcmt.lib >> lib.rsp

	
# change cw32mt to cw32 for single thread
gsview32.exe: $(OBJS) gvwin32.res gvwin32.def lib.rsp
	echo $(OBJ1) > link.rsp
	echo $(OBJ2) >> link.rsp
	echo $(OBJ3) >> link.rsp
	echo $(OBJ4) >> link.rsp
	echo $(OBJ5) >> link.rsp
	$(COMPDIR)\link $(DEBUGLINK) /DEF:gvwin32.def /OUT:gsview32.exe @link.rsp @lib.rsp gvwin32.res


gsvw32de.res: gvwin2.rc gvcde.h gvcde.rc gvwde.rc
	copy gvcde.h  gvclang.h
	copy gvcde.rc gvclang.rc
	copy gvwde.rc gvwlang.rc
	$(RCOMP32) -i$(INCDIR) -r -fogsvw32de.res gvwin2
	-del gvclang.rc
	-del gvwlang.rc

gsvw32de.dll: gvwlang.c gsvw32de.res gsvw32de.def
	$(COMPDIR)\$(CC) /c $(CFLAGS) /I$(INCDIR) gvwlang.c
	$(COMPDIR)\link $(DEBUGLINK) /DLL /DEF:gsvw32de.def /OUT:gsvw32de.dll gvwlang.obj gsvw32de.res


gsvw32fr.res: gvwin2.rc gvcfr.h gvcfr.rc gvwfr.rc
	copy gvcfr.h  gvclang.h
	copy gvcfr.rc gvclang.rc
	copy gvwfr.rc gvwlang.rc
	$(RCOMP32) -i$(INCDIR) -r -fogsvw32fr.res gvwin2
	-del gvclang.rc
	-del gvwlang.rc

gsvw32fr.dll: gvwlang.c gsvw32fr.res gsvw32fr.def
	$(COMPDIR)\$(CC) /c $(CFLAGS) /I$(INCDIR) gvwlang.c
	$(COMPDIR)\link $(DEBUGLINK) /DLL /DEF:gsvw32fr.def /OUT:gsvw32fr.dll gvwlang.obj gsvw32fr.res


gvwin32.res: gvwin1.rc gvwin2.rc gvcen.h gvcen.rc gvwen.rc
	copy gvcen.h  gvclang.h
	copy gvcen.rc gvclang.rc
	copy gvwen.rc gvwlang.rc
	$(RCOMP32) -i$(INCDIR) -r -fogvwin32.res gvwin1
	-del gvclang.rc
	-del gvwlang.rc

gvwin.obj: gvwin.c $(HDRS)

gvwclip.obj: gvwclip.c $(HDRS)

gvwdisp.obj: gvwdisp.c $(HDRS)

gvwdlg.obj: gvwdlg.c gvcrc.h $(HDRS)

gvwdll.obj: gvwdll.c gvcrc.h gsdll.h $(HDRS)

gvweps.obj: gvweps.c gvceps.h $(HDRS)

gvwinit.obj: gvwinit.c $(HDRS)

gvwmisc.obj: gvwmisc.c $(HDRS)

gvwprn.obj: gvwprn.c $(HDRS)

gvccmd.obj: gvccmd.c gvcrc.h $(HDRS)

gvcdisp.obj: gvcdisp.c $(HDRS)

gvcdll.obj: gvcdll.c gvcrc.h gsdll.h $(HDRS)

ps.obj: ps.c ps.h

gvcbeta.obj: gvcbeta.c gvcbeta.h $(HDRS)

gvceps.obj: gvceps.c gvceps.h $(HDRS)

gvcinit.obj: gvcinit.c gvcrc.h $(HDRS)

gvcmisc.obj: gvcmisc.c gvcrc.h $(HDRS)

gvcpdf.obj: gvcpdf.c gvcrc.h $(HDRS)

gvcprn.obj: gvcprn.c $(HDRS)

gvctext.obj: gvctext.c $(HDRS)

winunzip.obj: winunzip.c wizdll.h
	$(COMPDIR)\$(CC) -c $(CFLAGS) $*.c

winsetup.res: winsetup.rc setup.h gvc$(LANGUAGE).h
	copy gvc$(LANGUAGE).h  gvclang.h
!if $(WIN32)
	$(RCOMP32) -i$(INCDIR) -r $*.rc
!else
	$(RCOMP) -i$(INCDIR) -r $*.rc
!endif

setupc.obj: setupc.c setup.h setupc.h gvcrc.h gvcbeta.h
	$(COMPDIR)\$(CC) -c $(CFLAGS) setupc.c

winsetup.obj: winsetup.c setup.h setup.c gvcrc.h gvcbeta.h gvc$(LANGUAGE).h 
	copy gvc$(LANGUAGE).h gvclang.h
	$(COMPDIR)\$(CC) -c $(CFLAGS) winsetup.c

winsetup.exe: winsetup.obj winsetup.res winsetup.def setupc.obj winunzip.obj gvcbeta.obj lib.rsp
	$(COMPDIR)\link $(DEBUGLINK) /DEF:winsetup.def /OUT:winsetup.exe winsetup.obj winunzip.obj setupc.obj gvcbeta.obj @lib.rsp winsetup.res

setp32de.res: winsetup.rc gvcde.h gvcver.h gvcrc.h setup.h
	copy gvcde.h gvclang.h
	$(RCOMP32) -i$(INCDIR) -r -fosetp32de.res winsetup
	-del gvclang.h

setp32de.dll: gvwlang.c setp32de.res setp32de.def
	$(COMPDIR)\$(CC) /c $(CFLAGS) /I$(INCDIR) gvwlang.c
	$(COMPDIR)\link $(DEBUGLINK) /DLL /DEF:setp32de.def /OUT:setp32de.dll gvwlang.obj setp32de.res

setp32fr.res: winsetup.rc gvcfr.h gvcver.h gvcrc.h setup.h
	copy gvcfr.h gvclang.h
	$(RCOMP32) -i$(INCDIR) -r -fosetp32fr.res winsetup
	-del gvclang.h

setp32fr.dll: gvwlang.c setp32fr.res setp32fr.def
	$(COMPDIR)\$(CC) /c $(CFLAGS) /I$(INCDIR) gvwlang.c
	$(COMPDIR)\link $(DEBUGLINK) /DLL /DEF:setp32fr.def /OUT:setp32fr.dll gvwlang.obj setp32fr.res

gvdoc.exe: gvdoc.c
	$(COMPDIR)\$(CCAUX) -I$(INCDIR) -L$(LIBDIR) gvdoc.c

gsview.txt: gvc$(LANGUAGE).txt gvdoc.exe
	gvdoc W gvc$(LANGUAGE).txt gsview.txt

doc2rtf.exe: doc2rtf.c
	$(COMPDIR)\$(CCAUX) -I$(INCDIR) -L$(LIBDIR) doc2rtf.c

doc2html.exe: doc2html.c
	$(COMPDIR)\$(CCAUX) -I$(INCDIR) -L$(LIBDIR) doc2html.c

gsview.dvi: gsview.tex titlepag.tex
	-latex gsview
	-latex gsview

gsview.tex: gsview.txt doc2tex.exe
	doc2tex gsview.txt gsview.tex

doc2tex.exe: doc2tex.c
	$(COMPDIR)\$(CCAUX) -I$(INCDIR) -L$(LIBDIR) doc2tex.c

gsviewen.hlp: gvdoc.exe doc2rtf.exe gvcen.txt gsviewen.hpj
	gvdoc W gvcen.txt gsviewen.txt
	doc2rtf gsviewen.txt gsviewen.rtf
	-del gsviewen.txt
	$(COMPDIR)\$(HC) gsviewen.hpj
	rename gsviewen.hlp gsviewen.hlp
	-del gsviewen.rtf

gsviewde.hlp: gvdoc.exe doc2rtf.exe gvcde.txt gsviewde.hpj
	gvdoc W gvcde.txt gsviewde.txt
	doc2rtf gsviewde.txt gsviewde.rtf
	-del gsviewde.txt
	$(COMPDIR)\$(HC) gsviewde.hpj
	rename gsviewde.hlp gsviewde.hlp
	-del gsviewde.rtf

gsviewfr.hlp: gvdoc.exe doc2rtf.exe gvcfr.txt gsviewfr.hpj
	gvdoc W gvcfr.txt gsviewfr.txt
	doc2rtf gsviewfr.txt gsviewfr.rtf
	-del gsviewfr.txt
	$(COMPDIR)\$(HC) gsviewfr.hpj
	rename gsviewfr.hlp gsviewfr.hlp
	-del gsviewfr.rtf

gsview.htm: doc2html.exe gsview.txt
	doc2html gsview.txt gsview.htm

gvwgs32.res: gvwgs.rc gvwgs.h $(ICONS) gvc$(LANGUAGE).h 
	copy gvc$(LANGUAGE).h gvclang.h
	$(RCOMP) -i$(INCDIR) -r -fogvwgs32.res gvwgs

# Intel
gvwgs32.exe: gvwgs.c gvwgs.h gvwgs32.res lib.rsp
	$(COMPDIR)\$(CC) -c $(CFLAGS) -I$(INCDIR) gvwgs.c
	$(COMPDIR)\link $(DEBUGLINK) /DEF:gvwgs32.def /OUT:gvwgs32.exe gvwgs.obj @lib.rsp gvwgs32.res

# DEC Alpha
gvwgsda.exe: gvwgs.c gvwgs.h gvwgs32.res lib.rsp
	$(COMPDIR)\$(CC) -c $(CFLAGS) -I$(INCDIR) gvwgs.c
	$(COMPDIR)\link $(DEBUGLINK) /DEF:gvwgs32.def /OUT:gvwgsda.exe gvwgs.obj @lib.rsp gvwgs32.res

gsv16spl.exe: gsv16spl.c gsv16spl.rc gsv16spl.def  gvc$(LANGUAGE).h
	echo Can't build gsv16spl.exe with MSVC++

strip: gsview$(WINEXT).exe
	echo Don't know how to strip EXE with MSVC++
#	$(COMPDIR)\tdstrp32 gsview32.exe

gsv$(GSVIEW_VERSION)src.zip:
	copy README.TXT ..
	copy LICENCE ..
	copy FILE_ID.DIZ ..
	cd ..
	-del epstool.zip
	zip -9 -@ epstool.zip < src\gvcliste.txt
	-del pstotext.zip
	zip -9 -@ pstotext.zip     < src\gvclistp.txt
	-del src.zip
	copy src\gvclists.txt gvclists.txt
	zip -9 -@ src.zip     < gvclists.txt
	-del gvclists.txt
	-del gsv$(GSVIEW_VERSION)src.zip
	zip -9 gsv$(GSVIEW_VERSION)src.zip epstool.zip pstotext.zip src.zip README.TXT FILE_ID.DIZ LICENCE
	-del README.TXT
	-del LICENCE
	-del FILE_ID.DIZ
	cd src
	
gsv$(GSVIEW_VERSION)w32.zip:
	copy README.TXT ..\README.TXT
	copy LICENCE ..\LICENCE
	copy FILE_ID.DIZ ..\FILE_ID.DIZ
	copy gsview32.exe ..\gsview32.exe
	copy binary\gvwin1.ico ..\gsview32.ico
#	$(COMPDIR)\tdstrp32 ..\gsview32.exe
	copy gsviewen.hlp ..\gsviewen.hlp
	copy gsviewde.hlp ..\gsviewde.hlp
	copy gsviewfr.hlp ..\gsviewfr.hlp
	copy gsvw32de.dll ..\gsvw32de.dll
	copy gsvw32fr.dll ..\gsvw32fr.dll
#	$(COMPDIR)\tdstrp32 ..\gsvw32de.dll
#	$(COMPDIR)\tdstrp32 ..\gsvw32fr.dll
	copy gsv16spl.exe ..\gsv16spl.exe
	copy gvwgs32.exe ..\gvwgs32.exe
	copy printer.ini ..\printer.ini
	copy winsetup.exe ..\setup.exe
	copy setp32de.dll ..\setp32de.dll
	copy setp32fr.dll ..\setp32fr.dll
#	$(COMPDIR)\tdstrp32 ..\setup.exe
#	$(COMPDIR)\tdstrp32 ..\setp32de.dll
#	$(COMPDIR)\tdstrp32 ..\setp32fr.dll
	cd ..
	-del win32.zip
	zip -9 -@ win32.zip < src\gvclist3.txt
	echo Redistribution of this Win32 GSview MUST be accompanied by the> README32.TXT
	echo sources in gsv$(GSVIEW_VERSION)src.zip to meet the licence requirements. >> README32.TXT
	-del gsv$(GSVIEW_VERSION)w32.zip
	zip -9 gsv$(GSVIEW_VERSION)w32.zip win32.zip setup.exe wizunz32.dll setp32de.dll setp32fr.dll
	zip -9 gsv$(GSVIEW_VERSION)w32.zip README32.TXT README.TXT FILE_ID.DIZ LICENCE
	-del README32.TXT
	-del README.TXT
	-del LICENCE
	-del FILE_ID.DIZ
	-del gsview32.exe
	-del gsview32.ico
	-del gsviewen.hlp
	-del gsviewde.hlp
	-del gsviewfr.hlp
	-del gsvw32de.dll
	-del gsvw32fr.dll
	-del gsv16spl.exe
	-del gvwgs32.exe
	-del printer.ini
	-del setup.exe
	-del setp32de.dll
	-del setp32fr.dll
	cd src

gsv$(GSVIEW_VERSION)wda.zip:
	copy README.TXT ..\README.TXT
	copy LICENCE ..\LICENCE
	copy FILE_ID.DIZ ..\FILE_ID.DIZ
	copy gsviewda.exe ..\gsviewda.exe
	copy binary\gvwin1.ico ..\gsview32.ico
	copy gsviewen.hlp ..\gsviewen.hlp
	copy gsviewde.hlp ..\gsviewde.hlp
	copy gsviewfr.hlp ..\gsviewfr.hlp
	copy gsvwdade.dll ..\gsvwdade.dll
	copy gsvwdafr.dll ..\gsvwdafr.dll
	copy gvwgsda.exe ..\gvwgsda.exe
	copy printer.ini ..\printer.ini
	copy winsetup.exe ..\setupda.exe
	copy setpdade.dll ..\setpdade.dll
	copy setpdafr.dll ..\setpdafr.dll
	cd ..
	-del win32da.zip
	zip -9 -@ win32da.zip < src\gvclist4.txt
	echo Redistribution of this Win32 GSview MUST be accompanied by the> README32.TXT
	echo sources in gsv$(GSVIEW_VERSION)src.zip to meet the licence requirements. >> README32.TXT
	-del gsv$(GSVIEW_VERSION)wda.zip
	zip -9 gsv$(GSVIEW_VERSION)wda.zip win32da.zip setupda.exe wizunzda.dll setpdade.dll setpdafr.dll
	zip -9 gsv$(GSVIEW_VERSION)wda.zip README32.TXT README.TXT FILE_ID.DIZ LICENCE
	-del README32.TXT
	-del README.TXT
	-del LICENCE
	-del FILE_ID.DIZ
	-del gsviewda.exe
	-del gsviewda.ico
	-del gsviewen.hlp
	-del gsviewde.hlp
	-del gsviewfr.hlp
	-del gsvwdade.dll
	-del gsvwdafr.dll
	-del gvwgsda.exe
	-del printer.ini
	-del setupda.exe
	-del setpdade.dll
	-del setpdafr.dll
	cd src
	
gsv$(GSVIEW_VERSION)w16.zip:
	echo Can't build Win16 GSview using MSVC++

prezip: gsv$(GSVIEW_VERSION)w$(WINEXT).zip

zip: prezip gsv$(GSVIEW_VERSION)src.zip
	copy README.TXT ..\README.TXT
	copy LICENCE ..\LICENCE
	copy FILE_ID.DIZ ..\FILE_ID.DIZ
	cd ..
	-del gsview$(GSVIEW_VERSION).zip
	rename gsv$(GSVIEW_VERSION)w16.zip gsv$(GSVIEW_VERSION)w16.zip 
	rename gsv$(GSVIEW_VERSION)os2.zip gsv$(GSVIEW_VERSION)os2.zip 
	zip -9 gsview$(GSVIEW_VERSION) gsv$(GSVIEW_VERSION)src.zip gsv$(GSVIEW_VERSION)os2.zip gsv$(GSVIEW_VERSION)w16.zip 
	zip -9 gsview$(GSVIEW_VERSION) gsv$(GSVIEW_VERSION)w32.zip gsview$(GSVIEW_VERSION)wda.zip
	zip -9 gsview$(GSVIEW_VERSION) README.TXT FILE_ID.DIZ LICENCE
	-del README.TXT
	-del LICENCE
	-del FILE_ID.DIZ
	cd src


language:
	del *.res
	del gvclang.h
	del gvclang.rc
	del gvwlang.rc

clean: language
	del gvwin.obj
	del gvwclip.obj
	del gvwdisp.obj
	del gvwdlg.obj
	del gvwdll.obj
	del gvweps.obj
	del gvwinit.obj
	del gvwmisc.obj
	del gvwprf.obj
	del gvwprn.obj
	del gvcbeta.obj
	del gvccmd.obj
	del gvcdisp.obj
	del gvceps.obj
	del gvcinit.obj
	del gvcmisc.obj
	del gvcdll.obj
	del gvcpdf.obj
	del gvcprn.obj
	del gvctext.obj
	del ps.obj
	del gsview16.map
	del gsview32.map
	del gsview16.sym
	del gsview32.sym
	del gsvw32de.map
	del gsvw32en.map
	del gsvw32fr.map
	del gsvw16de.map
	del gsvw16en.map
	del gsvw16fr.map
	del gvwlang.obj
	del gvwin16.res
	del gvwin32.res
	del gsviewen.txt
	del gsviewde.txt
	del gsviewfr.txt
	del gsviewen.rtf
	del gsviewde.rtf
	del gsviewfr.rtf
	del doc2html.obj
	del doc2html.exe
	del doc2rtf.obj
	del doc2rtf.exe
	del doc2tex.obj
	del doc2tex.exe
	del gvdoc.exe
	del gvdoc.obj
	del gsview.txt
	del gsview.aux
	del gsview.dvi
	del gsview.log
	del gsview.toc
	del gsview.tex
	del gsv16spl.obj
	del gsv16spl.res
	del gsv16spl.map
	del winsetup.obj
	del winsetup.res
	del winsetup.map
	del setp16fr.res
	del setp16fr.map
	del setp32fr.res
	del setp32fr.map
	del setp16de.res
	del setp16de.map
	del setp32de.res
	del setp32de.map
	del setupc.obj
	del winunzip.obj
	del gvwgs.obj
	del gvwgs16.res
	del gvwgs16.map
	del gvwgs32.res
	del gvwgs32.map

veryclean: clean
	del gsview$(WINEXT).exe
	del gsviewen.hlp
	del gsviewde.hlp
	del gsviewfr.hlp
	del gsvw$(WINEXT)de.dll
	del gsvw$(WINEXT)fr.dll
	del gsview.htm
	del gsv16spl.exe
	del gvwgs$(WINEXT).exe
	del winsetup.exe
	del setp$(WINEXT)de.dll
	del setp$(WINEXT)fr.dll
