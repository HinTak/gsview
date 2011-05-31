/* Copyright (C) 1993, 1994, 1995, Russell Lang.  All rights reserved.
  
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

char szVersion[] = "0.71 alpha 1995-11-23";

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
int op = 0;
#define EXTRACTPS	1
#define EXTRACTPRE	2
#define INTERCHANGE	3
#define TIFF4 		4
#define TIFF5		5
#define TIFFGS		6
#define USER		7

/* KLUDGE variables */
PSDOC *doc;
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


int
main(int argc, char *argv[])
{
	if (scan_args(argc, argv))
	   return 1;
#if defined(__EMX__) || defined(MSDOS)
	setmode(fileno(stdout), O_BINARY);
#endif

	strcpy(psfile.name, iname);
	if ((psfile.file = fopen(psfile.name, READBIN)) == (FILE *)NULL) {
	   fprintf(stderr, "Can't open %s\n", psfile.name);
	   return 1;
	}
	doc = psscan(psfile.file);
	if (doc == (PSDOC *)NULL) {
	   fprintf(stderr, "File %s does not contain DSC comments\n", psfile.name);
	   fclose(psfile.file);
	   return 1;
	}

	if (op==INTERCHANGE || op==TIFF4 || op==TIFF5 || op==TIFFGS)
	   return add_preview();
	if (op==USER)
	   return make_eps_user();
	if (op==EXTRACTPS || op==EXTRACTPRE)
	   return extract_section();

	psfree(doc);
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
	if ((op == TIFFGS) && calc_bbox) {
	    calc_bbox = FALSE;
	    fprintf(stderr, "Can't calculate Bounding Box when using GS TIFF driver\n");
	    fprintf(stderr, "Using %%BoundingBox from EPS file\n");
	}
	if ( !calc_bbox &&
             ((doc->boundingbox[URX] == doc->boundingbox[LLX]) ||
	      (doc->boundingbox[URY] == doc->boundingbox[LLY])) ) {
	   fprintf(stderr, "Bounding Box is empty");
	   return 1;
	   /* if calc_bbox, this shouldn't be an error */
	}
	if (!quiet && doc->numpages==0) {
	    fprintf(stderr, "\nFile %s does not contain any pages.\n", psfile.name);
	    fprintf(stderr, "Using the entire file and hoping the DSC comments are wrong.\n\n");
	}
        if (doc->numpages > 1) {
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
	    psfree(doc);	/* forget original file */
	    /* scan new file */
	    strcpy(psfile.name, ename);
	    if ((psfile.file = fopen(psfile.name, READBIN)) == (FILE *)NULL) {
	       fprintf(stderr, "Can't open %s\n", psfile.name);
	       return 1;
	    }
	    doc = psscan(psfile.file);
	    if (doc == (PSDOC *)NULL) {
	       fprintf(stderr, "File %s does not contain DSC comments\n", psfile.name);
	       fclose(psfile.file);
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
	   fprintf(tempfile, "%d %d translate\r\n", -doc->boundingbox[LLX], -doc->boundingbox[LLY]);
	/* calculate page size */
	if (calc_bbox) {
	   if (doc->default_page_media) {
	       width = doc->default_page_media->width*resolution/72;
	       height = doc->default_page_media->height*resolution/72;
	   }
	   else {
	       width = 612*resolution/72;
	       height = 792*resolution/72;
	   }
	}
	else {
	   width = (doc->boundingbox[URX] - doc->boundingbox[LLX])*resolution/72;
	   height = (doc->boundingbox[URY] - doc->boundingbox[LLY])*resolution/72;
	}
	/* copy page to temporary file */
	if (doc->numpages != 0) {
	    psfile_extract_header(tempfile);
	    psfile_extract_page(tempfile, page);
	    if (doc->numpages > 1)
		fprintf(stderr,"Can't handle multiple page PostScript files\n");
	}
	else {
	    pscopyuntil(psfile.file, tempfile, doc->beginheader, doc->beginpreview, NULL);
	    pscopyuntil(psfile.file, tempfile, doc->endpreview, doc->endtrailer, NULL);
	}
	fprintf(tempfile, "\nquit\n");
	fclose(tempfile);
	if (op != TIFFGS) {
#ifdef UNIX
	    strcpy(devname, "pbmraw");
#else
	    strcpy(devname, "bmpmono");
#endif
        }
#ifdef UNIX
	sprintf(gscommand, "%s -dNOPAUSE -dQUIET -sDEVICE=%s -sOutputFile=\042%s\042 -r%d -g%dx%d %s",
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
	sprintf(gscommand, "%s @%s", gsname, rspname);
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
	    else if (op == TIFF5)
		code = make_eps_tiff(IDM_MAKEEPST, calc_bbox);
	    else
		fprintf(stderr, "Unknown operation %d\n", op);
	}

	if (*ename) {
	    fclose(psfile.file);
	    if (!debug)
	        unlink(ename);	/* remove temporary file */
	}

	if (!code && !quiet)
	    fprintf(stderr, "Add_preview was successful\n");

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
		case 'b':
		  calc_bbox = !calc_bbox;
		  break;
		case 'd':
		  debug = !debug;
		  break;
		case 'q':
		  quiet = !quiet;
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
		  if (argp[2]=='4') {
		    op = TIFF4;
		    got_op = TRUE;
		  }
		  else if (argp[2]=='5') {
		    op = TIFF5;
		    got_op = TRUE;
		  }
		  else if (argp[2]) {
		    op = TIFFGS;
		    got_op = TRUE;
		    strcpy(devname, argp+2);
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
   fprintf(stderr,"  Copyright (C) 1995 Russell Lang.  All rights reserved.\n");
   fprintf(stderr,"  Version: %s\n", szVersion);
   fprintf(stderr,"  Options:\n");
   fprintf(stderr,"     -b             Calculate BoundingBox from image\n");
   fprintf(stderr,"     -gcommand      Ghostscript command\n");
   fprintf(stderr,"     -nnumber       Page number to extract\n");
   fprintf(stderr,"     -ofilename     Output filename\n");
   fprintf(stderr,"     -q             Quiet (no messages)\n");
   fprintf(stderr,"     -rnumber       Preview resolution in dpi\n");
   fprintf(stderr,"  Operations: (one only)\n");
   fprintf(stderr,"     -i             Add Interchange preview   (EPSI)\n");
   fprintf(stderr,"     -t4            Add TIFF4 preview         (DOS EPS)\n");
   fprintf(stderr,"     -t5            Add TIFF5 preview         (DOS EPS)\n");
   fprintf(stderr,"     -ttiffg3       Add GS TIFF preview       (DOS EPS)\n");
   fprintf(stderr,"     -ufilename     Add user supplied preview (DOS EPS)\n");
   fprintf(stderr,"     -p             Extract PostScript        (DOS EPS)\n");
   fprintf(stderr,"     -v             Extract Preview           (DOS EPS)\n");
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
	if ( (temp = getenv("TEMP")) == NULL )
		_getcwd(fname, MAXSTR);
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
	mktemp(fname);
	return fopen(fname, mode);
}

char * 
_getcwd(char *dirname, int size)
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
  if ( (base = farmalloc(length)) == (char *)NULL )
#else
  if ( (base = malloc(length)) == (char *)NULL )
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
          bitmap_pbmi = (LPBITMAP2)( (char *)pbmf + sizeof(BITMAPFILE) );
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
    char text[PSLINELENGTH];
    char *comment;
    BOOL pages_written = FALSE;
    long position;

    fseek(psfile.file, doc->beginheader, SEEK_SET);
    fgets(text, PSLINELENGTH, psfile.file);
    if (doc->epsf)
        fputs(text,f);
    else {
	switch(text[11]) {
	    case 1:
                fputs("%!PS-Adobe-1.0 EPSF-1.0\r\n",f);
		break;
	    case 2:
                fputs("%!PS-Adobe-2.0 EPSF-2.0\r\n",f);
		break;
	    default:
                fputs("%!PS-Adobe-3.0 EPSF-3.0\r\n",f);
	}
    }
    position = ftell(psfile.file);
    while ( (comment = pscopyuntil(psfile.file, f, position,
			   doc->endheader, "%%Pages:")) != (char *)NULL ) {
	position = ftell(psfile.file);
	if (pages_written) {
	    free(comment);
	    continue;
	}
	fprintf(f, "%%%%Pages: 1\r\n");
	pages_written = TRUE;
	free(comment);
    }
}

/* Copy the selected page and trailer to file f */
void
psfile_extract_page(FILE *f, int page)
{
    char *comment;
    int i;
    long position;

    /* don't copy preview because we might be adding our own */
    pscopyuntil(psfile.file, f, doc->begindefaults, doc->enddefaults, NULL);
    pscopyuntil(psfile.file, f, doc->beginprolog, doc->endprolog, NULL);
    pscopyuntil(psfile.file, f, doc->beginsetup, doc->endsetup, NULL);

    if (doc->pageorder == DESCEND) 
	i = (doc->numpages - 1) - page;
    else
	i = page - 1;
    comment = pscopyuntil(psfile.file, f, doc->pages[i].begin,
			  doc->pages[i].end, "%%Page:");
    fprintf(f, "%%%%Page: %s %d\r\n",
	    doc->pages[i].label, page++);
    free(comment);
    pscopyuntil(psfile.file, f, -1, doc->pages[i].end, NULL);

    position = doc->begintrailer;
    while ( (comment = pscopyuntil(psfile.file, f, position,
			   doc->endtrailer, "%%Pages:")) != (char *)NULL ) {
	position = ftell(psfile.file);
	free(comment);
    }
}

