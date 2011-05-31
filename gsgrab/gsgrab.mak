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

# Makefile for GSgrab
# Russell Lang 1994-04-10
# using Borland C++ 3.1 or Borland C++ 4.5
# 'make -fgsgrab.mak'
#
COMPBASE = f:\bc45
#COMPBASE = d:\borlandc
#
COMPDIR = $(COMPBASE)\bin
INCDIR = $(COMPBASE)\include
LIBDIR = $(COMPBASE)\lib
MODEL=m
CFLAGS=-v -m$(MODEL) -W -2 -h -w -I$(INCDIR)
OBJS=gsgrab.obj gsginit.obj gsgdlg.obj

all: gsgrab.exe gsgrab.hlp

.c.obj:
	$(COMPDIR)\bcc -c $(CFLAGS) {$< }
	
gsgrab.exe: $(OBJS) gsgrab.res gsgrab.def
	$(COMPDIR)\tlink /Twe /c /m /s /v /l @&&!
$(LIBDIR)\c0w$(MODEL) +
$(OBJS) +
,gsgrab.exe,gsgrab, +
$(LIBDIR)\import +
$(LIBDIR)\mathw$(MODEL) +
$(LIBDIR)\cw$(MODEL), +
gsgrab.def
!
	$(COMPDIR)\rlink -30 -t gsgrab.res gsgrab.exe

gsgrab.res: gsgrab.rc gsgrab.h
	$(COMPDIR)\brcc -i$(INCDIR) -r gsgrab

gsgrab.obj: gsgrab.c gsgrab.h

doc2rtf.exe: ..\src\doc2rtf.c
	$(COMPDIR)\bcc -w-pro -I$(INCDIR) -L$(LIBDIR) ..\src\doc2rtf.c

gsgrab.hlp: doc2rtf.exe gsgrab.doc gsgrab.hpj
	doc2rtf gsgrab.doc gsgrab.rtf
	$(COMPDIR)\hc31 gsgrab.hpj

clean:
	del gsgrab.obj
	del gsginit.obj
	del gsgdlg.obj
	del gsgrab.map
	del gsgrab.sym
	del gsgrab.res
	del gsgrab.rws
	del gsgrab.rtf
	del gsgrab.rws
	del gsgrab.trw
	del doc2rtf.obj
	del doc2rtf.exe
	tdstrip gsgrab.exe

veryclean: clean
	del gsgrab.map
	del gsgrab.exe
	del gsgrab.hlp
