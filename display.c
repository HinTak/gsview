/*
 * display.c -- Ghostscript display operations for GSVIEW.EXE, 
 *              a graphical interface for MS-Windows Ghostscript
 * Copyright (C) 1993  Russell Lang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Author: Russell Lang
 * Internet: rjl@monu1.cc.monash.edu.au
 */

#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dir.h>
#include <io.h>
#define NeedFunctionPrototypes 1
#include "ps.h"
#include "gsview.h"

struct ftime dftime;	/* time/date of selected file */
long dflength;		/* length of selected file */

/* get current media index to papersizes[], or -1 if no match */
int
get_papersizes_index()
{
int i;
char medianame[20];
	GetMenuString(hmenu, media, medianame, sizeof(medianame), MF_BYCOMMAND);
	for (i=0; papersizes[i].name != (char *)NULL; i++) {
	    if (!stricmp(papersizes[i].name, medianame))
		return i;
	}
	return -1;
}

/* calculate bitmap size for gswin */
void
gswin_size()
{
int i = get_papersizes_index();
	if ( (xdpi == 0.0) || (ydpi == 0.0) )
		xdpi = ydpi = DEFAULT_RESOLUTION;
	epsf_clipped = FALSE;
	switch (orientation) {
	    case IDM_LANDSCAPE:
	    case IDM_SEASCAPE:
		if (i < 0) {
		    bitmap_width = user_height;
		    bitmap_height = user_width;
		}
		else {
		    bitmap_width = papersizes[i].height;
		    bitmap_height = papersizes[i].width;
		}
		break;
	    default:
		if ((doc != (struct document *)NULL) && doc->epsf
		    && epsf_clip) {
		    epsf_clipped = TRUE;
		    bitmap_width = doc->boundingbox[URX] - doc->boundingbox[LLX];
		    bitmap_height = doc->boundingbox[URY] - doc->boundingbox[LLY];
		}
		else if (i < 0) {
		    bitmap_width = user_width;
		    bitmap_height = user_height;
		}
		else {
		    bitmap_width = papersizes[i].width;
		    bitmap_height = papersizes[i].height;
		}
	}
	bitmap_width  = (unsigned int)(bitmap_width  / 72.0 * xdpi);
	bitmap_height = (unsigned int)(bitmap_height / 72.0 * ydpi);
}

/* change the size of the gswin image if open */
void
gswin_resize()
{
BOOL display = FALSE;
BOOL opened_dfile = FALSE;
	gswin_size();
	if (gswin_hinst == (HINSTANCE)NULL)
	    return;
	if ( (dfile == (FILE *)NULL) && (doc != (struct document *)NULL) ) {
	    dfreopen();
	    opened_dfile = TRUE;
	}
	if (redisplay && page_ready && (doc != (struct document *)NULL))
	    display = TRUE;	/* redisplay page after resize */
	gsview_endfile();
	if (gswin_hinst != (HINSTANCE)NULL) {
	    fprintf(cfile,"mark /HWSize [%u %u]\r\n",bitmap_width,bitmap_height);
	    fprintf(cfile,"/HWResolution [%g %g]\r\n",xdpi,ydpi);
	    fprintf(cfile,"currentdevice putdeviceprops pop erasepage flushpage\r\n");
	    pipeflush();
	}
	if (display) {
	    if (gswin_hinst != (HINSTANCE)NULL)
	        gswin_open();	/* we need it open to redisplay */
   	    fix_orientation(cfile);
   	    dsc_header(cfile);
	    dsc_getpages(cfile,pagenum,pagenum);
	    pipeflush();
	}

	if (opened_dfile)
	    dfclose();
}

void
gsview_orientation(int new_orientation)
{
	if (new_orientation == orientation)
		return;
	if (new_orientation == IDM_SWAPLANDSCAPE) {
	    swap_landscape = !swap_landscape;
	    if (swap_landscape) 
	        CheckMenuItem(hmenu, IDM_SWAPLANDSCAPE, MF_BYCOMMAND | MF_CHECKED);
	    else
	        CheckMenuItem(hmenu, IDM_SWAPLANDSCAPE, MF_BYCOMMAND | MF_UNCHECKED);
	    if ((orientation != IDM_LANDSCAPE) && (orientation != IDM_SEASCAPE))
	        return;
	}
	else {
	    CheckMenuItem(hmenu, orientation, MF_BYCOMMAND | MF_UNCHECKED);
	    orientation = new_orientation;
	    CheckMenuItem(hmenu, orientation, MF_BYCOMMAND | MF_CHECKED);
	}
	gswin_resize();
	return;
}

void
gsview_media(int new_media)
{
	if ( (new_media == media) && (new_media != IDM_USERSIZE) )
		return;
	CheckMenuItem(hmenu, media, MF_BYCOMMAND | MF_UNCHECKED);
	media = new_media;
	CheckMenuItem(hmenu, media, MF_BYCOMMAND | MF_CHECKED);
	gswin_resize();
	return;
}

/* run Ghostscript for previewing document */
/* return TRUE if ok, FALSE if error */
BOOL
gswin_open()
{
char command[256];
	/* return if already open */
	if ((gswin_hinst != (HINSTANCE)NULL) && IsWindow(hwndimgchild))
		return TRUE;

	pipeinit();		/* so we wait for first request */
	gswin_size();
	sprintf(command,"%s -r%gx%g -g%ux%u -sGSVIEW=%u -",
		szGSwin, xdpi, ydpi, bitmap_width, bitmap_height,
		(unsigned int)hwndimg);
	if (strlen(command) > 126) {
		info_wait(FALSE);
		gserror(IDS_TOOLONG, command, MB_ICONSTOP, SOUND_ERROR);
		gswin_hinst = (HINSTANCE)NULL;
		return FALSE;
	}
	gswin_hinst = (HINSTANCE)WinExec(command, SW_SHOWMINNOACTIVE);

	if (gswin_hinst < HINSTANCE_ERROR) {
		info_wait(FALSE);
		gserror(IDS_CANNOTRUN, command, MB_ICONSTOP, SOUND_ERROR);
		gswin_hinst = (HINSTANCE)NULL;
		return FALSE;
	}
	if (hwndtext == (HWND)NULL) {
		/* we are running an incompatible version of Ghostscript */
		hwndtext = FindWindow("BCEasyWin","Ghostscript");
		if (hwndtext) {
		    SendMessage(hwndtext, WM_CHAR, 'q', 1L);
		    SendMessage(hwndtext, WM_CHAR, 'u', 1L);
		    SendMessage(hwndtext, WM_CHAR, 'i', 1L);
		    SendMessage(hwndtext, WM_CHAR, 't', 1L);
		    SendMessage(hwndtext, WM_CHAR, '\r', 1L);
		}
		hwndtext = (HWND)NULL;
		hwndimgchild = (HWND)NULL;
		gswin_hinst = (HINSTANCE)NULL;
		clear_timer();
		info_wait(FALSE);
		gserror(IDS_WRONGGS, NULL, MB_ICONSTOP, SOUND_ERROR);
		return FALSE;
	}
	saved = FALSE;

	/* wait for gswin to initialise */
	if (set_timer(CLOSE_TIMEOUT))
	    EnableWindow(hwndimg, FALSE);
	while (!is_pipe_done()&&  !bTimeout)
		do_message();	/* wait for pipe data request from gswin */
	clear_timer();
	EnableWindow(hwndimg, TRUE);

	cfile = pipeopen();	/* open pipe to gswin */
	BringWindowToTop(hwndimg);
	SetFocus(hwndimg);	/* kludge: without this desktop gets focus */
	return TRUE;
}

/* close Ghostscript */
BOOL
gswin_close()
{
BOOL force = FALSE;
	if (gswin_hinst == (HINSTANCE)NULL)
	    return TRUE;

	if (doc == (struct document*)NULL) {
	    /* we don't know how many pages remain so we must force an exit */
	    if (!is_pipe_done())
		force = TRUE;
	}
	else {
	    if (page_ready)
		next_page();
	}

	if (!force) {
	    /* try to close Ghostscript cleanly */
	    pipeclose();
	    if (set_timer(CLOSE_TIMEOUT))
		EnableWindow(hwndimg, FALSE);
	    while (GetModuleUsage(gswin_hinst) &&  !bTimeout)
		do_message();	/* wait for gswin to close */
	    clear_timer();
	    EnableWindow(hwndimg, TRUE);
	}
	do_message();

	/* if still there try killing it a using a brute force method */
	if (IsWindow(hwndtext)) {
	    if (is_win31) {
	        SendMessage(hwndtext, WM_CLOSE, 0, 0L);
	        if (IsWindow(hwndtext))
		    SendMessage(hwndtext, WM_CLOSE, 0, 0L);
	    }
	    else {
	        /* Windows 3.0 hangs if we use SendMessage */
	        PostMessage(hwndtext, WM_CLOSE, 0, 0L);
	        do_message();
	    }
	}

	do_message();
	gswin_hinst = (HINSTANCE)NULL;
	hwndimgchild = (HWND)NULL;
	hwndtext = (HWND)NULL;
	bitmap_scrollx = bitmap_scrolly = 0;
	page_ready = FALSE;
	saved = FALSE;
	pipeclose();
	return TRUE;
}

/* send a NEXT_PAGE message to Ghostscript */
void
next_page()
{
int i;
	if (hwndimgchild && IsWindow(hwndimgchild)) {
		SendMessage(hwndimgchild, WM_GSVIEW, NEXT_PAGE, 0L);
		page_ready = FALSE;
	}
	do_message();	/* wait for Ghostscript to process message */
	for (i=0; i<32; i++) {
	   /* Wait a bit for pipe contents after showpage to be read */
	   do_message();
	   if (is_pipe_done())
		break;
	}
}

/* handle messages while we are waiting */
void
do_message()
{
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
	if ((hDlgModeless == 0) || !IsDialogMessage(hDlgModeless, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}


/* end of file - get ready for new file */
void
gsview_endfile()
{
	info_wait(TRUE);
	if (gswin_hinst == (HINSTANCE)NULL)
	    return;
	if (!quick ||
             ((doc == (struct document *)NULL) && !is_pipe_done())) {
		gswin_close();
		return;
	}

	if (page_ready)
	    next_page();

	if ((saved) && (doc != (struct document *)NULL) && (doc->pages)) {
	    /* send trailer if needed */
	    pscopy(dfile, cfile, doc->begintrailer, doc->endtrailer);
	}
	if (saved) {
	    /* restore interpreter state */
	    fputs("gsview_cleanup\r\n",cfile);
	    fputs("gsview_save restore\r\n",cfile);
	}
	else
	    fputs("clear cleardictstack\r\n",cfile);
	pipeflush();
	saved = FALSE;
}

/* open a new document */
void
gsview_openfile(char *filename)
{
int i;
	pagenum = 1;
	if (dsc_scan(filename)) {
	    /* found DSC comments */
	    if (doc->orientation == PORTRAIT)
		gsview_orientation(IDM_PORTRAIT);
	    if (doc->orientation == LANDSCAPE)
		gsview_orientation(IDM_LANDSCAPE);
	    if (doc->default_page_media) {
		char medianame[20];
		for (i=IDM_LETTER; i<IDM_USERSIZE; i++) {
		    GetMenuString(hmenu, i, medianame, sizeof(medianame), MF_BYCOMMAND);
		    if (!stricmp(medianame, doc->default_page_media->name)) {
		        gsview_media(i);
		        break;
		    }
		}
		if (i == IDM_USERSIZE) {
		    gsview_media(IDM_USERSIZE);
		    user_width  = doc->default_page_media->width;
		    user_height = doc->default_page_media->height;
		}
	    }
	}
}


/* get filename then open new file for printing or extract */
void 
gsview_select()
{
	LoadString(phInstance, IDS_TOPICOPEN, szHelpTopic, sizeof(szHelpTopic));
	if (GetOpenFileName(&ofn))
		gsview_selectfile(szOFilename);
}

/* open new file for printing or extract */
void
gsview_selectfile(char *filename)
{
	if (gswin_hinst != (HINSTANCE)NULL)
	    gsview_endfile();
	gsview_openfile(filename);
	info_wait(FALSE);
}

/* get filename then open a new document and display it */
void 
gsview_display()
{
	LoadString(phInstance, IDS_TOPICOPEN, szHelpTopic, sizeof(szHelpTopic));
	if (GetOpenFileName(&ofn))
		gsview_displayfile(szOFilename);
}

/* open a new document and display it */
void
gsview_displayfile(char *filename)
{
char *p;
	gsview_endfile();

	info_wait(TRUE);

	gsview_openfile(filename);
	if (epsf_clipped ||
	     ((doc != (struct document *)NULL) && doc->epsf && epsf_clip))
	    gswin_resize();

	if (!gswin_open()) {
	    MessageBox(hwndimg, "panic gsview_displayfile", szAppName, MB_OK);
	}

	fix_orientation(cfile);
	if (doc != (struct document *)NULL) {
	    /* found DSC comments */
	    dsc_header(cfile);
	    dsc_getpages(cfile,pagenum,pagenum);
	}
	else {
	    /* non conformant file - send unmodified */
	    fputs("(Displaying ",cfile);
	    for (p=filename; *p; p++) {
		if (*p != '\\')
			fputc(*p,cfile);
		else
			fputc('/',cfile);
	    }
	    fputs("\\n) print flush\r\n",cfile);
	    fputc('(',cfile);
	    for (p=filename; *p; p++) {
		if (*p != '\\')
			fputc(*p,cfile);
		else
			fputc('/',cfile);
	    }
	    fputs(") run flushpage\r\n",cfile);
	}
	
	pipeflush();
}


void
send_prolog(FILE *f, char *resource)
{  
HGLOBAL hglobal;
LPSTR prolog;
	hglobal = LoadResource(phInstance, FindResource(phInstance, resource, RT_RCDATA));
	if ( (prolog = (LPSTR)LockResource(hglobal)) != (LPSTR)NULL) {
	    while (*prolog)
	        fputc(*prolog++, f);
	    FreeResource(hglobal);
	}
}


/* add Ghostscript code to change orientation */
void
fix_orientation(FILE *f)
{
int real_orientation;
	/* save interpreter state */
	fputs("clear cleardictstack save /gsview_save exch def\r\n",f);
	saved = TRUE;
	/* provide epsf offset */
	if (epsf_clipped)
	    fprintf(f,"/gsview_offset {%d %d translate} def\r\n",
	        -doc->boundingbox[LLX], -doc->boundingbox[LLY]);
	else
	    fprintf(f,"/gsview_offset {} def\r\n");
	real_orientation = orientation;
	if (swap_landscape) {
	    if (orientation == IDM_LANDSCAPE)
		real_orientation = IDM_SEASCAPE;
	    else if (orientation == IDM_SEASCAPE)
		real_orientation = IDM_LANDSCAPE;
	}
	fprintf(f,"/gsview_landscape  %s def\r\n",
	    real_orientation == IDM_LANDSCAPE ? "true" : "false");
	fprintf(f,"/gsview_upsidedown %s def\r\n",
	    real_orientation ==  IDM_UPSIDEDOWN ? "true" : "false");
	fprintf(f,"/gsview_seascape   %s def\r\n",
	    real_orientation == IDM_SEASCAPE ? "true" : "false");
	send_prolog(f, "gsview_orientation");
	if (epsf_warn)
	    send_prolog(f, "gsview_epswarn");
}

/* Create and open a scratch file with a given name prefix. */
/* Write the actual file name at fname. */
FILE *
gp_open_scratch_file(const char *prefix, char *fname, const char *mode)
{	char *temp;
	if ( (temp = getenv("TEMP")) == NULL )
		*fname = 0;
	else
	{	strcpy(fname, temp);
		/* Prevent X's in path from being converted by mktemp. */
		for ( temp = fname; *temp; temp++ )
			*temp = tolower(*temp);
		if ( strlen(fname) && (fname[strlen(fname)-1] != '\\') )
			strcat(fname, "\\");
	}
	strcat(fname, prefix);
	strcat(fname, "XXXXXX");
	mktemp(fname);
	return fopen(fname, mode);
}

/* reopen dfile */
/* if dfile time/date or length has changed, kill gswin and rescan the file */
BOOL
dfreopen()
{
struct ftime thisftime;
long thisflength;
	if (doc == (struct document *)NULL)
		return TRUE;
	dfclose();
	if (dfname[0] == '\0')
		return TRUE;
	if ( (dfile = fopen(efname[0] ? efname : dfname, "rb")) 
	        == (FILE *)NULL ) {
	    if (debug)
	        MessageBox(hwndimg, "file missing", "dfreopen", MB_OK);
	    dfname[0] = '\0';
	    return FALSE;
	}
	getftime(fileno(dfile), &thisftime);
	thisflength = filelength(fileno(dfile));
	if ( (thisflength != dflength) ||
		memcmp(&thisftime, &dftime, sizeof(thisftime)) ) {
	    if (debug)
	        MessageBox(hwndimg, "file changed", "dfreopen", MB_OK);
	    /* file may have changed beyond recognition so we must kill gswin */
	    gswin_close();
	    if (dsc_scan(dfname))
	        if ( (dfile = fopen(efname[0] ? efname : dfname, "rb")) 
	            == (FILE *)NULL ) {
		        dfname[0] = '\0';
		        return FALSE;
	        }
	}
	return TRUE;
}

void
dfclose()
{
	if (dfile != (FILE *)NULL)
		fclose(dfile);
	dfile = (FILE *)NULL;
}

/* scan file for PostScript Document Structuring Conventions */
/* return TRUE if valid DSC comments found */
BOOL
dsc_scan(char *filename)
{
unsigned char eps[4];
	strcpy(dfname, filename);
	dfclose();
	if ((efname[0] != '\0') && !debug)
		unlink(efname);
	efname[0] = '\0';
	if ( (dfile = fopen(dfname, "rb")) == (FILE *)NULL ) {
		dfname[0] = '\0';
		return FALSE;
	}
	getftime(fileno(dfile), &dftime);
	dflength = filelength(fileno(dfile));
	if (page_list.select)
		free(page_list.select);
	page_list.select = NULL;
	if (doc)
		psfree(doc);
	is_ctrld = FALSE;
	fread(eps, 1, 4, dfile);
	if ((eps[0]==0xc5) && (eps[1]==0xd0) && (eps[2]==0xd3) && (eps[3]==0xc6))
	    extract_eps();
	else
	    preview = 0;
	doc = psscan(dfile);
	if (doc == (struct document *)NULL) {
	    dfclose();
	    return FALSE;
	}
	if (eps[0] == '\004')
	    is_ctrld = TRUE;
	if (!preview && doc->beginpreview)
	    preview = IDS_EPSI;
	page_list.select = (BOOL *)malloc( doc->numpages * sizeof(BOOL) );
	return TRUE;
}



/* Copy specified pages from dfile to file f */
void
dsc_getpages(FILE *f, int first, int last)
{
int i, page;
	for (i=first-1; i<last; i++) {
	    page = map_page(i);
	    if (doc->pages) {
	        fprintf(f,"(Page: %s %d\\n) print flush\r\n", doc->pages[page].label ? doc->pages[page].label : " ", page+1);
		pscopy(dfile, f, doc->pages[page].begin, doc->pages[page].end);
	    }
	    else {
	        fprintf(f,"(Page: %d\\n) print flush\r\n",page); 
		pscopy(dfile, f, doc->endsetup, doc->endtrailer);
	    }
	}
}


/* Copy dsc header to file f */
void
dsc_header(FILE *f)
{
char *p;
	fputs("(Displaying ",f);
	for (p=dfname; *p; p++) {
	    if (*p != '\\')
		fputc(*p,f);
	    else
		fputc('/',f);
	}
	fputs("\\n) print flush\r\n",f);
	pscopy(dfile, f, doc->beginheader, doc->endheader);
	pscopy(dfile, f, doc->begindefaults, doc->enddefaults);
	pscopy(dfile, f, doc->beginprolog, doc->endprolog);
	pscopy(dfile, f, doc->beginsetup, doc->endsetup);
}


/* Send commands to gswin to display page */
void
dsc_dopage(void)
{
	info_wait(TRUE);
	if (!saved) {
   	    fix_orientation(cfile);
   	    dsc_header(cfile);
	}
	dsc_getpages(cfile,pagenum,pagenum);
	pipeflush();
}

/* go forward skip pages */
void
dsc_next(int skip)
{
	if (pagenum == doc->numpages || doc->numpages == 0) {
	    play_sound(SOUND_NOPAGE);
	    info_wait(FALSE);
	    return;
	}
	pagenum += skip;
	if (pagenum > doc->numpages)
	     pagenum = doc->numpages;
	info_wait(TRUE);
	if (page_ready)
	    next_page();
	if (gswin_open())
	    dsc_dopage();
}

/* go back skip pages */
void
dsc_prev(int skip)
{
	if (pagenum == 1 || doc->numpages == 0) {
		play_sound(SOUND_NOPAGE);
		return;
	}
	pagenum -= skip;
	if (pagenum < 1)
	    pagenum = 1;
	info_wait(TRUE);
	if (page_ready)
	    next_page();
	if (gswin_open())
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
