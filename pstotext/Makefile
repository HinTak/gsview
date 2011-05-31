# Copyright (C) 1995, Digital Equipment Corporation.
# All rights reserved.
# See the file pstotext.txt for a full description.
# Last modified on Thu Aug  1 15:26:37 PDT 1996 by mcjones

# For debugging, use cc -g.
CC=cc -g

BUNDLE = ocr.h rot270.h rot90.h

all:	pstotext

main.o: ptotdll.h bundle.h ocr.h rot270.h rot90.h
	$(CC) -c $*.c

ptotdll.o: ptotdll.h
	$(CC) -c $*.c

pstotext: bundle.o main.o ptotdll.o
	$(CC) -o pstotext main.o bundle.o ptotdll.o -lm

.SUFFIXES: .ps

.c.o:
	$(CC) -c $*.c

# "Bundle" an Ascii file.
.ps.h:
	echo "char *$*[] = {" > $*.h
	sed -e 's/"/\\"/g' -e 's/\(.*\)/  "\1\\n",/' $*.ps >> $*.h
	echo "  0" >> $*.h
	echo "};" >> $*.h

tidy:
	rm -f ,* .,* .emacs_[0-9]* core *~ 


clean:
	rm -f pstotext *.o core $(BUNDLE)
