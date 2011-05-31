#  Copyright (C) 1993, 1994, 1995, Russell Lang.  All rights reserved.
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
# using Borland C++ 4.5
# 'make -fgvwin.mak'
#

# Edit COMPBASE and WIN32 as required
COMPBASE = c:\bc45
# DEBUG=1 for Debugging options
DEBUG=1

# Shouldn't need editing below here
COMPDIR = $(COMPBASE)\bin
INCDIR = $(COMPBASE)\include
LIBDIR = $(COMPBASE)\lib
WINEXT=32
CCAUX = bcc
MODEL=32
CFLAGS=-v -W -w -tWM -H=gsview32.sym -I$(INCDIR)
CC = bcc32
!if $(DEBUG)
DEBUGLINK=-v
!endif

OBJS=gvwin.obj gvwdll.obj gvwdisp.obj gvwdlg.obj\
  gvwclip.obj gvweps.obj gvwmisc.obj gvwprf.obj gvwprn.obj\
  gvcmisc.obj gvcdisp.obj ps.obj gvccmd.obj gvcprn.obj\
  gvceps.obj gvcinit.obj gvctext.obj\
  gvcdll.obj gvcpdf.obj gvwinit.obj gvcbeta.obj

HDRS=gvwin.h ps.h gvcfn.h gvcver.h

all: gsview$(WINEXT).exe gsview.hlp gvwgs.exe gsv16spl.exe doc2tex.exe winsetup.exe

.c.obj:
	$(COMPDIR)\$(CC) -c $(CFLAGS) {$< }

	
# change cw32mt to cw32 for single thread
gsview32.exe: $(OBJS) gvwin32.res gvwin32.def
	$(COMPDIR)\tlink32 -Tpe -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0w32 +
$(OBJS) +
,gsview32.exe,gsview32, +
$(LIBDIR)\import32 +
$(LIBDIR)\cw32mt, +
gvwin32.def, +
gvwin32.res
!

gvwin32.res: gvwin.rc gvwin2.rc gvcrc.h $(ICONS)
	$(COMPDIR)\brcc32 -i$(INCDIR) -r -fogvwin32 gvwin

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
	$(COMPDIR)\$(CC) -W -c -v -I$(INCDIR) $*.c

gvwtxt.obj: gvwtxt.c $(HDRS)
	$(COMPDIR)\$(CC) -W -c -v -I$(INCDIR) $*.c

winsetup.res: winsetup.rc setup.h
	$(COMPDIR)\brcc32 -i$(INCDIR) -r $*.rc

winsetup.obj: winsetup.c setup.h setup.c
	$(COMPDIR)\$(CC) -W -c -v -I$(INCDIR) winsetup.c

winsetup.exe: winsetup.obj winsetup.res winsetup.def winunzip.obj gvwtxt.obj gvcbeta.obj
	$(COMPDIR)\tlink32 -Tpe -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0w32 +
winsetup.obj +
winunzip.obj gvwtxt.obj gvcbeta.obj +
,winsetup.exe,winsetup, +
$(LIBDIR)\import32 +
$(LIBDIR)\cw32, +
winsetup.def,
winsetup.res
!
# tlink32 is buggy and won't bind winsetup.res so try again...
	$(COMPDIR)\brc32 winsetup.res winsetup.exe

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

gvwgs.res: gvwgs.rc gvwgs.h $(ICONS)
	$(COMPDIR)\brcc32 -i$(INCDIR) -r gvwgs

gvwgs.exe: gvwgs.c gvwgs.h gvwgs.res
	$(COMPDIR)\bcc32 -c -v -tWM -WE -w -I$(INCDIR) gvwgs.c
	$(COMPDIR)\tlink32 -Tpe -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0w32 +
gvwgs.obj +
,gvwgs.exe,gvwgs, +
$(LIBDIR)\import32 +
$(LIBDIR)\cw32mt, +
gvwgs.def, +
gvwgs.res
!

gsv16spl.exe: gsv16spl.c gsv16spl.rc gsv16spl.def
	$(COMPDIR)\$(CCAUX) -W -ms -c -v -I$(INCDIR) $*.c
	$(COMPDIR)\brcc -i$(INCDIR) -r $*.rc
	$(COMPDIR)\tlink /Twe /c /m /s /l $(DEBUGLINK) @&&!
$(LIBDIR)\c0ws +
$*.obj +
,$*.exe,$*, +
$(LIBDIR)\import +
$(LIBDIR)\mathws +
$(LIBDIR)\cws, +
$*.def
!
	$(COMPDIR)\rlink -30 -t $*.res $*.exe

strip: gsview$(WINEXT).exe
	$(COMPDIR)\tdstrp32 gsview32.exe

prezip:
	copy gsview$(WINEXT).exe ..\gsview$(WINEXT).exe
	copy binary\gvwin1.ico ..\gsview32.ico
	# used to do nothing, rely on  gsview32 being without symbol table
	$(COMPDIR)\tdstrp32 ..\gsview32.exe
	copy gsview.hlp ..\gsview.hlp
	copy gsv16spl.exe ..\gsv16spl.exe
	copy gvwgs.exe ..\gvwgs.exe
	copy printer.ini ..\printer.ini
	copy winsetup.exe ..\setup.exe
	$(COMPDIR)\tdstrp32 ..\setup.exe
	# change OS/2 EXEs to lower case
	cd ..
	rename gvpm.exe gvpm.exe
	rename gvpm.eas gvpm.eas
	rename gvpgs.exe gvpgs.exe
	rename os2setup.exe os2setup.exe
	rename os2unzip.exe os2unzip.exe
	rename gvpm.hlp gvpm.hlp
	cd src
	copy README.TXT ..\README.TXT
	copy FILE_ID.DIZ ..\FILE_ID.DIZ
	copy LICENCE ..\LICENCE
	-del ..\epstool.zip
	-del ..\gsview.zip
	-del ..\pstotext.zip
	-del ..\src.zip
	-del ..\gsviewXX.zip

zip: prezip
	cd ..
	copy src\gvcliste.txt gvcliste.txt
	copy src\gvclistp.txt gvclistp.txt
	copy src\gvclists.txt gvclists.txt
	copy src\gvclist.txt gvclist.txt
	zip -9 -@ epstool.zip < gvcliste.txt
	zip -9 -@ pstotext.zip     < gvclistp.txt
	zip -9 -@ src.zip     < gvclists.txt
	del gvcliste.txt
	del gvclistp.txt
	del gvclists.txt
	cd ..
	zip -9 -@ gsview\gsview.zip  < gsview\gvclist.txt
	cd gsview
	del gvclist.txt
	zip -9 gsviewXX.zip gsview.zip README.TXT FILE_ID.DIZ LICENCE os2setup.exe os2unzip.exe setup.exe wizunz32.dll
	cd src


clean:
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
	del gsv16spl.obj
	del gsv16spl.res
	del gsv16spl.map
	del winsetup.obj
	del winsetup.res
	del winsetup.map
	del winunzip.obj
	del gvwtxt.obj
	del gvwgs.obj
	del gvwgs.res
	del gvwgs.map

veryclean: clean
	del gsview$(WINEXT).exe
	del gsview.hlp
	del gsview.htm
	del gsv16spl.exe
	del winsetup.exe
	del gvwgs.exe
