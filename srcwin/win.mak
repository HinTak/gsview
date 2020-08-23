#  Copyright (C) 1993-2011, Ghostgum Software Pty Ltd.  All rights reserved.
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

# This makefile is for Microsoft Visual Studio 2019 Community Edition running on Windows 10 64-bit.
# To build for 64-bit, Start Menu, Visual Studio 2019, x64 Native Tools Command Prompt for VS 2019
# nmake WIN64=1

# To build for 32-bit, Start Menu, Visual Studio 2019, x64_x86 Cross Tools Command Prompt for VS 2019
# nmake WIN32=1

# DEBUG=1 for Debugging options
!ifndef DEBUG
DEBUG=0
!endif

!ifdef WIN64
WIN32=0
WIN64=1
!else ifdef WIN32
WIN32=1
WIN64=0
!else
!error Must specify WIN64=1 or WIN32=1
!endif

DEVBASE=$(VCINSTALLDIR)
COMMONBASE=$(WindowsSdkDir)

NUL=
D=\$(NUL)

# To disable print/convert, set VIEWONLY=1
!ifndef VIEWONLY
VIEWONLY=0
!endif

BINDIR=.\bin
OBJDIR=.\obj
SRCDIR=.\src
SRCWINDIR=.\srcwin
SRCOS2DIR=.\srcos2
SRCUNXDIR=.\srcunx


PLATLIBDIRD =
LIBDIRD =
INCDIR = $(INCLUDE)

# MSVC 8 (2005) and later warns about deprecated common functions like fopen.
VC8WARN=/wd4996

!if $(WIN32)
CDEFS=-D_Windows -D__WIN32__  -I"$(INCDIR)"
WINEXT=32
CFLAGS=$(CDEFS) /MT /nologo $(VC8WARN)
LINKMACHINE=x86
!if $(DEBUG)
DEBUGLINK=/DEBUG
CDEBUG=/Zi
!endif
MODEL=32
CCAUX = "cl" -I"$(INCDIR)" $(VC8WARN) /nologo
CC = "cl" $(CDEBUG)
CPP = "cl" $(CDEBUG)
LINK = "link"

!else if $(WIN64)
CDEFS=-D_Windows -D__WIN32__  -I"$(INCDIR)"
WINEXT=64
CFLAGS=$(CDEFS) /MT /nologo $(VC8WARN)
LINKMACHINE=x64
!if $(DEBUG)
DEBUGLINK=/DEBUG
CDEBUG=/Zi
!endif
MODEL=64
CCAUX = "cl" -I"$(INCDIR)" $(VC8WARN) /nologo
CC = "cl" $(CDEBUG)
CPP = "cl" $(CDEBUG)
LINK = "link"

!else
!message Only Win32 or Win64 is supported
!endif

CLFLAG=
RIPATH=-i
ROFILE=-fo
RLANG=-c
RIFLAGS=$(RIPATH)"$(INCDIR)" $(RIPATH)"$(SRCDIR)" $(RIPATH)"$(SRCWINDIR)" $(RIPATH)"$(OBJDIR)"

# Help Compiler is no longer included in the SDK.
# Search on the Internet for hcw403_setup.zip
HC="$(PROGRAMFILES)\Help Workshop\hcw" /C /E
HTMLHELP="C:\Program Files (x86)\HTML Help Workshop\hhc.exe"
RCOMP="rc" -D_MSC_VER $(CDEFS) $(RIFLAGS)


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
