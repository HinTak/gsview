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

# Makefile for GSview for Windows - GSVIEW.EXE or GSVIEW32.EXE
# using Borland C++ 3.1 or Borland C++ 4.5
# 'make -fgvwin.mak'
#

# Edit COMPBASE and WIN32 as required
COMPBASE = f:\bc45
# WIN32=1 for Win32s version
WIN32=0
# DEBUG=1 for Debugging options
DEBUG=0

# Shouldn't need editing below here
COMPDIR = $(COMPBASE)\bin
INCDIR = $(COMPBASE)\include
LIBDIR = $(COMPBASE)\lib
!if $(WIN32)
WINEXT=32
CCAUX = bcc
MODEL=32
CFLAGS=-v -W -w -H=gsview32.sym -I$(INCDIR)
CC = bcc32
!if $(DEBUG)
DEBUGLINK=-v
!endif
!else
WINEXT=
CCAUX = bcc
MODEL=m
CFLAGS=-v -m$(MODEL) -W -2 -h -w -H=gsview.sym -I$(INCDIR) $(OLD)
DEBUGLINK=/v
CC = bcc
# uncomment following line if using GSview with gs 2.6.1
OLD=-DGS261
!endif
OBJS=gvwin.obj gvwinit.obj gvwclip.obj gvwdisp.obj gvwdlg.obj\
  gvweps.obj gvwmisc.obj gvwpipe.obj gvwprf.obj gvwprn.obj\
  gvcmisc.obj gvcdisp.obj ps.obj gvccmd.obj gvcprn.obj\
  gvceps.obj gvctext.obj

all: gsview$(WINEXT).exe gsview.hlp doc2tex.exe

.c.obj:
	$(COMPDIR)\$(CC) -c $(CFLAGS) {$< }

	
gsview32.exe: $(OBJS) gvwin32.res gvwin32.def
	$(COMPDIR)\tlink32 -Tpe -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0w32 +
$(OBJS) +
,gsview32.exe,gsview32, +
$(LIBDIR)\import32 +
$(LIBDIR)\cw32, +
gvwin32.def, +
gvwin32.res
!

gsview.exe: $(OBJS) gvwin.res gvwin.def
	$(COMPDIR)\tlink /Twe /c /m /s /l $(DEBUGLINK) @&&!
$(LIBDIR)\c0w$(MODEL) +
$(OBJS) +
,gsview.exe,gsview, +
$(LIBDIR)\import +
$(LIBDIR)\mathw$(MODEL) +
$(LIBDIR)\cw$(MODEL), +
gvwin.def
!
	$(COMPDIR)\rlink -30 -t gvwin.res gsview.exe

gvwin32.res: gvwin.rc gvwin2.rc gvcrc.h $(ICONS)
	$(COMPDIR)\brcc32 -i$(INCDIR) -r -fogvwin32 gvwin

gvwin.res: gvwin.rc gvwin2.rc gvcrc.h $(ICONS)
	$(COMPDIR)\brcc -i$(INCDIR) $(OLD) -r gvwin


gvwin.obj: gvwin.c gvwin.h ps.h

gvwclip.obj: gvwclip.c gvwin.h ps.h

gvwdisp.obj: gvwdisp.c gvwin.h ps.h

gvwdlg.obj: gvwdlg.c gvwin.h ps.h gvcrc.h

gvweps.obj: gvweps.c gvceps.h gvwin.h ps.h

gvwinit.obj: gvwinit.c gvwin.h ps.h

gvwmisc.obj: gvwmisc.c gvwin.h ps.h

gvwpipe.obj: gvwpipe.c gvwin.h ps.h

gvwprn.obj: gvwprn.c gvwin.h ps.h

gvccmd.obj: gvccmd.c gvwin.h ps.h gvcrc.h

gvcdisp.obj: gvcdisp.c gvwin.h ps.h

ps.obj: ps.c gvwin.h ps.h

gvceps.obj: gvceps.c gvceps.h gvwin.h ps.h

gvcmisc.obj: gvcmisc.c gvwin.h ps.h gvcrc.h

gvcprn.obj: gvcprn.c gvwin.h ps.h

gvctext.obj: gvctext.c gvwin.h ps.h

gvdoc.exe: gvdoc.c
	$(COMPDIR)\$(CCAUX) -w-pro -I$(INCDIR) -L$(LIBDIR) gvdoc.c

gsview.doc: gvc.doc gvdoc.exe
	gvdoc W gvc.doc gsview.doc

doc2rtf.exe: doc2rtf.c
	$(COMPDIR)\$(CCAUX) -w-pro -I$(INCDIR) -L$(LIBDIR) doc2rtf.c

doc2html.exe: doc2html.c
	$(COMPDIR)\$(CCAUX) -w-pro -I$(INCDIR) -L$(LIBDIR) doc2html.c

gsview.dvi: gsview.tex titlepag.tex
	-latex gsview
	-latex gsview

gsview.tex: gsview.doc doc2tex.exe
	doc2tex gsview.doc gsview.tex

doc2tex.exe: doc2tex.c
	$(COMPDIR)\$(CCAUX) -w-pro -I$(INCDIR) -L$(LIBDIR) doc2tex.c

gsview.hlp: doc2rtf.exe gsview.doc gsview.hpj
	doc2rtf gsview.doc gsview.rtf
	$(COMPDIR)\hc31 gsview.hpj

gsview.htm: doc2html.exe gsview.doc
	doc2html gsview.doc gsview.htm

strip: gsview$(WINEXT).exe
!if $(WIN32)
	$(COMPDIR)\tdstrp32 gsview32.exe
!else
	$(COMPDIR)\tdstrip gsview.exe
!endif

prezip:
	copy gsview$(WINEXT).exe ..\gsview$(WINEXT).exe
!if $(WIN32)
	# do nothing, rely on  gsview32 being without symbol table
	# $(COMPDIR)\tdstrp32 ..\gsview32.exe
!else
	$(COMPDIR)\tdstrip ..\gsview.exe
!endif
	copy gsview.hlp ..\gsview.hlp
	copy README.GV ..\README.GV
	copy FILE_ID.DIZ ..\FILE_ID.DIZ
	copy LICENCE ..\LICENCE
	-del ..\gsview.zip

zip: prezip
	cd ..
	copy src\gvclist.doc gvclist.doc
	zip -@ gsview.zip < gvclist.doc
	del gvclist.doc
	cd src

clean:
	del gvwin.obj
	del gvwclip.obj
	del gvwdisp.obj
	del gvwdlg.obj
	del gvweps.obj
	del gvwinit.obj
	del gvwmisc.obj
	del gvwpipe.obj
	del gvwprn.obj
	del gvcmisc.obj
	del gvcdisp.obj
	del ps.obj
	del gvccmd.obj
	del gvceps.obj
	del gvwprf.obj
	del gvcprn.obj
	del gvctext.obj
	del gsview.map
	del gsview32.map
	del gsview.sym
	del gsview32.sym
	del gvwin.res
	del gvwin32.res
	del gsview.rtf
	del doc2html.obj
	del doc2html.exe
	del doc2rtf.obj
	del doc2rtf.exe
	del doc2tex.obj
	del doc2tex.exe
	del gvdoc.exe
	del gvdoc.obj
	del gsview.doc
	del gsview.aux
	del gsview.dvi
	del gsview.log
	del gsview.toc
	del gsview.tex

veryclean: clean
	del gsview$(WINEXT).exe
	del gsview.hlp
	del gsview.htm
