#  Copyright (C) 1993-1998, Russell Lang.  All rights reserved.
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
#   copy gvwin.mak makefile
#   make
#

# Edit COMPBASE and WIN32 as required
COMPBASE = c:\bc45
# DEBUG=1 for Debugging options
DEBUG=1
# WIN32 is the default
WIN32=1
# Language is English (en) or Deutsch (de) or French (fr) or Italian (it)
# This only applies to the utilties, not GSview itself.
LANGUAGE=en
# GSview version
GSVIEW_VERSION=26

# Shouldn't need editing below here
COMPDIR = $(COMPBASE)\bin
INCDIR = $(COMPBASE)\include
LIBDIR = $(COMPBASE)\lib
HC=$(COMPDIR)\hc31
CLFLAG = -L$(LIBDIR)
!if $(WIN32)
WINEXT=32
CCAUX = bcc
MODEL=32
CFLAGS=-v -WE -w -tWM -H=gsview32.sym -I$(INCDIR)
CC = bcc32
RCOMP = $(COMPDIR)\brcc32
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
RCOMP = $(COMPDIR)\brcc
!endif

all: gsview$(WINEXT).exe\
  gsviewen.hlp\
  gsvw$(WINEXT)de.dll gsviewde.hlp\
  gsvw$(WINEXT)fr.dll gsviewfr.hlp\
  gsvw$(WINEXT)it.dll gsviewit.hlp\
  gvwgs$(WINEXT).exe gsv16spl.exe\
  winsetup.exe setp$(WINEXT)de.dll setp$(WINEXT)fr.dll setp$(WINEXT)it.dll\
  ungsview.exe

.c.obj:
	$(COMPDIR)\$(CC) -c $(CFLAGS) {$< }

!include "gvwinc.mak"
	
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

gsvw32de.dll: gvwlang.c gsvw32de.res de\gvwin32.def
	$(COMPDIR)\bcc32 -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink32 -Tpd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0d32 +
gvwlang.obj +
,gsvw32de.dll,gsvw32de, +
$(LIBDIR)\import32 +
$(LIBDIR)\cw32mt, +
de\gvwin32.def, +
gsvw32de.res
!

gsvw16de.dll: gvwlang.c gsvw16de.res de\gvwin16.def
	$(COMPDIR)\$(CC) -ml -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink -Twd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0dl +
gvwlang.obj +
,gsvw16de.dll,gsvw16de, +
$(LIBDIR)\import +
$(LIBDIR)\cwl, +
de\gvwin16.def, +
gsvw16de.res
!

gsvw32fr.dll: gvwlang.c gsvw32fr.res fr\gvwin32.def
	$(COMPDIR)\bcc32 -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink32 -Tpd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0d32 +
gvwlang.obj +
,gsvw32fr.dll,gsvw32fr, +
$(LIBDIR)\import32 +
$(LIBDIR)\cw32mt, +
fr\gvwin32.def, +
gsvw32fr.res
!

gsvw16fr.dll: gvwlang.c gsvw16fr.res fr\gvwin16.def
	$(COMPDIR)\$(CC) -ml -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink -Twd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0dl +
gvwlang.obj +
,gsvw16fr.dll,gsvw16fr, +
$(LIBDIR)\import +
$(LIBDIR)\cwl, +
fr\gvwin16.def, +
gsvw16fr.res
!

gsvw32it.dll: gvwlang.c gsvw32it.res it\gvwin32.def
	$(COMPDIR)\bcc32 -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink32 -Tpd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0d32 +
gvwlang.obj +
,gsvw32it.dll,gsvw32it, +
$(LIBDIR)\import32 +
$(LIBDIR)\cw32mt, +
it\gvwin32.def, +
gsvw32it.res
!

gsvw16it.dll: gvwlang.c gsvw16it.res it\gvwin16.def
	$(COMPDIR)\$(CC) -ml -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink -Twd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0dl +
gvwlang.obj +
,gsvw16it.dll,gsvw16it, +
$(LIBDIR)\import +
$(LIBDIR)\cwl, +
it\gvwin16.def, +
gsvw16it.res
!

winsetup.exe: winsetup.obj winsetup.res winsetup.def setupc.obj winunzip.obj gvcbeta.obj gvwdde.obj
!if $(WIN32)
	$(COMPDIR)\tlink32 -Tpe -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0w32 +
winsetup.obj +
winunzip.obj setupc.obj gvcbeta.obj gvwdde.obj +
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
winunzip.obj setupc.obj gvcbeta.obj gvwdde.obj +
,winsetup.exe,winsetup, +
$(LIBDIR)\import +
$(LIBDIR)\mathw$(MODEL) +
$(LIBDIR)\cw$(MODEL), +
winsetup.def, +
winsetup.res
!
!endif

setp32de.dll: gvwlang.c setp32de.res de\setup32.def
	$(COMPDIR)\bcc32 -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink32 -Tpd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0d32 +
gvwlang.obj +
,setp32de.dll,setp32de, +
$(LIBDIR)\import32 +
$(LIBDIR)\cw32mt, +
de\setup32.def, +
setp32de.res
!

setp32fr.dll: gvwlang.c setp32fr.res fr\setup32.def
	$(COMPDIR)\bcc32 -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink32 -Tpd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0d32 +
gvwlang.obj +
,setp32fr.dll,setp32fr, +
$(LIBDIR)\import32 +
$(LIBDIR)\cw32mt, +
fr\setup32.def, +
setp32fr.res
!

setp32it.dll: gvwlang.c setp32it.res it\setup32.def
	$(COMPDIR)\bcc32 -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink32 -Tpd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0d32 +
gvwlang.obj +
,setp32it.dll,setp32it, +
$(LIBDIR)\import32 +
$(LIBDIR)\cw32mt, +
it\setup32.def, +
setp32it.res
!

setp16de.dll: gvwlang.c setp16de.res de\setup16.def
	$(COMPDIR)\$(CC) -ml -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink -Twd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0dl +
gvwlang.obj +
,setp16de.dll,setp16de, +
$(LIBDIR)\import +
$(LIBDIR)\cwl, +
de\setup16.def, +
setp16de.res
!

setp16fr.dll: gvwlang.c setp16fr.res fr\setup16.def
	$(COMPDIR)\$(CC) -ml -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink -Twd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0dl +
gvwlang.obj +
,setp16fr.dll,setp16fr, +
$(LIBDIR)\import +
$(LIBDIR)\cwl, +
fr\setup16.def, +
setp16fr.res
!

setp16it.dll: gvwlang.c setp16it.res it\setup16.def
	$(COMPDIR)\$(CC) -ml -c -v -WD -w -I$(INCDIR) gvwlang.c
	$(COMPDIR)\tlink -Twd -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0dl +
gvwlang.obj +
,setp16it.dll,setp16it, +
$(LIBDIR)\import +
$(LIBDIR)\cwl, +
it\setup16.def, +
setp16it.res
!

ungsview.exe: ungsview.obj ungsview.res ungsview.def
!if $(WIN32)
	$(COMPDIR)\tlink32 -Tpe -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0w32 +
ungsview.obj +
,ungsview.exe,ungsview, +
$(LIBDIR)\import32 +
$(LIBDIR)\cw32, +
ungsview.def,
ungsview.res
!
# tlink32 is buggy and won't bind winsetup.res so try again...
	$(COMPDIR)\brc32 ungsview.res ungsview.exe
!else
	$(COMPDIR)\tlink -Twe -c -m -s $(DEBUGLINK) @&&!
$(LIBDIR)\c0w$(MODEL) +
ungsview.obj +
,ungsview.exe,ungsview, +
$(LIBDIR)\import +
$(LIBDIR)\mathw$(MODEL) +
$(LIBDIR)\cw$(MODEL), +
ungsview.def, +
ungsview.res
!
!endif


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

gsv16spl.exe: gsv16spl.c gsv16spl.rc gsv16spl.def  $(LANGUAGE)\gvclang.h
	copy $(LANGUAGE)\gvclang.h gvclang.h
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

strip: all
	$(COMPDIR)\tdstrp32 gsview32.exe
	$(COMPDIR)\tdstrp32 gsvw32de.dll
	$(COMPDIR)\tdstrp32 gsvw32fr.dll
	$(COMPDIR)\tdstrp32 gsvw32it.dll
	$(COMPDIR)\tdstrp32 winsetup.exe
	$(COMPDIR)\tdstrp32 setp32de.dll
	$(COMPDIR)\tdstrp32 setp32fr.dll
	$(COMPDIR)\tdstrp32 setp32it.dll
	$(COMPDIR)\tdstrp32 ungsview.exe

	
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
	copy gsviewit.hlp ..\gsviewit.hlp
	copy gsvw16de.dll ..\gsvw16de.dll
	copy gsvw16fr.dll ..\gsvw16fr.dll
	copy gsvw16it.dll ..\gsvw16it.dll
	$(COMPDIR)\tdstrip ..\gsvw16de.dll
	$(COMPDIR)\tdstrip ..\gsvw16fr.dll
	$(COMPDIR)\tdstrip ..\gsvw16it.dll
	copy gvwgs16.exe ..\gvwgs16.exe
	copy printer.ini ..\printer.ini
	copy winsetup.exe ..\setup16.exe
	copy setp16de.dll ..\setp16de.dll
	copy setp16fr.dll ..\setp16fr.dll
	copy setp16it.dll ..\setp16it.dll
	$(COMPDIR)\tdstrip ..\setup16.exe
	$(COMPDIR)\tdstrip ..\setp16de.dll
	$(COMPDIR)\tdstrip ..\setp16fr.dll
	$(COMPDIR)\tdstrip ..\setp16it.dll
	copy ungsview.exe ..\ungsvw16.exe
	$(COMPDIR)\tdstrip ..\ungsvw16.exe
	cd ..
	# convert names to lower case
	-rename gsview16.exe gsview16.exe
	-rename gsvw16de.dll gsvw16de.dll
	-rename gsvw16fr.dll gsvw16fr.dll
	-rename gsvw16it.dll gsvw16it.dll
	-rename gvwgs16.exe gvwgs16.exe
	-rename setup16.exe setup16.exe
	-rename ungsvw16.exe ungsvw16.exe
	-del win16.zip
	zip -9 -@ win16.zip < src\gvclist1.txt
	echo You are advised to use the 32-bit version of GSview > README16.TXT
	echo instead of this 16-bit version. >> README16.TXT
	echo Redistribution of this Win16 GSview MUST be accompanied by the >> README16.TXT
	echo sources in gsv$(GSVIEW_VERSION)src.zip to meet the licence requirements. >> README16.TXT
	echo Do not ask the author any questions about the 16-bit version. >> README16.TXT
	-del gsv$(GSVIEW_VERSION)w16.zip
	zip -9 gsv$(GSVIEW_VERSION)w16.zip win16.zip setup16.exe wizunz16.dll setp16de.dll setp16fr.dll 
#setp16it.dll
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
	-del gsviewit.hlp
	-del gsvw16de.dll
	-del gsvw16fr.dll
	-del gsvw16it.dll
	-del gvwgs16.exe
	-del printer.ini
	-del setup16.exe
	-del setp16de.dll
	-del setp16fr.dll
	-del setp16it.dll
	-del ungsvw16.exe
	cd src
	

