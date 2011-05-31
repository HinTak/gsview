# pstotxta.mak
# Makefile for pstotxta.dll, for use with Win32 DEC Alpha GSview

# UNTESTED

# makefile created by
# Russell Lang, 1996-10-01

# For debugging, use link /DEBUG

DEVBASE = e:\devstudio

COMPBASE = e:\devstudio\vc
COMPDIR = $(COMPBASE)\bin
INCDIR = $(COMPBASE)\include
LIBDIR = $(COMPBASE)\lib
RCOMP32=$(DEVBASE)\sharedide\bin\rc -D_MSC_VER
RCOMP=$(RCOMP32)

CC=$(COMPDIR)\cl -DNEED_PROTO $(CFLAGS) /I$(INCDIR)
CCAUX=$(CC)

all:	pstotxta.dll pstotxta.exe

.c.obj:
	$(CC) -c $*.c

ocr.h: ocr.ps mkrch.exe
	mkrch $*.ps $*.h 1

rot270.h: rot270.ps mkrch.exe
	mkrch $*.ps $*.h 2

rot90.h: rot90.ps mkrch.exe
	mkrch $*.ps $*.h 3

mkrch.exe: mkrch.c
	$(CCAUX) $*.c

pstotxta.obj: ptotdll.c ptotdll.h
	$(COMPDIR)\$(CC) /c $(CFLAGS) /I$(INCDIR) /fo:pstotxta.obj ptotdll.c

pstotxta.rc:  ocr.h rot270.h rot90.h
	copy ocr.h+rot270.h+rot90.h pstotxta.rc

pstotxta.res: pstotxt3.rc
	$(RCOMP32) -i$(INCDIR) -r pstotxta.rc

pstotxta.dll: pstotxta.obj pstotxta.res
	$(COMPDIR)\link $(DEBUGLINK) /DLL /DEF:pstotxt3.def /OUT:pstotxta.dll pstotxta.obj pstotxta.res

pstotxta.exe: pstotxtd.c
	$(CC) /c /fo:pstotxta.exe pstotxtd.c

prezip: all
	copy pstotxta.dll ..\pstotxta.dll
	copy pstotxta.exe ..\pstotxta.exe
	copy pstotext.txt ..\pstotext.txt

clean:
	-del pstotxtd.exe
	-del pstotxta.exe
	-del pstotxta.dll
	-del pstotxta.res
	-del pstotxta.rc
	-del *.obj
	-del ocr.h
	-del rot270.h
	-del rot90.h
	-del mkrch.exe

