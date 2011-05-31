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

# Makefile for GSview for Windows - GSVIEW32.EXE
# using Microsoft Visual C++ 5.0
#  copy gvwinvc.mak makefile
#  make
#
# Path to Microsoft Visual C++ must NOT include spaces

# Edit VCVER and DEVBASE as required
VCVER=5
#DEVBASE = f:\progra~1\micros~1
DEVBASE = c:\devstudio
# DEBUG=1 for Debugging options
DEBUG=1
# WIN32 is the default - don't change this
WIN32=1
# USEBC=1 if you have Borland C++ 4.5, 0 otherwise
# Borland C++ is needed to build the 16-bit gsv16spl.exe
# needed for Win32s.
USEBC=0

# ALPHA=1 for DEC Alpha
!if "$(PROCESSOR_ARCHITECTURE)"=="ALPHA"
ALPHA=1
!else
ALPHA=0
!endif

# Shouldn't need editing below here
!if $(VCVER) <= 5
COMPBASE = $(DEVBASE)\vc
!else
COMPBASE = $(DEVBASE)\vc98
!endif
COMPDIR = $(COMPBASE)\bin
INCDIR = $(COMPBASE)\include
LIBDIR = $(COMPBASE)\lib
!if $(WIN32)
CDEFS=-D_Windows -D__WIN32__ -I"$(INCDIR)"
!if $(ALPHA)
WINEXT=da
CFLAGS= $(CDEFS) /MT /nologo -DDECALPHA /QA21164
LINKMACHINE=ALPHA
!else
WINEXT=32
CFLAGS=$(CDEFS) /MT /nologo
LINKMACHINE=IX86
!endif
!if $(DEBUG)
DEBUGLINK=/DEBUG
CDEBUG=/Zi
!endif
CCAUX = "$(COMPDIR)\cl"
MODEL=32
CC = "$(COMPDIR)\cl" $(CDEBUG)
LINK = "$(COMPDIR)\link"
!else
    echo Win16 not supported with MSVC++
!endif
CLFLAG=
!if $(VCVER) <= 5
HC="$(COMPDIR)\hcw" /C /E
RCOMP="$(DEVBASE)\sharedide\bin\rc" -D_MSC_VER $(CDEFS)
!else
HC="$(DEVBASE)\common\tools\hcw" /C /E
RCOMP="$(DEVBASE)\common\msdev98\bin\rc" -D_MSC_VER $(CDEFS)
!endif

all: gsview$(WINEXT).exe\
  gsviewen.hlp\
  gsvw$(WINEXT)de.dll gsviewde.hlp setp$(WINEXT)de.dll\
  gsvw$(WINEXT)es.dll gsviewes.hlp setp$(WINEXT)es.dll\
  gsvw$(WINEXT)fr.dll gsviewfr.hlp setp$(WINEXT)fr.dll\
  gsvw$(WINEXT)it.dll gsviewit.hlp setp$(WINEXT)it.dll\
  gvwgs$(WINEXT).exe winsetup.exe uninstgs.exe\
  gsprint.exe ..\epstool\epstool.exe\
  gsv16spl.exe

.cpp.obj:
	$(CC) -c $(CFLAGS) $< 

!include "gvcver.mak"
!include "gvwinc.mak"

lib.rsp: makefile
        echo "$(LIBDIR)\shell32.lib" > lib.rsp
        echo "$(LIBDIR)\comdlg32.lib" >> lib.rsp
        echo "$(LIBDIR)\gdi32.lib" >> lib.rsp
        echo "$(LIBDIR)\user32.lib" >> lib.rsp
        echo "$(LIBDIR)\winspool.lib" >> lib.rsp
        echo "$(LIBDIR)\advapi32.lib" >> lib.rsp
        echo "$(LIBDIR)\ole32.lib" >> lib.rsp
        echo "$(LIBDIR)\uuid.lib" >> lib.rsp
	echo /NODEFAULTLIB:LIBC.lib >> lib.rsp
        echo "$(LIBDIR)\libcmt.lib" >> lib.rsp

..\epstool\epstool.exe: ..\epstool\epstool.cpp ..\epstool\epstool.h $(HDRS)
	cd ..\epstool
	nmake -f makefile.msc
	cd ..\src

gsprint.exe: gsprint.obj gvwfile.obj gvwdib.obj gvwpdib.obj gvwgsver.obj lib.rsp
	$(LINK) $(DEBUGLINK) /SUBSYSTEM:CONSOLE /OUT:gsprint.exe gsprint.obj gvwfile.obj gvwdib.obj gvwpdib.obj gvwgsver.obj @lib.rsp
	
# change cw32mt to cw32 for single thread
gsview$(WINEXT).exe: $(OBJS) gvwin$(WINEXT).res gvwin$(WINEXT).def lib.rsp
	echo $(OBJ1) > link.rsp
	echo $(OBJ2) >> link.rsp
	echo $(OBJ3) >> link.rsp
	echo $(OBJ4) >> link.rsp
	echo $(OBJ5) >> link.rsp
	echo $(OBJ6) >> link.rsp
	$(LINK) $(DEBUGLINK) /DEF:gvwin$(WINEXT).def /OUT:gsview$(WINEXT).exe @link.rsp @lib.rsp gvwin$(WINEXT).res


gsvw$(WINEXT)de.dll: gsvw$(WINEXT)de.res de\gvwin32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:de\gvwin32.def /OUT:gsvw$(WINEXT)de.dll gsvw$(WINEXT)de.res


gsvw$(WINEXT)es.dll: gsvw$(WINEXT)es.res es\gvwin32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:es\gvwin32.def /OUT:gsvw$(WINEXT)es.dll gsvw$(WINEXT)es.res

gsvw$(WINEXT)fr.dll: gsvw$(WINEXT)fr.res fr\gvwin32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:fr\gvwin32.def /OUT:gsvw$(WINEXT)fr.dll gsvw$(WINEXT)fr.res

gsvw$(WINEXT)it.dll: gsvw$(WINEXT)it.res it\gvwin32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:it\gvwin32.def /OUT:gsvw$(WINEXT)it.dll gsvw$(WINEXT)it.res

uninstgs.exe: dwuninst.obj dwuninst.h dwuninst.res dwuninst.def
	$(LINK) $(DEBUGLINK) /DEF:dwuninst.def /OUT:uninstgs.exe dwuninst.obj @lib.rsp dwuninst.res

winsetup.exe: winsetup.obj winsetup.res winsetup.def dwinst.obj gvcbeta.obj lib.rsp
	$(LINK) $(DEBUGLINK) /DEF:winsetup.def /OUT:winsetup.exe winsetup.obj dwinst.obj gvcbeta.obj @lib.rsp winsetup.res

setp$(WINEXT)de.dll: setp$(WINEXT)de.res de\setup32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:de\setup32.def /OUT:setp$(WINEXT)de.dll setp$(WINEXT)de.res

setp$(WINEXT)es.dll: setp$(WINEXT)es.res es\setup32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:es\setup32.def /OUT:setp$(WINEXT)es.dll setp$(WINEXT)es.res

setp$(WINEXT)fr.dll: setp$(WINEXT)fr.res fr\setup32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:fr\setup32.def /OUT:setp$(WINEXT)fr.dll setp$(WINEXT)fr.res

setp$(WINEXT)it.dll: setp$(WINEXT)it.res de\setup32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:it\setup32.def /OUT:setp$(WINEXT)it.dll setp$(WINEXT)it.res

ungsview.exe: ungsview.obj ungsview.res ungsview.def
	$(LINK) $(DEBUGLINK) /DEF:ungsview.def /OUT:ungsview.exe ungsview.obj @lib.rsp ungsview.res


# Intel
gvwgs32.exe: gvwgs.cpp gvwgs.h gvwgs32.res lib.rsp
	$(CC) -c $(CFLAGS) -I"$(INCDIR)" gvwgs.cpp
	$(LINK) $(DEBUGLINK) /DEF:gvwgs32.def /OUT:gvwgs32.exe gvwgs.obj @lib.rsp gvwgs32.res

# DEC Alpha
gvwgsda.exe: gvwgs.cpp gvwgs.h gvwgsda.res lib.rsp
	$(CC) -c $(CFLAGS) -I"$(INCDIR)" gvwgs.cpp
	$(LINK) $(DEBUGLINK) /DEF:gvwgs32.def /OUT:gvwgsda.exe gvwgs.obj @lib.rsp gvwgs32.res

gsv16spl.exe: gsv16spl.c gsv16spl.rc gsv16spl.def  $(LANGUAGE)\gvclang.h
	echo Can't build gsv16spl.exe with MSVC++
!if $(USEBC)==1
	-make -fgvwin.mak gsv16spl.exe
!endif

strip: gsview$(WINEXT).exe
	echo Don't know how to strip EXE with MSVC++
#	$(COMPDIR)\tdstrp32 gsview32.exe

gsv$(GSVIEW_VERSION)wda.zip:
	copy Readme.htm ..\Readme.htm
	copy LICENCE ..\LICENCE
	copy FILE_ID.DIZ ..\FILE_ID.DIZ
	copy gsviewda.exe ..\gsviewda.exe
	copy binary\gvwin1.ico ..\gsview32.ico
	copy gsviewen.hlp ..\gsviewen.hlp
	copy gsviewde.hlp ..\gsviewde.hlp
	copy gsviewes.hlp ..\gsviewes.hlp
	copy gsviewfr.hlp ..\gsviewfr.hlp
	copy gsviewit.hlp ..\gsviewit.hlp
	copy gsvwdade.dll ..\gsvwdade.dll
	copy gsvwdaes.dll ..\gsvwdaes.dll
	copy gsvwdafr.dll ..\gsvwdafr.dll
	copy gsvwdait.dll ..\gsvwdait.dll
	copy gvwgsda.exe ..\gvwgsda.exe
	copy printer.ini ..\printer.ini
	copy winsetup.exe ..\setupda.exe
	copy setpdade.dll ..\setpdade.dll
	copy setpdait.dll ..\setpdait.dll
	cd ..
	-del win32da.zip
	zip -9 -@ win32da.zip < src\gvclist4.txt
	echo Redistribution of this Win32 GSview MUST be accompanied by the> README32.TXT
	echo sources in gsv$(GSVIEW_VERSION)src.zip to meet the licence requirements. >> README32.TXT
	-del gsv$(GSVIEW_VERSION)wda.zip
	zip -9 gsv$(GSVIEW_VERSION)wda.zip win32da.zip setupda.exe wizunzda.dll setpdade.dll setpdaes.dll setpdafr.dll setpdait.dll
	zip -9 gsv$(GSVIEW_VERSION)wda.zip README32.TXT Readme.htm FILE_ID.DIZ LICENCE
	-del README32.TXT
	-del Readme.htm
	-del LICENCE
	-del FILE_ID.DIZ
	-del gsviewda.exe
	-del gsviewda.ico
	-del gsviewen.hlp
	-del gsviewde.hlp
	-del gsviewes.hlp
	-del gsviewfr.hlp
	-del gsviewit.hlp
	-del gsvwdade.dll
	-del gsvwdaes.dll
	-del gsvwdafr.dll
	-del gsvwdait.dll
	-del gvwgsda.exe
	-del printer.ini
	-del setupda.exe
	-del setpdade.dll
	-del setpdaes.dll
	-del setpdafr.dll
	-del setpdait.dll
	cd src
	
gsv$(GSVIEW_VERSION)w16.zip:
	echo Can't build Win16 GSview using MSVC++

