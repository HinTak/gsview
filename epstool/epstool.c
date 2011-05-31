/* Copyright (C) 1993, 1994, Russell Lang.  All rights reserved.
  
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

char iname[MAXSTR];
char oname[MAXSTR];
char gsname[MAXSTR] = GSCOMMAND;
char bmpname[MAXSTR];
char szScratch[] = "ep";
char szAppName[] = "epstool";
char szVersion[] = "0.1 alpha 1994-05-24";
int resolution = 72;
int page = 1;	/* default is page 1 */
BOOL calc_bbox = FALSE;
BOOL got_op = FALSE;
BOOL debug = FALSE;
int op = 0;
#define EXTRACTPS	1
#define EXTRACTPRE	2
#define INTERCHANGE	3
#define TIFF4 		4
#define TIFF5		5

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

	if (calc_bbox && op!=INTERCHANGE) {
	   fprintf(stderr, "Calculation of bbox only works for interchange preview\n");
	}

	strcpy(psfile.name, iname);
	if ((psfile.file = fopen(psfile.name, READBIN)) == (FILE *)NULL) {
	   fprintf(stderr, "Can't open %s\n", psfile.name);
	   return 1;
	}
	doc = dsc_scan_file(psfile.file);
	if (doc == (PSDOC *)NULL) {
	   fprintf(stderr, "File %s does not contain DSC comments\n", psfile.file);
	   fclose(psfile.file);
	   return 1;
	}

	if (op==INTERCHANGE || op==TIFF4 || op==TIFF5)
	   return add_preview();
	if (op==EXTRACTPS || op==EXTRACTPRE)
	   return extract_section();

	dsc_scan_clean(doc);
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
int width, height;
	if ( !calc_bbox &&
             ((doc->bbox.urx == doc->bbox.llx) ||
	      (doc->bbox.ury == doc->bbox.lly)) ) {
	   fprintf(stderr, "Bounding Box is empty");
	   return 1;
	   /* if calc_bbox, this shouldn't be an error */
	}
	if (doc->numpages==0) {
	    fprintf(stderr, "\nFile %s does not contain any pages\n", psfile.name);
	    return 1;
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
	    dsc_scan_clean(doc);	/* forget original file */
	    /* scan new file */
	    strcpy(psfile.name, ename);
	    if ((psfile.file = fopen(psfile.name, READBIN)) == (FILE *)NULL) {
	       fprintf(stderr, "Can't open %s\n", psfile.file);
	       return 1;
	    }
	    doc = dsc_scan_file(psfile.file);
	    if (doc == (PSDOC *)NULL) {
	       fprintf(stderr, "File %s does not contain DSC comments\n", psfile.file);
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
	bmpfile = gp_open_scratch_file(szScratch, bmpname, WRITEBIN);
	if (bmpfile == (FILE *)NULL) {
	    fprintf(stderr, "Couldn't open temporary file %s\n", bmpname);
	    return 1;
	}
	fclose(bmpfile);
	if (!debug)
	   unlink(bmpname);
	/* offset to bottom left corner of bounding box */
	if (!calc_bbox)
	   fprintf(tempfile, "%d %d translate\r\n", -doc->bbox.llx, -doc->bbox.lly);
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
	   width = (doc->bbox.urx - doc->bbox.llx)*resolution/72;
	   height = (doc->bbox.ury - doc->bbox.lly)*resolution/72;
	}
	/* copy page to temporary file */
	if (doc->numpages != 0) {
	    psfile_extract_header(tempfile);
	    psfile_extract_page(tempfile, page);
	    if (doc->numpages > 1)
		fprintf(stderr,"Can't handle multiple page PostScript files\n");
	}
	else {
	    dsc_copy(psfile.file, tempfile, doc->begincomments, doc->beginpreview, NULL);
	    dsc_copy(psfile.file, tempfile, doc->endpreview, doc->endtrailer, NULL);
	}
	fprintf(tempfile, "\nquit\n");
	fclose(tempfile);
#ifdef UNIX
#define DEVICE "pbmraw"
#else
#define DEVICE "bmpmono"
#endif
	sprintf(gscommand, "%s -dNOPAUSE -sDEVICE=%s -sOutputFile=%s -r%d -g%dx%d %s",
	   gsname, DEVICE, bmpname, resolution, width, height, tempname);
	fprintf(stderr,"%s\n", gscommand);
	system(gscommand);
	if (!load_bitmap()) {
	    if (!debug) {
	        unlink(bmpname);
	        unlink(tempname);
	    }
	    fprintf(stderr, "no bitmap\n");
	    return 1;
	}
	if (!debug) {
	    unlink(bmpname);
	    unlink(tempname);
	}
	/* now create new file with preview */
	if (op == INTERCHANGE)
	    make_eps_interchange(calc_bbox);
	else if (op == TIFF4)
	    make_eps_tiff(IDM_MAKEEPST4);
	else if (op == TIFF5)
	    make_eps_tiff(IDM_MAKEEPST);
	else
	    fprintf(stderr, "Unknown operation %d\n", op);

	if (*ename) {
	    fclose(psfile.file);
	    if (!debug)
	        unlink(ename);	/* remove temporary file */
	}

	fprintf(stderr, "Add_preview was successful\n");
	return 0;
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
		    count++;
		    if (count >= argc) {
		      fprintf(stderr,"Missing output filename for -o\n");
		      return 1;
		    }
		    argp=argv[count];
		    strcpy(oname, argp);
		  }
		  break;
		case 'n':
		  if (argp[2])
		    page = atoi(argp+2);
		  else {
		    count++;
		    if (count >= argc) {
		      fprintf(stderr,"Missing page number for -n\n");
		      return 1;
		    }
		    argp=argv[count];
		    page = atoi(argp);
		  }
		  break;
		case 'r':
		  if (argp[2])
		    resolution = atoi(argp+2);
		  else {
		    count++;
		    if (count >= argc) {
		      fprintf(stderr,"Missing resolution for -r\n");
		      return 1;
		    }
		    argp=argv[count];
		    resolution = atoi(argp);
		  }
		  break;
		case 'b':
		  calc_bbox = !calc_bbox;
		  break;
		case 'd':
		  debug = !debug;
		  break;
		case 'g':
		  if (argp[2])
		    strcpy(gsname, argp+2);
		  else {
		    count++;
		    if (count >= argc) {
		      fprintf(stderr,"Missing Ghostscript command for -g\n");
		      return 1;
		    }
		    argp=argv[count];
		    strcpy(gsname, argp);
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
		  else {
		    fprintf(stderr,"Unknown option %s\n", argp);
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
   fprintf(stderr,"  Version: %s\n", szVersion);
   fprintf(stderr,"  Options:\n");
   fprintf(stderr,"     -b             Calculate BoundingBox from image\n");
   fprintf(stderr,"     -g command     Ghostscript command\n");
   fprintf(stderr,"     -n number      Page number to extract\n");
   fprintf(stderr,"     -o filename    Output filename\n");
   fprintf(stderr,"     -r number      Preview resolution in dpi\n");
   fprintf(stderr,"  Operations: (one only)\n");
   fprintf(stderr,"     -i             Add Interchange preview (EPSI)\n");
   fprintf(stderr,"     -t4            Add TIFF4 preview       (DOS EPS)\n");
   fprintf(stderr,"     -t5            Add TIFF5 preview       (DOS EPS)\n");
   fprintf(stderr,"     -p             Extract PostScript      (DOS EPS)\n");
   fprintf(stderr,"     -v             Extract Preview         (DOS EPS)\n");
}

char *err_msgs[] = {"No preview in input file", ""};
void 
gserror(UINT id, char *str, UINT icon, int sound)
{
	fprintf(stderr, "%s %s\n", err_msgs[id], str);
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
  if ( (base = farmalloc(length)) == (char *)NULL ) {
#else
  if ( (base = malloc(length)) == (char *)NULL ) {
#endif
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

    fseek(psfile.file, doc->begincomments, SEEK_SET);
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
    while ( (comment = dsc_copy(psfile.file, f, position,
			   doc->endcomments, "%%Pages:")) != (char *)NULL ) {
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
    dsc_copy(psfile.file, f, doc->begindefaults, doc->enddefaults, NULL);
    dsc_copy(psfile.file, f, doc->beginprolog, doc->endprolog, NULL);
    dsc_copy(psfile.file, f, doc->beginsetup, doc->endsetup, NULL);

    if (doc->pageorder == DESCEND) 
	i = (doc->numpages - 1) - page;
    else
	i = page - 1;
    comment = dsc_copy(psfile.file, f, doc->pages[i].begin,
			  doc->pages[i].end, "%%Page:");
    fprintf(f, "%%%%Page: %s %d\r\n",
	    doc->pages[i].label, page++);
    free(comment);
    dsc_copy(psfile.file, f, -1, doc->pages[i].end, NULL);

    position = doc->begintrailer;
    while ( (comment = dsc_copy(psfile.file, f, position,
			   doc->endtrailer, "%%Pages:")) != (char *)NULL ) {
	position = ftell(psfile.file);
	free(comment);
    }
}

