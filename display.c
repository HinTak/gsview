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
#define NeedFunctionPrototypes 1
#include "ps.h"
#include "gsview.h"

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
gswin_resize(void)
{
BOOL cfile_was_not_open = (cfile == (FILE *)NULL);
BOOL display = FALSE;
	gswin_size();
	if (gswin_hinst == (HINSTANCE)NULL)
	    return;
	if (cfile_was_not_open && redisplay 
	    && page_ready && (doc != (struct document *)NULL))
	        display = TRUE;	/* redisplay page after resize */
	gsview_endfile();
	if (cfile == (FILE *)NULL)
	    if (!open_cfile())
	        return;
	if (gswin_hinst != (HINSTANCE)NULL) {
	    fprintf(cfile,"mark /HWSize [%u %u]\r\n",bitmap_width,bitmap_height);
	    fprintf(cfile,"/HWResolution [%g %g]\r\n",xdpi,ydpi);
	    fprintf(cfile,"currentdevice putdeviceprops pop erasepage flushpage\r\n");
	}
	if (display) {
   	    fix_orientation(cfile);
   	    dsc_header(cfile);
	    dsc_getpages(cfile,pagenum,pagenum);
	}
	if (cfile_was_not_open) {
	    close_cfile();
	    if (gswin_hinst != (HINSTANCE)NULL) {
	        set_timer(timeout);
	        pipe_file(cfname);
	    }
	}
	else {
	    if (cfile == (FILE *)NULL)
	        open_cfile();
	}
}


/* run Ghostscript for previewing document */
/* return 0 if ok, 1 if error */
int
gswin_open()
{
char command[256];
	/* return if already open */
	if ((gswin_hinst != (HINSTANCE)NULL) && IsWindow(hwndimgchild))
		return 0;

	bPipeDone = FALSE;	/* so we wait for first request */
	gswin_size();
	sprintf(command,"%s -r%gx%g -g%ux%u -sGSVIEW=%u -",
		szGSwin, xdpi, ydpi, bitmap_width, bitmap_height,
		(unsigned int)hwndimg);
	if (strlen(command) > 126) {
		info_wait(FALSE);
		gserror(IDS_TOOLONG, command, MB_ICONSTOP, SOUND_ERROR);
		gswin_hinst = (HINSTANCE)NULL;
		return 1;
	}
	gswin_hinst = (HINSTANCE)WinExec(command, SW_SHOWMINNOACTIVE);

	if (gswin_hinst < HINSTANCE_ERROR) {
		info_wait(FALSE);
		gserror(IDS_CANNOTRUN, command, MB_ICONSTOP, SOUND_ERROR);
		gswin_hinst = (HINSTANCE)NULL;
		return 1;
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
		pipe_clean();
		info_wait(FALSE);
		gserror(IDS_WRONGGS, NULL, MB_ICONSTOP, SOUND_ERROR);
		return 1;
	}
	saved = FALSE;
	if (set_timer(CLOSE_TIMEOUT))
	    EnableWindow(hwndimg, FALSE);
	while (!bPipeDone &&  !bTimeout)
		do_message();	/* wait for pipe data request from gswin */
	clear_timer();
	EnableWindow(hwndimg, TRUE);
	BringWindowToTop(hwndimg);
	SetFocus(hwndimg);	/* kludge: without this desktop gets focus */
	return 0;
}

/* close Ghostscript */
int
gswin_close()
{
BOOL force = FALSE;
	if (gswin_hinst == (HINSTANCE)NULL)
	    return 0;

	if (doc == (struct document*)NULL) {
	    /* we don't know how many pages remain so we must force an exit */
	    if (!bPipeDone)
		force = TRUE;
	}
	else {
	    if (page_ready)
		next_page();
	}

	if (!force) {
	    /* try to close Ghostscript cleanly */
	    pipe_close();
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
	pipe_clean();
	return 0;
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
	   if (bPipeDone)
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
/* cfile is opened if needed, but not closed */
void
gsview_endfile()
{
	info_wait(TRUE);
	if (gswin_hinst == (HINSTANCE)NULL)
	    return;
	if (!quick ||
             ((doc == (struct document *)NULL) && !bPipeDone)) {
		gswin_close();
		return;
	}

	if (page_ready)
	    next_page();

	if (cfile == (FILE *)NULL)
	    if (!open_cfile())
	        return;

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
	    is_ctrld = FALSE;
	    rewind(dfile);
	    if (fgetc(dfile) == '\004')
		is_ctrld = TRUE;
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
	if (gswin_hinst != (HINSTANCE)NULL) {
	    gsview_endfile();
	    if (cfile != (FILE *)NULL) {
	        close_cfile();
	        set_timer(timeout);
	        pipe_file(cfname);
	    }
	}
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

	if (cfile == (FILE *)NULL)
	    if (!open_cfile())
	        return;

	info_wait(TRUE);

	gsview_openfile(filename);
	if (epsf_clipped ||
	     ((doc != (struct document *)NULL) && doc->epsf && epsf_clip))
	    gswin_resize();

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
	close_cfile();

	if (gswin_open())
	    return;
	set_timer(timeout);
	saved = TRUE;	/* because gswin_open() killed it */
	pipe_file(cfname);
}



/* add Ghostscript code to change orientation */
void
fix_orientation(FILE *f)
{
int real_orientation;
	/* save interpreter state */
	fputs("clear cleardictstack save /gsview_save exch def\r\n",f);
	saved = TRUE;
	/* define this orientation */
	fputs("/gsview_orientation {\r\n",f);
	if (epsf_clipped) {
	    /* provide epsf offset */
	    fprintf(f," %d %d translate\r\n",
	        -doc->boundingbox[LLX], -doc->boundingbox[LLY]);
	}
	fputs(" gsave clippath pathbbox grestore\r\n",f);
	fputs(" 4 dict begin\r\n",f);
	fputs(" /ury exch def /urx exch def /lly exch def /llx exch def\r\n",f);
	real_orientation = orientation;
	if (swap_landscape) {
	    if (orientation == IDM_LANDSCAPE)
		real_orientation = IDM_SEASCAPE;
	    else if (orientation == IDM_SEASCAPE)
		real_orientation = IDM_LANDSCAPE;
	}
	switch(real_orientation) {
	    case IDM_PORTRAIT:
		break;
	    case IDM_LANDSCAPE:
		fputs(" -90 rotate\r\n llx ury add neg llx lly sub translate\r\n",f);
		break;
	    case IDM_UPSIDEDOWN:
		fputs(" 180 rotate\r\n llx urx add neg lly ury add neg translate\r\n",f);
		break;
	    case IDM_SEASCAPE:
		fputs("  90 rotate\r\n lly llx sub lly urx add neg translate\r\n",f);
		break;
	  }
	fputs("  end\r\n} def\r\n",f);
	/* redefine showpage */
	fputs("/showpage\r\n{1 true .outputpage\r\n",f);
	fputs("erasepage initgraphics\r\ngsview_orientation\r\n} bind def\r\n",f);
	/* do the transformation now */
	fputs("gsview_orientation\r\n",f);
	fputs("/gsview_cleanup {clear cleardictstack} def\r\n",cfile);
	if (epsf_warn)
	    epsf_warnprolog(f);
}

/* open temporary command file */
BOOL
open_cfile()
{
	if (cfile != (FILE *)NULL) {
		close_cfile();
		gserror(IDS_CFILEOPEN, NULL, NULL, SOUND_ERROR);
	}
	/* remove old file */
	if ((cfname[0] != '\0') & !debug)
		unlink(cfname);
	cfname[0] = '\0';
	/* get new scratch file */
	if ( (cfile = gp_open_scratch_file(szScratch, cfname, "wb")) == (FILE *)NULL) {
		info_wait(FALSE);
		gserror(IDS_CFILEERR, NULL, MB_ICONSTOP, SOUND_ERROR);
		return FALSE;
	}
	return TRUE;
}

void
close_cfile()
{
	if (cfile)
	    fclose(cfile);
	cfile = (FILE *)NULL;
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

/* scan file for PostScript Document Structuring Conventions */
/* return TRUE if valid DSC comments found */
BOOL
dsc_scan(char *filename)
{
unsigned char eps[4];
	strcpy(dfname, filename);
	if (dfile != (FILE *)NULL)
		fclose(dfile);
	if ( (dfile = fopen(dfname, "rb")) == (FILE *)NULL ) {
		dfname[0] = '\0';
		return FALSE;
	}
	if (page_list.select)
		free(page_list.select);
	page_list.select = NULL;
	if (doc)
		psfree(doc);
	fread(eps, 1, 4, dfile);
	if ((eps[0]==0xc5) && (eps[1]==0xd0) && (eps[2]==0xd3) && (eps[3]==0xc6))
	    extract_eps();
	else
	    preview = 0;
	doc = psscan(dfile);
	if (doc == (struct document *)NULL) {
		fclose(dfile);
		dfile = (FILE *)NULL;
		return FALSE;
	}
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
	if (!open_cfile())
	    return;
	if (!saved) {
   	    fix_orientation(cfile);
   	    dsc_header(cfile);
	}
	dsc_getpages(cfile,pagenum,pagenum);
	close_cfile();
	set_timer(timeout);
	pipe_file(cfname);
}

/* reverse zero based page number if needed */
int
map_page(int page)
{
    	if (doc->pageorder == DESCEND) 
		return (doc->numpages - 1) - page;
	return page;
}

/* imitation pipe */

/* Data is passed in a global shareable memory block.
 * The global handle is passed in the LOWORD of lParam
 * and the HIWORD contains the byte count.
 * The maximum number of bytes passed is PIPE_DATASIZE.
 * EOF is signified by count = 0 (hglobal must still be valid)
 */

/* pipe file to gswin */
/* return TRUE if OK, FALSE if pipe overflow or file error */
BOOL
pipe_file(char *fname)
{
	if ((hfPipe!=(HFILE)NULL) || (!bPipeDone)) {
		gserror(IDS_PIPEERR, NULL, NULL, SOUND_ERROR);
		pipe_clean();
		return FALSE;
	}
	hfPipe = _lopen(fname, READ);
	if (hfPipe == HFILE_ERROR) {
		gserror(IDS_PIPEERR, NULL, NULL, SOUND_ERROR);
		return FALSE;
	}
	bPipeDone = FALSE;
	info_wait(TRUE);
	pipe_blk(hfPipe);
	return TRUE;
}

/* copy up to PIPE_DATASIZE bytes from file to global block */
/* send block to gswin */
/* return number of bytes sent or -1 for error */
int
pipe_blk(HFILE hf)
{
HGLOBAL hglobal;
LPBYTE lpb;
UINT count;
	hglobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, PIPE_DATASIZE);
	if (hglobal == (HGLOBAL)NULL) {
	    gserror(IDS_PIPEERR, NULL, NULL, SOUND_ERROR);
	    return -1;
	}
	lpb = GlobalLock(hglobal);
	count = _lread(hf, lpb, PIPE_DATASIZE);
	GlobalUnlock(hglobal);
	if ((HFILE)count == HFILE_ERROR) {
	    GlobalFree(hglobal);
	    gserror(IDS_PIPEERR, NULL, NULL, SOUND_ERROR);
	    return -1;
	}
	/* we may be processing SendMessage so use PostMessage to avoid lockups */
	if (count)
	    PostMessage(hwndtext, WM_GSVIEW, PIPE_DATA, MAKELPARAM(hglobal,count));
	else
	    GlobalFree(hglobal);
	return count;
}

/* send an EOF (zero length block) */
void
pipe_close()
{
HGLOBAL hglobal;
	pipe_clean();
	hglobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, 1);
	if (hglobal == (HGLOBAL)NULL) {
	    gserror(IDS_PIPEERR, NULL, NULL, SOUND_ERROR);
	    return;
	}
	PostMessage(hwndtext, WM_GSVIEW, PIPE_DATA, MAKELPARAM(hglobal,0));
	
}

/* clean up pipe overflow or close empty pipe input file */
void
pipe_clean()
{
	if (hfPipe) {
		_lclose(hfPipe);
		hfPipe = NULL;
    	}
	bPipeDone = TRUE;
	clear_timer();
	info_wait(FALSE);
}
