# pstotxt3.mak
# Makefile for pstotxt3.dll, for use with Win32 GSview

# makefile created by
# Russell Lang, 1996-07-29

# For debugging, use bcc -v

COMPBASE = c:\bc45
COMPDIR = $(COMPBASE)\bin
INCDIR = $(COMPBASE)\include
LIBDIR = $(COMPBASE)\lib
#DEBUGLINK = -v

CCAUX=$(COMPDIR)\bcc
CC=$(COMPDIR)\bcc32 $(DEBUGLINK) -WD

all:	pstotxt3.dll

.c.obj:
	$(CC) -c $*.c

ocr.h: ocr.ps mkbundle.exe
	mkbundle $*.ps $*.h 1

rot270.h: rot270.ps mkbundle.exe
	mkbundle $*.ps $*.h 2

rot90.h: rot90.ps mkbundle.exe
	mkbundle $*.ps $*.h 3

mkbundle.exe: mkbundle.c
	$(CCAUX) $*.c

pstotxt3.obj: ptotdll.c ptotdll.h
	$(CC) -c -w-pro -opstotxt3.obj ptotdll.c

pstotxt3.rc:  ocr.h rot270.h rot90.h
	copy ocr.h+rot270.h+rot90.h pstotxt3.rc

pstotxt3.res: pstotxt3.rc
	$(COMPDIR)\brcc32 -i$(INCDIR) -r pstotxt3.rc


pstotxt3.dll: pstotxt3.obj pstotxt3.res
	$(CC) -epstotxt3.dll pstotxt3.obj
	$(COMPDIR)\brc32 pstotxt3.res pstotxt3.dll

prezip: pstotxt3.dll
	copy pstotxt3.dll ..\pstotxt3.dll
	copy pstotext.txt ..\pstotext.txt

clean:
	-del pstotxt3.dll
	-del pstotxt3.res
	-del pstotxt3.rc
	-del *.obj
	-del ocr.h
	-del rot270.h
	-del rot90.h
	-del mkbundle.exe
	-del pstotext.tmp

