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

# Common makefile for BC++ and MSVC++
# See gvwin.mak or gvwinvc.mak

# Define the location of the WinZip self-extracting-archive-maker.
!ifndef WINZIPSE_XE
WINZIPSE_XE="C:\Program Files\WinZip Self-Extractor\WZIPSE32.EXE"
!endif

OBJ1=gvwin.obj gvwdde.obj gvwdde2.obj gvwdll.obj gvwdisp.obj gvwdlg.obj
OBJ2=gvwclip.obj gvwedit.obj gvweps.obj gvwmeas.obj gvwmisc.obj gvwprf.obj 
OBJ3=gvwprn.obj gvcmeas.obj gvcmeas2.obj gvcmisc.obj gvcdisp.obj gvccmd.obj 
OBJ4=gvcprn.obj gvceps.obj gvcinit.obj gvctext.obj dscparse.obj
OBJ5=dscutil.obj gvcdll.obj gvcpdf.obj gvwinit.obj gvcbeta.obj gvwgsver.obj
OBJ6=gvcreg.obj gvwreg.obj gvwfile.obj gvwdib.obj gvwpdib.obj gvwpgdi.obj
OBJS=$(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6)

HDRS=gsvver.h gvwin.h dscparse.h gvcfn.h gvcver.h gvcfile.h gvwdib.h gvwpdib.h

echogsv.exe: echogsv.c
	$(CC) $(CFLAGS) echogsv.c

gsvw$(WINEXT)de.res: $(HDRS) gvwin2.rc de\gvclang.h de\gvclang.rc de\gvwlang.rc
	copy de\gvclang.h  gvclang.h
	copy de\gvclang.rc gvclang.rc
	copy de\gvwlang.rc gvwlang.rc
	$(RCOMP) -i"$(INCDIR)" -r -fogsvw$(WINEXT)de.res gvwin2
	-del gvclang.h
	-del gvclang.rc
	-del gvwlang.rc

gsvw$(WINEXT)es.res: $(HDRS) gvwin2.rc es\gvclang.h es\gvclang.rc es\gvwlang.rc
	copy es\gvclang.h  gvclang.h
	copy es\gvclang.rc gvclang.rc
	copy es\gvwlang.rc gvwlang.rc
	$(RCOMP) -i"$(INCDIR)" -r -fogsvw$(WINEXT)es.res gvwin2
	-del gvclang.h
	-del gvclang.rc
	-del gvwlang.rc

gsvw$(WINEXT)fr.res: $(HDRS) gvwin2.rc fr\gvclang.h fr\gvclang.rc fr\gvwlang.rc
	copy fr\gvclang.h  gvclang.h
	copy fr\gvclang.rc gvclang.rc
	copy fr\gvwlang.rc gvwlang.rc
	$(RCOMP) -i"$(INCDIR)" -r -fogsvw$(WINEXT)fr.res gvwin2
	-del gvclang.h
	-del gvclang.rc
	-del gvwlang.rc

gsvw$(WINEXT)it.res: $(HDRS) gvwin2.rc it\gvclang.h it\gvclang.rc it\gvwlang.rc
	copy it\gvclang.h  gvclang.h
	copy it\gvclang.rc gvclang.rc
	copy it\gvwlang.rc gvwlang.rc
	$(RCOMP) -i"$(INCDIR)" -r -fogsvw$(WINEXT)it.res gvwin2
	-del gvclang.h
	-del gvclang.rc
	-del gvwlang.rc

gvwin$(WINEXT).res: $(HDRS) gvwin1.rc gvwin2.rc en\gvclang.h en\gvclang.rc en\gvwlang.rc
	copy en\gvclang.h  gvclang.h
	copy en\gvclang.rc gvclang.rc
	copy en\gvwlang.rc gvwlang.rc
	$(RCOMP) -i"$(INCDIR)" -r -fogvwin$(WINEXT).res gvwin1
#	-del gvclang.h
#	-del gvclang.rc
#	-del gvwlang.rc

gsprint.obj: gsprint.cpp gvcfile.h gvwdib.h gvwpdib.h gsvver.h gvcver.h

gvwin.obj: gvwin.cpp $(HDRS)

gvwclip.obj: gvwclip.cpp $(HDRS)

gvwdisp.obj: gvwdisp.cpp $(HDRS)

gvwdlg.obj: gvwdlg.cpp gvcrc.h $(HDRS)

gvwdll.obj: gvwdll.cpp gvcrc.h gsdll.h $(HDRS)

gvwedit.obj: gvwedit.cpp $(HDRS)

gvweps.obj: gvweps.cpp gvceps.h $(HDRS)

gvwgsver.obj: gvwgsver.cpp $(HDRS)

gvwinit.obj: gvwinit.cpp $(HDRS)

gvwmeas.obj: gvwmeas.cpp $(HDRS)

gvwmisc.obj: gvwmisc.cpp $(HDRS)

gvwprn.obj: gvwprn.cpp $(HDRS)

gvccmd.obj: gvccmd.cpp gvcrc.h $(HDRS)

gvcdisp.obj: gvcdisp.cpp $(HDRS)

gvcdll.obj: gvcdll.cpp gvcrc.h gsdll.h $(HDRS)

dscparse.obj: dscparse.cpp dscparse.h

dscutil.obj: dscutil.cpp dscparse.h

gvcbeta.obj: gvcbeta.cpp gvcbeta.h $(HDRS)

gvceps.obj: gvceps.cpp gvceps.h $(HDRS)

gvcinit.obj: gvcinit.cpp gvcrc.h $(HDRS)

gvcmeas.obj: gvcmeas.cpp gvcrc.h $(HDRS)

gvcmeas2.obj: gvcmeas2.cpp gvcrc.h $(HDRS)

gvcmisc.obj: gvcmisc.cpp gvcrc.h $(HDRS)

gvcpdf.obj: gvcpdf.cpp gvcrc.h $(HDRS)

gvcprn.obj: gvcprn.cpp $(HDRS)

gvctext.obj: gvctext.cpp $(HDRS)

gvcreg.obj: gvcreg.cpp $(HDRS)

gvwreg.obj: gvwreg.cpp $(HDRS)

gvwfile.obj: gvwfile.cpp gvcfile.h

gvwdib.obj: gvwdib.cpp $(HDRS)

gvwpdib.obj: gvwpdib.cpp $(HDRS)

gvwpgdi.obj: gvwpgdi.cpp $(HDRS)

winunzip.obj: winunzip.cpp wizdll.h
	$(CC) -c $(CFLAGS) $*.cpp

dwuninst.res: dwuninst.rc dwuninst.h
	$(RCOMP) -i"$(INCDIR)" -r $*.rc

winsetup.res: winsetup.rc setup.h $(LANGUAGE)\gvclang.h
	copy $(LANGUAGE)\gvclang.h  gvclang.h
	$(RCOMP) -i"$(INCDIR)" -r $*.rc

setupc.obj: setupc.cpp setup.h setupc.h gvcrc.h gvcbeta.h
	$(CC) -c $(CFLAGS) setupc.cpp

winsetup.obj: winsetup.cpp setup.h gvcrc.h gvcbeta.h dwinst.h $(LANGUAGE)\gvclang.h 
	copy $(LANGUAGE)\gvclang.h gvclang.h
	$(CC) -c $(CFLAGS) winsetup.cpp

dwinst.obj: dwinst.cpp dwinst.h
	$(CC) -c $(CFLAGS) dwinst.cpp

dwuninst.obj: dwuninst.cpp dwuninst.h
	$(CC) -c $(CFLAGS) dwuninst.cpp


setp$(WINEXT)de.res: winsetup.rc de\gvclang.h gvcver.h gvcrc.h setup.h
	copy de\gvclang.h gvclang.h
	$(RCOMP) -i"$(INCDIR)" -r -fosetp$(WINEXT)de.res winsetup
	-del gvclang.h

setp$(WINEXT)es.res: winsetup.rc es\gvclang.h gvcver.h gvcrc.h setup.h
	copy es\gvclang.h gvclang.h
	$(RCOMP) -i"$(INCDIR)" -r -fosetp$(WINEXT)es.res winsetup
	-del gvclang.h

setp$(WINEXT)fr.res: winsetup.rc fr\gvclang.h gvcver.h gvcrc.h setup.h
	copy fr\gvclang.h gvclang.h
	$(RCOMP) -i"$(INCDIR)" -r -fosetp$(WINEXT)fr.res winsetup
	-del gvclang.h

setp$(WINEXT)it.res: winsetup.rc it\gvclang.h gvcver.h gvcrc.h setup.h
	copy it\gvclang.h gvclang.h
	$(RCOMP) -i"$(INCDIR)" -r -fosetp$(WINEXT)it.res winsetup
	-del gvclang.h

ungsview.res: ungsview.rc ungsview.h
	$(RCOMP) -i"$(INCDIR)" -r $*.rc

ungsview.obj: ungsview.cpp ungsview.h
	$(CC) -c $(CFLAGS) ungsview.cpp

gvdoc.exe: gvdoc.cpp
	$(CCAUX) -I"$(INCDIR)" $(CLFLAG) gvdoc.cpp

gsview.txt: $(LANGUAGE)\gvclang.txt gvdoc.exe
	gvdoc W $(LANGUAGE)\gvclang.txt gsview.txt

doc2rtf.exe: doc2rtf.cpp
	$(CCAUX) -I"$(INCDIR)" $(CLFLAG) doc2rtf.cpp

doc2html.exe: doc2html.cpp
	$(CCAUX) -I"$(INCDIR)" $(CLFLAG) doc2html.cpp

gsview.dvi: gsview.tex titlepag.tex
	-latex gsview
	-latex gsview

gsview.tex: gsview.txt doc2tex.exe
	doc2tex gsview.txt gsview.tex

doc2tex.exe: doc2tex.cpp
	$(CCAUX) -I"$(INCDIR)" $(CLFLAG) doc2tex.cpp

gsviewen.hlp: gvdoc.exe doc2rtf.exe en\gvclang.txt en\gsview.hpj
	copy en\gsview.hpj gsviewen.hpj
	gvdoc W en\gvclang.txt gsview.txt
	doc2rtf gsview.txt gsview.rtf
	$(HC) gsviewen.hpj
	rename gsviewen.hlp gsviewen.hlp
	-del gsviewen.hpj
	-del gsview.txt
	-del gsview.rtf

gsviewde.hlp: gvdoc.exe doc2rtf.exe de\gvclang.txt de\gsview.hpj
	copy de\gsview.hpj gsviewde.hpj
	gvdoc W de\gvclang.txt gsview.txt
	doc2rtf gsview.txt gsview.rtf
	$(HC) gsviewde.hpj
	rename gsviewde.hlp gsviewde.hlp
	-del gsviewde.hpj
	-del gsview.txt
	-del gsview.rtf

gsviewes.hlp: gvdoc.exe doc2rtf.exe es\gvclang.txt es\gsview.hpj
	copy es\gsview.hpj gsviewes.hpj
	gvdoc W es\gvclang.txt gsview.txt
	doc2rtf gsview.txt gsview.rtf
	$(HC) gsviewes.hpj
	rename gsviewes.hlp gsviewes.hlp
	-del gsviewes.hpj
	-del gsview.txt
	-del gsview.rtf

gsviewfr.hlp: gvdoc.exe doc2rtf.exe fr\gvclang.txt fr\gsview.hpj
	copy fr\gsview.hpj gsviewfr.hpj
	gvdoc W fr\gvclang.txt gsview.txt
	doc2rtf gsview.txt gsview.rtf
	$(HC) gsviewfr.hpj
	rename gsviewfr.hlp gsviewfr.hlp
	-del gsviewfr.hpj
	-del gsview.txt
	-del gsview.rtf

gsviewit.hlp: gvdoc.exe doc2rtf.exe it\gvclang.txt it\gsview.hpj
	copy it\gsview.hpj gsviewit.hpj
	gvdoc W it\gvclang.txt gsview.txt
	doc2rtf gsview.txt gsview.rtf
	$(HC) gsviewit.hpj
	rename gsviewit.hlp gsviewit.hlp
	-del gsviewit.hpj
	-del gsview.txt
	-del gsview.rtf

gsview.htm: doc2html.exe gsview.txt
	doc2html gsview.txt gsview.htm

html: doc2html.exe en\gvclang.txt de\gvclang.txt es\gvclang.txt fr\gvclang.txt it\gvclang.txt
	gvdoc W en\gvclang.txt gsview.txt
	doc2html gsview.txt gsviewen.htm
	-del gsview.txt
	gvdoc W de\gvclang.txt gsview.txt
	doc2html gsview.txt gsviewde.htm
	-del gsview.txt
	gvdoc W es\gvclang.txt gsview.txt
	doc2html gsview.txt gsviewes.htm
	-del gsview.txt
	gvdoc W fr\gvclang.txt gsview.txt
	doc2html gsview.txt gsviewfr.htm
	-del gsview.txt
	gvdoc W it\gvclang.txt gsview.txt
	doc2html gsview.txt gsviewit.htm
	-del gsview.txt
	gvdoc P en\gvclang.txt gsview.txt
	doc2html gsview.txt os2help.htm
	-del gsview.txt


gvwgs$(WINEXT).res: gvwgs.rc gvwgs.h $(ICONS) $(LANGUAGE)\gvclang.h 
	copy $(LANGUAGE)\gvclang.h gvclang.h
	$(RCOMP) -i"$(INCDIR)" -r -fogvwgs$(WINEXT).res gvwgs


src: gsv$(GSVIEW_VERSION)src.zip

gsv$(GSVIEW_VERSION)src.zip:
	copy Readme.htm ..
	copy gsview.css ..
	copy cdorder.txt ..
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
	zip -9 gsv$(GSVIEW_VERSION)src.zip epstool.zip pstotext.zip src.zip Readme.htm gsview.css cdorder.txt FILE_ID.DIZ LICENCE
	-del Readme.htm
	-del gsview.css
	-del cdorder.txt
	-del LICENCE
	-del FILE_ID.DIZ
	cd src
	
prezip: gsv$(GSVIEW_VERSION)w$(WINEXT).exe

zip: prezip gsv$(GSVIEW_VERSION)src.zip
	copy Readme.htm ..\Readme.htm
	copy gsview.css ..\gsview.css
	copy cdorder.txt ..\cdorder.txt
	copy LICENCE ..\LICENCE
	copy FILE_ID.DIZ ..\FILE_ID.DIZ
	cd ..
	-del gsview$(GSVIEW_VERSION).zip
#	rename gsv$(GSVIEW_VERSION)w16.zip gsv$(GSVIEW_VERSION)w16.zip 
	rename gsv$(GSVIEW_VERSION)os2.zip gsv$(GSVIEW_VERSION)os2.zip 
	zip -9 gsview$(GSVIEW_VERSION) gsv$(GSVIEW_VERSION)src.zip gsv$(GSVIEW_VERSION)os2.zip
# gsv$(GSVIEW_VERSION)w16.zip 
	zip -9 gsview$(GSVIEW_VERSION) gsv$(GSVIEW_VERSION)w32.exe
# gsv$(GSVIEW_VERSION)wda.zip
	zip -9 gsview$(GSVIEW_VERSION) Readme.htm gsview.css cdorder.txt FILE_ID.DIZ LICENCE
	-del Readme.htm
	-del gsview.css
	-del cdorder.txt
	-del LICENCE
	-del FILE_ID.DIZ
	cd src

# Create DEC Alpha zip file
# Since the author can't build this, it isn't in the main distribution.
gsv$(GSVIEW_VERSION)wda.zip:
#	Create list of files to archive
 	copy gvclista.txt filesda.txt
	echo Readme.htm >> filesda.txt
	echo FILE_ID.DIZ >> filesda.txt
	echo filelist.txt >> filesda.txt
	echo setup.exe >> filesda.txt
	echo setpdade.dll >> filesda.txt
	echo setpdaes.dll >> filesda.txt
	echo setpdafr.dll >> filesda.txt
	echo setpdait.dll >> filesda.txt
#	Copy the files into place
	-mkdir ..\gsview
	copy refresh.htm ..\Readme.htm
	copy FILE_ID.DIZ ..\FILE_ID.DIZ
	copy LICENCE ..\gsview\LICENCE
	copy Readme.htm ..\gsview\Readme.htm
	copy cdorder.txt ..\gsview\cdorder.txt
	copy regorder.txt ..\gsview\regorder.txt
	copy gsviewda.exe ..\gsview\gsviewda.exe
	copy binary\gvwin1.ico ..\gsview\gsview32.ico
	copy gsviewen.hlp ..\gsview\gsviewen.hlp
	copy gsviewde.hlp ..\gsview\gsviewde.hlp
	copy gsviewes.hlp ..\gsview\gsviewes.hlp
	copy gsviewfr.hlp ..\gsview\gsviewfr.hlp
	copy gsviewit.hlp ..\gsview\gsviewit.hlp
	copy gsvwdade.dll ..\gsview\gsvwdade.dll
	copy gsvwdaes.dll ..\gsview\gsvwdaes.dll
	copy gsvwdafr.dll ..\gsview\gsvwdafr.dll
	copy gsvwdait.dll ..\gsview\gsvwdait.dll
	copy gvwgsda.exe ..\gsview\gvwgsda.exe
	copy printer.ini ..\gsview\printer.ini
	copy uninstgs.exe ..\gsview\uninstgs.exe
	copy winsetup.exe ..\setup.exe
	copy setpdade.dll ..\setpdade.dll
	copy setpdaes.dll ..\setpdaes.dll
	copy setpdafr.dll ..\setpdafr.dll
	copy setpdait.dll ..\setpdait.dll
# assume zlib32.dll is in ..\gsview\zlib32.dll
# assume libbz2.dll is in ..\gsview\libbz2.dll
	echo GSview $(GSVIEW_DOT_VERSION)> filel_da.tmp
	echo gsview>> filel_da.tmp
	copy filel_da.tmp+gvclista.txt ..\filelist.txt
	del filel_da.tmp
#	Create the DEC Alpha zip file
	cd ..
	-del gsv$(GSVIEW_VERSION)wda.zip 
	zip -9 -@ gsv$(GSVIEW_VERSION)wda.zip < src\filesda.txt
	cd src
	-del filesda.txt

# create self extracting archive
test:	all distcopy

distcopy:
	-mkdir ..\gsview
	copy refresh.htm ..\Readme.htm
	copy FILE_ID.DIZ ..\FILE_ID.DIZ
	copy LICENCE ..\gsview\LICENCE
	copy Readme.htm ..\gsview\Readme.htm
	copy gsview.css ..\gsview\gsview.css
	copy cdorder.txt ..\gsview\cdorder.txt
	copy regorder.txt ..\gsview\regorder.txt
	copy gsview32.exe ..\gsview\gsview32.exe
	copy binary\gvwin1.ico ..\gsview\gsview32.ico
	copy gsviewen.hlp ..\gsview\gsviewen.hlp
	copy gsviewde.hlp ..\gsview\gsviewde.hlp
	copy gsviewes.hlp ..\gsview\gsviewes.hlp
	copy gsviewfr.hlp ..\gsview\gsviewfr.hlp
	copy gsviewit.hlp ..\gsview\gsviewit.hlp
	copy gsvw32de.dll ..\gsview\gsvw32de.dll
	copy gsvw32es.dll ..\gsview\gsvw32es.dll
	copy gsvw32fr.dll ..\gsview\gsvw32fr.dll
	copy gsvw32it.dll ..\gsview\gsvw32it.dll
	copy gvwgs32.exe ..\gsview\gvwgs32.exe
	copy printer.ini ..\gsview\printer.ini
	copy uninstgs.exe ..\gsview\uninstgs.exe
	copy winsetup.exe ..\setup.exe
	copy setp32de.dll ..\setp32de.dll
	copy setp32es.dll ..\setp32es.dll
	copy setp32fr.dll ..\setp32fr.dll
	copy setp32it.dll ..\setp32it.dll
# assume zlib32.dll is in ..\gsview\zlib32.dll
# assume libbz2.dll is in ..\gsview\libbz2.dll
	copy pstoedit.htm ..\gsview\pstoedit.htm
	copy gsprint.htm ..\gsview\gsprint.htm
	copy gsprint.exe ..\gsview\gsprint.exe
	copy ..\epstool\epstool.htm ..\gsview\epstool.htm
	copy ..\epstool\epstool.exe ..\gsview\epstool.exe
	copy gsv16spl.exe ..\gsview\gsv16spl.exe
	echo GSview $(GSVIEW_DOT_VERSION)> filelist.tmp
	echo gsview>> filelist.tmp
	copy filelist.tmp+gvclist3.txt ..\filelist.txt
	del filelist.tmp

files32.txt:
 	copy gvclist3.txt files32.txt
	echo Readme.htm >> files32.txt
	echo FILE_ID.DIZ >> files32.txt
	echo filelist.txt >> files32.txt
	echo setup.exe >> files32.txt
	echo setp32de.dll >> files32.txt
	echo setp32es.dll >> files32.txt
	echo setp32fr.dll >> files32.txt
	echo setp32it.dll >> files32.txt

gsv$(GSVIEW_VERSION)w32.zip: strip distcopy files32.txt
	cd ..
	-del gsv$(GSVIEW_VERSION)w32.zip 
	zip -9 -@ gsv$(GSVIEW_VERSION)w32.zip < src\files32.txt
	cd src



# Now convert to a self extracting archive.
# This involves making a few temporary files.
gsv$(GSVIEW_VERSION)w32.exe: strip distcopy gsv$(GSVIEW_VERSION)w32.zip
	cd ..
	echo -win32 -setup > setup.rsp
	echo -st "GSview $(GSVIEW_DOT_VERSION) for Win32" >> setup.rsp
	echo -i gsview\gsview32.ico >> setup.rsp
	echo -a about.txt >> setup.rsp
	echo -t dialog.txt >> setup.rsp
	echo -c .\setup.exe >> setup.rsp
	echo GSview is Copyright (C) 2000 Ghostgum Software Pty Ltd. > about.txt
	echo See licence in gsview\LICENCE >> about.txt
	echo This installs GSview $(GSVIEW_DOT_VERSION) for Win32. > dialog.txt
	echo GSview uses Aladdin Ghostscript to display, print and convert PostScript and PDF files. >> dialog.txt
	$(WINZIPSE_XE) gsv$(GSVIEW_VERSION)w32 @setup.rsp
# Don't delete temporary files, because make continues
# before these files are used.
#	-del setup.rsp 
#	-del about.txt
#	-del dialog.txt
	cd src

language:
	-del *.res
	-del gvclang.h
	-del gvclang.rc
	-del gvwlang.rc

clean: language
	-del gsvver.h
	-del gvwin.obj
	-del gvwclip.obj
	-del gvwdib.obj
	-del gvwdisp.obj
	-del gvwdlg.obj
	-del gvwdll.obj
	-del gvwedit.obj
	-del gvweps.obj
	-del gvwfile.obj
	-del gvwinit.obj
	-del gvwdde.obj
	-del gvwdde2.obj
	-del gvwgsver.obj
	-del gvwmeas.obj
	-del gvwmisc.obj
	-del gvwpdib.obj
	-del gvwpgdi.obj
	-del gvwprf.obj
	-del gvwprn.obj
	-del gvwreg.obj
	-del gvcbeta.obj
	-del gvccmd.obj
	-del dscparse.obj
	-del dscutil.obj
	-del gvcdisp.obj
	-del gvceps.obj
	-del gvcinit.obj
	-del gvcmeas.obj
	-del gvcmeas2.obj
	-del gvcmisc.obj
	-del gvcdll.obj
	-del gvcpdf.obj
	-del gvcprn.obj
	-del gvcreg.obj
	-del gvctext.obj
	-del gsview16.map
	-del gsview16.sym
	-del gsview32.map
	-del gsview32.sym
	-del gsview32.ilc
	-del gsview32.ild
	-del gsview32.ilf
	-del gsview32.ils
	-del gsview32.ilk
	-del gsview32.lib
	-del gsview32.exp
	-del gsview32.pdb
	-del gsview32.tds
	-del gsvw32de.map
	-del gsvw32de.ilc
	-del gsvw32de.ild
	-del gsvw32de.ilf
	-del gsvw32de.ils
	-del gsvw32de.ilk
	-del gsvw32de.lib
	-del gsvw32de.exp
	-del gsvw32de.pdb
	-del gsvw32de.tds
	-del gsvw32en.map
	-del gsvw32en.ilc
	-del gsvw32en.ild
	-del gsvw32en.ilf
	-del gsvw32en.ils
	-del gsvw32en.ilk
	-del gsvw32en.lib
	-del gsvw32en.exp
	-del gsvw32en.pdb
	-del gsvw32en.tds
	-del gsvw32es.map
	-del gsvw32es.ilc
	-del gsvw32es.ild
	-del gsvw32es.ilf
	-del gsvw32es.ils
	-del gsvw32es.ilk
	-del gsvw32es.lib
	-del gsvw32es.exp
	-del gsvw32es.pdb
	-del gsvw32es.tds
	-del gsvw32fr.map
	-del gsvw32fr.ilc
	-del gsvw32fr.ild
	-del gsvw32fr.ilf
	-del gsvw32fr.ils
	-del gsvw32fr.ilk
	-del gsvw32fr.lib
	-del gsvw32fr.exp
	-del gsvw32fr.pdb
	-del gsvw32fr.tds
	-del gsvw32it.map
	-del gsvw32it.ilc
	-del gsvw32it.ild
	-del gsvw32it.ilf
	-del gsvw32it.ils
	-del gsvw32it.ilk
	-del gsvw32it.lib
	-del gsvw32it.exp
	-del gsvw32it.pdb
	-del gsvw32it.tds
	-del gsvw16de.map
	-del gsvw16en.map
	-del gsvw16es.map
	-del gsvw16fr.map
	-del gsvw16it.map
	-del gvwlang.obj
	-del gvwin16.res
	-del gvwin32.res
	-del gsviewen.txt
	-del gsviewde.txt
	-del gsviewes.txt
	-del gsviewfr.txt
	-del gsviewit.txt
	-del gsview.rtf
	-del doc2html.obj
	-del doc2html.exe
	-del doc2rtf.obj
	-del doc2rtf.exe
	-del doc2rtf.tds
	-del doc2tex.obj
	-del doc2tex.exe
	-del gvdoc.exe
	-del gvdoc.obj
	-del gvdoc.tds
	-del gsview.txt
	-del gsview.aux
	-del gsview.dvi
	-del gsview.log
	-del gsview.toc
	-del gsview.tex
	-del gsv16spl.obj
	-del gsv16spl.res
	-del gsv16spl.map
	-del dwinst.obj
	-del dwuninst.obj
	-del winsetup.obj
	-del winsetup.res
	-del winsetup.map
	-del winsetup.ilc
	-del winsetup.ild
	-del winsetup.ilf
	-del winsetup.ils
	-del winsetup.ilk
	-del winsetup.lib
	-del winsetup.exp
	-del winsetup.pdb
	-del winsetup.tds
	-del uninstgs.obj
	-del uninstgs.res
	-del uninstgs.map
	-del uninstgs.ilc
	-del uninstgs.ild
	-del uninstgs.ilf
	-del uninstgs.ils
	-del uninstgs.ilk
	-del uninstgs.lib
	-del uninstgs.exp
	-del uninstgs.pdb
	-del uninstgs.tds
	-del setp16de.res
	-del setp16de.map
	-del setp16es.res
	-del setp16es.map
	-del setp16fr.res
	-del setp16fr.map
	-del setp16it.res
	-del setp16it.map
	-del setp32de.res
	-del setp32de.map
	-del setp32de.ilc
	-del setp32de.ild
	-del setp32de.ilf
	-del setp32de.ils
	-del setp32de.ilk
	-del setp32de.lib
	-del setp32de.exp
	-del setp32de.pdb
	-del setp32de.tds
	-del setp32es.res
	-del setp32es.map
	-del setp32es.ilc
	-del setp32es.ild
	-del setp32es.ilf
	-del setp32es.ils
	-del setp32es.ilk
	-del setp32es.lib
	-del setp32es.exp
	-del setp32es.pdb
	-del setp32es.tds
	-del setp32fr.res
	-del setp32fr.map
	-del setp32fr.ilc
	-del setp32fr.ild
	-del setp32fr.ilf
	-del setp32fr.ils
	-del setp32fr.ilk
	-del setp32fr.lib
	-del setp32fr.exp
	-del setp32fr.pdb
	-del setp32fr.tds
	-del setp32it.res
	-del setp32it.map
	-del setp32it.ilc
	-del setp32it.ild
	-del setp32it.ilf
	-del setp32it.ils
	-del setp32it.ilk
	-del setp32it.lib
	-del setp32it.exp
	-del setp32it.pdb
	-del setp32it.tds
	-del setupc.obj
	-del winunzip.obj
	-del gvwgs.obj
	-del gvwgs16.res
	-del gvwgs16.map
	-del gvwgs32.res
	-del gvwgs32.map
	-del gvwgs32.ilc
	-del gvwgs32.ild
	-del gvwgs32.ilf
	-del gvwgs32.ils
	-del gvwgs32.ilk
	-del gvwgs32.lib
	-del gvwgs32.exp
	-del gvwgs32.pdb
	-del gvwgs32.tds
	-del lib.rsp
	-del link.rsp
	-del vc60.pdb
	-del vc50.pdb
	-del gsprint.ilk
	-del gsprint.obj
	-del gsprint.pdb
	-del echogsv.exe
	-del echogsv.ilk
	-del echogsv.obj
	-del echogsv.pdb

veryclean: clean
	-del gsview$(WINEXT).exe
	-del gsviewen.hlp
	-del gsviewde.hlp
	-del gsviewes.hlp
	-del gsviewfr.hlp
	-del gsviewit.hlp
	-del gsvw$(WINEXT)de.dll
	-del gsvw$(WINEXT)es.dll
	-del gsvw$(WINEXT)fr.dll
	-del gsvw$(WINEXT)it.dll
	-del gsview.htm
	-del gsv16spl.exe
	-del gvwgs$(WINEXT).exe
	-del winsetup.exe
	-del setp$(WINEXT)de.dll
	-del setp$(WINEXT)es.dll
	-del setp$(WINEXT)fr.dll
	-del setp$(WINEXT)it.dll
	-del uninstgs.exe
	-del files32.txt
	-del gsviewen.htm
	-del gsviewde.htm
	-del gsviewes.htm
	-del gsviewfr.htm
	-del gsviewit.htm
	-del os2help.htm
	-del gsprint.exe
