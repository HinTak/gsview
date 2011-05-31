# pstotxt2.mak
# Makefile for pstotxt2.dll, for use with OS/2 GSview

# makefile created by
# Russell Lang, 1996-07-29

DRIVE=c:
COMP=gcc
COMPBASE=$(DRIVE)\emx
EMXPATH=$(DRIVE)/emx
FLAGS=-Zdll -Zso -Zsys -Zomf -D__DLL__

COMPDIR=$(COMPBASE)\bin
INCDIR=$(EMXPATH)/include
LIBDIR=$(EMXPATH)/lib

all:	pstotxt2.dll

.c.obj:
	$(COMP) $(FLAGS) -c $*.c

ocr.h: ocr.ps mkbundle.exe
	mkbundle $*.ps $*.h 1

rot270.h: rot270.ps mkbundle.exe
	mkbundle $*.ps $*.h 2

rot90.h: rot90.ps mkbundle.exe
	mkbundle $*.ps $*.h 3

mkbundle.exe: mkbundle.c
	$(COMP) -o $*.exe $*.c

pstotxt2.obj: ptotdll.c ptotdll.h
	$(COMP) $(FLAGS) -c -o pstotxt2.obj ptotdll.c

pstotxt2.rc:  ocr.h rot270.h rot90.h
	copy ocr.h+rot270.h+rot90.h pstotxt2.rc

pstotxt2.res: pstotxt2.rc
	rc -i $(COMPBASE)\include -r $*.rc

pstotxt2.dll: pstotxt2.obj pstotxt2.def pstotxt2.res
	$(COMP) $(FLAGS) -o $*.dll $*.obj $*.def
	rc $*.res $*.dll

prezip: pstotxt2.dll pstotext.txt
	copy pstotxt2.dll ..\pstotxt2.dll
	copy pstotext.txt ..\pstotext.txt

clean:
	-del pstotxt2.dll
	-del pstotxt2.res
	-del pstotxt2.rc
	-del *.obj
	-del ocr.h
	-del rot270.h
	-del rot90.h
	-del mkbundle.exe
