#  Copyright (C) 1993-2003, Ghostgum Software Pty Ltd.  All rights reserved.
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
VCVER=71
!endif

!ifndef DEVBASE
!if $(VCVER) <= 5
DEVBASE=C:\Program Files\devstudio
!endif
!if $(VCVER) == 6
DEVBASE=C:\Program Files\Microsoft Visual Studio
!endif
!if $(VCVER) == 7
DEVBASE=C:\Program Files\Microsoft Visual Studio .NET
!endif
!if $(VCVER) == 71
DEVBASE=C:\Program Files\Microsoft Visual Studio .NET 2003
!endif
!endif

# DEBUG=1 for Debugging options
DEBUG=1
# WIN32 is the default - don't change this
WIN32=1

NUL=
D=\$(NUL)

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
!if (($(VCVER) == 7) || ($(VCVER) == 71))
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
CPP = "$(COMPDIR)\cl" $(CDEBUG)
LINK = "$(COMPDIR)\link"
!else
    echo Only Win32 is supported
!endif

CLFLAG=
RIPATH=-i
ROFILE=-fo
RLANG=-c
RIFLAGS=$(RIPATH)"$(INCDIR)" $(RIPATH)"$(SRCDIR)" $(RIPATH)"$(SRCWINDIR)" $(RIPATH)"$(OBJDIR)"
!if $(VCVER) <= 5
HC="$(COMPDIR)\hcw" /C /E
RCOMP="$(DEVBASE)\sharedide\bin\rc" -D_MSC_VER $(CDEFS) $(RIFLAGS)
!endif
!if $(VCVER) == 6
HC="$(DEVBASE)\common\tools\hcw" /C /E
RCOMP="$(DEVBASE)\common\msdev98\bin\rc" -D_MSC_VER $(CDEFS) $(RIFLAGS)
!endif
!if (($(VCVER) == 7) || ($(VCVER) == 71))
HC="$(DEVBASE)\Common7\Tools\hcw" /C /E
RCOMP="$(DEVBASE)\Vc7\bin\rc" -D_MSC_VER $(CDEFS) $(RIFLAGS)
!endif

!if $(VIEWONLY)
VIEWFLAGS=-DVIEWONLY -DPREREGISTER
!else
VIEWFLAGS=
!endif

COMP=$(CC) -I$(SRCDIR) -I$(SRCWINDIR) -I$(OBJDIR) $(CFLAGS) $(VIEWFLAGS)
CPPCOMP=$(CC) -I$(SRCDIR) -I$(SRCWINDIR) -I$(OBJDIR) $(CFLAGS) $(VIEWFLAGS)


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

LDEF=/DEF:
LOUT=/OUT:
LDLL=/DLL /NODEFAULTLIB /NOENTRY /MACHINE:$(LINKMACHINE)
LCONSOLE=/SUBSYSTEM:CONSOLE 
LGUI=/SUBSYSTEM:WINDOWS
LIBRSP=@$(OD)lib.rsp

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

!include "$(SRCWIN)wincom.mak"

$(BD)gsview$(WINEXT).exe: $(OBJS) $(OD)gsvw$(WINEXT)en.res $(SRCWIN)gvwin$(WINEXT).def $(OD)lib.rsp
	$(LINK) $(DEBUGLINK) $(LGUI) $(LDEF)$(SRCWIN)gvwin$(WINEXT).def $(LOUT)$(BD)gsview$(WINEXT).exe $(OBJS) $(OD)gsvw$(WINEXT)en.res $(LIBRSP)


#################################################################
