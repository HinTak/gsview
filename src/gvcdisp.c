/* Copyright (C) 1993-1998, Ghostgum Software Pty Ltd.  All rights reserved.
  
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
	    *x = *x * 72.0/option.xdpi;
	    *y = *y * 72.0/option.ydpi;
	    transform_point(x,y);
	    *x = *x + (display.epsf_clipped ? psfile.doc->boundingbox[LLX] : 0);
	    *y = *y + (display.epsf_clipped ? psfile.doc->boundingbox[LLY] : 0);
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

	switch(d_orientation(psfile.pagenum)) {
	    default:
	    case 0:
		real_orientation = IDM_PORTRAIT;
		break;
	    case 1:
		real_orientation = IDM_SEASCAPE;
		break;
	    case 2:
		real_orientation = IDM_UPSIDEDOWN;
		break;
	    case 3:
		real_orientation = IDM_LANDSCAPE;
		break;
	}

	switch (real_orientation) {
	    case IDM_PORTRAIT:
		break;
	    case IDM_LANDSCAPE:
	    	*x = width - oldy;	/* display.width = bitmap.height */
	    	*y = oldx;
	    	break;
	    case IDM_UPSIDEDOWN:
	    	*x = width - oldx;
	    	*y = height - oldy;
		break;
	    case IDM_SEASCAPE:
	    	*x = oldy;
	    	*y = height - oldx;
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

	switch(d_orientation(psfile.pagenum)) {
	    default:
	    case 0:
		real_orientation = IDM_PORTRAIT;
		break;
	    case 1:
		real_orientation = IDM_SEASCAPE;
		break;
	    case 2:
		real_orientation = IDM_UPSIDEDOWN;
		break;
	    case 3:
		real_orientation = IDM_LANDSCAPE;
		break;
	}

	switch (real_orientation) {
	    case IDM_PORTRAIT:
		break;
	    case IDM_LANDSCAPE:
	    	*y = width - oldx;
	    	*x = oldy;
	    	break;
	    case IDM_UPSIDEDOWN:
	    	*x = width - oldx;
	    	*y = height - oldy;
		break;
	    case IDM_SEASCAPE:
	    	*y = oldx;
	    	*x = height - oldy;
	    	break;
	}
	return;
}

/* calculate depth */
int
real_depth(int depth)
{
    if (depth == 0)
        depth = display.planes * display.bitcount;
    if (depth > 8)
	depth = 24;
    else if (depth >=8)
	depth = 8;
    else if (depth >=4)
	depth = 4;
    else 
	depth = 1;
    return depth;
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


/* change the size of the gs image if open */
void
gs_resize(void)
{
	pending.resize = TRUE;

	if ( gsdll.hmodule &&  (psfile.doc==(PSDOC *)NULL) && (gsdll.state != IDLE) )
	    /* don't know where we are so close and reopen */
	    pending.abort = TRUE;

	if (option.redisplay && (gsdll.state == PAGE)) {
	    if (psfile.doc != (PSDOC *)NULL)
	        pending.now = TRUE;
	    else {
		pending.abort = TRUE;	/* must restart from page 1 */
		pending.now = TRUE;
	    }
	}
	if (option.redisplay && (gsdll.state == IDLE)
	   && (psfile.doc != (PSDOC *)NULL)) {
	     /* zero page EPS file */
	    pending.now = TRUE;
	}
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
	zoom = FALSE;
	gs_resize();
}

void
gsview_orientation(int new_orientation)
{
	if (new_orientation == IDM_AUTOORIENT) {
	    check_menu_item(IDM_ORIENTMENU, option.orientation, option.auto_orientation);
	    option.auto_orientation = !option.auto_orientation;
	    check_menu_item(IDM_ORIENTMENU, IDM_AUTOORIENT, option.auto_orientation);
	    zoom = FALSE;
	    gs_resize();
	    return;
	}

	if (option.auto_orientation && (new_orientation != IDM_SWAPLANDSCAPE)) {
	    option.auto_orientation = FALSE;
	    check_menu_item(IDM_ORIENTMENU, IDM_AUTOORIENT, option.auto_orientation);
	} 
  	else if (new_orientation == option.orientation)
		return;

	if (new_orientation == IDM_SWAPLANDSCAPE) {
	    option.swap_landscape = !option.swap_landscape;
	    if (option.swap_landscape) 
	        check_menu_item(IDM_ORIENTMENU, IDM_SWAPLANDSCAPE, TRUE);
	    else
	        check_menu_item(IDM_ORIENTMENU, IDM_SWAPLANDSCAPE, FALSE);
	    if ((option.orientation != IDM_LANDSCAPE) && 
		(option.orientation != IDM_SEASCAPE) && 
		(option.auto_orientation == FALSE))
	        return;
	}
	else {
	    check_menu_item(IDM_ORIENTMENU, option.orientation, FALSE);
	    option.orientation = new_orientation;
	    check_menu_item(IDM_ORIENTMENU, option.orientation, TRUE);
	}
        zoom = FALSE;
	gs_resize();
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
	return;
}

/* free a PSFILE and contents */
/* Do NOT use this if you have just copied PSFILE to psfile */
void
e_free_psfile(PSFILE *ppsfile)
{
    psfile_free(ppsfile);
    free(ppsfile);
}


/* open a new document */
PSFILE *
gsview_openfile(char *filename)
{
int i;
PSFILE *tpsfile;
    history_reset();
    tpsfile = (PSFILE *)malloc(sizeof(PSFILE));
    if (tpsfile == NULL)
	return NULL;
    memset((char *)tpsfile, 0, sizeof(PSFILE));

    strcpy(tpsfile->name, filename);
    tpsfile->pagenum = 1;
    info_wait(IDS_WAITREAD);
    if (dsc_scan(tpsfile)) {
        PSDOC *doc = tpsfile->doc;
	/* found DSC comments */
#ifdef OLD
	if (doc->orientation == PORTRAIT)
	    gsview_orientation(IDM_PORTRAIT);
	if (doc->orientation == LANDSCAPE)
	    gsview_orientation(IDM_LANDSCAPE);
#endif
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
    if (tpsfile->name[0] == '\0') {
	e_free_psfile(tpsfile);
	info_wait(IDS_NOWAIT);
	return NULL;
    }
    return tpsfile;
}


void
rotate_last_files(int count)
{
int i;
char buf[MAXSTR];
    strcpy(buf, last_files[count]);
    for (i=count; i>0; i--)
	strcpy(last_files[i], last_files[i-1]);
    strcpy(last_files[0], buf);
}

void
update_last_files(char *filename)
{
int i;
    for (i=0; i<last_files_count; i++) {
	if (strcmp(filename, last_files[i]) == 0)
	    break;
    }
    if (i < last_files_count) {
	/* already in list */
	rotate_last_files(i);
	return;
    }
    if (last_files_count < 4)
        last_files_count++;
    rotate_last_files(last_files_count-1);
    strcpy(last_files[0], filename);
}

/* get filename then open new file for printing or extract */
void 
gsview_select()
{
char buf[MAXSTR];
	strcpy(buf, psfile.name);
	if (get_filename(buf, FALSE, FILTER_PSALL, 0, IDS_TOPICOPEN))
		gsview_selectfile(buf);
}

/* open new file for printing or extract */
void
gsview_selectfile(char *filename)
{
	while (*filename && *filename==' ')
	     filename++;

	update_last_files(filename);

	if (gsdll.valid && (gsdll.state!=UNLOADED)) {
	    /* remember name for later */
	    strncpy(selectname, filename, sizeof(selectname));
	    /* close file and wait for notification */
	    post_img_message(WM_COMMAND, IDM_CLOSE);
	}
	else {
	    /* open it ourselves */
	    PSFILE *tpsfile = gsview_openfile(filename);
	    if (tpsfile) {
		psfile_free(&psfile);
		psfile = *tpsfile;
		post_img_message(WM_GSTITLE, 0);
		free(tpsfile);	/* Do NOT free doc and page_list.select */
	    }
	}
	info_wait(IDS_NOWAIT);
}

/* get filename then open a new document and display it */
void 
gsview_display()
{
char buf[MAXSTR];
	strcpy(buf, psfile.name);
	if (get_filename(buf, FALSE, FILTER_PSALL, 0, IDS_TOPICOPEN))
		gsview_displayfile(buf);
}

/* open a new document and display it */
void
gsview_displayfile(char *filename)
{
PSFILE *tpsfile;
	tpsfile = gsview_openfile(filename);
	if (!tpsfile)
	    return;

	update_last_files(filename);

	if (pending.psfile) {
	    message_box("pending.psfile is already set", 0);
	    e_free_psfile(tpsfile);
	    return;
	}
	pending.psfile = tpsfile;
	if ( gsdll.hmodule &&  (psfile.doc==(PSDOC *)NULL) && (gsdll.state != IDLE) )
	    /* don't know where we are so close and reopen */
	    pending.abort = TRUE;
	pending.now = TRUE;
	history_add(1);
}


/* Create and open a scratch file with a given name prefix. */
/* Write the actual file name at fname. */
FILE *
gp_open_scratch_file(const char *prefix, char *fname, const char *mode)
{	char *temp;
	if ( (temp = getenv("TEMP")) == NULL )
		gs_getcwd(fname, MAXSTR);
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
#ifdef __IBMC__
	{ char *p;
	_tempnam(NULL, fname);
	strcpy(fname, p);
	free(p);
	}
#else
	mktemp(fname);
#endif
	return fopen(fname, mode);
}

/* This is triggered by WM_ACTIVATE.
 * If the file has changed when we are activated, and Auto
 * redisplay is set, cause the file to be reloaded and 
 * redisplayed
 */
void
reload_if_changed()
{
char *filename;
FILE *f;
BOOL changed = FALSE;
PSFILE temp_psfile;
	begin_crit_section();
	if (psfile.locked) {
	    end_crit_section();	/* someone else has it */
	    return;
	}
	psfile.locked = TRUE;	/* stop others using it */
	end_crit_section();

	filename = psfile_name(&psfile);

	if (filename[0] != '\0') {
	    if ( (f = fopen(filename, "rb")) != (FILE *)NULL ) {
		temp_psfile = psfile;	/* copy structure */
		temp_psfile.file = f;
		changed = psfile_changed(&temp_psfile);
		fclose(f);
	    }
	    psfile.locked = FALSE;
	}
        psfile.locked = FALSE;

        if (changed && option.redisplay)
	    gsview_command(IDM_REDISPLAY);
}


/* reopen psfile */
/* psfile will then be locked until closed */
/* return TRUE if OK */
/* if psfile time/date or length has changed, return FALSE */
/* return FALSE if file can not be opened */
BOOL
dfreopen(void)
{
char *filename;
	begin_crit_section();
	if (psfile.locked) {
	    end_crit_section();	/* someone else has it */
	    delayed_message_box(IDS_DEBUG_DFISLOCKED, 0);
	    return FALSE;
	}
	psfile.locked = TRUE;	/* stop others using it */
	end_crit_section();

	filename = psfile_name(&psfile);

	if (psfile.file) {	/* should never happen */
	    fclose(psfile.file);
	    delayed_message_box(IDS_DEBUG_DFISOPEN, 0);
	}

	if (filename[0] == '\0') {
	    psfile.locked = FALSE;
	    delayed_message_box(IDS_NOTOPEN, 0);
	    return FALSE;
	}
	if ( (psfile.file = fopen(filename, "rb")) == (FILE *)NULL ) {
	    if (debug)
	        delayed_message_box(IDS_DEBUG_DFISMISSING, 0);
	    filename[0] = '\0';
	    psfile.locked = FALSE;
	    return FALSE;
	}
	if (psfile_changed(&psfile)) {  
	    /* doesn't cope with pdf file changing */
	    dfclose();
	    if (debug)
	        delayed_message_box(IDS_DEBUG_DFCHANGED, 0);
	    return FALSE;
	}
        if (psfile.ispdf) {
	    /* We needed to open the PDF file to check for changes */
	    /* but we don't need it open for displaying. */
	    dfclose();
	}
	return TRUE;
}

void
dfclose()
{
	if (debug) {
	    if (psfile.file == (FILE *)NULL)
		delayed_message_box(IDS_DEBUG_DFISCLOSED, 0);
	}
	if (psfile.file != (FILE *)NULL)
		fclose(psfile.file);
	psfile.file = (FILE *)NULL;
	psfile.locked = FALSE;
}


/* gunzip to temporary file */
BOOL
dsc_gunzip(PSFILE *psf)
{
FILE *outfile;
gzFile *infile;
char *buffer;
int count;
    
    if (!load_zlib())
	return FALSE;

    /* create buffer for file copy */
    buffer = malloc(COPY_BUF_SIZE);
    if (buffer == (char *)NULL) {
	play_sound(SOUND_ERROR);
	unload_zlib();
	return FALSE;
    }

    if ((infile = gzopen(psf->name, "rb")) == (gzFile)NULL) {
	play_sound(SOUND_ERROR);
	unload_zlib();
	free(buffer);
	return FALSE;
    }

    if ( (outfile = gp_open_scratch_file(szScratch, psf->tname, "wb")) == (FILE *)NULL) {
	gserror(IDS_NOTEMP, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	gzclose(infile);
	unload_zlib();
	free(buffer);
	return FALSE;
    }
	
    while ( (count = gzread(infile, buffer, COPY_BUF_SIZE)) > 0 ) {
	fwrite(buffer, 1, count, outfile);
    }
    free(buffer);
    gzclose(infile);
    fclose(outfile);
    /* unload_zlib(); */
    if (count < 0)
	return FALSE;
    return TRUE;
}

/* scan file for PostScript Document Structuring Conventions */
/* return TRUE if valid DSC comments found */
BOOL
dsc_scan(PSFILE *psf)
{
char line[MAXSTR];
PSDOC *doc;
long file_length;
	if (psf->file) {
	    message_box("dsc_scan: file is open but shouldn't be", 0);
	    fclose(psf->file);
	    psf->file = NULL;
	}

	if (psf->locked) {
	    char buf[MAXSTR];
	    load_string(IDS_DEBUG_DFISLOCKED, buf, sizeof(buf));
	    message_box(buf, 0);
	    return FALSE;
	}
	psf->locked = TRUE;	/* stop others using it */
	if ( (psf->file = fopen(psf->name, "rb")) == (FILE *)NULL ) {
	    char buf[MAXSTR+MAXSTR];
	    sprintf(buf, "File '%s' does not exist", psf->name);
	    message_box(buf, 0);
	    psf->name[0] = '\0';
	    psf->locked = FALSE;
	    return FALSE;
	}

	/* these shouldn't be needed */
	if (psf->page_list.select)
	    free(psf->page_list.select);
	psf->page_list.select = NULL;
	if (psf->doc)
		psfree(psf->doc);
	psf->preview = 0;
	
	/* get first line to look for magic numbers */
	fgets(line, sizeof(line)-1, psf->file);
        rewind(psf->file);

	/* check for gzip */
	psf->gzip = FALSE;
	if ( (line[0]=='\037') && (line[1]=='\213') ) { /* 1F 8B */
	    psf->gzip = TRUE;
	    fclose(psf->file);
	    psf->file = NULL;
	    if (!dsc_gunzip(psf)) {
/* ENGLISH */
		message_box("Failed to gunzip file", 0);
		psf->name[0] = '\0';
		psf->locked = FALSE;
		return FALSE;
	    }
	    if ( (psf->file = fopen(psfile_name(psf), "rb")) == (FILE *)NULL ) {
		char buf[MAXSTR+MAXSTR];
		sprintf(buf, "File '%s' does not exist", psfile_name(psf));
		message_box(buf, 0);
		psf->name[0] = '\0';
		psf->locked = FALSE;
		return FALSE;
	    }
	    fgets(line, sizeof(line)-1, psf->file);
            rewind(psf->file);
	}

	/* save file date and length */
	psfile_savestat(psf);

	/* check for PDF */
	psf->ispdf = FALSE;
	if ( strncmp("%PDF-", line, 5) == 0 ) {
	    fclose(psf->file);
	    psf->locked = FALSE;
	    psf->file = NULL;
	    psf->ispdf = TRUE;
	    return FALSE;	/* we don't know how many pages yet */
	}


	/* check for documents that start with Ctrl-D */
	psf->ctrld = (line[0] == '\004');
	/* check for HP LaserJet prologue */
	psf->pjl = FALSE;
	if (strncmp("\033%-12345X", line, 9) == 0)
	    psf->pjl = TRUE;
	if (option.ignore_dsc)
	    psf->doc = (PSDOC *)NULL;
	else 
	    psf->doc = psscan(psf->file);
	fseek(psf->file, 0, SEEK_END);
	file_length = ftell(psf->file);
	fclose(psf->file);
	psf->file = NULL;
	psf->locked = FALSE;
	/* check for DSC comments */
	doc = psf->doc;
	if (doc == (PSDOC *)NULL)
	    return FALSE;
	if (doc->doseps) {
	    BOOL bad_header = FALSE;
	    /* check what sort of preview is present */
	    if (doc->doseps->tiff_begin)
		psf->preview = IDS_EPST;
	    if (doc->doseps->mf_begin)
		psf->preview = IDS_EPSW;
	    /* check for errors in header */
	    if (doc->doseps->ps_begin > file_length)
		bad_header = TRUE;
	    if (doc->doseps->ps_begin + doc->doseps->ps_length > file_length)
		bad_header = TRUE;
	    if (doc->doseps->mf_begin > file_length)
		bad_header = TRUE;
	    if (doc->doseps->mf_begin + doc->doseps->mf_length > file_length)
		bad_header = TRUE;
	    if (doc->doseps->tiff_begin > file_length)
		bad_header = TRUE;
	    if (doc->doseps->tiff_begin + doc->doseps->tiff_length > file_length)
		bad_header = TRUE;
	    if (bad_header) {
		char buf[MAXSTR];
	        load_string(IDS_BAD_DOSEPS_HEADER, buf, sizeof(buf));
	        message_box(buf, 0);
		/* Ignore the bad information */
		psfree(psf->doc);
		psf->doc = (PSDOC *)NULL;
		return FALSE;
	    }
	}
	if (!psf->preview && (doc->beginpreview != doc->endpreview))
	    psf->preview = IDS_EPSI;
	if (doc->numpages) {
	    int i;
	    char *label;
	    for (i=0; i<doc->numpages; i++) {
		if ( (label = doc->pages[i].label) != NULL)  {
		    if (strlen(label)==0) {	/* remove old empty label */
			free(label);
			sprintf(line, "%d", i+1);
			label = malloc(strlen(line)+1);
			if (label)
			    strcpy(label, line);
			doc->pages[i].label = label;
		    }
		}
	    }
	    psf->page_list.select = (BOOL *)malloc( doc->numpages * sizeof(BOOL) );
	    if (psf->page_list.select)
	        memset(psf->page_list.select, 0, doc->numpages * sizeof(BOOL));
	}
	if (doc->epsf) {
	    /* warn if bounding box off the page */
	    int i = get_paper_size_index();
	    int width, height;
	    if (i < 0) {
	        width = option.user_width;
	        height = option.user_height;
	    }
	    else {
	        width = papersizes[i].width;
	        height = papersizes[i].height;
	    }
	    if ( !option.epsf_clip &&
	        ((doc->boundingbox[LLX] > width) || 
		 (doc->boundingbox[LLY] > height) ||
	         (doc->boundingbox[URX] < 0) || 
		 (doc->boundingbox[URY] < 0)) )
	    {
		char buf[MAXSTR];
	        load_string(IDS_EPS_OFF_PAGE, buf, sizeof(buf));
	        message_box(buf, 0);
	    }
	    if ( ((doc->boundingbox[LLX] > doc->boundingbox[URX]) || 
		 (doc->boundingbox[LLY] > doc->boundingbox[URY]))
		&& option.epsf_clip)
	    {
		char buf[MAXSTR];
	        load_string(IDS_EPS_BAD_BBOX, buf, sizeof(buf));
	        message_box(buf, 0);
	    }
	}
	return TRUE;
}



/* reverse zero based page number if needed */
int
map_page(int page)
{
    if (psfile.doc != (PSDOC *)NULL)
        if (psfile.doc->pageorder == DESCEND) 
	    return (psfile.doc->numpages - 1) - page;
    return page;
}

void
psfile_free(PSFILE *psf)
{
    if (psf == (PSFILE *)NULL)
	return;

    psf->name[0] = '\0';

    /* same as dfclose() */
    if (psf->file != (FILE *)NULL)
	fclose(psf->file);
    psf->file = (FILE *)NULL;
    psf->locked = FALSE;

    if ((psf->tname[0] != '\0') && (!debug))
	unlink(psf->tname);
    psf->tname[0] = '\0';


    if (psf->page_list.select)
	free(psf->page_list.select);
    psf->page_list.select = NULL;
    if (psf->doc)
	psfree(psf->doc);
    psf->doc = (PSDOC *)NULL;
    if (psf->text_name) {
	if (!debug)
	    unlink(psf->text_name);
        psf->text_name[0] = '\0';
	free_text_index();
    }
}

char *
psfile_name(PSFILE *psf)
{
    /* if original file was gzipped, give name of gunzipped file */
    if ((psf->tname[0]!='\0') && (psf->gzip))
	return psf->tname;
    /* otherwise return original file name */
    return psf->name;
}

#ifdef UNUSED
void history_debug(void)
{
char buf[256];
int i;
    gs_addmess("history_debug: ");
    for (i=0; i<history.count; i++) {
	if (i == history.index)
	    gs_addmess("[");
        sprintf(buf, "%d", history.pages[i]);
	gs_addmess(buf);
	if (i == history.index)
	    gs_addmess("]");
	gs_addmess(" ");
    }
    gs_addmess("\n");
}
#endif

void
history_add(int pagenum)
{
    /* scroll if history full */
    if (history.index >= HISTORY_MAX) {
	memmove(&history.pages[0], &history.pages[1], 
 	    (HISTORY_MAX-1)*sizeof(history.pages[0]));
	history.index--;
	history.count = history.index;
    }

    if ((history.index > 1) && (history.pages[history.index -1] == pagenum)) {
	/* Don't insert duplicate page */
	return;
    }
    if (history.pages[history.index] != pagenum) {
	/* If we are following a different path to last time,
	 * truncate history
	 */ 
	history.count = history.index;
    }

    history.pages[history.index] = pagenum;

    history.index++;
    if (history.index > history.count)
	history.count = history.index;
}


void
history_reset(void)
{
    history.index = history.count = 0;
}

void
history_back(void)
{
    if (history.index < 2)
	return;	/* can't do anything */

    history.index--;	/* point to current page */

    request_mutex();
    pending.pagenum = history.pages[history.index - 1];
    if (pending.pagenum > (int)psfile.doc->numpages)
	 pending.pagenum = psfile.doc->numpages;
    if (pending.pagenum < 1)
	pending.pagenum = 1;
    gsview_unzoom();
    pending.now = TRUE;
    release_mutex();
}

void
history_forward(void)
{
    if (history.index >= history.count) {
	gs_page_skip(1);
	return;
    }

    request_mutex();
    pending.pagenum = history.pages[history.index];
    if (pending.pagenum > (int)psfile.doc->numpages)
	 pending.pagenum = psfile.doc->numpages;
    if (pending.pagenum < 1)
	pending.pagenum = 1;
    gsview_unzoom();
    pending.now = TRUE;
    release_mutex();

    history.index++;
}
