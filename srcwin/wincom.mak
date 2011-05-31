#  Copyright (C) 1993-2004, Ghostgum Software Pty Ltd.  All rights reserved.
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

# Partial Windows makefile for GSview
# Common targets used by MSVC and cygwin


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

#!if $(VIEWONLY)
#OBJS=$(OBJCOM1) $(OBJPLAT1) # $(OD)viewonly$(OBJ) 
#!else
OBJS=$(OBJCOM1) $(OBJCOM2) $(OBJPLAT1) $(OBJPLAT2)
#!endif


ICONS=binary/gvwgs.ico binary/gvwinun.ico binary/gvwdoc.ico \
 binary/gvwin1.ico binary/gvwin2.ico binary/gvwin3.ico binary/gvwin4.ico


# Windows target

all: $(BD)gsview$(WINEXT).exe \
  $(BD)gsviewen.hlp \
  $(BD)gsvw$(WINEXT)ct.dll $(BD)gsviewct.hlp $(BD)setp$(WINEXT)ct.dll \
  $(BD)gsvw$(WINEXT)de.dll $(BD)gsviewde.hlp $(BD)setp$(WINEXT)de.dll \
  $(BD)gsvw$(WINEXT)es.dll $(BD)gsviewes.hlp $(BD)setp$(WINEXT)es.dll \
  $(BD)gsvw$(WINEXT)fr.dll $(BD)gsviewfr.hlp $(BD)setp$(WINEXT)fr.dll \
  $(BD)gsvw$(WINEXT)gr.dll $(BD)gsviewgr.hlp $(BD)setp$(WINEXT)gr.dll \
  $(BD)gsvw$(WINEXT)it.dll $(BD)gsviewit.hlp $(BD)setp$(WINEXT)it.dll \
  $(BD)gsvw$(WINEXT)nl.dll $(BD)gsviewnl.hlp $(BD)setp$(WINEXT)nl.dll \
  $(BD)gsvw$(WINEXT)ru.dll $(BD)gsviewru.hlp $(BD)setp$(WINEXT)ru.dll \
  $(BD)gsvw$(WINEXT)se.dll $(BD)gsviewse.hlp $(BD)setp$(WINEXT)se.dll \
  $(BD)gsvw$(WINEXT)sk.dll $(BD)gsviewsk.hlp $(BD)setp$(WINEXT)sk.dll \
  $(BD)gvwgs$(WINEXT).exe $(BD)setup.exe $(BD)uninstgs.exe \
  $(BD)epstool$(EXE) $(BD)gsprint.exe 

$(OD)lib.rsp: makefile
	echo "$(PLATLIBDIR)$(D)shell32.lib" > $(OD)lib.rsp
	echo "$(PLATLIBDIR)$(D)comdlg32.lib" >> $(OD)lib.rsp
	echo "$(PLATLIBDIR)$(D)gdi32.lib" >> $(OD)lib.rsp
	echo "$(PLATLIBDIR)$(D)user32.lib" >> $(OD)lib.rsp
	echo "$(PLATLIBDIR)$(D)winspool.lib" >> $(OD)lib.rsp
	echo "$(PLATLIBDIR)$(D)advapi32.lib" >> $(OD)lib.rsp
	echo "$(PLATLIBDIR)$(D)ole32.lib" >> $(OD)lib.rsp
	echo "$(PLATLIBDIR)$(D)uuid.lib" >> $(OD)lib.rsp
	echo /NODEFAULTLIB:LIBC.lib >> $(OD)lib.rsp
	echo "$(LIBDIR)$(D)libcmt.lib" >> $(OD)lib.rsp


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
$(OD)gsvw$(WINEXT)en.res: $(HDRS) $(SRCWIN)gvwin1.rc $(SRCWIN)gvwin2.rc en/gvclang.h en/gvclang.rc en/gvwlang.rc
	$(RCOMP) $(RIPATH)"en" $(VIEWFLAGS) $(ROFILE)$(OD)gsvw$(WINEXT)en.res $(SRCWIN)gvwin1.rc

$(OD)gvwgs$(WINEXT).res: $(SRCWIN)gvwgs.rc $(SRCWIN)gvwgs.h $(ICONS) $(LANGUAGE)/gvclang.h 
	$(RCOMP) $(RIPATH)"$(LANGUAGE)" $(VIEWFLAGS) $(ROFILE)$(OD)gvwgs$(WINEXT).res $(SRCWIN)gvwgs.rc

$(BD)gvwgs$(WINEXT).exe: $(SRCWIN)gvwgs.c $(SRCWIN)gvwgs.h $(OD)gvwgs$(WINEXT).res $(OD)lib.rsp $(OD)cdll$(OBJ)
	$(COMP) $(FOO)gvwgs$(WINEXT)$(OBJ) $(CO) $(SRCWIN)gvwgs.c
	$(LINK) $(DEBUGLINK) $(LGUI) $(LDEF)$(SRCWIN)gvwgs$(WINEXT).def $(LOUT)$(BD)gvwgs$(WINEXT).exe $(OD)gvwgs$(WINEXT)$(OBJ) $(OD)cdll$(OBJ)  $(OD)gvwgs$(WINEXT).res $(LIBRSP)


##########
# Catalan

$(OD)gsvw$(WINEXT)ct.res: $(HDRS) $(SRCWIN)gvwin2.rc ct/gvclang.h ct/gvclang.rc ct/gvwlang.rc
	$(RCOMP) $(RIPATH)"ct" $(ROFILE)$(OD)gsvw$(WINEXT)ct.res $(SRCWIN)gvwin2.rc

$(BD)gsvw$(WINEXT)ct.dll: $(OD)gsvw$(WINEXT)ct.res ct/gvwin32.def
	$(LINK) $(LDLL) $(LDEF)ct$(D)gvwin32.def $(LOUT)$(BD)gsvw$(WINEXT)ct.dll $(OD)gsvw$(WINEXT)ct.res

$(OD)setp$(WINEXT)ct.res: $(SRCWIN)winsetup.rc ct/gvclang.h $(SRC)gvcver.h $(SRC)gvcrc.h $(SRCWIN)winsetup.h
	$(RCOMP) $(RIPATH)"ct" $(ROFILE)$(OD)setp$(WINEXT)ct.res $(SRCWIN)winsetup.rc

$(BD)setp$(WINEXT)ct.dll: $(OD)setp$(WINEXT)ct.res ct/setup32.def
	$(LINK) $(LDLL) $(LDEF)ct$(D)setup32.def $(LOUT)$(BD)setp$(WINEXT)ct.dll $(OD)setp$(WINEXT)ct.res

##########
# German

$(OD)gsvw$(WINEXT)de.res: $(HDRS) $(SRCWIN)gvwin2.rc de/gvclang.h de/gvclang.rc de/gvwlang.rc
	$(RCOMP) $(RIPATH)"de" $(ROFILE)$(OD)gsvw$(WINEXT)de.res $(SRCWIN)gvwin2.rc

$(BD)gsvw$(WINEXT)de.dll: $(OD)gsvw$(WINEXT)de.res de/gvwin32.def
	$(LINK) $(LDLL) $(LDEF)de$(D)gvwin32.def $(LOUT)$(BD)gsvw$(WINEXT)de.dll $(OD)gsvw$(WINEXT)de.res

$(OD)setp$(WINEXT)de.res: $(SRCWIN)winsetup.rc de/gvclang.h $(SRC)gvcver.h $(SRC)gvcrc.h $(SRCWIN)winsetup.h
	$(RCOMP) $(RIPATH)"de" $(ROFILE)$(OD)setp$(WINEXT)de.res $(SRCWIN)winsetup.rc

$(BD)setp$(WINEXT)de.dll: $(OD)setp$(WINEXT)de.res de/setup32.def
	$(LINK) $(LDLL) $(LDEF)de$(D)setup32.def $(LOUT)$(BD)setp$(WINEXT)de.dll $(OD)setp$(WINEXT)de.res

##########
# Spanish

$(OD)gsvw$(WINEXT)es.res: $(HDRS) $(SRCWIN)gvwin2.rc es/gvclang.h es/gvclang.rc es/gvwlang.rc
	$(RCOMP) $(RIPATH)"es" $(ROFILE)$(OD)gsvw$(WINEXT)es.res $(SRCWIN)gvwin2.rc

$(BD)gsvw$(WINEXT)es.dll: $(OD)gsvw$(WINEXT)es.res es/gvwin32.def
	$(LINK) $(LDLL) $(LDEF)es$(D)gvwin32.def $(LOUT)$(BD)gsvw$(WINEXT)es.dll $(OD)gsvw$(WINEXT)es.res

$(OD)setp$(WINEXT)es.res: $(SRCWIN)winsetup.rc es/gvclang.h $(SRC)gvcver.h $(SRC)gvcrc.h $(SRCWIN)winsetup.h
	$(RCOMP) $(RIPATH)"es" $(ROFILE)$(OD)setp$(WINEXT)es.res $(SRCWIN)winsetup.rc

$(BD)setp$(WINEXT)es.dll: $(OD)setp$(WINEXT)es.res es/setup32.def
	$(LINK) $(LDLL) $(LDEF)es$(D)setup32.def $(LOUT)$(BD)setp$(WINEXT)es.dll $(OD)setp$(WINEXT)es.res

##########
# French

$(OD)gsvw$(WINEXT)fr.res: $(HDRS) $(SRCWIN)gvwin2.rc fr/gvclang.h fr/gvclang.rc fr/gvwlang.rc
	$(RCOMP) $(RIPATH)"fr" $(ROFILE)$(OD)gsvw$(WINEXT)fr.res $(SRCWIN)gvwin2.rc

$(BD)gsvw$(WINEXT)fr.dll: $(OD)gsvw$(WINEXT)fr.res fr/gvwin32.def
	$(LINK) $(LDLL) $(LDEF)fr$(D)gvwin32.def $(LOUT)$(BD)gsvw$(WINEXT)fr.dll $(OD)gsvw$(WINEXT)fr.res

$(OD)setp$(WINEXT)fr.res: $(SRCWIN)winsetup.rc fr/gvclang.h $(SRC)gvcver.h $(SRC)gvcrc.h $(SRCWIN)winsetup.h
	$(RCOMP) $(RIPATH)"fr" $(ROFILE)$(OD)setp$(WINEXT)fr.res $(SRCWIN)winsetup.rc

$(BD)setp$(WINEXT)fr.dll: $(OD)setp$(WINEXT)fr.res fr/setup32.def
	$(LINK) $(LDLL) $(LDEF)fr$(D)setup32.def $(LOUT)$(BD)setp$(WINEXT)fr.dll $(OD)setp$(WINEXT)fr.res

##########
# Greek

$(OD)gsvw$(WINEXT)gr.res: $(HDRS) $(SRCWIN)gvwin2.rc gr/gvclang.h gr/gvclang.rc gr/gvwlang.rc
	$(RCOMP) $(RIPATH)"gr" $(ROFILE)$(OD)gsvw$(WINEXT)gr.res $(RLANG)1253 $(SRCWIN)gvwin2.rc

$(BD)gsvw$(WINEXT)gr.dll: $(OD)gsvw$(WINEXT)gr.res gr/gvwin32.def
	$(LINK) $(LDLL) $(LDEF)gr$(D)gvwin32.def $(LOUT)$(BD)gsvw$(WINEXT)gr.dll $(OD)gsvw$(WINEXT)gr.res

$(OD)setp$(WINEXT)gr.res: $(SRCWIN)winsetup.rc gr/gvclang.h $(SRC)gvcver.h $(SRC)gvcrc.h $(SRCWIN)winsetup.h
	$(RCOMP) $(RIPATH)"gr" $(ROFILE)$(OD)setp$(WINEXT)gr.res $(RLANG)1253 $(SRCWIN)winsetup.rc

$(BD)setp$(WINEXT)gr.dll: $(OD)setp$(WINEXT)gr.res gr/setup32.def
	$(LINK) $(LDLL) $(LDEF)gr$(D)setup32.def $(LOUT)$(BD)setp$(WINEXT)gr.dll $(OD)setp$(WINEXT)gr.res

##########
# Italian

$(OD)gsvw$(WINEXT)it.res: $(HDRS) $(SRCWIN)gvwin2.rc it/gvclang.h it/gvclang.rc it/gvwlang.rc
	$(RCOMP) $(RIPATH)"it" $(ROFILE)$(OD)gsvw$(WINEXT)it.res $(SRCWIN)gvwin2.rc

$(BD)gsvw$(WINEXT)it.dll: $(OD)gsvw$(WINEXT)it.res it/gvwin32.def
	$(LINK) $(LDLL) $(LDEF)it$(D)gvwin32.def $(LOUT)$(BD)gsvw$(WINEXT)it.dll $(OD)gsvw$(WINEXT)it.res

$(OD)setp$(WINEXT)it.res: $(SRCWIN)winsetup.rc it/gvclang.h $(SRC)gvcver.h $(SRC)gvcrc.h $(SRCWIN)winsetup.h
	$(RCOMP) $(RIPATH)"it" $(ROFILE)$(OD)setp$(WINEXT)it.res $(SRCWIN)winsetup.rc

$(BD)setp$(WINEXT)it.dll: $(OD)setp$(WINEXT)it.res it/setup32.def
	$(LINK) $(LDLL) $(LDEF)it$(D)setup32.def $(LOUT)$(BD)setp$(WINEXT)it.dll $(OD)setp$(WINEXT)it.res

##########
# Dutch

$(OD)gsvw$(WINEXT)nl.res: $(HDRS) $(SRCWIN)gvwin2.rc nl/gvclang.h nl/gvclang.rc nl/gvwlang.rc
	$(RCOMP) $(RIPATH)"nl" $(ROFILE)$(OD)gsvw$(WINEXT)nl.res $(SRCWIN)gvwin2.rc

$(BD)gsvw$(WINEXT)nl.dll: $(OD)gsvw$(WINEXT)nl.res nl/gvwin32.def
	$(LINK) $(LDLL) $(LDEF)nl$(D)gvwin32.def $(LOUT)$(BD)gsvw$(WINEXT)nl.dll $(OD)gsvw$(WINEXT)nl.res

$(OD)setp$(WINEXT)nl.res: $(SRCWIN)winsetup.rc nl/gvclang.h $(SRC)gvcver.h $(SRC)gvcrc.h $(SRCWIN)winsetup.h
	$(RCOMP) $(RIPATH)"nl" $(ROFILE)$(OD)setp$(WINEXT)nl.res $(SRCWIN)winsetup.rc

$(BD)setp$(WINEXT)nl.dll: $(OD)setp$(WINEXT)nl.res nl/setup32.def
	$(LINK) $(LDLL) $(LDEF)nl$(D)setup32.def $(LOUT)$(BD)setp$(WINEXT)nl.dll $(OD)setp$(WINEXT)nl.res

##########
# Russian

$(OD)gsvw$(WINEXT)ru.res: $(HDRS) $(SRCWIN)gvwin2.rc ru/gvclang.h ru/gvclang.rc ru/gvwlang.rc
	$(RCOMP) $(RIPATH)"ru" $(ROFILE)$(OD)gsvw$(WINEXT)ru.res $(RLANG)1251 $(SRCWIN)gvwin2.rc

$(BD)gsvw$(WINEXT)ru.dll: $(OD)gsvw$(WINEXT)ru.res ru/gvwin32.def
	$(LINK) $(LDLL) $(LDEF)ru$(D)gvwin32.def $(LOUT)$(BD)gsvw$(WINEXT)ru.dll $(OD)gsvw$(WINEXT)ru.res

$(OD)setp$(WINEXT)ru.res: $(SRCWIN)winsetup.rc ru/gvclang.h $(SRC)gvcver.h $(SRC)gvcrc.h $(SRCWIN)winsetup.h
	$(RCOMP) $(RIPATH)"ru" $(ROFILE)$(OD)setp$(WINEXT)ru.res $(RLANG)1251 $(SRCWIN)winsetup.rc

$(BD)setp$(WINEXT)ru.dll: $(OD)setp$(WINEXT)ru.res ru/setup32.def
	$(LINK) $(LDLL) $(LDEF)ru$(D)setup32.def $(LOUT)$(BD)setp$(WINEXT)ru.dll $(OD)setp$(WINEXT)ru.res


##########
# Swedish

$(OD)gsvw$(WINEXT)se.res: $(HDRS) $(SRCWIN)gvwin2.rc se/gvclang.h se/gvclang.rc se/gvwlang.rc
	$(RCOMP) $(RIPATH)"se" $(ROFILE)$(OD)gsvw$(WINEXT)se.res $(SRCWIN)gvwin2.rc

$(BD)gsvw$(WINEXT)se.dll: $(OD)gsvw$(WINEXT)se.res se/gvwin32.def
	$(LINK) $(LDLL) $(LDEF)se$(D)gvwin32.def $(LOUT)$(BD)gsvw$(WINEXT)se.dll $(OD)gsvw$(WINEXT)se.res

$(OD)setp$(WINEXT)se.res: $(SRCWIN)winsetup.rc se/gvclang.h $(SRC)gvcver.h $(SRC)gvcrc.h $(SRCWIN)winsetup.h
	$(RCOMP) $(RIPATH)"se" $(ROFILE)$(OD)setp$(WINEXT)se.res $(SRCWIN)winsetup.rc

$(BD)setp$(WINEXT)se.dll: $(OD)setp$(WINEXT)se.res se/setup32.def
	$(LINK) $(LDLL) $(LDEF)se$(D)setup32.def $(LOUT)$(BD)setp$(WINEXT)se.dll $(OD)setp$(WINEXT)se.res


##########
# Slovak

$(OD)gsvw$(WINEXT)sk.res: $(HDRS) $(SRCWIN)gvwin2.rc sk/gvclang.h sk/gvclang.rc sk/gvwlang.rc
	$(RCOMP) $(RIPATH)"sk" $(ROFILE)$(OD)gsvw$(WINEXT)sk.res $(RLANG)1250 $(SRCWIN)gvwin2.rc

$(BD)gsvw$(WINEXT)sk.dll: $(OD)gsvw$(WINEXT)sk.res sk/gvwin32.def
	$(LINK) $(LDLL) $(LDEF)sk$(D)gvwin32.def $(LOUT)$(BD)gsvw$(WINEXT)sk.dll $(OD)gsvw$(WINEXT)sk.res

$(OD)setp$(WINEXT)sk.res: $(SRCWIN)winsetup.rc sk/gvclang.h $(SRC)gvcver.h $(SRC)gvcrc.h $(SRCWIN)winsetup.h
	$(RCOMP) $(RIPATH)"sk" $(ROFILE)$(OD)setp$(WINEXT)sk.res $(RLANG)1250 $(SRCWIN)winsetup.rc

$(BD)setp$(WINEXT)sk.dll: $(OD)setp$(WINEXT)sk.res sk/setup32.def
	$(LINK) $(LDLL) $(LDEF)sk$(D)setup32.def $(LOUT)$(BD)setp$(WINEXT)sk.dll $(OD)setp$(WINEXT)sk.res


##########
# Windows setup

$(OD)winsetup$(OBJ): $(SRCWIN)winsetup.cpp $(SRCWIN)winsetup.h $(SRC)gvcrc.h $(SRC)gvcbeta.h $(SRCWIN)dwinst.h $(LANGUAGE)/gvclang.h 
	$(COMP) -I$(LANGUAGE) $(FOO)winsetup$(OBJ) $(CO) $(SRCWIN)winsetup.cpp

$(OD)dwinst$(OBJ): $(SRCWIN)dwinst.cpp $(SRCWIN)dwinst.h
	$(COMP) $(FOO)dwinst$(OBJ) $(CO) $(SRCWIN)dwinst.cpp

$(OD)dwuninst$(OBJ): $(SRCWIN)dwuninst.cpp $(SRCWIN)dwuninst.h
	$(COMP) $(FOO)dwuninst$(OBJ) $(CO) $(SRCWIN)dwuninst.cpp

$(OD)winsetup.res: $(SRCWIN)winsetup.rc $(SRCWIN)winsetup.h $(LANGUAGE)/gvclang.h
	$(RCOMP) $(RIPATH)"$(LANGUAGE)" $(ROFILE)$(OD)winsetup.res $(SRCWIN)winsetup.rc

$(BD)setup.exe: $(OD)winsetup$(OBJ) $(OD)winsetup.res $(SRCWIN)winsetup.def $(OD)dwinst$(OBJ) $(OD)gvcbetaa$(OBJ) $(OD)gvwgsver$(OBJ) $(OD)lib.rsp
	$(LINK) $(DEBUGLINK) $(LGUI) $(LDEF)$(SRCWIN)winsetup.def $(LOUT)$(BD)setup.exe $(OD)winsetup$(OBJ) $(OD)dwinst$(OBJ) $(OD)gvcbetaa$(OBJ) $(OD)gvwgsver$(OBJ) $(LIBRSP) $(OD)winsetup.res

$(OD)dwuninst.res: $(SRCWIN)dwuninst.rc $(SRCWIN)dwuninst.h
	$(RCOMP) $(RIPATH)"$(LANGUAGE)" $(ROFILE)$(OD)dwuninst.res $(SRCWIN)dwuninst.rc

$(BD)uninstgs.exe: $(OD)dwuninst$(OBJ) $(SRCWIN)dwuninst.h $(OD)dwuninst.res $(SRCWIN)dwuninst.def
	$(LINK) $(DEBUGLINK) $(LGUI) $(LDEF)$(SRCWIN)dwuninst.def $(LOUT)$(BD)uninstgs.exe $(OD)dwuninst$(OBJ) $(LIBRSP) $(OD)dwuninst.res

##########
# Documentation

$(BD)gsviewen.hlp: $(GVDOC) $(DOC2RTF) en/gvclang.txt en/gsview.hpj
	$(CP) en$(D)gsview.hpj $(OD)gsviewen.hpj
	$(GVDOC) W en$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewen.hpj
	$(CP) $(OD)gsviewen.hlp $(BD)gsviewen.hlp

$(BD)gsviewde.hlp: $(GVDOC) $(DOC2RTF) de/gvclang.txt de/gsview.hpj
	$(CP) de$(D)gsview.hpj $(OD)gsviewde.hpj
	$(GVDOC) W de$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewde.hpj
	$(CP) $(OD)gsviewde.hlp $(BD)gsviewde.hlp

$(BD)gsviewes.hlp: $(GVDOC) $(DOC2RTF) es/gvclang.txt es/gsview.hpj
	$(CP) es$(D)gsview.hpj $(OD)gsviewes.hpj
	$(GVDOC) W es$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewes.hpj
	$(CP) $(OD)gsviewes.hlp $(BD)gsviewes.hlp

$(BD)gsviewfr.hlp: $(GVDOC) $(DOC2RTF) fr/gvclang.txt fr/gsview.hpj
	$(CP) fr$(D)gsview.hpj $(OD)gsviewfr.hpj
	$(GVDOC) W fr$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewfr.hpj
	$(CP) $(OD)gsviewfr.hlp $(BD)gsviewfr.hlp

$(BD)gsviewgr.hlp: $(GVDOC) $(DOC2RTF) gr/gvclang.txt gr/gsview.hpj
	$(CP) gr$(D)gsview.hpj $(OD)gsviewgr.hpj
	$(GVDOC) W gr$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewgr.hpj
	$(CP) $(OD)gsviewgr.hlp $(BD)gsviewgr.hlp

$(BD)gsviewit.hlp: $(GVDOC) $(DOC2RTF) it/gvclang.txt it/gsview.hpj
	$(CP) it$(D)gsview.hpj $(OD)gsviewit.hpj
	$(GVDOC) W it$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewit.hpj
	$(CP) $(OD)gsviewit.hlp $(BD)gsviewit.hlp

$(BD)gsviewnl.hlp: $(GVDOC) $(DOC2RTF) nl/gvclang.txt nl/gsview.hpj
	$(CP) nl$(D)gsview.hpj $(OD)gsviewnl.hpj
	$(GVDOC) W nl$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewnl.hpj
	$(CP) $(OD)gsviewnl.hlp $(BD)gsviewnl.hlp

$(BD)gsviewru.hlp: $(GVDOC) $(DOC2RTF) ru/gvclang.txt ru/gsview.hpj
	$(CP) ru$(D)gsview.hpj $(OD)gsviewru.hpj
	$(GVDOC) W ru$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewru.hpj
	$(CP) $(OD)gsviewru.hlp $(BD)gsviewru.hlp

$(BD)gsviewse.hlp: $(GVDOC) $(DOC2RTF) se/gvclang.txt se/gsview.hpj
	$(CP) se$(D)gsview.hpj $(OD)gsviewse.hpj
	$(GVDOC) W se$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewse.hpj
	$(CP) $(OD)gsviewse.hlp $(BD)gsviewse.hlp

$(BD)gsviewsk.hlp: $(GVDOC) $(DOC2RTF) sk/gvclang.txt sk/gsview.hpj
	$(CP) sk$(D)gsview.hpj $(OD)gsviewsk.hpj
	$(GVDOC) W sk$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewsk.hpj
	$(CP) $(OD)gsviewsk.hlp $(BD)gsviewsk.hlp

$(BD)gsviewct.hlp: $(GVDOC) $(DOC2RTF) ct/gvclang.txt ct/gsview.hpj
	$(CP) ct$(D)gsview.hpj $(OD)gsviewct.hpj
	$(GVDOC) W ct$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2RTF) $(OD)gsview.txt $(OD)gsview.rtf
	$(HC) $(OD)gsviewct.hpj
	$(CP) $(OD)gsviewct.hlp $(BD)gsviewct.hlp

html: $(DOC2HTML) $(CODEPAGE) $(GVDOC) en/gvclang.txt de/gvclang.txt es/gvclang.txt fr/gvclang.txt gr/gvclang.txt it/gvclang.txt nl/gvclang.txt ru/gvclang.txt se/gvclang.txt
	$(GVDOC) W en$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewen.htm
	$(RM) $(OD)gsview.txt
	$(GVDOC) W de$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewde.htm
	$(RM) $(OD)gsview.txt
	$(GVDOC) W es$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewes.htm
	$(RM) $(OD)gsview.txt
	$(GVDOC) W fr$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewfr.htm
	$(RM) $(OD)gsview.txt
	$(GVDOC) W gr$(D)gvclang.txt $(OD)temp.txt
#	$(CODEPAGE) 1253 8859-7 $(OD)temp.txt $(OD)gsview.txt
#	$(RM) $(OD)temp.txt
#	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewgr.htm ISO-8859-7
	$(CODEPAGE) 1253 UTF-8 $(OD)temp.txt $(OD)gsview.txt
	$(RM) $(OD)temp.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewgr.htm UTF-8
	$(RM) $(OD)gsview.txt
	$(GVDOC) W it$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewit.htm
	$(RM) $(OD)gsview.txt
	$(GVDOC) W nl$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewnl.htm
	$(RM) $(OD)gsview.txt
	$(GVDOC) W ru$(D)gvclang.txt $(OD)temp.txt
	$(CODEPAGE) 1251 UTF-8 $(OD)temp.txt $(OD)gsview.txt
	$(RM) $(OD)temp.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewru.htm UTF-8
	$(RM) $(OD)gsview.txt
	$(GVDOC) W se$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewse.htm
	$(RM) $(OD)gsview.txt
	$(GVDOC) W sk$(D)gvclang.txt $(OD)temp.txt
	$(CODEPAGE) 1250 UTF-8 $(OD)temp.txt $(OD)gsview.txt
	$(RM) $(OD)temp.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewsk.htm UTF-8
	$(RM) $(OD)gsview.txt
	$(GVDOC) W ct$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)gsviewct.htm
	$(RM) $(OD)gsview.txt
	$(GVDOC) P en$(D)gvclang.txt $(OD)gsview.txt
	$(DOC2HTML) $(OD)gsview.txt $(BD)os2help.htm
	$(RM) $(OD)gsview.txt

##########
# gsprint

GSPRINTOBJS=$(OD)gsprint$(OBJ) $(OD)gvwfile$(OBJ) $(OD)gvwdib$(OBJ) \
 $(OD)gvwpdib$(OBJ) $(OD)gvwgsver$(OBJ)

$(BD)gsprint.exe: $(GSPRINTOBJS) $(OD)lib.rsp
	$(LINK) $(DEBUGLINK) $(LCONSOLE) $(LOUT)$(BD)gsprint.exe $(GSPRINTOBJS) $(LIBRSP)

$(OD)gsprint$(OBJ): $(SRCWIN)gsprint.cpp $(SRC)gvcfile.h $(SRCWIN)gvwdib.h $(SRCWIN)gvwpdib.h $(SRC)gvcver.h $(OD)gsvver.h
	$(CPPCOMP) $(FOO)gsprint$(OBJ) $(CO) $(SRCWIN)gsprint.cpp

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
	-$(RM) $(OD)*.HLP
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
	-$(RM) $(BD)*.htm

veryclean: clean distclean
	-$(RM) $(BD)*.exe
	-$(RM) $(BD)*.dll
	-$(RM) $(BD)*.hlp


srczip:
	-$(RM) $(OD)src.txt
	-$(RM) gsv$(GSVIEW_VERSION)src.zip
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
	echo $(DISTDIR)/ct >> $(OD)src.txt
	echo $(DISTDIR)/de >> $(OD)src.txt
	echo $(DISTDIR)/es >> $(OD)src.txt
	echo $(DISTDIR)/fr >> $(OD)src.txt
	echo $(DISTDIR)/gr >> $(OD)src.txt
	echo $(DISTDIR)/it >> $(OD)src.txt
	echo $(DISTDIR)/nl >> $(OD)src.txt
	echo $(DISTDIR)/ru >> $(OD)src.txt
	echo $(DISTDIR)/se >> $(OD)src.txt
	echo $(DISTDIR)/sk >> $(OD)src.txt
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
	zip -X -9 -r -@ $(DISTDIR)$(D)gsv$(GSVIEW_VERSION)src.zip < $(DISTDIR)$(D)$(OD)src.txt
	cd $(DISTDIR)
	$(RM) $(OD)src.txt

srctar:
	-$(RM) gsview-$(GSVIEW_DOT_VERSION)_src.zip
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
	echo $(DISTDIR)/ct >> $(OD)src_tar.txt
	echo $(DISTDIR)/de >> $(OD)src_tar.txt
	echo $(DISTDIR)/es >> $(OD)src_tar.txt
	echo $(DISTDIR)/fr >> $(OD)src_tar.txt
	echo $(DISTDIR)/gr >> $(OD)src_tar.txt
	echo $(DISTDIR)/it >> $(OD)src_tar.txt
	echo $(DISTDIR)/nl >> $(OD)src_tar.txt
	echo $(DISTDIR)/ru >> $(OD)src_tar.txt
	echo $(DISTDIR)/se >> $(OD)src_tar.txt
	echo $(DISTDIR)/sk >> $(OD)src_tar.txt
	cd ..
	zip -X -r -ll -@ $(DISTDIR)$(D)gsview-$(GSVIEW_DOT_VERSION)_src.zip < $(DISTDIR)$(D)$(OD)src_tar.txt
	zip -X $(DISTDIR)$(D)gsview-$(GSVIEW_DOT_VERSION)_src.zip $(DISTDIR)$(D)binary$(D)gsview48.png
	cd $(DISTDIR)
	echo Copy gsview-$(GSVIEW_DOT_VERSION)_src.zip to Unix, unzip then tar and gzip.
	-$(RM) $(OD)src_tar.txt

viewonlydist:
	-mkdir dist
	-mkdir dist$(D)gsview
	$(CP) FILE_ID.DIZ dist$(D)FILE_ID.DIZ
	$(CP) $(SRC)refresh.htm dist$(D)Readme.htm
	$(CP) LICENCE dist$(D)LICENCE
	$(CP) LICENCE dist$(D)gsview$(D)LICENCE
	$(CP) Readme.htm dist$(D)gsview$(D)Readme.htm
	$(CP) gsview.css dist$(D)gsview$(D)gsview.css
	$(CP) cdorder.txt dist$(D)gsview$(D)cdorder.txt
	$(CP) regorder.txt dist$(D)gsview$(D)regorder.txt
	$(CP) $(BD)gsview32.exe dist$(D)gsview$(D)gsview32.exe
	$(CP) binary$(D)gvwin4.ico dist$(D)gsview$(D)gsview32.ico
	$(CP) $(BD)gsviewen.hlp dist$(D)gsview$(D)gsviewen.hlp
	$(CP) $(SRC)printer.ini dist$(D)gsview$(D)printer.ini
	$(CP) NUL dist$(D)gsview$(D)gsview32.ini
	$(CP) $(BD)uninstgs.exe dist$(D)gsview$(D)uninstgs.exe
	$(CP) $(BD)setup.exe dist$(D)setup.exe
	$(CP) gsview$(D)zlib32.dll dist$(D)gsview$(D)zlib32.dll
	$(CP) gsview$(D)libbz2.dll dist$(D)gsview$(D)libbz2.dll
	echo gsview$(D)gsview32.exe> $(OD)viewlist.txt
	echo gsview$(D)gsview32.ico>> $(OD)viewlist.txt
	echo gsview$(D)uninstgs.exe>> $(OD)viewlist.txt
	echo gsview$(D)printer.ini>> $(OD)viewlist.txt
	echo gsview$(D)gsview32.ini>> $(OD)viewlist.txt
	echo gsview$(D)zlib32.dll>> $(OD)viewlist.txt
	echo gsview$(D)libbz2.dll>> $(OD)viewlist.txt
	echo gsview$(D)Readme.htm>> $(OD)viewlist.txt
	echo gsview$(D)gsview.css>> $(OD)viewlist.txt
	echo gsview$(D)cdorder.txt>> $(OD)viewlist.txt
	echo gsview$(D)regorder.txt>> $(OD)viewlist.txt
	echo gsview$(D)LICENCE>> $(OD)viewlist.txt
	echo GSview $(GSVIEW_DOT_VERSION)> $(OD)$(D)viewlist.tmp
	echo gsview>> $(OD)viewlist.tmp
	$(CP) $(OD)viewlist.tmp+$(OD)viewlist.txt dist$(D)filelist.txt
	$(CP) $(OD)viewlist.txt $(OD)files32.txt
	echo Readme.htm>> $(OD)files32.txt
	echo FILE_ID.DIZ>> $(OD)files32.txt
	echo filelist.txt>> $(OD)files32.txt
	echo setup.exe>> $(OD)files32.txt
	cd dist
	-$(RM) ..$(D)gsv$(GSVIEW_VERSION)w32.zip
	-$(RM) ..$(D)gsv$(GSVIEW_VERSION)w32.exe
	zip -X -9 -@ ..$(D)gsv$(GSVIEW_VERSION)w32.zip < ..$(D)$(OD)files32.txt
	cd ..
	cd dist
	echo -win32 -setup > setup.rsp
	echo -st "GSview $(GSVIEW_DOT_VERSION) for Win32" >> setup.rsp
	echo -i gsview$(D)gsview32.ico >> setup.rsp
	echo -a about.txt >> setup.rsp
	echo -t dialog.txt >> setup.rsp
	echo -c .$(D)setup.exe >> setup.rsp
	echo GSview is Copyright (C) 2004 Ghostgum Software Pty Ltd. > about.txt
	echo See licence in gsview$(D)LICENCE >> about.txt
	echo This installs GSview $(GSVIEW_DOT_VERSION) for Win32. > dialog.txt
	echo GSview uses Ghostscript to display, print and convert PostScript and PDF files. >> dialog.txt
	$(WINZIPSE_XE) ..$(D)gsv$(GSVIEW_VERSION)w32 @setup.rsp
# Don't delete temporary files, because make continues
# before these files are used.
#	-$(RM) setup.rsp 
#	-$(RM) about.txt
#	-$(RM) dialog.txt
	cd ..

distcopy:
	-mkdir dist
	-mkdir dist$(D)gsview
	-mkdir dist$(D)pstotext
	$(CP) FILE_ID.DIZ dist$(D)FILE_ID.DIZ
	$(CP) $(SRC)refresh.htm dist$(D)Readme.htm
	$(CP) LICENCE dist$(D)LICENCE
	$(CP) LICENCE dist$(D)gsview$(D)LICENCE
	$(CP) Readme.htm dist$(D)gsview$(D)Readme.htm
	$(CP) gsview.css dist$(D)gsview$(D)gsview.css
	$(CP) cdorder.txt dist$(D)gsview$(D)cdorder.txt
	$(CP) regorder.txt dist$(D)gsview$(D)regorder.txt
	$(CP) $(BD)gsview32.exe dist$(D)gsview$(D)gsview32.exe
	$(CP) binary$(D)gvwin4.ico dist$(D)gsview$(D)gsview32.ico
	$(CP) $(BD)gsviewen.hlp dist$(D)gsview$(D)gsviewen.hlp
	$(CP) $(BD)gsviewde.hlp dist$(D)gsview$(D)gsviewde.hlp
	$(CP) $(BD)gsviewes.hlp dist$(D)gsview$(D)gsviewes.hlp
	$(CP) $(BD)gsviewfr.hlp dist$(D)gsview$(D)gsviewfr.hlp
	$(CP) $(BD)gsviewgr.hlp dist$(D)gsview$(D)gsviewgr.hlp
	$(CP) $(BD)gsviewit.hlp dist$(D)gsview$(D)gsviewit.hlp
	$(CP) $(BD)gsviewnl.hlp dist$(D)gsview$(D)gsviewnl.hlp
	$(CP) $(BD)gsviewru.hlp dist$(D)gsview$(D)gsviewru.hlp
	$(CP) $(BD)gsviewse.hlp dist$(D)gsview$(D)gsviewse.hlp
	$(CP) $(BD)gsviewsk.hlp dist$(D)gsview$(D)gsviewsk.hlp
	$(CP) $(BD)gsviewct.hlp dist$(D)gsview$(D)gsviewct.hlp
	$(CP) $(BD)gsvw32de.dll dist$(D)gsview$(D)gsvw32de.dll
	$(CP) $(BD)gsvw32es.dll dist$(D)gsview$(D)gsvw32es.dll
	$(CP) $(BD)gsvw32fr.dll dist$(D)gsview$(D)gsvw32fr.dll
	$(CP) $(BD)gsvw32gr.dll dist$(D)gsview$(D)gsvw32gr.dll
	$(CP) $(BD)gsvw32it.dll dist$(D)gsview$(D)gsvw32it.dll
	$(CP) $(BD)gsvw32nl.dll dist$(D)gsview$(D)gsvw32nl.dll
	$(CP) $(BD)gsvw32ru.dll dist$(D)gsview$(D)gsvw32ru.dll
	$(CP) $(BD)gsvw32se.dll dist$(D)gsview$(D)gsvw32se.dll
	$(CP) $(BD)gsvw32sk.dll dist$(D)gsview$(D)gsvw32sk.dll
	$(CP) $(BD)gsvw32ct.dll dist$(D)gsview$(D)gsvw32ct.dll
	$(CP) $(BD)gvwgs32.exe dist$(D)gsview$(D)gvwgs32.exe
	$(CP) $(SRC)printer.ini dist$(D)gsview$(D)printer.ini
	$(CP) NUL dist$(D)gsview$(D)gsview32.ini
	$(CP) $(BD)uninstgs.exe dist$(D)gsview$(D)uninstgs.exe
	$(CP) $(BD)setup.exe dist$(D)setup.exe
	$(CP) $(BD)setp32de.dll dist$(D)setp32de.dll
	$(CP) $(BD)setp32es.dll dist$(D)setp32es.dll
	$(CP) $(BD)setp32fr.dll dist$(D)setp32fr.dll
	$(CP) $(BD)setp32gr.dll dist$(D)setp32gr.dll
	$(CP) $(BD)setp32it.dll dist$(D)setp32it.dll
	$(CP) $(BD)setp32nl.dll dist$(D)setp32nl.dll
	$(CP) $(BD)setp32ru.dll dist$(D)setp32ru.dll
	$(CP) $(BD)setp32se.dll dist$(D)setp32se.dll
	$(CP) $(BD)setp32sk.dll dist$(D)setp32sk.dll
	$(CP) $(BD)setp32ct.dll dist$(D)setp32ct.dll
	$(CP) gsprint.htm dist$(D)gsview$(D)gsprint.htm
	$(CP) $(BD)gsprint.exe dist$(D)gsview$(D)gsprint.exe
	$(CP) epstool.htm dist$(D)gsview$(D)epstool.htm
	$(CP) $(BD)epstool.exe dist$(D)gsview$(D)epstool.exe
	$(CP) gsview$(D)gsv16spl.exe dist$(D)gsview$(D)gsv16spl.exe
	$(CP) gsview$(D)zlib32.dll dist$(D)gsview$(D)zlib32.dll
	$(CP) gsview$(D)libbz2.dll dist$(D)gsview$(D)libbz2.dll
	$(CP) pstotext$(D)pstotext.1 dist$(D)pstotext$(D)pstotext.1
	$(CP) pstotext$(D)pstotext.txt dist$(D)pstotext$(D)pstotext.txt
	$(CP) pstotext$(D)pstotxt3.dll dist$(D)pstotext$(D)pstotxt3.dll
	$(CP) pstotext$(D)pstotxt3.exe dist$(D)pstotext$(D)pstotxt3.exe
	echo GSview $(GSVIEW_DOT_VERSION)> $(OD)filelist.tmp
	echo gsview>> $(OD)filelist.tmp
	$(CP) $(OD)filelist.tmp+$(SRCWIN)distlist.txt dist$(D)filelist.txt
	$(RM) $(OD)filelist.tmp

$(OD)files32.txt: $(SRCWIN)distlist.txt $(SRCWIN)win.mak makefile
	$(CP) $(SRCWIN)distlist.txt $(OD)files32.txt
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
	echo setp32ru.dll >> $(OD)files32.txt
	echo setp32se.dll >> $(OD)files32.txt
	echo setp32sk.dll >> $(OD)files32.txt
	echo setp32ct.dll >> $(OD)files32.txt

gsv$(GSVIEW_VERSION)w32.zip: distcopy $(OD)files32.txt
	-$(RM) gsv$(GSVIEW_VERSION)w32.zip 
	cd dist
	zip -X -9 -@ ..$(D)gsv$(GSVIEW_VERSION)w32.zip < ..$(D)$(OD)files32.txt
	cd ..



# Now convert to a self extracting archive.
# This involves making a few temporary files.
gsv$(GSVIEW_VERSION)w32.exe: distcopy gsv$(GSVIEW_VERSION)w32.zip
	cd dist
	echo -win32 -setup > setup.rsp
	echo -st "GSview $(GSVIEW_DOT_VERSION) for Win32" >> setup.rsp
	echo -i gsview$(D)gsview32.ico >> setup.rsp
	echo -a about.txt >> setup.rsp
	echo -t dialog.txt >> setup.rsp
	echo -c .$(D)setup.exe >> setup.rsp
	echo GSview is Copyright (C) 2004 Ghostgum Software Pty Ltd. > about.txt
	echo See licence in gsview$(D)LICENCE >> about.txt
	echo This installs GSview $(GSVIEW_DOT_VERSION) for Win32. > dialog.txt
	echo GSview uses Ghostscript to display, print and convert PostScript and PDF files. >> dialog.txt
	$(WINZIPSE_XE) ..$(D)gsv$(GSVIEW_VERSION)w32 @setup.rsp
# Don't delete temporary files, because make continues
# before these files are used.
#	-$(RM) setup.rsp 
#	-$(RM) about.txt
#	-$(RM) dialog.txt
	cd ..

zip: gsv$(GSVIEW_VERSION)w32.exe

