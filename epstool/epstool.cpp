/* Copyright (C) 1993-2001, Ghostgum Software Pty Ltd.  All rights reserved.
  
  This file is part of GSview.
  
  This program is distributed with NO WARRANTY OF ANY KIND.  No author
  or distributor accepts any responsibility for the consequences of using it,
  or for whether it serves any particular purpose or works at all, unless he
  or she says so in writing.  Refer to the GSview Free Public Licence 
  (the "Licence") for full details.
  
  Every copy of GSview must include a copy of the Licence, normally in a 
  plain ASCII text file named LICENCE.  The Licence grants you the right 
  to copy, modify and redistribute GSview, but only under certain conditions 
  described in the Licence.  Among other things, the Licence requires that 
  the copyright notice and this notice be preserved on all copies.
*/

/* epstool.c */
#include "epstool.h"

char szVersion[] = "1.8  2001-06-02";

char iname[MAXSTR];
char oname[MAXSTR];
char upname[MAXSTR];
char gsname[MAXSTR] = GSCOMMAND;
char bmpname[MAXSTR];
char devname[MAXSTR];
char szScratch[] = "ep";
char szAppName[] = "epstool";
int resolution = 72;
int page = 1;	/* default is page 1 */
BOOL calc_bbox = FALSE;
BOOL got_op = FALSE;
BOOL debug = FALSE;
BOOL quiet = FALSE;
BOOL ptsize = FALSE;
int ptwidth = 612;	/* letter width */
int ptheight = 842;	/* A4 height */
BOOL got_dsc_error = FALSE;
BOOL ignore_dsc_warning = FALSE;
BOOL ignored_dsc_warning = FALSE;
int op = 0;
#define EXTRACTPS	1
#define EXTRACTPRE	2
#define INTERCHANGE	3
#define TIFF4 		4
#define TIFF6U		5
#define TIFF6P		6
#define TIFFGS		7
#define USER		8
#define WMF		9
#define COPY		10

/* KLUDGE variables */
CDSC *dsc;
PSFILE psfile;
OPTION option;
char GVFAR *bitmap_base;
LPBITMAP2 bitmap_pbmi;

/* function prototypes */
BOOL load_bitmap(void);
int scan_args(int argc, char *argv[]);
void do_help(void);
int extract_section(void);
int add_preview(void);
void psfile_extract_header(FILE *f);
void psfile_extract_page(FILE *f, int page);
int make_eps_copy(void);

// in gvwgsver.cpp
BOOL find_gs(char *gspath, int len, int minver, BOOL bDLL);

// in dscutil.cpp
void dsc_display(P2(CDSC *dsc, void (*dfn)(P2(void *ptr, const char *str))));

/* KLUDGE functions */
LPBITMAP2 get_bitmap(void)
{
    return bitmap_pbmi;
}
void release_bitmap(void)
{
}
void play_sound(int i)
{
}

char *
psfile_name(PSFILE *psf)
{
    /* don't support gzipped files in epstool */
    /* so return original file name */
    return psf->name;
}

void debug_print(void *caller_data, const char *str)
{
    fputs(str, stdout);
}


int 
show_dsc_error(P5(void *caller_data, CDSC *dsc, unsigned int explanation, 
	const char *line, unsigned int line_len))
{
    int response = CDSC_RESPONSE_CANCEL;
    int severity;
    char buf[MAXSTR];
    if (explanation > dsc->max_error)
	return CDSC_RESPONSE_OK;

    severity = dsc->severity[explanation];

    /* If debug function provided, copy messages there */
    if (dsc->debug_print_fn) {
	switch (severity) {
	    case CDSC_ERROR_INFORM:
		dsc_debug_print(dsc, "\nDSC Information");
		break;
	    case CDSC_ERROR_WARN:
		dsc_debug_print(dsc, "\nDSC Warning");
		break;
	    case CDSC_ERROR_ERROR:
		dsc_debug_print(dsc, "\nDSC Error");
	}
	dsc_debug_print(dsc, "\n");
	if (explanation <= dsc->max_error) {
	    if (line && line_len) {
		int length = min(line_len, sizeof(buf)-1);
		sprintf(buf, "At line %d:\n", dsc->line_count);
		dsc_debug_print(dsc, buf);
		strncpy(buf, line, length);
		buf[length]='\0';
		dsc_debug_print(dsc, "  ");
		dsc_debug_print(dsc, buf);
	    }
	    dsc_debug_print(dsc, dsc_message[explanation]);
	}
    }

    fputs(dsc_message[explanation], stdout);
    fprintf(stdout, "Assuming Cancel\n");

    got_dsc_error = TRUE;

    if ((severity == CDSC_ERROR_WARN) && ignore_dsc_warning) {
	ignored_dsc_warning = TRUE;
	dsc_debug_print(dsc, "\n****************************************************************");
	dsc_debug_print(dsc, "\nYou have recklessly told epstool to ignore DSC Warnings.");
	dsc_debug_print(dsc, "\nA warning has been given and you have ignored it.");
	dsc_debug_print(dsc, "\nIf you distribute this so called EPS file, you name will be mud.");
	dsc_debug_print(dsc, "\n****************************************************************\n\n");
	got_dsc_error = FALSE;
    }

    return response;
}



int
main(int argc, char *argv[])
{
#ifdef __WIN32__
	find_gs(gsname, sizeof(gsname)-1, 550, FALSE);
#endif
#ifdef UNIX
	strcpy(devname, "pbmraw");
#else
	strcpy(devname, "bmpmono");
#endif
	if (scan_args(argc, argv))
	   return 1;
#if defined(__EMX__) || defined(MSDOS) || defined(__WIN32__)
	setmode(fileno(stdout), O_BINARY);
#endif

	strcpy(psfile.name, iname);

	psfile.dsc = NULL;
	dsc = NULL;
	if ( (psfile.file = fopen(psfile.name, READBIN)) == (FILE *)NULL ) {
	    fprintf(stderr, "Can't open %s\n", psfile.name);
	    return 1;
	}
	else {
	    int code = 0;
	    int count;
	    char *d;
	    if ( (d = (char *) malloc(COPY_BUF_SIZE)) == NULL) {
		fclose(psfile.file);
		return 1;
	    }

	    dsc = dsc_init(NULL);
	    if (dsc == (CDSC*)NULL) {
		fprintf(stderr, "Failed to initalise DSC parser\n"); 
		return 1;
	    }
	    dsc_set_debug_function(dsc, debug_print);
	    dsc_set_error_function(dsc, show_dsc_error);
	    while ((count = fread(d, 1, COPY_BUF_SIZE, psfile.file))!=0) {
		code = dsc_scan_data(dsc, d, count);
		if ((code == CDSC_ERROR)  || (code == CDSC_NOTDSC)) {
		    /* not DSC or an error */
		    fclose(psfile.file);
		    dsc_free(dsc);
		    dsc = NULL;
		    fprintf(stderr, "File %s does not contain DSC comments\n", 
			psfile.name);
		    return 1 ;
		}
	    }
	    fclose(psfile.file);
	    if ((code == CDSC_ERROR) || (code == CDSC_NOTDSC)) {
		dsc_free(dsc);
		dsc = NULL;
		fprintf(stderr, "File %s does not contain DSC comments\n", 
		    psfile.name);
		return 1;
	    }
	    dsc_fixup(dsc);
	    if (debug)
		dsc_display(dsc, debug_print);
	}

        if (got_dsc_error) {
	    dsc_free(dsc);
	    dsc = NULL;
	    fprintf(stderr, "\nAborting: EPS file has fatal errors");
	    return 1;
        }

	psfile.dsc = dsc;

	if ((psfile.file = fopen(psfile.name, READBIN)) == (FILE *)NULL) {
	    fprintf(stderr, "Can't open %s\n", psfile.name);
	    return 1;
	}

	if (op==INTERCHANGE || op==TIFF4 || op==TIFF6U || op==TIFF6P || op==TIFFGS 
		|| op==WMF || op==COPY)
	    return add_preview();
	if (op==USER)
	    return make_eps_user();
	if (op==EXTRACTPS || op==EXTRACTPRE)
	    return extract_section();

	delete dsc;
	return 0;
}

int
extract_section(void)
{
	if (op == EXTRACTPS)
	    extract_doseps(IDM_EXTRACTPS);
	else if (op == EXTRACTPRE)
	    extract_doseps(IDM_EXTRACTPRE);
	else
	  fprintf(stderr, "Unknown operation %d\n", op);
	return 0;
}

int
add_preview(void)
{
char ename[MAXSTR];
char tempname[MAXSTR];
FILE *tempfile;
FILE *bmpfile;
char gscommand[MAXSTR+MAXSTR];
char rspname[MAXSTR];
FILE *rspfile;
int width, height;
int code = 0;
char line[DSC_LINE_LENGTH+1];
	if (!calc_bbox) {
	    if (dsc->bbox == NULL) {
	        fprintf(stderr, "Bounding Box is missing\n");
	        return 1;
	    }
            if ((dsc->bbox->urx < dsc->bbox->llx) ||
	        (dsc->bbox->ury < dsc->bbox->lly)) {
		fprintf(stderr, "Bounding Box is inconsistent\n");
		return 1;
	    }
	    if ((dsc->bbox->urx == dsc->bbox->llx) ||
	        (dsc->bbox->ury == dsc->bbox->lly)) {
	        fprintf(stderr, "Bounding Box is empty");
		return 1;
	    }
	}


	if ((op == TIFFGS) && calc_bbox) {
	    calc_bbox = FALSE;
	    fprintf(stderr, "Can't calculate Bounding Box when using GS TIFF driver\n");
	    fprintf(stderr, "Using %%BoundingBox from EPS file\n");
	}

        if ( (dsc->page_count > 1) || (dsc->page_pages > 1)) {
	    /* create temporary file to hold extracted page */
	    tempfile = gp_open_scratch_file(szScratch, ename, WRITEBIN);
	    if (tempfile == (FILE *)NULL) {
	        fprintf(stderr, "Couldn't open temporary file %s\n", ename);
	        return 1;
	    }
	    /* copy page to new file */
	    psfile_extract_header(tempfile);
	    psfile_extract_page(tempfile, page);
	    fclose(tempfile);
	    delete dsc;
	    dsc = NULL;
	    psfile.dsc = NULL;
	    /* scan new file */
	    strcpy(psfile.name, ename);

	    psfile.dsc = NULL;
	    dsc = NULL;
	    if ( (psfile.file = fopen(psfile.name, READBIN)) == (FILE *)NULL ) {
		fprintf(stderr, "Can't open %s\n", psfile.name);
		return 1;
	    }
	    else {
		int code = 0;
		int count;
		char *d;
		if ( (d = (char *) malloc(COPY_BUF_SIZE)) == NULL) {
		    fclose(psfile.file);
		    return 1;
		}

		dsc = dsc_init(NULL);
		if (dsc == (CDSC*)NULL) {
		    fprintf(stderr, "Failed to initalise DSC parser\n"); 
		    return 1;
		}
		dsc_set_debug_function(dsc, debug_print);
		dsc_set_error_function(dsc, show_dsc_error);
		while ((count = fread(d, 1, COPY_BUF_SIZE, psfile.file))!=0) {
		    code = dsc_scan_data(dsc, d, count);
		    if ((code == CDSC_ERROR) || (code == CDSC_NOTDSC)) {
			/* not DSC or an error */
			fclose(psfile.file);
			dsc_free(dsc);
			dsc = NULL;
			fprintf(stderr, "File %s does not contain DSC comments\n", 
			    psfile.name);
			return 1 ;
		    }
		}
		fclose(psfile.file);
		if ((code == CDSC_ERROR) || (code == CDSC_NOTDSC)) {
		    dsc_free(dsc);
		    dsc = NULL;
		    fprintf(stderr, "File %s does not contain DSC comments\n", 
			psfile.name);
		    return 1;
		}
		dsc_fixup(dsc);
		if (debug)
		    dsc_display(dsc, debug_print);
	    }

	    if (got_dsc_error) {
		delete dsc;
		dsc = NULL;
		fprintf(stderr, "\nAborting: EPS file has fatal errors");
		return 1;
	    }

	    if (dsc == (CDSC *)NULL) {
		fprintf(stderr, "File %s does not contain DSC comments\n", 
		    psfile.name);
		return 1;
	    }

	    psfile.dsc = dsc;


	    if ((psfile.file = fopen(psfile.name, READBIN)) == (FILE *)NULL) {
	       fprintf(stderr, "Can't open %s\n", psfile.name);
	       return 1;
	    }

	    page = 1;	/* we want the one and only remaining page */
	}
	tempfile = gp_open_scratch_file(szScratch, tempname, WRITEBIN);
	if (tempfile == (FILE *)NULL) {
	    fprintf(stderr, "Couldn't open temporary file %s\n", tempname);
	    return 1;
	}
	rspfile = gp_open_scratch_file(szScratch, rspname, WRITEBIN);
	if (rspfile == (FILE *)NULL) {
	    fprintf(stderr, "Couldn't open temporary response file %s\n", rspname);
	    if (!debug)
		unlink(tempname);
	    return 1;
	}
	bmpfile = gp_open_scratch_file(szScratch, bmpname, WRITEBIN);
	if (bmpfile == (FILE *)NULL) {
	    fprintf(stderr, "Couldn't open temporary bitmap file %s\n", bmpname);
	    if (!debug) {
		unlink(tempname);
		unlink(rspname);
	    }
	    return 1;
	}
	fclose(bmpfile);
	if (!debug)
	   unlink(bmpname);
	/* offset to bottom left corner of bounding box */
	if (!calc_bbox)
	   fprintf(tempfile, "%d %d translate\r\n", -dsc->bbox->llx, -dsc->bbox->lly);
	/* calculate page size */
	if (calc_bbox) {
	   if (dsc->page_media && !ptsize) {
	       width = (int)(dsc->page_media->width*(long)resolution/72L);
	       height = (int)(dsc->page_media->height*(long)resolution/72L);
	   }
	   else {
	       width = (int)(((long)ptwidth)*resolution/72L);
	       height = (int)(((long)ptheight)*resolution/72L);
	   }
	}
	else {
	   width = (int)((dsc->bbox->urx - dsc->bbox->llx)*(long)resolution/72L);
	   height = (int)((dsc->bbox->ury - dsc->bbox->lly)*(long)resolution/72L);
	}
	/* cope with EPS files with and without showpage */
	fprintf(tempfile, " /EPSTOOL_save save def\r\n /showpage {} def\r\n");
	fprintf(tempfile, " count /EPSTOOL_count exch def\r\n");
	fprintf(tempfile, " /EPSTOOL_countdictstack countdictstack def\r\n");

	/* copy page to temporary file */
	psfile_extract_header(tempfile);
	if (dsc->page_count != 0) {
	    psfile_extract_page(tempfile, page);
	    if ((dsc->page_count > 1) || (dsc->page_pages > 1))
		fprintf(stderr,"Can't handle multiple page PostScript files\n");
	}
	else {
	    psfile_extract_page(tempfile, -1);
	}
	/* cope with EPS files with and without showpage */
	fprintf(tempfile, "\n count EPSTOOL_count sub {pop} repeat\r\n");
	fprintf(tempfile, " countdictstack EPSTOOL_countdictstack sub {end} repeat\r\n");
	fprintf(tempfile, " EPSTOOL_save restore\r\n showpage\r\n");
	fprintf(tempfile, "\nquit\n");
	fclose(tempfile);
#ifdef UNIX
	sprintf(gscommand, "\042%s\042 -dNOPAUSE -dQUIET -sDEVICE=%s -sOutputFile=\042%s\042 -r%d -g%dx%d %s",
	   gsname, devname, bmpname, resolution, width, height, tempname);
#else
	sprintf(gscommand, "-dNOPAUSE\n-dQUIET\n-sDEVICE=%s\n-sOutputFile=\042%s\042\n-r%d\n-g%dx%d\n\042%s\042",
	   devname, bmpname, resolution, width, height, tempname);
	if (!quiet) {
	    fputs(gscommand, stderr);
	    fputs("\n", stderr);
	}
	fputs(gscommand, rspfile);
	fclose(rspfile);
#if defined(__WIN32__) || defined(__EMX__)
	sprintf(gscommand, "\042%s\042 @%s", gsname, rspname);
#else
	sprintf(gscommand, "%s @%s", gsname, rspname);
#endif
#endif
	if (!quiet)
	    fprintf(stderr,"%s\n", gscommand);
	system(gscommand);

	if (!debug) {
	    unlink(rspname);
	    unlink(tempname);
	}

	if (op == TIFFGS) {
	    strcpy(upname, bmpname);
	    code = make_eps_user();	/* create user TIFF preview */
	    if (!debug)
		unlink(bmpname);
	}
	else {
	    if (!load_bitmap()) {
		if (!debug)
		    unlink(bmpname);
		fprintf(stderr, "no bitmap\n");
		return 1;
	    }
	    if (!debug)
		unlink(bmpname);
	    /* now create new file with preview */
	    if (op == INTERCHANGE)
		code = make_eps_interchange(calc_bbox);
	    else if (op == TIFF4)
		code = make_eps_tiff(IDM_MAKEEPST4, calc_bbox);
	    else if (op == TIFF6U)
		code = make_eps_tiff(IDM_MAKEEPST6U, calc_bbox);
	    else if (op == TIFF6P)
		code = make_eps_tiff(IDM_MAKEEPST6P, calc_bbox);
	    else if (op == WMF)
		code = make_eps_metafile(calc_bbox);
	    else if (op == COPY)
		code = make_eps_copy();
	    else
		fprintf(stderr, "Unknown operation %d\n", op);
	}

	if (*ename) {
	    fclose(psfile.file);
	    if (!debug)
	        unlink(ename);	/* remove temporary file */
	}

	if (!quiet)
	    fprintf(stderr, "Operation %s\n", 
		code ? "failed" : "was successful");

	return code;
}

int 
scan_args(int argc, char *argv[])
{
char *argp;
int count;
	if (argc == 1) {
	    do_help();
	    return 1;
	}
	for (count=1, argp = argv[1]; count < argc; count++, argp=argv[count]) {
	  if (*argp == '-') {
	    switch(argp[1]) {
		case 'h':
		  do_help();
		  return 1;
		case 'o':
		  if (argp[2])
		      strcpy(oname, argp+2);
		  else {
		      fprintf(stderr,"Missing output filename for -o\n");
		      return 1;
		  }
		  break;
		case 'n':
		  if (argp[2])
		      page = atoi(argp+2);
		  else {
		      fprintf(stderr,"Missing page number for -n\n");
		      page = atoi(argp);
		  }
		  break;
		case 'r':
		  if (argp[2])
		      resolution = atoi(argp+2);
		  else {
		      fprintf(stderr,"Missing resolution for -r\n");
		      return 1;
		  }
		  break;
		case 's':
		   if (argp[2]) {
		     if (sscanf(argp+2, "%dx%d", &ptwidth, &ptheight)
			   == 2)
			ptsize = TRUE;
		     else {
		        fprintf(stderr,"Incorrect size specified with -s\n");
		        return 1;
		     }
		   }
		   break;
		case 'b':
		  calc_bbox = !calc_bbox;
		  break;
		case 'd':
		  debug = !debug;
		  break;
		case 'q':
		  quiet = !quiet;
		  break;
		case 'e':
		  ignore_dsc_warning = !ignore_dsc_warning;
		  if (ignore_dsc_warning)
		    fprintf(stderr, "You have recklessly told epstool to ignore DSC Warnings.\nBad things might happen...\n");
		  break;
		case 'g':
		  if (argp[2])
		    strcpy(gsname, argp+2);
		  else {
		      fprintf(stderr,"Missing Ghostscript command for -g\n");
		      return 1;
		  }
		  break;
		case 't':
		  if (got_op) {
		    fprintf(stderr,"Can't select two operations");
		    return 1;
		  }
		  if (argp[2] && argp[2]=='4') {
		    op = TIFF4;
		    got_op = TRUE;
		  }
		  else if (argp[2] && argp[2]=='6' && argp[3] && toupper(argp[3])=='U') {
		    op = TIFF6U;
		    got_op = TRUE;
		  }
		  else if (argp[2] && argp[2]=='6' && argp[3] && toupper(argp[3])=='P') {
		    op = TIFF6P;
		    got_op = TRUE;
		  }
		  else if (argp[2] && toupper(argp[2])=='G') {
		    op = TIFFGS;
		    got_op = TRUE;
		  }
		  else {
		      fprintf(stderr,"Missing TIFF type or device name for -t\n");
		      return 1;
		  }
		  break;
		case 'i':
		  if (got_op) {
		    fprintf(stderr,"Can't select two operations");
		    return 1;
		  }
		  op = INTERCHANGE;
		  got_op = TRUE;
		  break;
		case 'z':
		  if (argp[2])
		      strcpy(devname, argp+2);
		  break;
		case 'c':
		  if (got_op) {
		    fprintf(stderr,"Can't select two operations");
		    return 1;
		  }
		  op = COPY;
		  got_op = TRUE;
		  break;
		case 'w':
		  if (got_op) {
		    fprintf(stderr,"Can't select two operations");
		    return 1;
		  }
		  op = WMF;
		  got_op = TRUE;
		  break;
		case 'u':
		  if (got_op) {
		    fprintf(stderr,"Can't select two operations");
		    return 1;
		  }
		  op = USER;
		  got_op = TRUE;
		  if (argp[2])
		      strcpy(upname, argp+2);
		  else {
		      fprintf(stderr,"Missing input filename for -u\n");
		      return 1;
		  }
		  break;
		case 'p':
		  if (got_op) {
		    fprintf(stderr,"Can't select two operations");
		    return 1;
		  }
		  op = EXTRACTPS;
		  got_op = TRUE;
		  break;
		case 'v':
		  if (got_op) {
		    fprintf(stderr,"Can't select two operations");
		    return 1;
		  }
		  op = EXTRACTPRE;
		  got_op = TRUE;
		  break;
		default:
		  fprintf(stderr,"Unknown option %s\n", argp);
		  return 1;
	    }
	  }
	  else {
	      /* input filename */
	      if (*iname) {
	          fprintf(stderr,"Only one input file permitted\n");
	          return 1;
	      }
	      strcpy(iname, argp);
	  }
	}
	option.xdpi = option.ydpi = resolution;
	if (*iname == '\0') {
	    fprintf(stderr, "No input file specified");
	    return 1;
	}
	if (!got_op) {
	    fprintf(stderr, "No operation specified");
	    return 1;
	}
	return 0;
}

void
do_help(void)
{
   fprintf(stderr,"Usage:  epstool [option] operation filename\n");
   fprintf(stderr,"  Copyright (C) 1995-2000, Ghostgum Software Pty Ltd.  All rights reserved.\n");
   fprintf(stderr,"  Version: %s\n", szVersion);
   fprintf(stderr,"  Options:\n");
   fprintf(stderr,"     -b             Calculate BoundingBox from image\n");
   fprintf(stderr,"     -gcommand      Ghostscript command\n");
   fprintf(stderr,"     -nnumber       Page number to extract\n");
   fprintf(stderr,"     -ofilename     Output filename\n");
   fprintf(stderr,"     -q             Quiet (no messages)\n");
   fprintf(stderr,"     -rnumber       Preview resolution in dpi\n");
   fprintf(stderr,"     -sWIDTHxHEIGHT Size of page used with -b\n");
   fprintf(stderr,"     -zdevice       Ghostscript device name\n");
   fprintf(stderr,"  Operations: (one only)\n");
   fprintf(stderr,"     -i             Add Interchange preview   (EPSI)\n");
   fprintf(stderr,"     -t4            Add TIFF4 preview         (DOS EPS)\n");
   fprintf(stderr,"     -t6u           Add TIFF6 uncompressed    (DOS EPS)\n");
   fprintf(stderr,"     -t6p           Add TIFF6 packbits        (DOS EPS)\n");
   fprintf(stderr,"     -tg            Add GS TIFF preview       (DOS EPS)\n");
   fprintf(stderr,"     -w             Add WMF preview           (DOS EPS)\n");
   fprintf(stderr,"     -ufilename     Add user supplied preview (DOS EPS)\n");
   fprintf(stderr,"     -p             Extract PostScript        (DOS EPS)\n");
   fprintf(stderr,"     -v             Extract Preview           (DOS EPS)\n");
   fprintf(stderr,"     -c             Copy without preview      (use with -b)\n");
}

char *err_msgs[] = {"", "No preview in input file", "Preview file is not TIFF or Windows Metafile", ""};

void 
gserror(UINT id, char *str, UINT icon, int sound)
{
	fprintf(stderr, "%s %s\n", err_msgs[id], str ? str : "");
}

/* Create and open a scratch file with a given name prefix. */
/* Write the actual file name at fname. */
FILE *
gp_open_scratch_file(const char *prefix, char *fname, const char *mode)
{	char *temp;
        int fd;
	if ( (temp = getenv("TEMP")) == NULL )
#if defined(UNIX) || defined(__UNIX) || defined(__unix)
		strcpy(fname, "/tmp");
#else
		gs_getcwd(fname, MAXSTR);
#endif
	else
		strcpy(fname, temp);

	/* Prevent X's in path from being converted by mktemp. */
	for ( temp = fname; *temp; temp++ ) {
		*temp = (char)tolower(*temp);
		if (*temp == '/')
		    *temp = DIRSEP;
	}
	if ( strlen(fname) && (fname[strlen(fname)-1] != DIRSEP ) ) {
		fname[strlen(fname)+1] = '\0';
		fname[strlen(fname)] = DIRSEP;
	}

	strcat(fname, prefix);
	strcat(fname, "XXXXXX");
#if defined(UNIX) || defined(OS2)
	fd = mkstemp(fname);
	return fdopen(fd, mode);
#else
	mktemp(fname);
	return fopen(fname, mode);
#endif
}

char * 
gs_getcwd(char *dirname, int size)
{
#ifdef __EMX__
	return _getcwd2(dirname, size);
#else
	return getcwd(dirname, size);
#endif
}


void pserror(char *str)
{
	fputs(str, stderr);
}



/* general purpose read file into memory */
/* should work for files > 64k under MSDOS */
/* malloc's memory to hold file contents and returns pointer to this memory */
char GVFAR *
read_file(char *fname)
{
  FILE *f;
  LONG length, nread, count;
  char GVFAR *base;
  char GVHUGE *bp;

  if ( (f = fopen(fname, READBIN)) == (FILE *)NULL ) {
    fprintf(stderr, "Can't open %s\n", fname);
    return NULL;
  }
  fseek(f, 0, SEEK_END);
  length = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (length == 0) {
    fprintf(stderr, "File %s is empty\n", fname);
  }
#ifdef MSDOS	/* I hate segmented architectures */
  if ( (base = (char GVFAR *)farmalloc(length)) == (char *)NULL )
#else
  if ( (base = (char GVFAR *)malloc(length)) == (char *)NULL )
#endif
  {
    fprintf(stderr, "Can't malloc memory to hold file %s\n", fname);
    fclose(f);
    return NULL;
  }
  bp = base;
  while (length > 0) {
#ifdef MSDOS
	/* get smaller of 16k, length, remaining bytes in segment */
	count = min( min(16384, length), (DWORD)(65536UL-((WORD)(bp))) );
#else
	count = length;
#endif
	nread = fread(bp, 1, (int)count, f);
	if (nread == 0) {
	    fprintf(stderr, "Can't read file %s\n", fname);
	    fclose(f);
	    free(base);
	    return NULL;
	}
        length -= nread;
	bp += nread;
  }
  fclose(f);
  return base;  
}

BOOL
load_bitmap(void) 
{
  LPBITMAPFILE pbmf;

  /* extract some info about bitmap */
  pbmf = (LPBITMAPFILE)read_file(bmpname);
  if (pbmf == NULL)
    return FALSE;
  switch (*(char *)(pbmf)) {
      case 'B':  /* BMP format */
          bitmap_pbmi = (LPBITMAP2)( (char *)pbmf + BITMAPFILE_LENGTH );
	  break;
      case 'P': /* PBMPLUS format */
          bitmap_pbmi = (LPBITMAP2)(pbmf);  /* a KLUDGE */
	  break;
      default:
	  fprintf(stderr,"Unknown bitmap format\n");
	  return FALSE;
  }
  return TRUE;
}


/* Copy the header to file f */
/* change first line to EPSF if needed */
void
psfile_extract_header(FILE *f)
{
    char text[DSC_LINE_LENGTH+1];
    BOOL pages_written = FALSE;

    fseek(psfile.file, dsc->begincomments, SEEK_SET);
    fgets(text, DSC_LINE_LENGTH, psfile.file);
    if (dsc->epsf)
        fputs(text,f);
    else {
	switch(text[11]) {
	    case '1':
                fputs("%!PS-Adobe-1.0 EPSF-1.0\r\n",f);
		break;
	    case '2':
                fputs("%!PS-Adobe-2.0 EPSF-2.0\r\n",f);
		break;
	    default:
                fputs("%!PS-Adobe-3.0 EPSF-3.0\r\n",f);
	}
    }
    while (ps_copy_find(f, psfile.file, dsc->endcomments, 
	text, sizeof(text), "%%Pages:")) {
	if (pages_written)
	    continue;
	fprintf(f, "%%%%Pages: 1\r\n");
	pages_written = TRUE;
    }
}

/* Copy the selected page and trailer to file f */
void
psfile_extract_page(FILE *f, int page)
{
    int i;
    long position;
    char line[DSC_LINE_LENGTH+1];

    /* don't copy preview because we might be adding our own */
    ps_copy(f, psfile.file, dsc->begindefaults, dsc->enddefaults); 
    ps_copy(f, psfile.file, dsc->beginprolog, dsc->endprolog);
    ps_copy(f, psfile.file, dsc->beginsetup, dsc->endsetup);

    /* map page number to zero based index */
    if (dsc->page_count > 0) {
	if (dsc->page_order == CDSC_DESCEND) 
	    i = dsc->page_count - page;
	else
	    i = page - 1;
	fseek(psfile.file, dsc->page[i].begin, SEEK_SET);
	ps_copy_find(f, psfile.file, dsc->page[i].end, 
	    line, sizeof(line), "%%Page:");
	fprintf(f, "%%%%Page: %s 1\r\n",
		dsc->page[i].label);
	position = ftell(psfile.file);
	ps_copy(f, psfile.file, position, dsc->page[i].end);
    }

    fseek(psfile.file, dsc->begintrailer, SEEK_SET);
    while (ps_copy_find(f, psfile.file, dsc->endtrailer, 
	/* copy trailer, removing %%%Pages: since it is now in comments */
	line, sizeof(line), "%%Pages:")) {
    }
}

/* Copy the header to file f */
/* change bbox line if present, or add bbox line */
void
copy_eps_bbox_header(FILE *f)
{
    char text[DSC_LINE_LENGTH+1];
    BOOL bbox_written = FALSE;
    long position;
    CDSC *dsc = psfile.dsc;

    fseek(psfile.file, dsc->begincomments, SEEK_SET);
    /* make sure first line is EPS */
    fgets(text, DSC_LINE_LENGTH, psfile.file);
    if (dsc->epsf)
        fputs(text,f);
    else {
	switch(text[11]) {
	    case '1':
                fputs("%!PS-Adobe-1.0 EPSF-1.0\r\n",f);
		break;
	    case '2':
                fputs("%!PS-Adobe-2.0 EPSF-2.0\r\n",f);
		break;
	    default:
                fputs("%!PS-Adobe-3.0 EPSF-3.0\r\n",f);
	}
    }
    position = ftell(psfile.file);
    if (dsc->bbox) {
      /* BoundingBox was in original file, replace it */
      while ( ps_copy_find(f, psfile.file, dsc->endcomments, 
	text, sizeof(text), "%%BoundingBox:") )  {
	if (bbox_written)
	    continue;
	fprintf(f, "%%%%BoundingBox: %d %d %d %d\r\n",
	    bbox.llx, bbox.lly, bbox.urx, bbox.ury);
	bbox_written = TRUE;
      }
    }
    else {
      /* BoundingBox was not in original file, add it */
      fgets(text, DSC_LINE_LENGTH, psfile.file);
      fputs(text,f);
      fprintf(f, "%%%%BoundingBox: %d %d %d %d\r\n",
	    bbox.llx, bbox.lly, bbox.urx, bbox.ury);
      ps_copy(f, psfile.file, position, dsc->endcomments); 
    }
    if (ignored_dsc_warning)
	fputs("\
% The user who created this EPS file recklessly ignored a warning about\n\
% incorrect DSC comments.  Blame them when things go wrong...\n", f);
}

/* copy psfile, updating %%BoundingBox */
int
make_eps_copy(void)
{
char epsname[MAXSTR];
FILE *epsfile;
PREBMAP prebmap;
PSBBOX devbbox;	/* in pixel units */
unsigned char *pbitmap;
int code;
    if ( (pbitmap = (unsigned char *)get_bitmap()) == (unsigned char *)NULL) {
	return 1;
    }
    if (*pbitmap == 'P')
	code = scan_pbmplus(&prebmap, pbitmap);
    else
	code = scan_dib(&prebmap, pbitmap);
    if (code) {
	release_bitmap();
	return code;
    }

    strcpy(epsname, oname);
    if (*epsname!='\0')
	epsfile = fopen(epsname,"wb");
    else
	epsfile = stdout;
    if (epsfile == (FILE *)NULL) {
	release_bitmap();
	return 1;
    }
    if (calc_bbox) {
	scan_bbox(&prebmap, &devbbox);
	if (devbbox.valid) {
	    /* copy to global bbox as if obtained by PS to EPS */
	    bbox.llx = (int)(devbbox.llx * 72.0 / option.xdpi - 0.5);
	    bbox.lly = (int)(devbbox.lly * 72.0 / option.ydpi - 0.5);
	    bbox.urx = (int)(devbbox.urx * 72.0 / option.xdpi + 1.5);
	    bbox.ury = (int)(devbbox.ury * 72.0 / option.ydpi + 1.5);
	    bbox.valid = TRUE;
	}



	copy_eps_bbox_header(epsfile); /* adjust %%BoundingBox: comment */

	ps_copy(epsfile, psfile.file, dsc->endcomments, dsc->endtrailer); 
    }
    else {
	ps_copy(epsfile, psfile.file, dsc->begincomments, 
		psfile.dsc->endcomments);
    }
    if (*epsname!='\0')
       fclose(epsfile);
    release_bitmap();
    return 0;
}
