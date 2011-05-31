/* Copyright (C) 1995-1998, Digital Equipment Corporation.    */
/* All rights reserved.                                       */
/* See the file pstotext.txt for a full description.          */
/* Last modified on Fri Oct 16 16:27:54 PDT 1998 by mcjones   */
/*      modified on Thu Nov 16 13:33:13 PST 1995 by deutsch   */
/*
 * Modified on 27-MAY-1998 13:08 by Hunter Goatley
 *      Ported to VMS.  Various minor changes to allow it to work on
 *      both VAX and Alpha (VAX C and DEC C).  VMS pipes don't work
 *      right, so the GS output is dumped to a temporary file that's
 *      read, instead of reading from pipes (which is, of course, how
 *      VMS implements pipes anyway).  Also added -output option.
 */

#ifdef VMS
#include "vms.h"
#else
#include <sys/param.h>
#include <sys/wait.h>
#endif

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
static char *outfile = "";

static char *cmd; /* = argv[0] */

static enum {
  portrait,
  landscape,
  landscapeOther} orientation = portrait;

static BOOLEAN bboxes = FALSE;

static int explicitFiles = 0; /* count of explicit file arguments */

usage() {
  fprintf(stderr, "pstotext 1.8g of 15 July 2000\n");
  fprintf(stderr, "Copyright (C) 1995-1998, Digital Equipment Corporation.\n");
  fprintf(stderr, "Modified by Ghostgum Software Pty Ltd for Ghostscript 6.0.\n");
  fprintf(stderr, "Comments to {mcjones,birrell}@pa.dec.com\n\n");
#ifdef VMS
  fprintf(stderr, "VMS Comments to goathunter@madgoat.com\n\n");
#endif
  fprintf(stderr, "Usage: %s [option|file]...\n", cmd);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -cork            assume Cork encoding for dvips output\n");
  fprintf(stderr, "  -landscape       rotate 270 degrees\n");
#ifdef VMS
  fprintf(stderr, "  -landscapeother  rotate 90 degrees\n");
#else
  fprintf(stderr, "  -landscapeOther  rotate 90 degrees\n");
#endif
  fprintf(stderr, "  -portrait        don't rotate (default)\n");
  fprintf(stderr, "  -bboxes          output one word per line with bounding box\n");
  fprintf(stderr, "  -debug           show Ghostscript output and error messages\n");
  fprintf(stderr, "  -gs \"command\"    Ghostscript command\n");
  fprintf(stderr, "  -                read from stdin (default if no files specified)\n");
  fprintf(stderr, "  -output file     output results to \"file\" (default is stdout)\n");
}

#ifdef VMS
#define OCRPATH "pstotext_dir:pstotext-ocr.ps"
#define ROT270PATH "pstotext_dir:pstotext-rot270.ps"
#define ROT90PATH "pstotext_dir:pstotext-rot90.ps"
#else
#define OCRPATH "/tmp/,pstotext-ocr.ps"
#define ROT270PATH "/tmp/,pstotext-rot270.ps"
#define ROT90PATH "/tmp/,pstotext-rot90.ps"
#endif

static char *make_temp(b) BUNDLE b; {
  /* Return pathname of temporary file containing bundle "b".  Caller
     should unlink file (and, technically, free pathname). */
  FILE *f;
#ifdef VMS
  char *path = tempnam("SYS$SCRATCH:", ".ps2t");
#else
  char *path = tempnam("/tmp", ",ps2t");
#endif
  f = fopen(path, "w");
  if (f==NULL) {perror(cmd); exit(1);}
  putbundle(b, f);
  fclose(f);
  return path;
}

static char *ocr_path = NULL, *rotate_path = NULL;
static FILE *gs = NULL;
static void *instance; /* pstotext state */
#ifdef VMS
static char *cmdfile = NULL, *gsoutfile = NULL;
#endif

static int cleanup() {
  int gsstatus, status = 0;
  pstotextExit(instance);
  if (gs!=NULL) {
#ifdef VMS
    gsstatus = fclose(gs);
#else
    gsstatus = pclose(gs);
#endif
    if (WIFEXITED(gsstatus)) {
      if (WEXITSTATUS(gsstatus)!=0) status = 3;
      else if (WIFSIGNALED(gsstatus)) status = 4;
    }
  }
  if (rotate_path!=NULL & strcmp(rotate_path, "")!=0) unlink(rotate_path);
  if (ocr_path!=NULL) unlink(ocr_path);
#ifdef VMS
  if (cmdfile!=NULL) unlink(cmdfile);
  if (gsoutfile!=NULL) unlink(gsoutfile);
#endif
  return status;
}

static void handler(int x) {
  int status = cleanup();
  if (status!=0) exit(status);
#ifdef VMS
  exit(1);
#else
  exit(2);
#endif
}

static do_it(path) char *path; {
  /* If "path" is NULL, then "stdin" should be processed. */
  char gs_cmdline[2*MAXPATHLEN];
  char input[MAXPATHLEN];
  int status;
  FILE *fileout;
#ifdef VMS
  FILE *cfile;
#endif

  fileout = stdout;
  if (strlen(outfile) != 0) {
#ifdef VMS
     fileout = fopen(outfile, "w", "rfm=var","rat=cr");
#else
     fileout = fopen(outfile, "w");
#endif /* VMS */
     if (fileout == NULL) {perror(cmd); exit(1);}
  }

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
#ifdef VMS
    "%s -r72 \"-dNODISPLAY\" \"-dFIXEDMEDIA\" \"-dDELAYBIND\" \"-dWRITESYSTEMDICT\" %s \"-dNOPAUSE\" %s %s %s",
#else
    "%s -r72 -dNODISPLAY -dFIXEDMEDIA -dDELAYBIND -dWRITESYSTEMDICT %s -dNOPAUSE %s %s %s",
#endif
    gs_cmd,
    (debug ? "" : "-q"),
    rotate_path,
    ocr_path,
    input
    );
  if (debug) fprintf(stderr, "%s\n", gs_cmdline);
#ifdef VMS
  cmdfile = tempnam("SYS$SCRATCH:","PS2TGS");
  gsoutfile = tempnam("SYS$SCRATCH:","GSRES");
  if ((cfile = fopen(cmdfile,"w")) == NULL) {perror(cmd);exit(1);}
  fprintf (cfile, "$ define/user sys$output %s\n", gsoutfile);
  fprintf (cfile, "$ %s\n", gs_cmdline);
  fprintf (cfile, "$ deletee/nolog %s;*\n", cmdfile);
  fputs ("$ exit\n", cfile);
  fclose (cfile);
  sprintf(gs_cmdline, "@%s.", cmdfile);
  system(gs_cmdline);
  if ((gs = fopen(gsoutfile, "r")) == NULL) {
	fprintf(stderr, "Error opening output file %s from GS command\n", gsoutfile);
	perror(cmd);
	exit(1);
  }
#else
  gs = popen(gs_cmdline, "r");
  if (gs==0) {perror(cmd); exit(1);}
#endif
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
        fputs(pre, fileout); fputs(word, fileout); fputs(post, fileout);
        if ( debug ) fputc('\n', stderr);
      }
      else {
	if (pre) {
	    if (*pre == ' ')
		pre++;
	    fputs(pre, fileout);
	}
        fprintf(fileout, "%6d\t%6d\t%6d\t%6d\t%s\n", llx, lly, urx, ury, word);
	if (post)
	    fputs(post, fileout);
      }
  }
  if (fileout != stdout) fclose(fileout);
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
    else if (strcasecmp(arg, "-output")==0) {
      i++;
      if (i>=argc) {usage(); exit(1);}
      outfile = argv[i];
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
