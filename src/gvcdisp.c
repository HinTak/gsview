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

/* gvcdisp.c */
/* Display GSview routines common to Windows and PM */
#ifdef _Windows
#include "gvwin.h"
#else
#include "gvpm.h"
#endif

FILE *debug_file;

void
gs_puts(char *str, FILE *f)
{
    if (str != NULL) {
	fputs(str, f);
	if (debug_file != (FILE *)NULL)
	   fputs(str, debug_file);
    }
}

void 
gs_copy(FILE *from, FILE *to, long begin, long end)
{
	pscopyuntil(from, to, begin, end, NULL);
	if (debug_file != (FILE *)NULL)
	   pscopyuntil(from, debug_file, begin, end, NULL);
}

/* transform cursor position from device coordinates to points */
/* taking into account rotated pages */
void
transform_cursorpos(float *x, float *y)
{
	  if (zoom) {
            /* first figure out number of pixels to zoom origin point */
	    *x = *x * 72.0/option.xdpi;
	    *y = *y * 72.0/option.ydpi;
	    transform_point(x,y);
	    *x = *x * option.xdpi/72;
	    *y = *y * option.ydpi/72;
	    /* now convert to pts and offset it */
	    *x = *x * 72/option.zoom_xdpi + display.zoom_xoffset;
	    *y = *y * 72/option.zoom_ydpi + display.zoom_yoffset;
	  }
	  else {
	    *x = *x * 72.0/option.xdpi 
		+ (display.epsf_clipped ? doc->boundingbox[LLX] : 0);
	    *y = *y * 72.0/option.ydpi
		+ (display.epsf_clipped ? doc->boundingbox[LLY] : 0);
	    transform_point(x,y);
	  }
}


/* transform point from coordinates relative to bottom left
 * corner of paper to bottom left corner of rotated coordinate
 * system
 */
void
transform_point(float *x, float *y)
{
float oldx, oldy;
int real_orientation;
int width, height;
	oldx = *x;
	oldy = *y;
	width  = (unsigned int)(display.width  * 72.0 / option.xdpi);
	height = (unsigned int)(display.height * 72.0 / option.ydpi);
	real_orientation = option.orientation;
	if (option.swap_landscape) {
	    if (option.orientation == IDM_LANDSCAPE)
		real_orientation = IDM_SEASCAPE;
	    else if (option.orientation == IDM_SEASCAPE)
		real_orientation = IDM_LANDSCAPE;
	}
	switch (real_orientation) {
	    case IDM_PORTRAIT:
		break;
	    case IDM_LANDSCAPE:
	    	*x = height - oldy;
	    	*y = oldx;
	    	break;
	    case IDM_UPSIDEDOWN:
	    	*x = width - oldx;
	    	*y = height - oldy;
		break;
	    case IDM_SEASCAPE:
	    	*x = oldy;
	    	*y = width - oldx;
	    	break;
	}
	return;
}

/* inverse transform point from coordinates relative 
 * to bottom left corner of rotated coordinate system
 * to bottom left corner of paper 
 */
void
itransform_point(float *x, float *y)
{
float oldx, oldy;
int real_orientation;
int width, height;
	oldx = *x;
	oldy = *y;
	width  = (unsigned int)(display.width  * 72.0 / option.xdpi);
	height = (unsigned int)(display.height * 72.0 / option.ydpi);
	real_orientation = option.orientation;
	if (option.swap_landscape) {
	    if (option.orientation == IDM_LANDSCAPE)
		real_orientation = IDM_SEASCAPE;
	    else if (option.orientation == IDM_SEASCAPE)
		real_orientation = IDM_LANDSCAPE;
	}
	switch (real_orientation) {
	    case IDM_PORTRAIT:
		break;
	    case IDM_LANDSCAPE:
	    	*y = height - oldx;
	    	*x = oldy;
	    	break;
	    case IDM_UPSIDEDOWN:
	    	*x = width - oldx;
	    	*y = height - oldy;
		break;
	    case IDM_SEASCAPE:
	    	*y = oldx;
	    	*x = width - oldy;
	    	break;
	}
	return;
}

/* get current media index to paper_size[], or -1 if no match */
int
get_paper_size_index(void)
{
int i;
	for (i=0; papersizes[i].name != (char *)NULL; i++) {
	    if (!stricmp(papersizes[i].name, option.medianame))
		return i;
	}
	return -1;
}

/* calculate bitmap size for gs */
void
gs_size(void)
{
int i = get_paper_size_index();
	if ( (option.xdpi == 0.0) || (option.ydpi == 0.0) )
		option.xdpi = option.ydpi = DEFAULT_RESOLUTION;
	display.epsf_clipped = FALSE;
	switch (option.orientation) {
	    case IDM_LANDSCAPE:
	    case IDM_SEASCAPE:
		if (i < 0) {
		    display.width = option.user_height;
		    display.height = option.user_width;
		}
		else {
		    display.width = papersizes[i].height;
		    display.height = papersizes[i].width;
		}
		break;
	    default:
		if ((doc != (PSDOC *)NULL) && doc->epsf
		    && option.epsf_clip) {
		    display.epsf_clipped = TRUE;
		    display.width = doc->boundingbox[URX] - doc->boundingbox[LLX];
		    display.height = doc->boundingbox[URY] - doc->boundingbox[LLY];
		}
		else if (i < 0) {
		    display.width = option.user_width;
		    display.height = option.user_height;
		}
		else {
		    display.width = papersizes[i].width;
		    display.height = papersizes[i].height;
		}
	}
	display.width  = (unsigned int)(display.width  / 72.0 * option.xdpi);
	display.height = (unsigned int)(display.height / 72.0 * option.ydpi);
}

/* change the size of the gs image if open */
void
gs_resize(void)
{
	gs_size();
	if (!gsprog.valid)
	    return;
	if ( (psfile.file == (FILE *)NULL) && (doc != (PSDOC *)NULL) )
	    dfreopen();

	if (option.redisplay && display.page && (doc != (PSDOC *)NULL))
	    display.do_display = TRUE;

	gsview_endfile();
	display.do_resize = TRUE;
}

void
gs_magnify(float scale)
{
int xtemp, ytemp;
	xtemp = (int)(option.xdpi * scale + 0.5);
	ytemp = (int)(option.ydpi * scale + 0.5);
	if ( (xtemp == option.xdpi) && (scale > 1.0) ) {
	    option.xdpi++;	/* force magnification if requested */
	    option.ydpi++;
	}
	else {
	    option.xdpi = xtemp;
	    option.ydpi = ytemp;
	}
	dfreopen();
	gs_resize();
	zoom = FALSE;
	dfclose();
}

void
gsview_orientation(int new_orientation)
{
	if (new_orientation == option.orientation)
		return;
	if (new_orientation == IDM_SWAPLANDSCAPE) {
	    option.swap_landscape = !option.swap_landscape;
	    if (option.swap_landscape) 
	        check_menu_item(IDM_ORIENTMENU, IDM_SWAPLANDSCAPE, TRUE);
	    else
	        check_menu_item(IDM_ORIENTMENU, IDM_SWAPLANDSCAPE, FALSE);
	    if ((option.orientation != IDM_LANDSCAPE) && (option.orientation != IDM_SEASCAPE))
	        return;
	}
	else {
	    check_menu_item(IDM_ORIENTMENU, option.orientation, FALSE);
	    option.orientation = new_orientation;
	    check_menu_item(IDM_ORIENTMENU, option.orientation, TRUE);
	}
	gs_resize();
        zoom = FALSE;
	return;
}

void
gsview_media(int new_media)
{
	if ( (new_media == option.media) && (new_media != IDM_USERSIZE) )
		return;
	check_menu_item(IDM_MEDIAMENU, option.media, FALSE);
	option.media = new_media;
	check_menu_item(IDM_MEDIAMENU, option.media, TRUE);
	get_menu_string(IDM_MEDIAMENU, option.media, option.medianame, sizeof(option.medianame));
	gs_resize();
        zoom = FALSE;
	return;
}

void
gsview_unit(int new_unit)
{
	check_menu_item(IDM_UNITMENU, option.unit, FALSE);
	option.unit = new_unit;
	check_menu_item(IDM_UNITMENU, option.unit, TRUE);
	info_wait(FALSE);
	return;
}

void
gsview_endfile()
{
	if (!gsprog.valid)
	    return;
	if (!option.quick ||
             ((doc == (PSDOC *)NULL) && !is_pipe_done())) {
		gs_close();
		return;
	}

	if (display.page)
	    next_page();

	display.do_endfile = TRUE;
	psfile.previous_was_dsc = (doc != (PSDOC *)NULL) && doc->pages;
	if (psfile.previous_was_dsc) {
	    strcpy(psfile.previous_name, psfile.name);
	    psfile.previous_begintrailer = doc->begintrailer;
	    psfile.previous_endtrailer = doc->endtrailer;
	}
	else {
	    psfile.previous_name[0] = '\0';
	    psfile.previous_begintrailer = 0;
	    psfile.previous_endtrailer = 0;
	}

}

/* open a new document */
void
gsview_openfile(char *filename)
{
int i;
	load_string(IDS_WAITREAD, szWait, sizeof(szWait));
	info_wait(TRUE);
	psfile.pagenum = 1;
	page_extra = 0;
	if (dsc_scan(filename)) {
	    /* found DSC comments */
	    if (doc->orientation == PORTRAIT)
		gsview_orientation(IDM_PORTRAIT);
	    if (doc->orientation == LANDSCAPE)
		gsview_orientation(IDM_LANDSCAPE);
	    if (doc->default_page_media) {
		char thismedia[20];
		for (i=IDM_LETTER; i<IDM_USERSIZE; i++) {
		    get_menu_string(IDM_MEDIAMENU, i, thismedia, sizeof(thismedia));
		    if (!stricmp(thismedia, doc->default_page_media->name)) {
		        gsview_media(i);
		        break;
		    }
		}
		if (i == IDM_USERSIZE) {
		    gsview_media(IDM_USERSIZE);
		    option.user_width  = doc->default_page_media->width;
		    option.user_height = doc->default_page_media->height;
		    gsview_check_usersize();
		}
	    }
	}
}


/* get filename then open new file for printing or extract */
void 
gsview_select()
{
char buf[MAXSTR];
	strcpy(buf, previous_filename);
	if (get_filename(buf, FALSE, FILTER_PSALL, 0, IDS_TOPICOPEN))
		gsview_selectfile(buf);
}

/* open new file for printing or extract */
void
gsview_selectfile(char *filename)
{
	if (gsprog.valid)
	    gsview_endfile();
	while (*filename && *filename==' ')
	     filename++;
	gsview_openfile(filename);
	strcpy(previous_filename, filename);
	info_wait(FALSE);
}

/* get filename then open a new document and display it */
void 
gsview_display()
{
char buf[MAXSTR];
	strcpy(buf, previous_filename);
	if (get_filename(buf, FALSE, FILTER_PSALL, 0, IDS_TOPICOPEN))
		gsview_displayfile(buf);
}

/* open a new document and display it */
void
gsview_displayfile(char *filename)
{
	gsview_endfile();
	gsview_openfile(filename);
	strcpy(previous_filename, filename);
	if (display.epsf_clipped || ((doc != (PSDOC *)NULL) 
		&& doc->epsf && option.epsf_clip))
	    gs_resize();
	display.do_display = TRUE;
}


/* add Ghostscript code to change orientation */
void
fix_orientation(FILE *f)
{
int real_orientation;
char buf[MAXSTR];
	/* save interpreter state */
	gs_puts("clear cleardictstack save /gsview_save exch def\r\n",f);
	display.saved = TRUE;
	/* provide zoom or epsf offset */
        if (zoom) {
	    sprintf(buf,"/gsview_offset {%d %d translate} def\r\n",
	        -display.zoom_xoffset, -display.zoom_yoffset);
	}
	else if (display.epsf_clipped)
	    sprintf(buf,"/gsview_offset {%d %d translate} def\r\n",
	        -doc->boundingbox[LLX], -doc->boundingbox[LLY]);
	else
	    sprintf(buf,"/gsview_offset {} def\r\n");
	gs_puts(buf, f);
	real_orientation = option.orientation;
	if (option.swap_landscape) {
	    if (option.orientation == IDM_LANDSCAPE)
		real_orientation = IDM_SEASCAPE;
	    else if (option.orientation == IDM_SEASCAPE)
		real_orientation = IDM_LANDSCAPE;
	}
	sprintf(buf,"/gsview_landscape  %s def\r\n",
	    real_orientation == IDM_LANDSCAPE ? "true" : "false");
	gs_puts(buf, f);
	sprintf(buf,"/gsview_upsidedown %s def\r\n",
	    real_orientation ==  IDM_UPSIDEDOWN ? "true" : "false");
	gs_puts(buf, f);
	sprintf(buf,"/gsview_seascape   %s def\r\n",
	    real_orientation == IDM_SEASCAPE ? "true" : "false");
	gs_puts(buf, f);
	sprintf(buf,"/gsview_zoom %s def\r\n", zoom ? "true" : "false");
	gs_puts(buf, f);
	send_prolog(f, IDR_ORIENT);
#if !defined(__WIN32__) && defined(GS261)
	if (option.gsversion != IDM_GS261)
#endif
	    send_prolog(f, IDR_ORIENT3);
	if (option.epsf_warn)
	    send_prolog(f, IDR_EPSFWARN);
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
		    *temp = '\\';
	}
	if ( strlen(fname) && (fname[strlen(fname)-1] != '\\') )
		strcat(fname, "\\");

	strcat(fname, prefix);
	strcat(fname, "XXXXXX");
	mktemp(fname);
	return fopen(fname, mode);
}

/* reopen psfile */
/* if psfile time/date or length has changed, kill gs and rescan the file */
BOOL
dfreopen()
{
char *filename;
	if (psfile.ispdf) 
	    filename = psfile.pdftemp;
	else
	    filename = psfile.name;
	if (doc == (PSDOC *)NULL)
		return TRUE;
	dfclose();
	if (filename[0] == '\0')
		return TRUE;
	if ( (psfile.file = fopen(filename, "rb")) == (FILE *)NULL ) {
	    if (debug)
		message_box("dfreopen: file missing",0);
	    filename[0] = '\0';
	    return FALSE;
	}
	if (psfile_changed()) {  /* doesn't cope with pdf file changing */
	    if (debug)
		message_box("dfreopen: file changed",0);
	    /* file may have changed beyond recognition so we must kill gs */
	    gs_close();
	    if (dsc_scan(psfile.name))
		if (psfile.ispdf) 
		    filename = psfile.pdftemp;
		else
		    filename = psfile.name;
	        if ( (psfile.file = fopen(filename, "rb")) == (FILE *)NULL ) {
		        psfile.name[0] = '\0';
		        return FALSE;
	        }
	}
	return TRUE;
}

void
dfclose()
{
	if (psfile.file != (FILE *)NULL)
		fclose(psfile.file);
	psfile.file = (FILE *)NULL;
}

/* take a PDF file, process it with gs to produce a DSC index file */
/* then use the index file as a DSC document */
BOOL
dsc_pdf(void)
{
#ifdef OS2
	int flag;
	char command[MAXSTR+MAXSTR];
	char progname[256];
	char *args;
	FILE *tempfile;
	char temp[MAXSTR];
	int i;
	
	/* change directory separators from \ to / */
	strcpy(temp, psfile.name);
	for (args=temp; *args; args++) {
	    if (*args == '\\')
		*args = '/';
	}

	args = strchr(option.gscommand, ' ');
	if (args) {
	    strncpy(progname, option.gscommand, (int)(args-option.gscommand));
	    progname[(int)(args-option.gscommand)] = '\0';
	    args++;
	}
	else {
	    strncpy(progname, option.gscommand, MAXSTR);
	    args = "";
	}

	/* get a temporary filename for pdf DSC index */
	if ( (tempfile = gp_open_scratch_file(szScratch, psfile.pdftemp, "wb")) == (FILE *)NULL)
	    return FALSE;
	fclose(tempfile);
	
	sprintf(command,"%s -dNODISPLAY -sPDFname=%s -sDSCname=%s pdf2dsc.ps", args, temp, psfile.pdftemp);

	if (strlen(command) > MAXSTR-1) {
		/* command line too long */
		gserror(IDS_TOOLONG, command, MB_ICONHAND, SOUND_ERROR);
		if (!debug)
		    unlink(psfile.pdftemp);
		psfile.pdftemp[0] = '\0';
		return FALSE;
	}

	load_string(IDS_WAIT, szWait, sizeof(szWait));
	info_wait(TRUE);

	flag = pdf_convert(progname, command, &pdfconv);
	if (!flag) {
		gserror(IDS_CANNOTRUN, command, MB_ICONHAND, SOUND_ERROR);
		if (!debug)
		    unlink(psfile.pdftemp);
		psfile.pdftemp[0] = '\0';
		info_wait(FALSE);
		return FALSE;
	}
	/* open DSC index file */
	if ( (psfile.file = fopen(psfile.pdftemp, "rb")) == (FILE *)NULL ) {
		psfile.name[0] = '\0';
		return FALSE;
	}
	psfile.ispdf = TRUE;
	return TRUE;
#else
	message_box("Can't handle PDF files", 0);
	return FALSE;
#endif
}

/* scan file for PostScript Document Structuring Conventions */
/* return TRUE if valid DSC comments found */
BOOL
dsc_scan(char *filename)
{
char line[MAXSTR];
	dfclose();
	if (psfile.ispdf && psfile.name[0] && psfile.pdftemp[0])
	    unlink(psfile.pdftemp);  /* remove temporary DSC file */
	strcpy(psfile.name, filename);
	if ( (psfile.file = fopen(psfile.name, "rb")) == (FILE *)NULL ) {
		psfile.name[0] = '\0';
		return FALSE;
	}
	/* check for PDF */
	psfile.ispdf = FALSE;
	fgets(line, sizeof(line)-1, psfile.file);
        rewind(psfile.file);
	if ( strncmp("%PDF-", line, 5) == 0 ) {
	    dfclose();
	    if (!dsc_pdf())
		return FALSE;
	}
	/* save file */
	psfile_savestat();
	if (page_list.select)
		free(page_list.select);
	page_list.select = NULL;
	if (doc)
		psfree(doc);
	psfile.preview = 0;
	/* check for documents that start with Ctrl-D */
	psfile.ctrld = (line[0] == '\004');
	if (option.ignore_dsc)
	    doc = (PSDOC *)NULL;
	else 
	    doc = psscan(psfile.file);
	if (doc == (PSDOC *)NULL) {
	    dfclose();
	    return FALSE;
	}
	if (doc->doseps) {
	    if (doc->doseps->tiff_begin)
		psfile.preview = IDS_EPST;
	    if (doc->doseps->mf_begin)
		psfile.preview = IDS_EPSW;
	}
	if (!psfile.preview && (doc->beginpreview != doc->endpreview))
	    psfile.preview = IDS_EPSI;
	page_list.select = (BOOL *)malloc( doc->numpages * sizeof(BOOL) );
	return TRUE;
}



/* Copy specified pages from psfile.file to file f */
void
dsc_getpages(FILE *f, int first, int last)
{
int i, page;
char buf[MAXSTR];
	for (i=first-1; i<last; i++) {
	    page = map_page(i);
	    if (doc->pages) {
	        sprintf(buf,"(Page: %s %d\\n) print flush\r\n", doc->pages[page].label ? doc->pages[page].label : " ", page+1);
		gs_puts(buf, f);
    		if (debug)
		    gs_puts("%GSview beginpage\r\n", gsprog.input);
		gs_copy(psfile.file, f, doc->pages[page].begin, doc->pages[page].end);
    		if (debug)
		    gs_puts("%GSview endpage\r\n", gsprog.input);
	    }
	    else {
	        sprintf(buf,"(Page: %d\\n) print flush\r\n",page); 
		gs_puts(buf, f);
    		if (debug)
		    gs_puts("%GSview endsetup\r\n", gsprog.input);
		gs_copy(psfile.file, f, doc->endsetup, doc->endtrailer);
    		if (debug)
		    gs_puts("\n%GSview endtrailer\r\n", gsprog.input);
	    }
	}
}


/* Copy dsc header to file f */
void
dsc_header(FILE *f)
{
char *p, *d;
char buf[MAXSTR];
	d = buf;
	gs_puts("(Displaying ",f);
	for (p=psfile.name; *p; p++) {
	    if (*p != '\\')
		*d++ = *p;
	    else
	        *d++ = '/';
	}
	*d = '\0';
	gs_puts(buf, f);
	gs_puts("\\n) print flush\r\n", f);
	if (debug)
	    gs_puts("%GSview beginheader\r\n", gsprog.input);
	gs_copy(psfile.file, f, doc->beginheader, doc->endheader);
	if (debug)
	    gs_puts("%GSview endheader\r\n%GSview begindefaults\r\n", gsprog.input);
	gs_copy(psfile.file, f, doc->begindefaults, doc->enddefaults);
	if (debug)
	    gs_puts("%GSview enddefaults\r\n%GSview beginprolog\r\n", gsprog.input);
	gs_copy(psfile.file, f, doc->beginprolog, doc->endprolog);
	if (debug)
	    gs_puts("%GSview endprolog\r\n%GSview beginsetup\r\n", gsprog.input);
	gs_copy(psfile.file, f, doc->beginsetup, doc->endsetup);
	if (debug)
	    gs_puts("%GSview endsetup\r\n", gsprog.input);
}


/* Send commands to gs to display page */
void
dsc_dopage(void)
{
	load_string(IDS_WAITDRAW, szWait, sizeof(szWait));
	info_wait(TRUE);
	display.do_display = TRUE;
}

/* skip pages */
void
dsc_skip(int skip)
{
	if ( (skip == 0)
	  || ((skip > 0) && (psfile.pagenum == doc->numpages))
	  || ((skip < 0) && (psfile.pagenum == 1))
	  || (doc->numpages == 0) ) {
	    play_sound(SOUND_NOPAGE);
	    info_wait(FALSE);
	    return;
	}
	psfile.pagenum += skip;
	if (psfile.pagenum > (int)doc->numpages)
	     psfile.pagenum = doc->numpages;
	if (psfile.pagenum < 1)
	    psfile.pagenum = 1;
	load_string(IDS_WAIT, szWait, sizeof(szWait));
	info_wait(TRUE);
	if (display.page)
	    next_page();
	if (gs_open())
	    dsc_dopage();
}

/* reverse zero based page number if needed */
int
map_page(int page)
{
    	if (doc->pageorder == DESCEND) 
		return (doc->numpages - 1) - page;
	return page;
}

/* Send necessary output to display Ghostscript */
/* This must not be called from thread 1 because it is lengthy */
/* Functions called from here must NOT create windows since this */
/* thread does not have an anchor block or message queue */
/* returns TRUE if OK, FALSE if aborted */
BOOL
do_output()
{
    char *p, *d;
    char debug_filename[MAXSTR];
    char buf[256];

    if (debug_file != (FILE *)NULL)
	fclose(debug_file);
    if (debug)
        debug_file = gp_open_scratch_file(szScratch, debug_filename, "wb");

    if (gsprog.valid && display.page)
	next_page();

#if !defined(__WIN32__) && defined(GS261)
    if (option.gsversion != IDM_GS261) {
#endif
	/* Cause a GS_BEGIN message to be sent to GSview */
/*
	gs_puts("(Begin\\n) print flush\r\n", gsprog.input);
*/
	gs_puts("-1 false .outputpage\r\n", gsprog.input);
#if !defined(__WIN32__) && defined(GS261)
    }
#endif

    if (display.do_endfile && gsprog.valid) {
	if ((display.saved) && (psfile.previous_was_dsc)) {
	    /* send trailer if needed */
	    FILE *f;
	    if ( (f = fopen(psfile.previous_name, "rb")) != (FILE *)NULL ) {
    		if (debug)
		    gs_puts("%GSview begintrailer\r\n", gsprog.input);
	        gs_copy(f, gsprog.input, psfile.previous_begintrailer, psfile.previous_endtrailer);
    		if (debug)
		    gs_puts("%GSview endtrailer\r\n", gsprog.input);
		fclose(f);
	    }
	}
	if (display.saved) {
	    /* restore interpreter state */
	    gs_puts("gsview_cleanup\r\ngsview_save restore\r\n", gsprog.input);
	}
	else
	    gs_puts("clear cleardictstack\r\n", gsprog.input);
	gs_puts("erasepage\r\n", gsprog.input); /* needed for documents that don't use showpage */
	display.saved = FALSE;
    }

    if (display.abort)
	return FALSE;

    if (display.do_resize && gsprog.valid) {
	sprintf(buf, "mark /HWSize [%u %u]\r\n",display.width,display.height);
	gs_puts(buf, gsprog.input);
	if (zoom)
            sprintf(buf,"/HWResolution [%g %g]\r\n",option.zoom_xdpi,option.zoom_ydpi);
        else
            sprintf(buf,"/HWResolution [%g %g]\r\n",option.xdpi,option.ydpi);
	gs_puts(buf, gsprog.input);
        sprintf(buf,"currentdevice putdeviceprops pop initgraphics erasepage\r\n");
	gs_puts(buf, gsprog.input);
    }

    if (display.abort)
	return FALSE;

    if (display.do_display) {
	if (doc != (PSDOC *)NULL) {
	    /* found DSC comments */
	    if (!display.saved) {
	        fix_orientation(gsprog.input);
	        dsc_header(gsprog.input);
	    }
            if (display.abort)
	        return FALSE;
	    dsc_getpages(gsprog.input,psfile.pagenum,psfile.pagenum);
	}
	else {
	    if (!display.saved) {
	        fix_orientation(gsprog.input);
	    }
	    /* non conformant file - send unmodified */
	    gs_puts("(Displaying ",gsprog.input);
	    d = buf;
	    for (p=psfile.name; *p; p++) {
		if (*p != '\\')
			*d++ = *p;
		else
			*d++ = '/';
	    }
	    *d = '\0';
	    gs_puts(buf, gsprog.input);
	    gs_puts("\\n) print flush\r\n",gsprog.input);
	    d = buf;
	    *d++ = '(';
	    for (p=psfile.name; *p; p++) {
		if (*p != '\\')
			*d++ = *p;
		else
			*d++ = '/';
	    }
	    *d = '\0';
	    gs_puts(buf, gsprog.input);
	    gs_puts(") run\r\n",gsprog.input);
	}
    }

#if !defined(__WIN32__) && defined(GS261)
    if (option.gsversion != IDM_GS261) {
#endif
	/* Cause a GS_END message to be sent to GSview */
/*
	gs_puts("(End\\n) print flush\r\n", gsprog.input);
*/
	gs_puts("-2 false .outputpage\r\n", gsprog.input);
#if !defined(__WIN32__) && defined(GS261)
    }
#endif

    if (gsprog.valid)
        gs_puts("flushpage\r\n",gsprog.input);

    dfclose();
    if (debug_file)
        fclose(debug_file);
    debug_file = (FILE *)NULL;


    return TRUE;	/* all done */
}
