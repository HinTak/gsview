/* Copyright (C) 1995, Digital Equipment Corporation.         */
/* All rights reserved.                                       */
/* See the file pstotext.txt for a full description.          */
/* Last modified on Thu Aug 21 16:32:27 PDT 1997 by mcjones   */
/*      modified on Thu Nov 16 13:33:13 PST 1995 by deutsch   */

#include <sys/param.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bundle.h"
#include "ocr.h"
#include "rot270.h"
#include "rot90.h"
#include "ptotdll.h"

#define BOOLEAN int
#define FALSE 0
#define TRUE 1

#define LINELEN 2000 /* longest allowable line from gs ocr.ps output */

extern BUNDLE ocr, rot270, rot90;

static BOOLEAN cork = FALSE;
static BOOLEAN debug = FALSE;
static char *gs_cmd = "gs";

static char *cmd; /* = argv[0] */

static enum {
  portrait,
  landscape,
  landscapeOther} orientation = portrait;

static BOOLEAN bboxes = FALSE;

static int explicitFiles = 0; /* count of explicit file arguments */

usage() {
  fprintf(stderr, "pstotext 1.6 of 21 August 1997\n");
  fprintf(stderr, "Copyright (C) 1995-1997, Digital Equipment Corporation.\n");
  fprintf(stderr, "Comments to {mcjones,birrell}@pa.dec.com.\n\n");
  fprintf(stderr, "Usage: %s [option|file]...\n", cmd);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -cork            assume Cork encoding for dvips output\n");
  fprintf(stderr, "  -landscape       rotate 270 degrees\n");
  fprintf(stderr, "  -landscapeOther  rotate 90 degrees\n");
  fprintf(stderr, "  -portrait        don't rotate (default)\n");
  fprintf(stderr, "  -bboxes          output one word per line with bounding box\n");
  fprintf(stderr, "  -debug           show Ghostscript output and error messages\n");
  fprintf(stderr, "  -gs \"command\"    Ghostscript command\n");
  fprintf(stderr, "  -                read from stdin (default if no files specified)\n");
}

#define OCRPATH "/tmp/,pstotext-ocr.ps"
#define ROT270PATH "/tmp/,pstotext-rot270.ps"
#define ROT90PATH "/tmp/,pstotext-rot90.ps"

static char *make_temp(b) BUNDLE b; {
  /* Return pathname of temporary file containing bundle "b".  Caller
     should unlink file (and, technically, free pathname). */
  FILE *f;
  char *path = tempnam("/tmp", ",ps2t");
  f = fopen(path, "w");
  if (f==NULL) {perror(cmd); exit(1);}
  putbundle(b, f);
  fclose(f);
  return path;
}

static char *ocr_path = NULL, *rotate_path = NULL;
static FILE *gs = NULL;
static void *instance; /* pstotext state */

static int cleanup() {
  int gsstatus, status = 0;
  pstotextExit(instance);
  if (gs!=NULL) {
    gsstatus = pclose(gs);
    if (WIFEXITED(gsstatus)) {
      if (WEXITSTATUS(gsstatus)!=0) status = 3;
      else if (WIFSIGNALED(gsstatus)) status = 4;
    }
  }
  if (rotate_path!=NULL & strcmp(rotate_path, "")!=0) unlink(rotate_path);
  if (ocr_path!=NULL) unlink(ocr_path);
  return status;
}

static void handler() {
  int status = cleanup();
  if (status!=0)
    exit(status);
  exit(2);
}

static do_it(path) char *path; {
  /* If "path" is NULL, then "stdin" should be processed. */
  char gs_cmdline[2*MAXPATHLEN];
  char input[MAXPATHLEN];
  int status;

  signal(SIGINT, handler);
  signal(SIGHUP, handler);

  ocr_path = make_temp(ocr);

  switch (orientation) {
  case portrait: rotate_path = ""; break;
  case landscape: rotate_path = make_temp(rot270); break;
  case landscapeOther: rotate_path = make_temp(rot90); break;
  }

  if (path==NULL) strcpy(input, "-");
  else {strcpy(input, "-- "); strcat(input, path);}

  sprintf(
    gs_cmdline,
    "%s -r72 -dNODISPLAY -dDELAYBIND -dWRITESYSTEMDICT %s -dNOPAUSE %s %s %s",
    gs_cmd,
    (debug ? "" : "-q"),
    rotate_path,
    ocr_path,
    input
    );
  if (debug) fprintf(stderr, "%s\n", gs_cmdline);
  gs = popen(gs_cmdline, "r");
  if (gs==0) {perror(cmd); exit(1);}
  status = pstotextInit(&instance);
  if (status!=0) {
    fprintf(stderr, "%s: internal error %d\n", cmd, status);
    exit(5);
  }
  if (cork) {
    status = pstotextSetCork(instance, TRUE);
    if (status!=0) {
      fprintf(stderr, "%s: internal error %d\n", cmd, status);
      exit(5);
    }
  }
  while (TRUE) {
    char line[LINELEN];
    char *pre, *word, *post;
    int llx, lly, urx, ury;
    if (fgets(line, LINELEN, gs)==NULL) break;
    if (debug) fputs(line, stderr);
    status = pstotextFilter(
      instance, line, &pre, &word, &post, &llx, &lly, &urx, &ury);
    if (status!=0) {
      fprintf(stderr, "%s: internal error %d\n", cmd, status);
      exit(5);
    }
    if (word!=NULL)
      if (!bboxes) {
        fputs(pre, stdout); fputs(word, stdout); fputs(post, stdout);
        if ( debug ) fputc('\n', stderr);
      }
      else
        fprintf(stdout, "%6d\t%6d\t%6d\t%6d\t%s\n", llx, lly, urx, ury, word);
  }
  status = cleanup();
  if (status!=0) exit(status);
}

main(argc, argv) int argc; char *argv[]; {
  int i;
  char *arg;
  cmd = argv[0];
  for (i = 1; i<argc; i++) {
    arg = argv[i];
    if (strcasecmp(arg, "-landscape")==0) orientation = landscape;
    else if (strcasecmp(arg, "-cork")==0) cork = TRUE;
    else if (strcasecmp(arg, "-landscapeOther")==0) orientation = landscapeOther;
    else if (strcasecmp(arg, "-portrait")==0) orientation = portrait;
    else if (strcasecmp(arg, "-bboxes")==0) bboxes = TRUE;
    else if (strcasecmp(arg, "-debug")==0) debug = TRUE;
    else if (strcasecmp(arg, "-gs")==0) {
      i++;
      if (i>=argc) {usage(); exit(1);}
      gs_cmd = argv[i];
    }
    else if (strcmp(arg, "-")==0) do_it(NULL);
    else if (arg[0] == '-') {usage(); exit(1);}
    else /* file */ {
      explicitFiles++;
      do_it(arg);
    }
  }
  if (explicitFiles==0) do_it(NULL);
  exit(0);
}

