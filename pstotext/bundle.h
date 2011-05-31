/* Copyright (C) 1995, Digital Equipment Corporation.         */
/* All rights reserved.                                       */
/* See the file pstotext.txt for a full description.          */
/* Last modified on Thu Aug  1 11:32:23 PDT 1996 by mcjones   */

typedef char *BUNDLE[];

extern void putbundle(/* BUNDLE *b, FILE *f */);
/* Write bundle "b" to file "f".

   "b" should have been constructed from "b.ps" by the ".ps.h" rule in
   the pstotext Makefile. */
