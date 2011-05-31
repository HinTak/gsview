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

# Makefile for GSview for Windows - GSVIEW.EXE or GSVIEW32.EXE
# using Borland C++ 4.5
# 'make -fgvwin.mak'
#

# Edit COMPBASE and WIN32 as required
COMPBASE = e:\bc45
# DEBUG=1 for Debugging options
DEBUG=1
# WIN32 is the default
WIN32=1
# Language is English (en) or Deutsch (de) or French (fr)
# This only applies to the utilties, not GSview itself.
LANGUAGE=en
# GSview version
GSVIEW_VERSION=23

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
  gsvw$(WINEXT)fr.dll gsviewfr.hlp\
  gvwgs$(WINEXT).exe gsv16spl.exe\
  winsetup.exe setp$(WINEXT)fr.dll setp$(WINEXT)de.dll

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

gsvw32en.res: gvwin2.rc gvcen.h gvcen.rc gvwen.rc gvcver.h
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


gsvw16en.res: gvwin2.rc gvcen.h gvcen.rc gvwen.rc gvcver.h
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

gsvw32de.res: gvwin2.rc gvcde.h gvcde.rc gvwde.rc gvcver.h
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

gsvw16de.res: gvwin2.rc gvcde.h gvcde.rc gvwde.rc gvcver.h
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

gsvw32fr.res: gvwin2.rc gvcfr.h gvcfr.rc gvwfr.rc gvcver.h
	copy gvcfr.h  gvclang.h
	copy gvcfr.rc gvclang.rc
	copy gvwfr.rc gvwlang.rc
	$(COMPDIR)\brcc32 -i$(INCDIR) -r -fogsvw32fr gvwin2
	-del gvclang.rc
	-del gvwlang.rc

gsvw32fr.dll: gvwlang.c gsvw32fr.res gsvw32fr.def
	$(COMPDIR)\bcc32 -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink32 -Tpd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0d32 +
gvwlang.obj +
,gsvw32fr.dll,gsvw32fr, +
$(LIBDIR)\import32 +
$(LIBDIR)\cw32mt, +
gsvw32fr.def, +
gsvw32fr.res
!

gsvw16fr.res: gvwin2.rc gvcfr.h gvcfr.rc gvwfr.rc gvcver.h
	copy gvcfr.h  gvclang.h
	copy gvcfr.rc gvclang.rc
	copy gvwfr.rc gvwlang.rc
	$(COMPDIR)\brcc -i$(INCDIR) -r -fogsvw16fr gvwin2
	-del gvclang.rc
	-del gvwlang.rc

gsvw16fr.dll: gvwlang.c gsvw16fr.res gsvw16fr.def
	$(COMPDIR)\$(CC) -ml -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink -Twd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0dl +
gvwlang.obj +
,gsvw16fr.dll,gsvw16fr, +
$(LIBDIR)\import +
$(LIBDIR)\cwl, +
gsvw16fr.def, +
gsvw16fr.res
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

gvwinit.obj: gvwinit.c $(HDRS) gvcrc.h

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

winsetup.res: winsetup.rc setup.h gvcen.h
	copy gvcen.h  gvclang.h
!if $(WIN32)
	$(COMPDIR)\brcc32 -i$(INCDIR) -r $*.rc
!else
	$(COMPDIR)\brcc -i$(INCDIR) -r $*.rc
!endif

setupc.obj: setupc.c setup.h setupc.h gvcrc.h gvcbeta.h
	$(COMPDIR)\$(CC) -c $(CFLAGS) setupc.c

winsetup.obj: winsetup.c setup.h setup.c gvcrc.h gvcbeta.h gvcen.h gvcver.h
	copy gvcen.h gvclang.h
	$(COMPDIR)\$(CC) -c $(CFLAGS) winsetup.c

winsetup.exe: winsetup.obj winsetup.res winsetup.def setupc.obj winunzip.obj gvcbeta.obj
!if $(WIN32)
	$(COMPDIR)\tlink32 -Tpe -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0w32 +
winsetup.obj +
winunzip.obj setupc.obj gvcbeta.obj +
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
winunzip.obj setupc.obj gvcbeta.obj +
,winsetup.exe,winsetup, +
$(LIBDIR)\import +
$(LIBDIR)\mathw$(MODEL) +
$(LIBDIR)\cw$(MODEL), +
winsetup.def, +
winsetup.res
!
!endif

setp32de.res: winsetup.rc gvcfr.h gvcver.h gvcrc.h setup.h
	copy gvcde.h gvclang.h
	$(COMPDIR)\brcc32 -i$(INCDIR) -r -fosetp32de winsetup
	-del gvclang.h

setp32de.dll: gvwlang.c setp32de.res setp32de.def
	$(COMPDIR)\bcc32 -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink32 -Tpd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0d32 +
gvwlang.obj +
,setp32de.dll,setp32de, +
$(LIBDIR)\import32 +
$(LIBDIR)\cw32mt, +
setp32de.def, +
setp32de.res
!

setp32fr.res: winsetup.rc gvcfr.h gvcver.h gvcrc.h setup.h
	copy gvcfr.h gvclang.h
	$(COMPDIR)\brcc32 -i$(INCDIR) -r -fosetp32fr winsetup
	-del gvclang.h

setp32fr.dll: gvwlang.c setp32fr.res setp32fr.def
	$(COMPDIR)\bcc32 -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink32 -Tpd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0d32 +
gvwlang.obj +
,setp32fr.dll,setp32fr, +
$(LIBDIR)\import32 +
$(LIBDIR)\cw32mt, +
setp32fr.def, +
setp32fr.res
!

setp16de.res: winsetup.rc gvcfr.h gvcver.h gvcrc.h setup.h
	copy gvcde.h  gvclang.h
	$(COMPDIR)\brcc -i$(INCDIR) -r -fosetp16de winsetup

setp16de.dll: gvwlang.c setp16de.res setp16de.def
	$(COMPDIR)\$(CC) -ml -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink -Twd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0dl +
gvwlang.obj +
,setp16de.dll,setp16de, +
$(LIBDIR)\import +
$(LIBDIR)\cwl, +
setp16de.def, +
setp16de.res
!

setp16fr.res: winsetup.rc gvcfr.h gvcver.h gvcrc.h setup.h
	copy gvcfr.h  gvclang.h
	$(COMPDIR)\brcc -i$(INCDIR) -r -fosetp16fr winsetup

setp16fr.dll: gvwlang.c setp16fr.res setp16fr.def
	$(COMPDIR)\$(CC) -ml -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink -Twd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0dl +
gvwlang.obj +
,setp16fr.dll,setp16fr, +
$(LIBDIR)\import +
$(LIBDIR)\cwl, +
setp16fr.def, +
setp16fr.res
!

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

gsviewfr.hlp: gvdoc.exe doc2rtf.exe gvcfr.txt gsviewfr.hpj
	gvdoc W gvcfr.txt gsviewfr.txt
	doc2rtf gsviewfr.txt gsviewfr.rtf
	-del gsviewfr.txt
	$(COMPDIR)\hc31 gsviewfr.hpj
	rename gsviewfr.hlp gsviewfr.hlp
	-del gsviewfr.rtf

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
	# used to do nothing, rely on gsview32 being without symbol table
	$(COMPDIR)\tdstrp32 ..\gsview32.exe
	copy gsviewen.hlp ..\gsviewen.hlp
	copy gsviewde.hlp ..\gsviewde.hlp
	copy gsviewfr.hlp ..\gsviewfr.hlp
	copy gsvw32en.dll ..\gsvw32en.dll
	copy gsvw32de.dll ..\gsvw32de.dll
	copy gsvw32fr.dll ..\gsvw32fr.dll
	$(COMPDIR)\tdstrp32 ..\gsvw32en.dll
	$(COMPDIR)\tdstrp32 ..\gsvw32de.dll
	$(COMPDIR)\tdstrp32 ..\gsvw32fr.dll
	copy gsv16spl.exe ..\gsv16spl.exe
	copy gvwgs32.exe ..\gvwgs32.exe
	copy printer.ini ..\printer.ini
	copy winsetup.exe ..\setup.exe
	copy setp32de.dll ..\setp32de.dll
	copy setp32fr.dll ..\setp32fr.dll
	$(COMPDIR)\tdstrp32 ..\setup.exe
	$(COMPDIR)\tdstrp32 ..\setp32de.dll
	$(COMPDIR)\tdstrp32 ..\setp32fr.dll
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
	-del gsvw32en.dll
	-del gsvw32de.dll
	-del gsvw32fr.dll
	-del gsv16spl.exe
	-del gvwgs32.exe
	-del printer.ini
	-del setup.exe
	-del setp32de.dll
	-del setp32fr.dll
	cd src
	
gsv$(GSVIEW_VERSION)w16.zip:
	copy README.TXT ..\README.TXT
	copy LICENCE ..\LICENCE
	copy FILE_ID.DIZ ..\FILE_ID.DIZ
	copy gsview16.exe ..\gsview16.exe
	copy binary\gvwin1.ico ..\gsview32.ico
	$(COMPDIR)\tdstrip ..\gsview16.exe
	copy gsviewen.hlp ..\gsviewen.hlp
	copy gsviewde.hlp ..\gsviewde.hlp
	copy gsviewfr.hlp ..\gsviewfr.hlp
	copy gsvw16en.dll ..\gsvw16en.dll
	copy gsvw16de.dll ..\gsvw16de.dll
	copy gsvw16fr.dll ..\gsvw16fr.dll
	$(COMPDIR)\tdstrip ..\gsvw16en.dll
	$(COMPDIR)\tdstrip ..\gsvw16de.dll
	$(COMPDIR)\tdstrip ..\gsvw16fr.dll
	copy gvwgs16.exe ..\gvwgs16.exe
	copy printer.ini ..\printer.ini
	copy winsetup.exe ..\setup16.exe
	copy setp16de.dll ..\setp16de.dll
	copy setp16fr.dll ..\setp16fr.dll
	$(COMPDIR)\tdstrip ..\setup16.exe
	$(COMPDIR)\tdstrip ..\setp16de.dll
	$(COMPDIR)\tdstrip ..\setp16fr.dll
	cd ..
	# convert names to lower case
	-rename gsview16.exe gsview16.exe
	-rename gsvw16en.dll gsvw16en.dll
	-rename gsvw16de.dll gsvw16de.dll
	-rename gsvw16fr.dll gsvw16fr.dll
	-rename gvwgs16.exe gvwgs16.exe
	-rename setup16.exe setup16.exe
	-del win16.zip
	zip -9 -@ win16.zip < src\gvclist1.txt
	echo You are advised to use the 32-bit version of GSview > README16.TXT
	echo instead of this 16-bit version. >> README16.TXT
	echo Redistribution of this Win16 GSview MUST be accompanied by the >> README16.TXT
	echo sources in gsv$(GSVIEW_VERSION)src.zip to meet the licence requirements. >> README16.TXT
	echo Do not ask the author any questions about the 16-bit version. >> README16.TXT
	-del gsv$(GSVIEW_VERSION)w16.zip
	zip -9 gsv$(GSVIEW_VERSION)w16.zip win16.zip setup16.exe wizunz16.dll setp16de.dll setp16fr.dll
	zip -9 gsv$(GSVIEW_VERSION)w16.zip README16.TXT README.TXT FILE_ID.DIZ LICENCE
	-del README16.TXT
	-del README.TXT
	-del LICENCE
	-del FILE_ID.DIZ
	-del gsview16.exe
	-del gsview32.ico
	-del gsviewen.hlp
	-del gsviewde.hlp
	-del gsviewfr.hlp
	-del gsvw16en.dll
	-del gsvw16de.dll
	-del gsvw16fr.dll
	-del gvwgs16.exe
	-del printer.ini
	-del setup16.exe
	-del setp16de.dll
	-del setp16fr.dll
	cd src
	

prezip: gsv$(GSVIEW_VERSION)w$(WINEXT).zip

zip: prezip gsv$(GSVIEW_VERSION)src.zip
	copy README.TXT ..\README.TXT
	copy LICENCE ..\LICENCE
	copy FILE_ID.DIZ ..\FILE_ID.DIZ
	cd ..
	-del gsview$(GSVIEW_VERSION).zip
	rename gsv$(GSVIEW_VERSION)w16.zip gsv$(GSVIEW_VERSION)w16.zip 
	rename gsv$(GSVIEW_VERSION)os2.zip gsv$(GSVIEW_VERSION)os2.zip 
	zip -9 gsview$(GSVIEW_VERSION) gsv$(GSVIEW_VERSION)src.zip gsv$(GSVIEW_VERSION)os2.zip gsv$(GSVIEW_VERSION)w16.zip gsv$(GSVIEW_VERSION)w32.zip
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
	del gsvw$(WINEXT)en.dll
	del gsvw$(WINEXT)de.dll
	del gsvw$(WINEXT)fr.dll
	del gsview.htm
	del gsv16spl.exe
	del gvwgs$(WINEXT).exe
	del winsetup.exe
	del setp$(WINEXT)de.dll
	del setp$(WINEXT)fr.dll
