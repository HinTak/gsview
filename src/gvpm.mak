#  Copyright (C) 1993-2001, Ghostgum Software Pty Ltd.  All rights reserved.
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

# gvpm.mak
# PM GSview 
# requires emx 0.9b or later
#
# edit COMPBASE and EMXPATH as required.

# set one of the following to non-zero
# only USE_EMX currently works
USE_EMX=1
USE_BCC=0
USE_IBM=0
# USE_OMF=1 for EMX/GCC with LINK386
USE_OMF=1

# DEBUG=1 for debugging
DEBUG=0

!if $(USE_EMX)
# EMX
DRIVE=c:
COMP=gcc
COMPBASE=$(DRIVE)\emx
EMXPATH=$(DRIVE)/emx
!if $(USE_OMF)
OBJ=obj
!if $(DEBUG)
CDEBUG=-g
LDEBUG=/DEBUG
!endif
FLAGS=-O -Wall -Zomf -Zmts $(CDEBUG)
!else
OBJ=o
!if $(DEBUG)
CDEBUG=-g
LDEBUG=-g
!endif
FLAGS=-O -Zmts $(CDEBUG)
!endif
!endif

!if $(USE_IBM)
# ICC flags
COMP=icc
COMPBASE=d:\ibmcpp
EMXPATH=d:/ibmcpp
!if $(DEBUG)
CDEBUG=/Ti /Gm
LDEBUG=/DEBUG
!endif
FLAGS=/Q $(CDEBUG) /Sm /Id:\toolkit\h;d:\ibmcpp\include -DNO_MMOS2
OBJ=obj
!endif

!if $(USE_BCC)
# BCC flags
COMP=bcc
COMPBASE=d:\bcos2
EMXPATH=d:/bcos2
FLAGS=-v -I$(INCDIR) -sm
OBJ=obj
!endif


COMPDIR=$(COMPBASE)\bin
INCDIR=$(EMXPATH)/include
LIBDIR=$(EMXPATH)/lib


OBJS=gvpm.$(OBJ) gvpdlg.$(OBJ) gvpdisp.$(OBJ) gvpedit.$(OBJ) gvpeps.$(OBJ)\
   gvpgsver.$(OBJ) gvpinit.$(OBJ) gvpmeas.$(OBJ) gvpmisc.$(OBJ) gvpprn.$(OBJ)\
   gvccmd.$(OBJ) gvcdisp.$(OBJ) gvcedit.$(OBJ) gvceps.$(OBJ) gvcfile.$(OBJ) gvcinit.$(OBJ)\
   gvcbeta.$(OBJ) gvcmeas.$(OBJ) gvcmeas2.$(OBJ) gvcmisc.$(OBJ) gvcprf.$(OBJ)\
   gvcprn.$(OBJ) gvctext.$(OBJ) gvpdll.$(OBJ) gvcdll.$(OBJ) gvcpdf.$(OBJ)\
   dscparse.$(OBJ) dscutil.$(OBJ) gvcreg.$(OBJ) gvpreg.$(OBJ)\
   cdll.$(OBJ) cimg.$(OBJ) cview.$(OBJ)
GSHDRS=iapi.h errors.h gdevdsp.h
HDRS=gsvver.h gvcrc.h gvpm.h dscparse.h gvcfn.h gvcver.h gvcfile.h $(GSHDRS)


all: gvpm.exe\
 gvpmen.hlp\
 gvpmde.hlp gvpmde.dll setup2de.dll\
 gvpmes.hlp gvpmes.dll setup2es.dll\
 gvpmfr.hlp gvpmfr.dll setup2fr.dll\
 gvpmgr.hlp gvpmgr.dll setup2gr.dll\
 gvpmit.hlp gvpmit.dll setup2it.dll\
 gvpmnl.hlp gvpmnl.dll setup2nl.dll\
 gvpgs.exe\
 os2setup.exe

.c.$(OBJ):
	$(COMP) $(FLAGS) -DOS2 -c $*.c

ECHOGSV=echogsv.exe

!include "gvcver.mak"

echogsv.exe: echogsv.c
	$(COMP) $(FLAGS) echogsv.c

gvpm.$(OBJ): gvpm.c $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvpdlg.$(OBJ): gvpdlg.c gvcrc.h $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvpdll.$(OBJ): gvpdll.c gvcrc.h $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvpdisp.$(OBJ): gvpdisp.c  $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvpedit.$(OBJ): gvpedit.c $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvpeps.$(OBJ): gvpeps.c gvceps.h $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvpgsver.$(OBJ): gvpgsver.c $(HDRS) gvcrc.h
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvpinit.$(OBJ): gvpinit.c $(HDRS) gvcrc.h
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvpmeas.$(OBJ): gvpmeas.c $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvpmisc.$(OBJ): gvpmisc.c $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvpprn.$(OBJ): gvpprn.c $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvpreg.$(OBJ): gvpreg.c $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvccmd.$(OBJ): gvccmd.c gvcrc.h $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvcdisp.$(OBJ): gvcdisp.c $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvcdll.$(OBJ): gvcdll.c gvcrc.h $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

dscparse.$(OBJ): dscparse.c dscparse.h
	$(COMP) $(FLAGS) -DOS2 -c $*.c

dscutil.$(OBJ): dscutil.c dscparse.h
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvcbeta.obj: gvcbeta.c gvcbeta.h $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvceps.$(OBJ): gvceps.c gvceps.h $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvcedit.$(OBJ): gvcedit.c $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvcfile.$(OBJ): gvcfile.c gvcfile.h $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvcinit.$(OBJ): gvcinit.c $(HDRS) gvcrc.h
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvcmeas.$(OBJ): gvcmeas.c gvcrc.h $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvcmeas2.$(OBJ): gvcmeas2.c gvcrc.h $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvcmisc.$(OBJ): gvcmisc.c gvcrc.h $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvcpdf.$(OBJ): gvcpdf.c $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvcprn.$(OBJ): gvcprn.c $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvcprf.$(OBJ): gvcprf.c $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvcreg.$(OBJ): gvcreg.c $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvctext.$(OBJ): gvctext.c $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

cdll.$(OBJ): cdll.c $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

cimg.$(OBJ): cimg.c $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

cview.$(OBJ): cview.c $(HDRS)
	$(COMP) $(FLAGS) -DOS2 -c $*.c

codepage.exe: codepage.c
	$(COMP) codepage.c

gvpmde.res: gvpmde.hlp gvcrc.h de\gvclang.h gvpm2.rc de\gvclang.rc de\gvplang.rc gvpm3.rc binary\gvpm1.ico codepage.exe $(HDRS)
	codepage 1252_850 de\gvclang.h gvclang.h
	codepage 1252_850 de\gvclang.rc gvclang.rc
	codepage 1252_850 de\gvplang.rc gvplang.rc
	copy gvpm2.rc+gvplang.rc+gvphlpde.rc+gvclang.rc+gvpm3.rc gvpmde.rc
	rc -i $(COMPBASE)\include -r $*.rc
	-del gvclang.rc
	-del gvplang.rc
	-del gvclang.h
	codepage 1252_850 $(LANGUAGE)\gvclang.h gvclang.h

gvpmde.dll: gvpmde.res de\gvplang.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, gvpmde.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, de\gvplang.def
	rc gvpmde.res gvpmde.dll
!endif

gvpmes.res: gvpmes.hlp gvcrc.h es\gvclang.h gvpm2.rc es\gvclang.rc es\gvplang.rc gvpm3.rc binary\gvpm1.ico codepage.exe $(HDRS)
	codepage 1252_850 es\gvclang.h gvclang.h
	codepage 1252_850 es\gvclang.rc gvclang.rc
	codepage 1252_850 es\gvplang.rc gvplang.rc
	copy gvpm2.rc+gvplang.rc+gvphlpes.rc+gvclang.rc+gvpm3.rc gvpmes.rc
	rc -i $(COMPBASE)\include -r $*.rc
	-del gvclang.rc
	-del gvplang.rc
	-del gvclang.h
	codepage 1252_850 $(LANGUAGE)\gvclang.h gvclang.h

gvpmes.dll: gvpmes.res es\gvplang.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, gvpmes.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, es\gvplang.def
	rc gvpmes.res gvpmes.dll
!endif

gvpmfr.res: gvpmfr.hlp gvcrc.h fr\gvclang.h gvpm2.rc fr\gvclang.rc fr\gvplang.rc gvpm3.rc binary\gvpm1.ico codepage.exe $(HDRS)
	codepage 1252_850 fr\gvclang.h gvclang.h
	codepage 1252_850 fr\gvclang.rc gvclang.rc
	codepage 1252_850 fr\gvplang.rc gvplang.rc
	copy gvpm2.rc+gvplang.rc+gvphlpfr.rc+gvclang.rc+gvpm3.rc gvpmfr.rc
	rc -i $(COMPBASE)\include -r $*.rc
	-del gvclang.rc
	-del gvplang.rc
	-del gvclang.h
	codepage 1252_850 $(LANGUAGE)\gvclang.h gvclang.h

gvpmfr.dll: gvpmfr.res fr\gvplang.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, gvpmfr.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, fr\gvplang.def
	rc gvpmfr.res gvpmfr.dll
!endif

gvpmgr.res: gvpmgr.hlp gvcrc.h gr\gvclang.h gvpm2.rc gr\gvclang.rc gr\gvplang.rc gvpm3.rc binary\gvpm1.ico codepage.exe $(HDRS)
	codepage 1253_869 gr\gvclang.h gvclang.h
	codepage 1253_869 gr\gvclang.rc gvclang.rc
	codepage 1253_869 gr\gvplang.rc gvplang.rc
	copy gvpm2.rc+gvplang.rc+gvphlpgr.rc+gvclang.rc+gvpm3.rc gvpmgr.rc
	rc -i $(COMPBASE)\include -r $*.rc
	-del gvclang.rc
	-del gvplang.rc
	-del gvclang.h
	codepage 1252_850 $(LANGUAGE)\gvclang.h gvclang.h

gvpmgr.dll: gvpmgr.res gr\gvplang.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, gvpmgr.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, gr\gvplang.def
	rc gvpmgr.res gvpmgr.dll
!endif

gvpmit.res: gvpmit.hlp gvcrc.h gvpm2.rc it\gvclang.h it\gvclang.rc it\gvplang.rc gvpm3.rc binary\gvpm1.ico codepage.exe $(HDRS)
	codepage 1252_850 it\gvclang.h gvclang.h
	codepage 1252_850 it\gvclang.rc gvclang.rc
	codepage 1252_850 it\gvplang.rc gvplang.rc
	copy gvpm2.rc+gvplang.rc+gvphlpit.rc+gvclang.rc+gvpm3.rc gvpmit.rc
	rc -i $(COMPBASE)\include -r $*.rc
	-del gvclang.rc
	-del gvplang.rc
	-del gvclang.h
	codepage 1252_850 $(LANGUAGE)\gvclang.h gvclang.h

gvpmit.dll: gvpmit.res it\gvplang.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, gvpmit.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, it\gvplang.def
	rc gvpmit.res gvpmit.dll
!endif

gvpmnl.res: gvpmnl.hlp gvcrc.h gvpm2.rc nl\gvclang.h nl\gvclang.rc nl\gvplang.rc gvpm3.rc binary\gvpm1.ico codepage.exe $(HDRS)
	codepage 1252_850 nl\gvclang.h gvclang.h
	codepage 1252_850 nl\gvclang.rc gvclang.rc
	codepage 1252_850 nl\gvplang.rc gvplang.rc
	copy gvpm2.rc+gvplang.rc+gvphlpnl.rc+gvclang.rc+gvpm3.rc gvpmnl.rc
	rc -i $(COMPBASE)\include -r $*.rc
	-del gvclang.rc
	-del gvplang.rc
	-del gvclang.h
	codepage 1252_850 $(LANGUAGE)\gvclang.h gvclang.h

gvpmnl.dll: gvpmnl.res nl\gvplang.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, gvpmnl.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, nl\gvplang.def
	rc gvpmnl.res gvpmnl.dll
!endif

gvpm.res: gvpm1.rc gvpm.h binary\gvpm1.ico gvpmen.hlp gvcrc.h en\gvclang.h gvpm1.rc en\gvclang.rc en\gvplang.rc gvpm3.rc codepage.exe $(HDRS)
	codepage 1252_850 en\gvclang.h gvclang.h
	codepage 1252_850 en\gvclang.rc gvclang.rc
	codepage 1252_850 en\gvplang.rc gvplang.rc
	copy gvpm1.rc+gvplang.rc+gvphlpen.rc+gvclang.rc+gvpm3.rc gvpm.rc
	rc -i $(COMPBASE)\include -r $*.rc
	-del gvclang.rc
	-del gvplang.rc
	-del gvclang.h
	codepage 1252_850 $(LANGUAGE)\gvclang.h gvclang.h

gvpm.exe: $(OBJS) gvpm.res gvpm.def
!if $(USE_EMX)
!if $(USE_OMF)
	$(COMP) $(FLAGS) -o gvpm $(OBJS) gvpm.def
	rc gvpm.res gvpm.exe
!else
	$(COMP) $(FLAGS) -o gvpm $(OBJS)
	emxbind -p -rgvpm.res -dgvpm.def $(COMPDIR)\emxl.exe gvpm gvpm.exe
	del $*
!endif
!endif
!if $(USE_IBM)
#	$(COMP) $(FLAGS) /Fe gvpm.exe $(OBJS) gvpm.def
	LINK386 /NOE /nologo $(LDEBUG) /noi /align:16 /exepack /base:65536 $(OBJS), gvpm.exe, , ,gvpm.def
	RC gvpm.res gvpm.exe
!endif
!if $(USE_BCC)
	$(COMP) $(FLAGS) -egvpm.exe $(OBJS) gvpm.def
	RC gvpm.res gvpm.exe
!endif

os2setup.res: os2setup.rc setup.h $(LANGUAGE)\gvclang.h codepage.exe
	codepage 1252_850 $(LANGUAGE)\gvclang.h gvclang.h
	rc -i $(COMPBASE)\include -r $*.rc

os2beta.obj: gvcbeta.c gvcbeta.h gvcrc.h
!if $(USE_EMX)
	$(COMP) -Zomf -Zsys -DOS2 -c -o os2beta.obj gvcbeta.c
!else
	$(COMP) -c -DOS2 /Foos2beta.obj gvcbeta.c
!endif

os2prf.obj: gvcprf.c
!if $(USE_EMX)
	$(COMP) -Zomf -Zsys -DNODEBUG_MALLOC -DOS2 -c -o os2prf.obj gvcprf.c
!else
	$(COMP) -DNODEBUG_MALLOC -c /Foos2prf.obj gvcprf.c
!endif

os2unzip.obj: unzip2.h os2unzip.c
!if $(USE_EMX)
	$(COMP) -Zomf -Zsys -DOS2 -c os2unzip.c
!else
	$(COMP) -DOS2 -c os2unzip.c
!endif

setupc.obj: setup.h setupc.c
!if $(USE_EMX)
	$(COMP) -Zomf -Zsys -DOS2 -c setupc.c
!else
	$(COMP) -DOS2 -c setupc.c
!endif

os2setup.obj: os2setup.c setup.h os2setup.def gvcrc.h gvcbeta.h codepage.exe
	codepage 1252_850 $(LANGUAGE)\gvclang.h gvclang.h
!if $(USE_EMX)
	$(COMP) -Zomf -Zsys -c $(DEBUGFLAG) $*.c
!else
	$(COMP) -c $*.c
!endif

os2setup.exe: os2setup.obj os2setup.res os2setup.def gvcrc.h gvcbeta.h os2unzip.obj os2beta.obj os2prf.obj setupc.obj
!if $(USE_EMX)
	$(COMP) -Zomf -Zsys $(DEBUGFLAG) os2setup.obj os2unzip.obj setupc.obj os2prf.obj os2beta.obj os2setup.def
!else
	$(COMP) os2setup.obj os2unzip.obj setupc.obj os2prf.obj os2beta.obj os2setup.def
!endif
	rc os2setup.res os2setup.exe
	
setup2de.res: os2setup.rc setup.h gvcrc.h gvcver.h de\gvclang.h codepage.exe
	codepage 1252_850 de\gvclang.h gvclang.h
	rc -i $(COMPBASE)\include -r os2setup.rc setup2de.res

setup2de.dll: setup2de.res de\setup2.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, setup2de.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, de\setup2.def
	rc setup2de.res setup2de.dll
!endif

setup2es.res: os2setup.rc setup.h gvcrc.h gvcver.h es\gvclang.h codepage.exe
	codepage 1252_850 es\gvclang.h gvclang.h
	rc -i $(COMPBASE)\include -r os2setup.rc setup2es.res

setup2es.dll: setup2es.res es\setup2.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, setup2es.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, es\setup2.def
	rc setup2es.res setup2es.dll
!endif

setup2fr.res: os2setup.rc setup.h gvcrc.h gvcver.h fr\gvclang.h codepage.exe
	codepage 1252_850 fr\gvclang.h gvclang.h
	rc -i $(COMPBASE)\include -r os2setup.rc setup2fr.res

setup2fr.dll: setup2fr.res fr\setup2.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, setup2fr.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, fr\setup2.def
	rc setup2fr.res setup2fr.dll
!endif


setup2gr.res: os2setup.rc setup.h gvcrc.h gvcver.h gr\gvclang.h codepage.exe
	codepage 1253_869 gr\gvclang.h gvclang.h
	rc -i $(COMPBASE)\include -r os2setup.rc setup2gr.res

setup2gr.dll: setup2gr.res gr\setup2.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, setup2gr.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, gr\setup2.def
	rc setup2gr.res setup2gr.dll
!endif


setup2it.res: os2setup.rc setup.h gvcrc.h gvcver.h it\gvclang.h codepage.exe
	codepage 1252_850 it\gvclang.h gvclang.h
	rc -i $(COMPBASE)\include -r os2setup.rc setup2it.res

setup2it.dll: setup2it.res it\setup2.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, setup2it.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, it\setup2.def
	rc setup2it.res setup2it.dll
!endif


setup2nl.res: os2setup.rc setup.h gvcrc.h gvcver.h nl\gvclang.h codepage.exe
	codepage 1252_850 nl\gvclang.h gvclang.h
	rc -i $(COMPBASE)\include -r os2setup.rc setup2nl.res

setup2nl.dll: setup2nl.res nl\setup2.def gvplang.c
!if $(USE_EMX)
	$(COMP) -Zdll -Zso -Zsys -Zomf -c gvplang.c
	LINK386 $(LDEBUG) $(COMPBASE)\lib\dll0.obj gvplang.obj, setup2nl.dll, ,$(COMPBASE)\lib\gcc.lib $(COMPBASE)\lib\st\c.lib $(COMPBASE)\lib\st\c_dllso.lib $(COMPBASE)\lib\st\sys.lib $(COMPBASE)\lib\c_alias.lib $(COMPBASE)\lib\end.lib $(COMPBASE)\lib\os2.lib, nl\setup2.def
	rc setup2nl.res setup2nl.dll
!endif


gvdoc.exe: gvdoc.c
!if $(USE_EMX)
	$(COMP) -o $* $*.c
	emxbind $(COMPDIR)\emxl.exe $* $*.exe
	del $*
!else
	$(COMP) $*.c
!endif
	
doc2ipf.exe: doc2ipf.c
!if $(USE_EMX)
	$(COMP) -o $* $*.c
	emxbind $(COMPDIR)\emxl.exe $* $*.exe
	del $*
!else
	$(COMP) $*.c
!endif

doc2html.exe: doc2html.c
!if $(USE_EMX)
	$(COMP) -o $* $*.c
	emxbind $(COMPDIR)\emxl.exe $* $*.exe
	del $*
!else
	$(COMP) $*.c
!endif

gvc.txt: codepage.exe $(LANGUAGE)\gvclang.txt
	codepage 1252_850 $(LANGUAGE)\gvclang.txt gvc.txt

gvpmen.hlp: en\gvclang.txt  codepage.exe gvdoc.exe doc2ipf.exe
	codepage 1252_850 en\gvclang.txt gvc.txt
	gvdoc P gvc.txt gvpm.txt
	doc2ipf gvpm.txt gvpmen.ipf gvphlpen.rc
	ipfc gvpmen.ipf
	rename gvpmen.hlp gvpmen.hlp
	-del gvc.txt
	-del gvpm.txt

gvpmde.hlp: de\gvclang.txt  codepage.exe gvdoc.exe doc2ipf.exe
	codepage 1252_850 de\gvclang.txt gvc.txt
	gvdoc P gvc.txt gvpm.txt
	doc2ipf gvpm.txt gvpmde.ipf gvphlpde.rc
	ipfc gvpmde.ipf
	rename gvpmde.hlp gvpmde.hlp
	-del gvc.txt
	-del gvpm.txt

gvpmes.hlp: es\gvclang.txt  codepage.exe gvdoc.exe doc2ipf.exe
	codepage 1252_850 es\gvclang.txt gvc.txt
	gvdoc P gvc.txt gvpm.txt
	doc2ipf gvpm.txt gvpmes.ipf gvphlpes.rc
	ipfc gvpmes.ipf
	rename gvpmes.hlp gvpmes.hlp
	-del gvc.txt
	-del gvpm.txt

gvpmfr.hlp: fr\gvclang.txt  codepage.exe gvdoc.exe doc2ipf.exe
	codepage 1252_850 fr\gvclang.txt gvc.txt
	gvdoc P gvc.txt gvpm.txt
	doc2ipf gvpm.txt gvpmfr.ipf gvphlpfr.rc
	ipfc gvpmfr.ipf
	rename gvpmfr.hlp gvpmfr.hlp
	-del gvc.txt
	-del gvpm.txt

gvpmgr.hlp: gr\gvclang.txt  codepage.exe gvdoc.exe doc2ipf.exe
	codepage 1253_869 gr\gvclang.txt gvc.txt
	gvdoc P gvc.txt gvpm.txt
	doc2ipf gvpm.txt gvpmgr.ipf gvphlpgr.rc
	ipfc gvpmgr.ipf
	rename gvpmgr.hlp gvpmgr.hlp
	-del gvc.txt
	-del gvpm.txt

gvpmit.hlp: it\gvclang.txt  codepage.exe gvdoc.exe doc2ipf.exe
	codepage 1252_850 it\gvclang.txt gvc.txt
	gvdoc P gvc.txt gvpm.txt
	doc2ipf gvpm.txt gvpmit.ipf gvphlpit.rc
	ipfc gvpmit.ipf
	rename gvpmit.hlp gvpmit.hlp
	-del gvc.txt
	-del gvpm.txt

gvpmnl.hlp: nl\gvclang.txt  codepage.exe gvdoc.exe doc2ipf.exe
	codepage 1252_850 nl\gvclang.txt gvc.txt
	gvdoc P gvc.txt gvpm.txt
	doc2ipf gvpm.txt gvpmnl.ipf gvphlpnl.rc
	ipfc gvpmnl.ipf
	rename gvpmnl.hlp gvpmnl.hlp
	-del gvc.txt
	-del gvpm.txt

gvpmen.inf: gvpmen.hlp
	ipfc /INF gvpmen.ipf
	rename gvpmen.INF gvpmen.inf

html: gvpm.htm gsview.htm

gvpm.htm: doc2html.exe en\gvclang.txt
	gvdoc P en\gvclang.txt gvpm.txt
	doc2html gvpm.txt GSview.htm
	-del gvpm.htm
	rename GSview.htm gvpm.htm

gsview.txt: gvc.txt gvdoc.exe
	gvdoc W en\gvclang.txt gsview.txt

gsview.htm: doc2html.exe gsview.txt
	doc2html gsview.txt GSview.htm

gvpm.ps: gvpm.dvi
	dvips gvpm

gvpm.dvi: gvpm.tex titlepag.tex
	-latex gvpm
	-latex gvpm

gvpm.tex: gvpm.txt doc2tex.exe
	doc2tex gvpm.txt gvpm.tex

doc2tex.exe: doc2tex.c
!if $(USE_EMX)
	$(COMP) -o $* $*.c
	emxbind $(COMPDIR)\emxl.exe $* $*.exe
	del $*
!else
	$(COMP) $*.c
!endif


gvpgs.res: gvpgs.rc gvpgs.h gvcrc.h $(LANGUAGE)\gvclang.h codepage.exe
	codepage 1252_850 $(LANGUAGE)\gvclang.h gvclang.h
	rc -i $(COMPBASE)\include -r $*.rc

gvpgs.$(OBJ): gvpgs.c gvpgs.h gvcrc.h gsvver.h
	$(COMP) $(FLAGS) -DOS2 -c $*.c

gvpgs.exe: gvpgs.$(OBJ) gvpgs.res gvpgs.def
!if $(USE_EMX)
!if $(USE_OMF)
	$(COMP) $(FLAGS) -o gvpgs gvpgs.$(OBJ) gvpgs.def
	rc gvpgs.res gvpgs.exe
!else
	$(COMP) $(FLAGS) -o gvpgs gvpgs.$(OBJ)
	emxbind -p -rgvpgs.res -dgvpgs.def $(COMPDIR)\emxl.exe gvpgs gvpgs.exe
	del $*
!endif
!endif
!if $(USE_IBM)
	LINK386 /NOE /nologo $(LDEBUG) /noi /align:16 /exepack /base:65536 gvpgs.obj, gvpgs.exe, , ,gvpgs.def
	RC gvpgs.res gvpgs.exe
!endif


gsv$(GSVIEW_VERSION)os2.zip:
	copy Readme.htm ..
	copy gsview.css ..
	copy cdorder.txt ..
	copy regorder.txt ..
	copy LICENCE ..
	copy FILE_ID.DIZ ..
	copy gvpm.exe ..
!if $(USE_EMX) && !$(USE_OMF)
	emxbind -s ../gvpm.exe
!endif
	copy binary\gvpm1.ico ..\gvpm.ico
	copy gvpmen.hlp ..
	copy gvpmde.hlp ..
	copy gvpmes.hlp ..
	copy gvpmfr.hlp ..
	copy gvpmgr.hlp ..
	copy gvpmit.hlp ..
	copy gvpmnl.hlp ..
	copy gvpmde.dll ..
	copy gvpmes.dll ..
	copy gvpmfr.dll ..
	copy gvpmgr.dll ..
	copy gvpmit.dll ..
	copy gvpmnl.dll ..
	copy gvpgs.exe ..
	copy os2setup.exe ..
	copy setup2de.dll ..
	copy setup2es.dll ..
	copy setup2fr.dll ..
	copy setup2gr.dll ..
	copy setup2it.dll ..
	copy setup2nl.dll ..
	copy printer.ini ..\printer.ini
	cd ..
	-del os2.zip
	zip -9 -@ os2.zip < src\gvclist2.txt
	echo Redistribution of this OS/2 GSview MUST be accompanied by the> README2.TXT
	echo sources in gsv$(GSVIEW_VERSION)src.zip to meet the licence requirements. >> README2.TXT
	-del gsv$(GSVIEW_VERSION)os2.zip
	zip -9 gsv$(GSVIEW_VERSION)os2.zip os2.zip os2setup.exe unzip2.dll setup2de.dll setup2es.dll setup2fr.dll setup2gr.dll setup2it.dll setup2nl.dll
	zip -9 gsv$(GSVIEW_VERSION)os2.zip README2.TXT Readme.htm gsview.css cdorder.txt regorder.txt FILE_ID.DIZ LICENCE
	-del README2.TXT
	-del Readme.htm
	-del gsview.css
	-del cdorder.txt
	-del LICENCE
	-del FILE_ID.DIZ
	-del gvpm.exe
	-del gvpm.ico
	-del gvpmen.hlp
	-del gvpmde.hlp
	-del gvpmes.hlp
	-del gvpmfr.hlp
	-del gvpmgr.hlp
	-del gvpmit.hlp
	-del gvpmnl.hlp
	-del gvpmde.dll
	-del gvpmes.dll
	-del gvpmfr.dll
	-del gvpmgr.dll
	-del gvpmit.dll
	-del gvpmnl.dll
	-del gvpgs.exe
	-del os2setup.exe
	-del setup2de.dll
	-del setup2es.dll
	-del setup2fr.dll
	-del setup2gr.dll
	-del setup2it.dll
	-del setup2nl.dll
	-del printer.ini
	cd src

prezip: gsv$(GSVIEW_VERSION)os2.zip

zip: prezip
	echo Doesn't work from here
	echo Use the Windows makefile

language:
	-del gvclang.h
	-del gvclang.rc
	-del gvplang.rc
	-del gvc.txt
	-del *.res

clean: language
	-del gsvver.h
	-del gvpm.res
	-del gvpm.rc
	-del gvpm.$(OBJ)
	-del gvpdisp.$(OBJ)
	-del gvpdlg.$(OBJ)
	-del gvpdll.$(OBJ)
	-del gvpedit.$(OBJ)
	-del gvpeps.$(OBJ)
	-del gvpgsver.$(OBJ)
	-del gvpinit.$(OBJ)
	-del gvpmeas.$(OBJ)
	-del gvpmisc.$(OBJ)
	-del gvpprn.$(OBJ)
	-del gvpreg.$(OBJ)
	-del gvcbeta.$(OBJ)
	-del gvccmd.$(OBJ)
	-del dscparse.$(OBJ)
	-del dscutil.$(OBJ)
	-del gvcdll.$(OBJ)
	-del gvcdisp.$(OBJ)
	-del gvcedit.$(OBJ)
	-del gvceps.$(OBJ)
	-del gvcfile.$(OBJ)
	-del gvcinit.$(OBJ)
	-del gvcmeas.$(OBJ)
	-del gvcmeas2.obj
	-del gvcmisc.$(OBJ)
	-del gvcpdf.$(OBJ)
	-del gvcprf.$(OBJ)
	-del gvcprn.$(OBJ)
	-del gvcreg.$(OBJ)
	-del gvctext.$(OBJ)
	-del cdll.$(OBJ)
	-del cimg.$(OBJ)
	-del cview.$(OBJ)
	-del doc2ipf.$(OBJ)
	-del doc2ipf.exe
	-del doc2html.$(OBJ)
	-del doc2html.exe
	-del doc2tex.$(OBJ)
	-del doc2tex.exe
	-del gvdoc.$(OBJ)
	-del gvdoc.exe
	-del gvpm.ipf
	-del gvpm.txt
	-del gvpm.aux
	-del gvpm.dvi
	-del gvpm.log
	-del gvpm.toc
	-del gsview.txt
	-del gvpgs.res
	-del gvpgs.$(OBJ)
	-del os2setup.obj
	-del os2setup.res
	-del os2unzip.obj
	-del os2beta.obj
	-del os2prf.obj
	-del setupc.obj
	-del gvplang.obj
	-del setup2de.map
	-del setup2es.map
	-del setup2fr.map
	-del setup2gr.map
	-del setup2it.map
	-del setup2nl.map
	-del gvpmen.map
	-del gvpmde.map
	-del gvpmes.map
	-del gvpmfr.map
	-del gvpmgr.map
	-del gvpmit.map
	-del gvpmnl.map
	-del gvpmen.ipf
	-del gvpmde.ipf
	-del gvpmes.ipf
	-del gvpmfr.ipf
	-del gvpmgr.ipf
	-del gvpmit.ipf
	-del gvpmnl.ipf
	-del gvpmen.rc
	-del gvpmde.rc
	-del gvpmes.rc
	-del gvpmfr.rc
	-del gvpmgr.rc
	-del gvpmit.rc
	-del gvpmnl.rc
	-del gvphlpen.rc
	-del gvphlpde.rc
	-del gvphlpes.rc
	-del gvphlpfr.rc
	-del gvphlpgr.rc
	-del gvphlpit.rc
	-del gvphlpnl.rc
	-del codepage.exe

veryclean: clean
	-del gvpm.exe
	-del gvpmde.dll
	-del gvpmes.dll
	-del gvpmfr.dll
	-del gvpmgr.dll
	-del gvpmit.dll
	-del gvpmnl.dll
	-del gvpmen.hlp
	-del gvpmde.hlp
	-del gvpmes.hlp
	-del gvpmfr.hlp
	-del gvpmgr.hlp
	-del gvpmit.hlp
	-del gvpmnl.hlp
	-del gvpm.inf
	-del gvpm.tex
	-del gvpm.htm
	-del gsview.htm
	-del gvpgs.exe
	-del os2setup.exe
	-del setup2de.dll
	-del setup2es.dll
	-del setup2fr.dll
	-del setup2gr.dll
	-del setup2it.dll
	-del setup2nl.dll
	-del echogsv.exe
