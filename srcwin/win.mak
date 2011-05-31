#  Copyright (C) 1993-2002, Ghostgum Software Pty Ltd.  All rights reserved.
#  
# This file is part of GSview.
#  
# This program is distributed with NO WARRANTY OF ANY KIND.  No author
# or distributor accepts any responsibility for the consequences of using it,
# or for whether it serves any particular purpose or works at all, unless he
# or she says so in writing.  Refer to the GSview Licence (the "Licence") 
# for full details.
#  
# Every copy of GSview must include a copy of the Licence, normally in a 
# plain ASCII text file named LICENCE.  The Licence grants you the right 
# to copy, modify and redistribute GSview, but only under certain conditions 
# described in the Licence.  Among other things, the Licence requires that 
# the copyright notice and this notice be preserved on all copies.

# Windows makefile for GSview

#################################################################
# Windows

# Edit VCVER and DEVBASE as required
!ifndef VCVER
VCVER=7
!endif

!if $(VCVER) <= 5
DEVBASE=E:\Program Files\devstudio
!endif
!if $(VCVER) == 6
DEVBASE=E:\Program Files\Microsoft Visual Studio
!endif
!if $(VCVER) == 7
DEVBASE=E:\Program Files\Microsoft Visual Studio .NET
!endif

# DEBUG=1 for Debugging options
DEBUG=1
# WIN32 is the default - don't change this
WIN32=1

# To disable print/convert, set VIEWONLY=1
!ifndef VIEWONLY
VIEWONLY=0
!endif

# Define the location of the WinZip self-extracting-archive-maker.
!ifndef WINZIPSE_XE
WINZIPSE_XE="C:\Program Files\WinZip Self-Extractor\WZIPSE32.EXE"
!endif

BINDIR=.\bin
OBJDIR=.\obj
SRCDIR=.\src
SRCWINDIR=.\srcwin
SRCOS2DIR=.\srcos2
SRCUNXDIR=.\srcunx


!if $(VCVER) <= 5
COMPBASE = $(DEVBASE)\vc
!endif
!if $(VCVER) == 6
COMPBASE = $(DEVBASE)\vc98
!endif
!if $(VCVER) == 7
COMPBASE = $(DEVBASE)\Vc7
PLATLIBDIR=$(COMPBASE)\PlatformSDK\lib
!endif

COMPDIR = $(COMPBASE)\bin
INCDIR = $(COMPBASE)\include
LIBDIR = $(COMPBASE)\lib
!ifndef PLATLIBDIR
PLATLIBDIR=$(LIBDIR)
!endif

!if $(WIN32)
CDEFS=-D_Windows -D__WIN32__ -I"$(INCDIR)"
WINEXT=32
CFLAGS=$(CDEFS) /MT /nologo
LINKMACHINE=IX86
!if $(DEBUG)
DEBUGLINK=/DEBUG
CDEBUG=/Zi
!endif
CCAUX = "$(COMPDIR)\cl" -I"$(INCDIR)"
MODEL=32
CC = "$(COMPDIR)\cl" $(CDEBUG)
LINK = "$(COMPDIR)\link"
!else
    echo Only Win32 is supported
!endif

CLFLAG=
RIFLAGS=-i"$(INCDIR)" -i"$(SRCDIR)" -i"$(SRCWINDIR)" -i"$(OBJDIR)"
!if $(VCVER) <= 5
HC="$(COMPDIR)\hcw" /C /E
RCOMP="$(DEVBASE)\sharedide\bin\rc" -D_MSC_VER $(CDEFS) $(RIFLAGS)
!endif
!if $(VCVER) == 6
HC="$(DEVBASE)\common\tools\hcw" /C /E
RCOMP="$(DEVBASE)\common\msdev98\bin\rc" -D_MSC_VER $(CDEFS) $(RIFLAGS)
!endif
!if $(VCVER) == 7
HC="$(DEVBASE)\Common7\Tools\hcw" /C /E
RCOMP="$(DEVBASE)\Vc7\bin\rc" -D_MSC_VER $(CDEFS) $(RIFLAGS)
!endif

!if $(VIEWONLY)
VIEWFLAGS=-DVIEWONLY -DPREREGISTER
!else
VIEWFLAGS=
!endif

COMP=$(CC) -I$(SRCDIR) -I$(SRCWINDIR) -I$(OBJDIR) $(CFLAGS) $(VIEWFLAGS)


NUL=
SRC=$(SRCDIR)\$(NUL)
SRCWIN=$(SRCWINDIR)\$(NUL)
SRCOS2=$(SRCOS2DIR)\$(NUL)
SRCUNX=$(SRCUNX)\$(NUL)
OD=$(OBJDIR)\$(NUL)
BD=$(BINDIR)\$(NUL)
OBJ=.obj
EXE=.exe
CO=-c

FE=-Fe
FO=-Fo
FEO=-Fe$(OD)
FOO=-Fo$(OD)

HDRSPLAT=$(SRCWIN)gvwin.h $(SRCWIN)gvwdib.h $(SRCWIN)gvwpdib.h $(SRCWIN)gvwgsver.h 

CP=copy
RM=del

# if you have a strict ANSI compiler, add -D__STDC__
EPSDEF=-I$(SRC) -D__WIN32__ -DEPSTOOL -DSTDIO
EPSLIBS="$(PLATLIBDIR)\advapi32.lib"
EPSOBJ2=$(OD)gvwgsver$(OBJ)


target: all

#################################################################
# Common

!include "$(SRC)common.mak"
!include "$(SRC)gvcver.mak"
DISTDIR=gsview-$(GSVIEW_DOT_VERSION)


#################################################################
# Windows files



OBJPLAT1=$(OD)gvwin$(OBJ) $(OD)gvwcp$(OBJ) $(OD)gvwdll$(OBJ) \
 $(OD)gvwdisp$(OBJ) $(OD)gvwdlg$(OBJ) $(OD)gvwinit$(OBJ) \
 $(OD)gvwdde$(OBJ) $(OD)gvwdde2$(OBJ) $(OD)gvwmisc$(OBJ) \
 $(OD)gvwreg$(OBJ) $(OD)gvwfile$(OBJ) $(OD)gvwgsver$(OBJ) \
 $(OD)gvwimg$(OBJ) 

OBJPLAT2=$(OD)gvwclip$(OBJ) $(OD)gvwedit$(OBJ) $(OD)gvweps$(OBJ) \
 $(OD)gvwdib$(OBJ) $(OD)gvwpdib$(OBJ) $(OD)gvwpgdi$(OBJ) \
 $(OD)gvwprn$(OBJ) $(OD)gvwmeas$(OBJ) $(OD)gvcmeas2$(OBJ) 

!if $(VIEWONLY)
OBJS=$(OBJCOM1) $(OBJPLAT1) # $(OD)viewonly$(OBJ) 
!else
OBJS=$(OBJCOM1) $(OBJCOM2) $(OBJPLAT1) $(OBJPLAT2)
!endif


ICONS=binary\gvwgs.ico binary\gvwinun.ico binary\gvwdoc.ico \
 binary\gvwin1.ico binary\gvwin2.ico binary\gvwin3.ico binary\gvwin4.ico


# Windows target

all: $(BD)gsview$(WINEXT).exe \
  $(BD)gsviewen.hlp \
  $(BD)gsvw$(WINEXT)de.dll $(BD)gsviewde.hlp $(BD)setp$(WINEXT)de.dll \
  $(BD)gsvw$(WINEXT)es.dll $(BD)gsviewes.hlp $(BD)setp$(WINEXT)es.dll \
  $(BD)gsvw$(WINEXT)fr.dll $(BD)gsviewfr.hlp $(BD)setp$(WINEXT)fr.dll \
  $(BD)gsvw$(WINEXT)gr.dll $(BD)gsviewgr.hlp $(BD)setp$(WINEXT)gr.dll \
  $(BD)gsvw$(WINEXT)it.dll $(BD)gsviewit.hlp $(BD)setp$(WINEXT)it.dll \
  $(BD)gsvw$(WINEXT)nl.dll $(BD)gsviewnl.hlp $(BD)setp$(WINEXT)nl.dll \
  $(BD)gsvw$(WINEXT)se.dll $(BD)gsviewse.hlp $(BD)setp$(WINEXT)se.dll \
  $(BD)gvwgs$(WINEXT).exe $(BD)setup.exe $(BD)uninstgs.exe \
  $(BD)epstool$(EXE) $(BD)gsprint.exe 

$(OD)lib.rsp: makefile
        echo "$(PLATLIBDIR)\shell32.lib" > $(OD)lib.rsp
        echo "$(PLATLIBDIR)\comdlg32.lib" >> $(OD)lib.rsp
        echo "$(PLATLIBDIR)\gdi32.lib" >> $(OD)lib.rsp
        echo "$(PLATLIBDIR)\user32.lib" >> $(OD)lib.rsp
        echo "$(PLATLIBDIR)\winspool.lib" >> $(OD)lib.rsp
        echo "$(PLATLIBDIR)\advapi32.lib" >> $(OD)lib.rsp
        echo "$(PLATLIBDIR)\ole32.lib" >> $(OD)lib.rsp
        echo "$(PLATLIBDIR)\uuid.lib" >> $(OD)lib.rsp
	echo /NODEFAULTLIB:LIBC.lib >> $(OD)lib.rsp
        echo "$(LIBDIR)\libcmt.lib" >> $(OD)lib.rsp

$(BD)gsview$(WINEXT).exe: $(OBJS) $(OD)gsvw$(WINEXT)en.res $(SRCWIN)gvwin$(WINEXT).def $(OD)lib.rsp
	echo $(OBJS) > $(OD)link.rsp
	$(LINK) $(DEBUGLINK) /DEF:$(SRCWIN)gvwin$(WINEXT).def /OUT:$(BD)gsview$(WINEXT).exe @$(OD)link.rsp @$(OD)lib.rsp $(OD)gsvw$(WINEXT)en.res


$(OD)gvwin$(OBJ): $(SRCWIN)gvwin.c $(HDRS)
	$(COMP) $(FOO)gvwin$(OBJ) $(CO) $(SRCWIN)gvwin.c

$(OD)gvwcp$(OBJ): $(SRCWIN)gvwcp.c $(HDRS)
	$(COMP) $(FOO)gvwcp$(OBJ) $(CO) $(SRCWIN)gvwcp.c

$(OD)gvwclip$(OBJ): $(SRCWIN)gvwclip.c $(HDRS)
	$(COMP) $(FOO)gvwclip$(OBJ) $(CO) $(SRCWIN)gvwclip.c

$(OD)gvwdde$(OBJ): $(SRCWIN)gvwdde.c $(HDRS)
	$(COMP) $(FOO)gvwdde$(OBJ) $(CO) $(SRCWIN)gvwdde.c

$(OD)gvwdde2$(OBJ): $(SRCWIN)gvwdde2.c $(HDRS)
	$(COMP) $(FOO)gvwdde2$(OBJ) $(CO) $(SRCWIN)gvwdde2.c

$(OD)gvwdisp$(OBJ): $(SRCWIN)gvwdisp.c $(HDRS)
	$(COMP) $(FOO)gvwdisp$(OBJ) $(CO) $(SRCWIN)gvwdisp.c

$(OD)gvwdlg$(OBJ): $(SRCWIN)gvwdlg.c $(HDRS)
	$(COMP) $(FOO)gvwdlg$(OBJ) $(CO) $(SRCWIN)gvwdlg.c

$(OD)gvwdll$(OBJ): $(SRCWIN)gvwdll.c $(HDRS)
	$(COMP) $(FOO)gvwdll$(OBJ) $(CO) $(SRCWIN)gvwdll.c

$(OD)gvwedit$(OBJ): $(SRCWIN)gvwedit.c $(HDRS)
	$(COMP) $(FOO)gvwedit$(OBJ) $(CO) $(SRCWIN)gvwedit.c

$(OD)gvweps$(OBJ): $(SRCWIN)gvweps.c $(SRC)gvceps.h $(HDRS)
	$(COMP) $(FOO)gvweps$(OBJ) $(CO) $(SRCWIN)gvweps.c

$(OD)gvwgsver$(OBJ): $(SRCWIN)gvwgsver.c $(HDRS)
	$(COMP) $(FOO)gvwgsver$(OBJ) $(CO) $(SRCWIN)gvwgsver.c

$(OD)gvwimg$(OBJ): $(SRCWIN)gvwimg.c $(HDRS)
	$(COMP) $(FOO)gvwimg$(OBJ) $(CO) $(SRCWIN)gvwimg.c

$(OD)gvwinit$(OBJ): $(SRCWIN)gvwinit.c $(HDRS)
	$(COMP) $(FOO)gvwinit$(OBJ) $(CO) $(SRCWIN)gvwinit.c

$(OD)gvwmeas$(OBJ): $(SRCWIN)gvwmeas.c $(HDRS)
	$(COMP) $(FOO)gvwmeas$(OBJ) $(CO) $(SRCWIN)gvwmeas.c

$(OD)gvwmisc$(OBJ): $(SRCWIN)gvwmisc.c $(HDRS)
	$(COMP) $(FOO)gvwmisc$(OBJ) $(CO) $(SRCWIN)gvwmisc.c

$(OD)gvwprn$(OBJ): $(SRCWIN)gvwprn.c $(HDRS)
	$(COMP) $(FOO)gvwprn$(OBJ) $(CO) $(SRCWIN)gvwprn.c

$(OD)gvwreg$(OBJ): $(SRCWIN)gvwreg.c $(HDRS)
	$(COMP) $(FOO)gvwreg$(OBJ) $(CO) $(SRCWIN)gvwreg.c

$(OD)gvwfile$(OBJ): $(SRCWIN)gvwfile.c $(SRC)gvcfile.h
	$(COMP) $(FOO)gvwfile$(OBJ) $(CO) $(SRCWIN)gvwfile.c

$(OD)gvwdib$(OBJ): $(SRCWIN)gvwdib.cpp $(HDRS)
	$(COMP) $(FOO)gvwdib$(OBJ) $(CO) $(SRCWIN)gvwdib.cpp

$(OD)gvwpdib$(OBJ): $(SRCWIN)gvwpdib.cpp $(HDRS)
	$(COMP) $(FOO)gvwpdib$(OBJ) $(CO) $(SRCWIN)gvwpdib.cpp

$(OD)gvwpgdi$(OBJ): $(SRCWIN)gvwpgdi.cpp $(HDRS)
	$(COMP) $(FOO)gvwpgdi$(OBJ) $(CO) $(SRCWIN)gvwpgdi.cpp

$(OD)viewonly$(OBJ): $(SRC)viewonly.c $(HDRS)
	$(COMP) $(FOO)viewonly$(OBJ) $(CO) $(SRC)viewonly.c

# Windows resources also include common resources so use gvwin1.rc not gvwin2.rc
$(OD)gsvw$(WINEXT)en.res: $(HDRS) $(SRCWIN)gvwin1.rc en\gvclang.h en\gvclang.rc en\gvwlang.rc
	$(RCOMP) -i"en" $(VIEWFLAGS) -fo$(OD)gsvw$(WINEXT)en.res $(SRCWIN)gvwin1

$(OD)gvwgs$(WINEXT).res: $(SRCWIN)gvwgs.rc $(SRCWIN)gvwgs.h $(ICONS) $(LANGUAGE)\gvclang.h 
	$(RCOMP) -i"$(LANGUAGE)" $(VIEWFLAGS) -r -fo$(OD)gvwgs$(WINEXT).res $(SRCWIN)gvwgs

$(BD)gvwgs$(WINEXT).exe: $(SRCWIN)gvwgs.c $(SRCWIN)gvwgs.h $(OD)gvwgs$(WINEXT).res $(OD)lib.rsp $(OD)cdll$(OBJ)
	$(COMP) $(FOO)gvwgs$(WINEXT)$(OBJ) $(CO) $(SRCWIN)gvwgs.c
	$(LINK) $(DEBUGLINK) /DEF:$(SRCWIN)gvwgs$(WINEXT).def /OUT:$(BD)gvwgs$(WINEXT).exe $(OD)gvwgs$(WINEXT)$(OBJ) $(OD)cdll.obj @$(OD)lib.rsp $(OD)gvwgs$(WINEXT).res


##########
# German

$(OD)gsvw$(WINEXT)de.res: $(HDRS) $(SRCWIN)gvwin2.rc de\gvclang.h de\gvclang.rc de\gvwlang.rc
	$(RCOMP) -i"de" -fo$(OD)gsvw$(WINEXT)de.res $(SRCWIN)gvwin2

$(BD)gsvw$(WINEXT)de.dll: $(OD)gsvw$(WINEXT)de.res de\gvwin32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:de\gvwin32.def /OUT:$(BD)gsvw$(WINEXT)de.dll $(OD)gsvw$(WINEXT)de.res

$(OD)setp$(WINEXT)de.res: $(SRCWIN)winsetup.rc de\gvclang.h $(SRC)gvcver.h $(SRC)gvcrc.h $(SRCWIN)winsetup.h
	$(RCOMP) -i"de" -r -fo$(OD)setp$(WINEXT)de.res $(SRCWIN)winsetup

$(BD)setp$(WINEXT)de.dll: $(OD)setp$(WINEXT)de.res de\setup32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:de\setup32.def /OUT:$(BD)setp$(WINEXT)de.dll $(OD)setp$(WINEXT)de.res

##########
# Spanish

$(OD)gsvw$(WINEXT)es.res: $(HDRS) $(SRCWIN)gvwin2.rc es\gvclang.h es\gvclang.rc es\gvwlang.rc
	$(RCOMP) -i"es" -fo$(OD)gsvw$(WINEXT)es.res $(SRCWIN)gvwin2

$(BD)gsvw$(WINEXT)es.dll: $(OD)gsvw$(WINEXT)es.res es\gvwin32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:es\gvwin32.def /OUT:$(BD)gsvw$(WINEXT)es.dll $(OD)gsvw$(WINEXT)es.res

$(OD)setp$(WINEXT)es.res: $(SRCWIN)winsetup.rc es\gvclang.h $(SRC)gvcver.h $(SRC)gvcrc.h $(SRCWIN)winsetup.h
	$(RCOMP) -i"es" -r -fo$(OD)setp$(WINEXT)es.res $(SRCWIN)winsetup

$(BD)setp$(WINEXT)es.dll: $(OD)setp$(WINEXT)es.res es\setup32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:es\setup32.def /OUT:$(BD)setp$(WINEXT)es.dll $(OD)setp$(WINEXT)es.res

##########
# French

$(OD)gsvw$(WINEXT)fr.res: $(HDRS) $(SRCWIN)gvwin2.rc fr\gvclang.h fr\gvclang.rc fr\gvwlang.rc
	$(RCOMP) -i"fr" -fo$(OD)gsvw$(WINEXT)fr.res $(SRCWIN)gvwin2

$(BD)gsvw$(WINEXT)fr.dll: $(OD)gsvw$(WINEXT)fr.res fr\gvwin32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:fr\gvwin32.def /OUT:$(BD)gsvw$(WINEXT)fr.dll $(OD)gsvw$(WINEXT)fr.res

$(OD)setp$(WINEXT)fr.res: $(SRCWIN)winsetup.rc fr\gvclang.h $(SRC)gvcver.h $(SRC)gvcrc.h $(SRCWIN)winsetup.h
	$(RCOMP) -i"fr" -r -fo$(OD)setp$(WINEXT)fr.res $(SRCWIN)winsetup

$(BD)setp$(WINEXT)fr.dll: $(OD)setp$(WINEXT)fr.res fr\setup32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:fr\setup32.def /OUT:$(BD)setp$(WINEXT)fr.dll $(OD)setp$(WINEXT)fr.res

##########
# Greek

$(OD)gsvw$(WINEXT)gr.res: $(HDRS) $(SRCWIN)gvwin2.rc gr\gvclang.h gr\gvclang.rc gr\gvwlang.rc
	$(RCOMP) -i"gr" -fo$(OD)gsvw$(WINEXT)gr.res -c1253 $(SRCWIN)gvwin2

$(BD)gsvw$(WINEXT)gr.dll: $(OD)gsvw$(WINEXT)gr.res gr\gvwin32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:gr\gvwin32.def /OUT:$(BD)gsvw$(WINEXT)gr.dll $(OD)gsvw$(WINEXT)gr.res

$(OD)setp$(WINEXT)gr.res: $(SRCWIN)winsetup.rc gr\gvclang.h $(SRC)gvcver.h $(SRC)gvcrc.h $(SRCWIN)winsetup.h
	$(RCOMP) -i"gr" -r -fo$(OD)setp$(WINEXT)gr.res -c1253 $(SRCWIN)winsetup

$(BD)setp$(WINEXT)gr.dll: $(OD)setp$(WINEXT)gr.res gr\setup32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:gr\setup32.def /OUT:$(BD)setp$(WINEXT)gr.dll $(OD)setp$(WINEXT)gr.res

##########
# Italian

$(OD)gsvw$(WINEXT)it.res: $(HDRS) $(SRCWIN)gvwin2.rc it\gvclang.h it\gvclang.rc it\gvwlang.rc
	$(RCOMP) -i"it" -fo$(OD)gsvw$(WINEXT)it.res $(SRCWIN)gvwin2

$(BD)gsvw$(WINEXT)it.dll: $(OD)gsvw$(WINEXT)it.res it\gvwin32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:it\gvwin32.def /OUT:$(BD)gsvw$(WINEXT)it.dll $(OD)gsvw$(WINEXT)it.res

$(OD)setp$(WINEXT)it.res: $(SRCWIN)winsetup.rc it\gvclang.h $(SRC)gvcver.h $(SRC)gvcrc.h $(SRCWIN)winsetup.h
	$(RCOMP) -i"it" -r -fo$(OD)setp$(WINEXT)it.res $(SRCWIN)winsetup

$(BD)setp$(WINEXT)it.dll: $(OD)setp$(WINEXT)it.res it\setup32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:it\setup32.def /OUT:$(BD)setp$(WINEXT)it.dll $(OD)setp$(WINEXT)it.res

##########
# Dutch

$(OD)gsvw$(WINEXT)nl.res: $(HDRS) $(SRCWIN)gvwin2.rc nl\gvclang.h nl\gvclang.rc nl\gvwlang.rc
	$(RCOMP) -i"nl" -fo$(OD)gsvw$(WINEXT)nl.res $(SRCWIN)gvwin2

$(BD)gsvw$(WINEXT)nl.dll: $(OD)gsvw$(WINEXT)nl.res nl\gvwin32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:nl\gvwin32.def /OUT:$(BD)gsvw$(WINEXT)nl.dll $(OD)gsvw$(WINEXT)nl.res

$(OD)setp$(WINEXT)nl.res: $(SRCWIN)winsetup.rc nl\gvclang.h $(SRC)gvcver.h $(SRC)gvcrc.h $(SRCWIN)winsetup.h
	$(RCOMP) -i"nl" -r -fo$(OD)setp$(WINEXT)nl.res $(SRCWIN)winsetup

$(BD)setp$(WINEXT)nl.dll: $(OD)setp$(WINEXT)nl.res nl\setup32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:nl\setup32.def /OUT:$(BD)setp$(WINEXT)nl.dll $(OD)setp$(WINEXT)nl.res

##########
# Swedish

$(OD)gsvw$(WINEXT)se.res: $(HDRS) $(SRCWIN)gvwin2.rc se\gvclang.h se\gvclang.rc se\gvwlang.rc
	$(RCOMP) -i"se" -fo$(OD)gsvw$(WINEXT)se.res $(SRCWIN)gvwin2

$(BD)gsvw$(WINEXT)se.dll: $(OD)gsvw$(WINEXT)se.res se\gvwin32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:se\gvwin32.def /OUT:$(BD)gsvw$(WINEXT)se.dll $(OD)gsvw$(WINEXT)se.res

$(OD)setp$(WINEXT)se.res: $(SRCWIN)winsetup.rc se\gvclang.h $(SRC)gvcver.h $(SRC)gvcrc.h $(SRCWIN)winsetup.h
	$(RCOMP) -i"se" -r -fo$(OD)setp$(WINEXT)se.res $(SRCWIN)winsetup

$(BD)setp$(WINEXT)se.dll: $(OD)setp$(WINEXT)se.res se\setup32.def
	$(LINK) /DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE) /DEF:se\setup32.def /OUT:$(BD)setp$(WINEXT)se.dll $(OD)setp$(WINEXT)se.res


##########
# Windows setup

$(OD)winsetup$(OBJ): $(SRCWIN)winsetup.cpp $(SRCWIN)winsetup.h $(SRC)gvcrc.h $(SRC)gvcbeta.h $(SRCWIN)dwinst.h $(LANGUAGE)\gvclang.h 
	$(COMP) -I$(LANGUAGE) $(FOO)winsetup$(OBJ) $(CO) $(SRCWIN)winsetup.cpp

$(OD)dwinst$(OBJ): $(SRCWIN)dwinst.cpp $(SRCWIN)dwinst.h
	$(COMP) $(FOO)dwinst$(OBJ) $(CO) $(SRCWIN)dwinst.cpp

$(OD)dwuninst$(OBJ): $(SRCWIN)dwuninst.cpp $(SRCWIN)dwuninst.h
	$(COMP) $(FOO)dwuninst$(OBJ) $(CO) $(SRCWIN)dwuninst.cpp

$(OD)winsetup.res: $(SRCWIN)winsetup.rc $(SRCWIN)winsetup.h $(LANGUAGE)\gvclang.h
	$(RCOMP) -i"$(LANGUAGE)" -r -fo$(OD)winsetup.res $(SRCWIN)winsetup.rc

$(BD)setup.exe: $(OD)winsetup.obj $(OD)winsetup.res $(SRCWIN)winsetup.def $(OD)dwinst.obj $(OD)gvcbetaa.obj $(OD)gvwgsver.obj $(OD)lib.rsp
	$(LINK) $(DEBUGLINK) /DEF:$(SRCWIN)winsetup.def /OUT:$(BD)setup.exe $(OD)winsetup.obj $(OD)dwinst.obj $(OD)gvcbetaa.obj $(OD)gvwgsver.obj @$(OD)lib.rsp $(OD)winsetup.res

$(OD)dwuninst.res: $(SRCWIN)dwuninst.rc $(SRCWIN)dwuninst.h
	$(RCOMP) -i"$(LANGUAGE)" -r -fo$(OD)dwuninst.res $(SRCWIN)dwuninst.rc

$(BD)uninstgs.exe: $(OD)dwuninst.obj $(SRCWIN)dwuninst.h $(OD)dwuninst.res $(SRCWIN)dwuninst.def
	$(LINK) $(DEBUGLINK) /DEF:$(SRCWIN)dwuninst.def /OUT:$(BD)uninstgs.exe $(OD)dwuninst.obj @$(OD)lib.rsp $(OD)dwuninst.res

##########
# Documentation

$(BD)gsviewen.hlp: $(GVDOC) $(DOC2RTF) en\gvclang.txt en\gsview.hpj
	$(CP) en\gsview.hpj $(OD)gsviewen.hpj
	$(GVDOC) W en\gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewen.hpj
	$(CP) $(OD)gsviewen.hlp $(BD)gsviewen.hlp

$(BD)gsviewde.hlp: $(GVDOC) $(DOC2RTF) de\gvclang.txt de\gsview.hpj
	$(CP) de\gsview.hpj $(OD)gsviewde.hpj
	$(GVDOC) W de\gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewde.hpj
	$(CP) $(OD)gsviewde.hlp $(BD)gsviewde.hlp

$(BD)gsviewes.hlp: $(GVDOC) $(DOC2RTF) es\gvclang.txt es\gsview.hpj
	$(CP) es\gsview.hpj $(OD)gsviewes.hpj
	$(GVDOC) W es\gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewes.hpj
	$(CP) $(OD)gsviewes.hlp $(BD)gsviewes.hlp

$(BD)gsviewfr.hlp: $(GVDOC) $(DOC2RTF) fr\gvclang.txt fr\gsview.hpj
	$(CP) fr\gsview.hpj $(OD)gsviewfr.hpj
	$(GVDOC) W fr\gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewfr.hpj
	$(CP) $(OD)gsviewfr.hlp $(BD)gsviewfr.hlp

$(BD)gsviewgr.hlp: $(GVDOC) $(DOC2RTF) gr\gvclang.txt gr\gsview.hpj
	$(CP) gr\gsview.hpj $(OD)gsviewgr.hpj
	$(GVDOC) W gr\gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewgr.hpj
	$(CP) $(OD)gsviewgr.hlp $(BD)gsviewgr.hlp

$(BD)gsviewit.hlp: $(GVDOC) $(DOC2RTF) it\gvclang.txt it\gsview.hpj
	$(CP) it\gsview.hpj $(OD)gsviewit.hpj
	$(GVDOC) W it\gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewit.hpj
	$(CP) $(OD)gsviewit.hlp $(BD)gsviewit.hlp

$(BD)gsviewnl.hlp: $(GVDOC) $(DOC2RTF) nl\gvclang.txt nl\gsview.hpj
	$(CP) nl\gsview.hpj $(OD)gsviewnl.hpj
	$(GVDOC) W nl\gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewnl.hpj
	$(CP) $(OD)gsviewnl.hlp $(BD)gsviewnl.hlp

$(BD)gsviewse.hlp: $(GVDOC) $(DOC2RTF) se\gvclang.txt se\gsview.hpj
	$(CP) se\gsview.hpj $(OD)gsviewse.hpj
	$(GVDOC) W se\gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewse.hpj
	$(CP) $(OD)gsviewse.hlp $(BD)gsviewse.hlp


html: $(DOC2HTML) $(CODEPAGE) $(GVDOC) en\gvclang.txt de\gvclang.txt es\gvclang.txt fr\gvclang.txt gr\gvclang.txt it\gvclang.txt nl\gvclang.txt se\gvclang.txt
	$(GVDOC) W en\gvclang.txt $(OD)gsview.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewen.htm
	$(RM) $(OD)gsview.txt
	$(GVDOC) W de\gvclang.txt $(OD)gsview.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewde.htm
	$(RM) $(OD)gsview.txt
	$(GVDOC) W es\gvclang.txt $(OD)gsview.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewes.htm
	$(RM) $(OD)gsview.txt
	$(GVDOC) W fr\gvclang.txt $(OD)gsview.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewfr.htm
	$(RM) $(OD)gsview.txt
	$(GVDOC) W gr\gvclang.txt $(OD)temp.txt
	$(CODEPAGE) 1253_8859-7 $(OD)temp.txt $(OD)gsview.txt
	$(RM) $(OD)temp.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewgr.htm ISO-8859-7
	$(RM) $(OD)gsview.txt
	$(GVDOC) W it\gvclang.txt $(OD)gsview.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewit.htm
	$(RM) $(OD)gsview.txt
	$(GVDOC) W nl\gvclang.txt $(OD)gsview.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewnl.htm
	$(RM) $(OD)gsview.txt
	$(GVDOC) W se\gvclang.txt $(OD)gsview.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewse.htm
	$(RM) $(OD)gsview.txt
	$(GVDOC) P en\gvclang.txt $(OD)gsview.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)os2help.htm
	$(RM) $(OD)gsview.txt

##########
# gsprint

GSPRINTOBJS=$(OD)gsprint$(OBJ) $(OD)gvwfile$(OBJ) $(OD)gvwdib$(OBJ) \
 $(OD)gvwpdib$(OBJ) $(OD)gvwgsver$(OBJ)

$(BD)gsprint.exe: $(GSPRINTOBJS) $(OD)lib.rsp
	$(LINK) $(DEBUGLINK) /SUBSYSTEM:CONSOLE /OUT:$(BD)gsprint.exe $(GSPRINTOBJS) @$(OD)lib.rsp

$(OD)gsprint$(OBJ): $(SRCWIN)gsprint.cpp $(SRC)gvcfile.h $(SRCWIN)gvwdib.h $(SRCWIN)gvwpdib.h $(SRC)gvcver.h $(OD)gsvver.h
	$(COMP) $(FOO)gsprint$(OBJ) $(CO) $(SRCWIN)gsprint.cpp

#################################################################
# Testing of DSC parser

dsctest: $(OD)dsctest$(EXE)

$(OD)dsctest$(EXE): $(SRC)dscutil.c $(OD)dscparse$(OBJ) $(HDRS)
	$(COMP) $(FEO)dsctest$(EXE) -DSTANDALONE $(SRC)dscutil.c $(OD)dscparse$(OBJ)

#################################################################
# Cleanup and distribution

clean: commonclean
	-$(RM) $(OD)*.res
	-$(RM) $(OD)*.hpj
	-$(RM) $(OD)*.hlp
	-$(RM) $(OD)*.rsp
	-$(RM) $(OD)gsview.txt
	-$(RM) $(OD)gsview.rtf
	-$(RM) $(OD)files32.txt
	-$(RM) $(OD)viewlist.txt
	-$(RM) $(OD)viewlist.tmp

distclean:
	-$(RM) $(BD)*.lib
	-$(RM) $(BD)*.lib
	-$(RM) $(BD)*.pdb
	-$(RM) $(BD)*.ilk
	-$(RM) $(BD)*.exp

veryclean: clean distclean
	-$(RM) $(BD)*.exe
	-$(RM) $(BD)*.dll
	-$(RM) $(BD)*.hlp


srczip:
	-del $(OD)src.txt
	-del gsv$(GSVIEW_VERSION)src.zip
	echo $(DISTDIR)/Readme.htm > $(OD)src.txt
	echo $(DISTDIR)/epstool.htm >> $(OD)src.txt
	echo $(DISTDIR)/gsprint.htm >> $(OD)src.txt
	echo $(DISTDIR)/gsview.css >> $(OD)src.txt
	echo $(DISTDIR)/LICENCE >> $(OD)src.txt
	echo $(DISTDIR)/FILE_ID.DIZ >> $(OD)src.txt
	echo $(DISTDIR)/cdorder.txt >> $(OD)src.txt
	echo $(DISTDIR)/regorder.txt >> $(OD)src.txt
	echo $(DISTDIR)/src >> $(OD)src.txt
 	echo $(DISTDIR)/srcwin >> $(OD)src.txt
	echo $(DISTDIR)/srcos2 >> $(OD)src.txt
	echo $(DISTDIR)/srcunx >> $(OD)src.txt
	echo $(DISTDIR)/binary >> $(OD)src.txt
	echo $(DISTDIR)/en >> $(OD)src.txt
	echo $(DISTDIR)/de >> $(OD)src.txt
	echo $(DISTDIR)/es >> $(OD)src.txt
	echo $(DISTDIR)/fr >> $(OD)src.txt
	echo $(DISTDIR)/gr >> $(OD)src.txt
	echo $(DISTDIR)/it >> $(OD)src.txt
	echo $(DISTDIR)/nl >> $(OD)src.txt
	echo $(DISTDIR)/se >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/bundle.c >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/bundle.h >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/descrip.mms >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/main.c >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/Makefile >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/mkbundle.c >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/mkrch.c >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/ocr.ps >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/pstotext.1 >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/pstotext.txt >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/pstotext.hlp >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/pstotxt1.def >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/pstotxt1.mak >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/pstotxt2.def >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/pstotxt2.mak >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/pstotxt3.def >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/pstotxt3.mak >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/pstotxtd.c >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/pstotxtm.mak >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/pstotxtv.mak >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/ptotdll.h >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/ptotdll.c >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/rot270.ps >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/rot90.ps >> $(OD)src.txt
	echo $(DISTDIR)/pstotext/vms.h >> $(OD)src.txt
	cd ..
	zip -9 -r -@ $(DISTDIR)\gsv$(GSVIEW_VERSION)src.zip < $(DISTDIR)\$(OD)src.txt
	cd $(DISTDIR)
	$(RM) $(OD)src.txt

srctar:
	-del gsview-$(GSVIEW_DOT_VERSION)_src.zip
	echo $(DISTDIR)/Readme.htm > $(OD)src_tar.txt
	echo $(DISTDIR)/epstool.htm >> $(OD)src_tar.txt
	echo $(DISTDIR)/gsview.css >> $(OD)src_tar.txt
	echo $(DISTDIR)/cdorder.txt >> $(OD)src_tar.txt
	echo $(DISTDIR)/regorder.txt >> $(OD)src_tar.txt
	echo $(DISTDIR)/LICENCE >> $(OD)src_tar.txt
	echo $(DISTDIR)/FILE_ID.DIZ >> $(OD)src_tar.txt
	echo $(DISTDIR)/src >> $(OD)src_tar.txt
	echo $(DISTDIR)/srcunx >> $(OD)src_tar.txt
	echo $(DISTDIR)/en >> $(OD)src_tar.txt
	echo $(DISTDIR)/de >> $(OD)src_tar.txt
	echo $(DISTDIR)/es >> $(OD)src_tar.txt
	echo $(DISTDIR)/fr >> $(OD)src_tar.txt
	echo $(DISTDIR)/gr >> $(OD)src_tar.txt
	echo $(DISTDIR)/it >> $(OD)src_tar.txt
	echo $(DISTDIR)/nl >> $(OD)src_tar.txt
	echo $(DISTDIR)/se >> $(OD)src_tar.txt
	cd ..
	zip -r -ll -@ $(DISTDIR)\gsview-$(GSVIEW_DOT_VERSION)_src.zip < $(DISTDIR)\$(OD)src_tar.txt
	zip $(DISTDIR)\gsview-$(GSVIEW_DOT_VERSION)_src.zip $(DISTDIR)\binary\gsview48.png
	cd $(DISTDIR)
	echo Copy gsview-$(GSVIEW_DOT_VERSION)_src.zip to Unix, unzip then tar and gzip.

viewonlydist:
	-mkdir dist
	-mkdir dist\gsview
	copy FILE_ID.DIZ dist\FILE_ID.DIZ
	copy $(SRC)refresh.htm dist\Readme.htm
	copy LICENCE dist\LICENCE
	copy LICENCE dist\gsview\LICENCE
	copy Readme.htm dist\gsview\Readme.htm
	copy gsview.css dist\gsview\gsview.css
	copy cdorder.txt dist\gsview\cdorder.txt
	copy regorder.txt dist\gsview\regorder.txt
	copy $(BD)gsview32.exe dist\gsview\gsview32.exe
	copy binary\gvwin4.ico dist\gsview\gsview32.ico
	copy $(BD)gsviewen.hlp dist\gsview\gsviewen.hlp
	copy $(SRC)printer.ini dist\gsview\printer.ini
	copy NUL > dist\gsview\gsview32.ini
	copy $(BD)uninstgs.exe dist\gsview\uninstgs.exe
	copy $(BD)setup.exe dist\setup.exe
	copy gsview\zlib32.dll dist\gsview\zlib32.dll
	copy gsview\libbz2.dll dist\gsview\libbz2.dll
	echo gsview\gsview32.exe> $(OD)viewlist.txt
	echo gsview\gsview32.ico>> $(OD)viewlist.txt
	echo gsview\uninstgs.exe>> $(OD)viewlist.txt
	echo gsview\printer.ini>> $(OD)viewlist.txt
	echo gsview\gsview32.ini>> $(OD)viewlist.txt
	echo gsview\zlib32.dll>> $(OD)viewlist.txt
	echo gsview\libbz2.dll>> $(OD)viewlist.txt
	echo gsview\Readme.htm>> $(OD)viewlist.txt
	echo gsview\gsview.css>> $(OD)viewlist.txt
	echo gsview\cdorder.txt>> $(OD)viewlist.txt
	echo gsview\regorder.txt>> $(OD)viewlist.txt
	echo gsview\LICENCE>> $(OD)viewlist.txt
	echo GSview $(GSVIEW_DOT_VERSION)> $(OD)\viewlist.tmp
	echo gsview>> $(OD)viewlist.tmp
	copy $(OD)viewlist.tmp+$(OD)viewlist.txt dist\filelist.txt
 	copy $(OD)viewlist.txt $(OD)files32.txt
	echo Readme.htm>> $(OD)files32.txt
	echo FILE_ID.DIZ>> $(OD)files32.txt
	echo filelist.txt>> $(OD)files32.txt
	echo setup.exe>> $(OD)files32.txt
	cd dist
	-del ..\gsv$(GSVIEW_VERSION)w32.zip
	-del ..\gsv$(GSVIEW_VERSION)w32.exe
	zip -9 -@ ..\gsv$(GSVIEW_VERSION)w32.zip < ..\$(OD)files32.txt
	cd ..
	cd dist
	echo -win32 -setup > setup.rsp
	echo -st "GSview $(GSVIEW_DOT_VERSION) for Win32" >> setup.rsp
	echo -i gsview\gsview32.ico >> setup.rsp
	echo -a about.txt >> setup.rsp
	echo -t dialog.txt >> setup.rsp
	echo -c .\setup.exe >> setup.rsp
	echo GSview is Copyright (C) 2002 Ghostgum Software Pty Ltd. > about.txt
	echo See licence in gsview\LICENCE >> about.txt
	echo This installs GSview $(GSVIEW_DOT_VERSION) for Win32. > dialog.txt
	echo GSview uses Ghostscript to display, print and convert PostScript and PDF files. >> dialog.txt
	$(WINZIPSE_XE) ..\gsv$(GSVIEW_VERSION)w32 @setup.rsp
# Don't delete temporary files, because make continues
# before these files are used.
#	-del setup.rsp 
#	-del about.txt
#	-del dialog.txt
	cd ..

distcopy:
	-mkdir dist
	-mkdir dist\gsview
	-mkdir dist\pstotext
	copy FILE_ID.DIZ dist\FILE_ID.DIZ
	copy $(SRC)refresh.htm dist\Readme.htm
	copy LICENCE dist\LICENCE
	copy LICENCE dist\gsview\LICENCE
	copy Readme.htm dist\gsview\Readme.htm
	copy gsview.css dist\gsview\gsview.css
	copy cdorder.txt dist\gsview\cdorder.txt
	copy regorder.txt dist\gsview\regorder.txt
	copy $(BD)gsview32.exe dist\gsview\gsview32.exe
	copy binary\gvwin4.ico dist\gsview\gsview32.ico
	copy $(BD)gsviewen.hlp dist\gsview\gsviewen.hlp
	copy $(BD)gsviewde.hlp dist\gsview\gsviewde.hlp
	copy $(BD)gsviewes.hlp dist\gsview\gsviewes.hlp
	copy $(BD)gsviewfr.hlp dist\gsview\gsviewfr.hlp
	copy $(BD)gsviewgr.hlp dist\gsview\gsviewgr.hlp
	copy $(BD)gsviewit.hlp dist\gsview\gsviewit.hlp
	copy $(BD)gsviewnl.hlp dist\gsview\gsviewnl.hlp
	copy $(BD)gsviewse.hlp dist\gsview\gsviewse.hlp
	copy $(BD)gsvw32de.dll dist\gsview\gsvw32de.dll
	copy $(BD)gsvw32es.dll dist\gsview\gsvw32es.dll
	copy $(BD)gsvw32fr.dll dist\gsview\gsvw32fr.dll
	copy $(BD)gsvw32gr.dll dist\gsview\gsvw32gr.dll
	copy $(BD)gsvw32it.dll dist\gsview\gsvw32it.dll
	copy $(BD)gsvw32nl.dll dist\gsview\gsvw32nl.dll
	copy $(BD)gsvw32se.dll dist\gsview\gsvw32se.dll
	copy $(BD)gvwgs32.exe dist\gsview\gvwgs32.exe
	copy $(SRC)printer.ini dist\gsview\printer.ini
	copy NUL > dist\gsview\gsview32.ini
	copy $(BD)uninstgs.exe dist\gsview\uninstgs.exe
	copy $(BD)setup.exe dist\setup.exe
	copy $(BD)setp32de.dll dist\setp32de.dll
	copy $(BD)setp32es.dll dist\setp32es.dll
	copy $(BD)setp32fr.dll dist\setp32fr.dll
	copy $(BD)setp32gr.dll dist\setp32gr.dll
	copy $(BD)setp32it.dll dist\setp32it.dll
	copy $(BD)setp32nl.dll dist\setp32nl.dll
	copy $(BD)setp32se.dll dist\setp32se.dll
	copy gsprint.htm dist\gsview\gsprint.htm
	copy $(BD)gsprint.exe dist\gsview\gsprint.exe
	copy epstool.htm dist\gsview\epstool.htm
	copy $(BD)epstool.exe dist\gsview\epstool.exe
	copy gsview\gsv16spl.exe dist\gsview\gsv16spl.exe
	copy gsview\zlib32.dll dist\gsview\zlib32.dll
	copy gsview\libbz2.dll dist\gsview\libbz2.dll
	copy pstotext\pstotext.1 dist\pstotext\pstotext.1
	copy pstotext\pstotext.txt dist\pstotext\pstotext.txt
	copy pstotext\pstotxt3.dll dist\pstotext\pstotxt3.dll
	copy pstotext\pstotxt3.exe dist\pstotext\pstotxt3.exe
	echo GSview $(GSVIEW_DOT_VERSION)> $(OD)filelist.tmp
	echo gsview>> $(OD)filelist.tmp
	copy $(OD)filelist.tmp+$(SRCWIN)distlist.txt dist\filelist.txt
	del $(OD)filelist.tmp

$(OD)files32.txt: $(SRCWIN)distlist.txt $(SRCWIN)win.mak makefile
 	copy $(SRCWIN)distlist.txt $(OD)files32.txt
	echo Readme.htm >> $(OD)files32.txt
	echo FILE_ID.DIZ >> $(OD)files32.txt
	echo filelist.txt >> $(OD)files32.txt
	echo setup.exe >> $(OD)files32.txt
	echo setp32de.dll >> $(OD)files32.txt
	echo setp32es.dll >> $(OD)files32.txt
	echo setp32fr.dll >> $(OD)files32.txt
	echo setp32gr.dll >> $(OD)files32.txt
	echo setp32it.dll >> $(OD)files32.txt
	echo setp32nl.dll >> $(OD)files32.txt
	echo setp32se.dll >> $(OD)files32.txt

gsv$(GSVIEW_VERSION)w32.zip: distcopy $(OD)files32.txt
	-del gsv$(GSVIEW_VERSION)w32.zip 
	cd dist
	zip -9 -@ ..\gsv$(GSVIEW_VERSION)w32.zip < ..\$(OD)files32.txt
	cd ..



# Now convert to a self extracting archive.
# This involves making a few temporary files.
gsv$(GSVIEW_VERSION)w32.exe: distcopy gsv$(GSVIEW_VERSION)w32.zip
	cd dist
	echo -win32 -setup > setup.rsp
	echo -st "GSview $(GSVIEW_DOT_VERSION) for Win32" >> setup.rsp
	echo -i gsview\gsview32.ico >> setup.rsp
	echo -a about.txt >> setup.rsp
	echo -t dialog.txt >> setup.rsp
	echo -c .\setup.exe >> setup.rsp
	echo GSview is Copyright (C) 2002 Ghostgum Software Pty Ltd. > about.txt
	echo See licence in gsview\LICENCE >> about.txt
	echo This installs GSview $(GSVIEW_DOT_VERSION) for Win32. > dialog.txt
	echo GSview uses Ghostscript to display, print and convert PostScript and PDF files. >> dialog.txt
	$(WINZIPSE_XE) ..\gsv$(GSVIEW_VERSION)w32 @setup.rsp
# Don't delete temporary files, because make continues
# before these files are used.
#	-del setup.rsp 
#	-del about.txt
#	-del dialog.txt
	cd ..

zip: gsv$(GSVIEW_VERSION)w32.exe

