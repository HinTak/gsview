# Makefile for Ghostview for Windows - GSVIEW.EXE
# using Borland C++ 3.1
# 'make -fgsview.mak'
#
COMPBASE = c:\borlandc
#COMPBASE = g:\utility\bc31
# Language is a two letter code for selecting alternate
# .rc, .doc & .hpj files.
# Only the default International English is available.
LANG=
#
COMPDIR = $(COMPBASE)\bin
INCDIR = $(COMPBASE)\include
LIBDIR = $(COMPBASE)\lib
MODEL=m
CFLAGS=-v -m$(MODEL) -W -2 -h -w -H=gsview.sym -I$(INCDIR)
OBJS=gsview.obj dialog.obj display.obj pipe.obj print.obj clip.obj init.obj ps.obj

all: gsview$(LANG).exe gsview$(LANG).hlp

.c.obj:
	$(COMPDIR)\bcc -c $(CFLAGS) {$< }
	
gsview$(LANG).exe: $(OBJS) gsview$(LANG).res gsview.def
	$(COMPDIR)\tlink /Twe /c /m /s /v /l @&&!
$(LIBDIR)\c0w$(MODEL) +
$(OBJS) +
,gsview$(LANG).exe,gsview, +
$(LIBDIR)\import +
$(LIBDIR)\mathw$(MODEL) +
$(LIBDIR)\cw$(MODEL), +
gsview.def
!
	$(COMPDIR)\rlink -30 -t gsview$(LANG).res gsview$(LANG).exe

gsview$(LANG).res: gsview$(LANG).rc gsview2.rc gsview.h $(ICONS)
	$(COMPDIR)\brcc -i$(INCDIR) -r gsview$(LANG)

gsview.obj: gsview.c gsview.h ps.h

clip.obj: clip.c gsview.h ps.h

init.obj: init.c gsview.h ps.h

dialog.obj: dialog.c gsview.h ps.h

display.obj: display.c gsview.h ps.h

pipe.obj: pipe.c gsview.h ps.h

print.obj: print.c gsview.h ps.h

ps.obj: ps.c ps.h
	$(COMPDIR)\bcc -c $(CFLAGS) -w-pro -w-pin ps.c

doc2rtf.exe: doc2rtf.c
	$(COMPDIR)\bcc -w-pro -I$(INCDIR) -L$(LIBDIR) doc2rtf.c

gsview$(LANG).dvi: gsview$(LANG).tex titlepag.tex
	latex gsview$(LANG)
	latex gsview$(LANG)

gsview$(LANG).tex: gsview$(LANG).doc doc2tex.exe
	doc2tex gsview$(LANG).doc gsview$(LANG).tex

doc2tex.exe: doc2tex.c
	$(COMPDIR)\bcc -w-pro -I$(INCDIR) -L$(LIBDIR) doc2tex.c

gsview$(LANG).hlp: doc2rtf.exe gsview$(LANG).doc gsview$(LANG).hpj
	doc2rtf gsview$(LANG).doc gsview.rtf
	$(COMPDIR)\hc31 gsview$(LANG).hpj

strip: gsview$(LANG).exe
	$(COMPDIR)\tdstrip gsview$(LANG).exe

zip:
	zip2 -@ gsview.zip < manifest

clean:
	del gsview.obj
	del clip.obj
	del dialog.obj
	del display.obj
	del init.obj
	del pipe.obj
	del print.obj
	del ps.obj
	del gsview.map
	del gsview.sym
	del gsview$(LANG).res
	del gsview.rtf
	del doc2rtf.obj
	del doc2rtf.exe
	del doc2tex.obj
	del doc2tex.exe
	del gsview.aux
	del gsview.dvi
	del gsview.log
	del gsview.toc
	del gsview.tex

veryclean: clean
	del gsview$(LANG).exe
	del gsview$(LANG).hlp
