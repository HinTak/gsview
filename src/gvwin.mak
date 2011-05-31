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
# WIN32 is the default
WIN32=1
# Language is English (en) or Deutsch (de)
# This only applies to the utilties, not GSview itself.
LANGUAGE=en

# Shouldn't need editing below here
COMPDIR = $(COMPBASE)\bin
INCDIR = $(COMPBASE)\include
LIBDIR = $(COMPBASE)\lib
!if $(WIN32)
WINEXT=32
CCAUX = bcc
MODEL=32
CFLAGS=-v -WE -w -tWM -H=gsview32.sym -I$(INCDIR)
CC = bcc32
!if $(DEBUG)
DEBUGLINK=-v
!endif
!else
WINEXT=16
CCAUX = bcc
MODEL=l
CFLAGS=-v -m$(MODEL) -zEGV_FAR_DATA -Ff=256 -W -2 -h -w -H=gsview16.sym -I$(INCDIR) $(OLD)
DEBUGLINK=/v
CC = bcc
!endif

OBJS=gvwin.obj gvwdll.obj gvwdisp.obj gvwdlg.obj\
  gvwclip.obj gvweps.obj gvwmisc.obj gvwprf.obj gvwprn.obj\
  gvcmisc.obj gvcdisp.obj ps.obj gvccmd.obj gvcprn.obj\
  gvceps.obj gvcinit.obj gvctext.obj\
  gvcdll.obj gvcpdf.obj gvwinit.obj gvcbeta.obj

HDRS=gvwin.h ps.h gvcfn.h gvcver.h

all: gsview$(WINEXT).exe\
  gsvw$(WINEXT)en.dll gsviewen.hlp\
  gsvw$(WINEXT)de.dll gsviewde.hlp\
  gvwgs$(WINEXT).exe gsv16spl.exe winsetup.exe

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

gsview16.exe: $(OBJS) gvwin16.res gvwin16.def
	$(COMPDIR)\tlink -Twe -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0w$(MODEL) +
$(OBJS) +
,gsview16.exe,gsview16, +
$(LIBDIR)\import +
$(LIBDIR)\mathw$(MODEL) +
$(LIBDIR)\cw$(MODEL), +
gvwin16.def, +
gvwin16.res
!

gsvw32en.res: gvwin2.rc gvcen.h gvcen.rc gvwen.rc
	copy gvcen.h  gvclang.h
	copy gvcen.rc gvclang.rc
	copy gvwen.rc gvwlang.rc
	$(COMPDIR)\brcc32 -i$(INCDIR) -r -fogsvw32en gvwin2
	-del gvclang.rc
	-del gvwlang.rc

gsvw32en.dll: gvwlang.c gsvw32en.res gsvw32en.def
	$(COMPDIR)\bcc32 -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink32 -Tpd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0d32 +
gvwlang.obj +
,gsvw32en.dll,gsvw32en, +
$(LIBDIR)\import32 +
$(LIBDIR)\cw32mt, +
gsvw32en.def, +
gsvw32en.res
!


gsvw16en.res: gvwin2.rc gvcen.h gvcen.rc gvwen.rc
	copy gvcen.h  gvclang.h
	copy gvcen.rc gvclang.rc
	copy gvwen.rc gvwlang.rc
	$(COMPDIR)\brcc -i$(INCDIR) -r -fogsvw16en gvwin2
	-del gvclang.rc
	-del gvwlang.rc

gsvw16en.dll: gvwlang.c gsvw16en.res gsvw16en.def
	$(COMPDIR)\$(CC) -ml -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink -Twd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0dl +
gvwlang.obj +
,gsvw16en.dll,gsvw16en, +
$(LIBDIR)\import +
$(LIBDIR)\cwl, +
gsvw16en.def, +
gsvw16en.res
!

gsvw32de.res: gvwin2.rc gvcde.h gvcde.rc gvwde.rc
	copy gvcde.h  gvclang.h
	copy gvcde.rc gvclang.rc
	copy gvwde.rc gvwlang.rc
	$(COMPDIR)\brcc32 -i$(INCDIR) -r -fogsvw32de gvwin2
	-del gvclang.rc
	-del gvwlang.rc

gsvw32de.dll: gvwlang.c gsvw32de.res gsvw32de.def
	$(COMPDIR)\bcc32 -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink32 -Tpd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0d32 +
gvwlang.obj +
,gsvw32de.dll,gsvw32de, +
$(LIBDIR)\import32 +
$(LIBDIR)\cw32mt, +
gsvw32de.def, +
gsvw32de.res
!

gsvw16de.res: gvwin2.rc gvcde.h gvcde.rc gvwde.rc
	copy gvcde.h  gvclang.h
	copy gvcde.rc gvclang.rc
	copy gvwde.rc gvwlang.rc
	$(COMPDIR)\brcc -i$(INCDIR) -r -fogsvw16de gvwin2
	-del gvclang.rc
	-del gvwlang.rc

gsvw16de.dll: gvwlang.c gsvw16de.res gsvw16de.def
	$(COMPDIR)\$(CC) -ml -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink -Twd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0dl +
gvwlang.obj +
,gsvw16de.dll,gsvw16de, +
$(LIBDIR)\import +
$(LIBDIR)\cwl, +
gsvw16de.def, +
gsvw16de.res
!


gvwin32.res: gvwin1.rc
	$(COMPDIR)\brcc32 -i$(INCDIR) -r -fogvwin32 gvwin1

gvwin16.res: gvwin1.rc
	$(COMPDIR)\brcc -i$(INCDIR) -r -fogvwin16 gvwin1

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

gvwtxt.obj: gvwtxt.c $(HDRS)
	$(COMPDIR)\$(CC) -c $(CFLAGS) $*.c

winsetup.res: winsetup.rc setup.h gvc$(LANGUAGE).h
	copy gvc$(LANGUAGE).h  gvclang.h
!if $(WIN32)
	$(COMPDIR)\brcc32 -i$(INCDIR) -r $*.rc
!else
	$(COMPDIR)\brcc -i$(INCDIR) -r $*.rc
!endif

winsetup.obj: winsetup.c setup.h setup.c gvcrc.h gvcbeta.h gvc$(LANGUAGE).h 
	copy gvc$(LANGUAGE).h gvclang.h
	$(COMPDIR)\$(CC) -c $(CFLAGS) winsetup.c

winsetup.exe: winsetup.obj winsetup.res winsetup.def winunzip.obj gvwtxt.obj gvcbeta.obj
!if $(WIN32)
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
!else
	$(COMPDIR)\tlink -Twe -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0w$(MODEL) +
winsetup.obj +
winunzip.obj gvwtxt.obj gvcbeta.obj +
,winsetup.exe,winsetup, +
$(LIBDIR)\import +
$(LIBDIR)\mathw$(MODEL) +
$(LIBDIR)\cw$(MODEL), +
winsetup.def, +
winsetup.res
!
!endif

gvdoc.exe: gvdoc.c
	$(COMPDIR)\$(CCAUX) -w-pro -I$(INCDIR) -L$(LIBDIR) gvdoc.c

gsview.txt: gvc$(LANGUAGE).txt gvdoc.exe
	gvdoc W gvc$(LANGUAGE).txt gsview.txt

doc2rtf.exe: doc2rtf.c
	$(COMPDIR)\$(CCAUX) -w-pro -I$(INCDIR) -L$(LIBDIR) doc2rtf.c

doc2html.exe: doc2html.c
	$(COMPDIR)\$(CCAUX) -w-pro -I$(INCDIR) -L$(LIBDIR) doc2html.c

gsview.dvi: gsview.tex titlepag.tex
	-latex gsview
	-latex gsview

gsview.tex: gsview.txt doc2tex.exe
	doc2tex gsview.txt gsview.tex

doc2tex.exe: doc2tex.c
	$(COMPDIR)\$(CCAUX) -w-pro -I$(INCDIR) -L$(LIBDIR) doc2tex.c

gsviewen.hlp: gvdoc.exe doc2rtf.exe gvcen.txt gsviewen.hpj
	gvdoc W gvcen.txt gsviewen.txt
	doc2rtf gsviewen.txt gsviewen.rtf
	-del gsviewen.txt
	$(COMPDIR)\hc31 gsviewen.hpj
	rename gsviewen.hlp gsviewen.hlp
	-del gsviewen.rtf

gsviewde.hlp: gvdoc.exe doc2rtf.exe gvcde.txt gsviewde.hpj
	gvdoc W gvcde.txt gsviewde.txt
	doc2rtf gsviewde.txt gsviewde.rtf
	-del gsviewde.txt
	$(COMPDIR)\hc31 gsviewde.hpj
	rename gsviewde.hlp gsviewde.hlp
	-del gsviewde.rtf

gsview.htm: doc2html.exe gsview.txt
	doc2html gsview.txt gsview.htm

gvwgs32.res: gvwgs.rc gvwgs.h $(ICONS) gvc$(LANGUAGE).h 
	copy gvc$(LANGUAGE).h gvclang.h
	$(COMPDIR)\brcc32 -i$(INCDIR) -r -fogvwgs32 gvwgs

gvwgs16.res: gvwgs.rc gvwgs.h $(ICONS) gvc$(LANGUAGE).h 
	copy gvc$(LANGUAGE).h gvclang.h
	$(COMPDIR)\brcc -i$(INCDIR) -r -fogvwgs16 gvwgs

gvwgs32.exe: gvwgs.c gvwgs.h gvwgs32.res
	$(COMPDIR)\bcc32 -c -v -tWM -WE -w -I$(INCDIR) gvwgs.c
	$(COMPDIR)\tlink32 -Tpe -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0w32 +
gvwgs.obj +
,gvwgs32.exe,gvwgs32, +
$(LIBDIR)\import32 +
$(LIBDIR)\cw32mt, +
gvwgs32.def, +
gvwgs32.res
!

gvwgs16.exe: gvwgs.c gvwgs.h gvwgs16.res
	$(COMPDIR)\$(CC) -c $(CFLAGS) gvwgs.c
	$(COMPDIR)\tlink -Twe -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0w$(MODEL) +
gvwgs.obj +
,gvwgs16.exe,gvwgs16, +
$(LIBDIR)\import +
$(LIBDIR)\mathw$(MODEL) +
$(LIBDIR)\cw$(MODEL), +
gvwgs16.def, +
gvwgs16.res
!

gsv16spl.exe: gsv16spl.c gsv16spl.rc gsv16spl.def  gvc$(LANGUAGE).h
	copy gvc$(LANGUAGE).h gvclang.h
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
	$(COMPDIR)\rlink -t $*.res $*.exe

strip: gsview$(WINEXT).exe
	$(COMPDIR)\tdstrp32 gsview32.exe

prezip:
	copy gsview$(WINEXT).exe ..\gsview$(WINEXT).exe
	copy binary\gvwin1.ico ..\gsview32.ico
	# used to do nothing, rely on  gsview32 being without symbol table
	$(COMPDIR)\tdstrp32 ..\gsview32.exe
	copy gsviewen.hlp ..\gsviewen.hlp
	copy gsviewde.hlp ..\gsviewde.hlp
	copy gsvw32en.dll ..\gsvw32en.dll
	copy gsvw32de.dll ..\gsvw32de.dll
	$(COMPDIR)\tdstrp32 ..\gsvw32en.dll
	$(COMPDIR)\tdstrp32 ..\gsvw32de.dll
	copy gsv16spl.exe ..\gsv16spl.exe
	copy gvwgs32.exe ..\gvwgs32.exe
	copy printer.ini ..\printer.ini
	copy winsetup.exe ..\setup.exe
	$(COMPDIR)\tdstrp32 ..\setup.exe
	# change OS/2 filenames to lower case
	cd ..
	rename gvpm.exe gvpm.exe
	rename gvpm.eas gvpm.eas
	rename gvpm.ico gvpm.ico
	rename gvpmen.dll gvpmen.dll
	rename gvpmde.dll gvpmde.dll
	rename gvpmen.hlp gvpmen.hlp
	rename gvpmde.hlp gvpmde.hlp
	rename gvpgs.exe gvpgs.exe
	rename os2setup.exe os2setup.exe
	rename os2unzip.exe os2unzip.exe
	rename pstotxt2.dll pstotxt2.dll
	rename pstotxt2.exe pstotxt2.exe
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
	zip -9 -@ gsview.zip  < gvclist.txt
	del gvcliste.txt
	del gvclistp.txt
	del gvclists.txt
	del gvclist.txt
	zip -9 gsviewXX.zip gsview.zip README.TXT FILE_ID.DIZ LICENCE os2setup.exe os2unzip.exe setup.exe wizunz32.dll
	cd src

zip16:
	tdstrip gsview16.exe
	tdstrip gsvw16en.dll
	tdstrip gsvw16de.dll
	echo You are advised to use the 32-bit version of GSview > README16.TXT
	echo instead of this 16-bit version. >> README16.TXT
	echo You must use manual installation.  See the help file GSVIEWEN.HLP. >> README16.TXT
	echo Redistribution of this 16-bit GSview MUST be accompanied >> README16.TXT
	echo by the 32-bit version, to meet the licence requirement >> README16.TXT
	echo that it be accompanied by source code. >> README16.TXT
	echo Do not ask the author any questions about the 16-bit version. >> README16.TXT
	-del ..\gsview16.zip
	zip -9 ..\gsview16.zip README16.TXT README.TXT LICENCE gsview16.exe gsvw16en.dll gsvw16de.dll 
	zip -9 ..\gsview16.zip gsviewen.hlp gsviewde.hlp gvwgs16.exe pstotxt1.dll printer.ini
	zip -z ..\gsview16.zip < README16.TXT

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
	del gsvw16de.map
	del gsvw16en.map
	del gvwlang.obj
	del gvwin16.res
	del gvwin32.res
	del gsviewen.txt
	del gsviewde.txt
	del gsviewen.rtf
	del gsviewde.rtf
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
	del winunzip.obj
	del gvwtxt.obj
	del gvwgs.obj
	del gvwgs16.res
	del gvwgs16.map
	del gvwgs32.res
	del gvwgs32.map

veryclean: clean
	del gsview$(WINEXT).exe
	del gsviewen.hlp
	del gsviewde.hlp
	del gsvw$(WINEXT)en.dll
	del gsvw$(WINEXT)de.dll
	del gsview.htm
	del gsv16spl.exe
	del winsetup.exe
	del gvwgs$(WINEXT).exe
