/*  Copyright (C) 1996, Russell Lang.  All rights reserved.

 This file is part of GSview.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GSVIEW General Public License for more details.

 Everyone is granted permission to copy, modify and redistribute
 this program, but only under the conditions described in the GSVIEW
 General Public License.  A copy of this license is supposed to have been
 given to you along with this program so you can know your rights and
 responsibilities.  It should be in a file named COPYING.  Among other
 things, the copyright notice and this notice must be preserved on all
 copies. */

/* gvcdll.c */
/* GS DLL associated routines */
#ifdef _Windows
#include "gvwin.h"
#else
#include "gvpm.h"
#endif
#include <stdarg.h>


GSDLL gsdll;		/* the main DLL structure */
PENDING pending;	/* operations that must wait */
int execute_code;	/* return code from gsdll.execute_cont */

/* forward declarations */
int gs_process_pstotext(void);

int
gs_execute(char *str, int len)
{
    if (gsdll.execute_cont == NULL)
	return 255;
    execute_code = gsdll.execute_cont(str, len);
    return execute_code;
}

/* returns error code from GS */
int
gs_printf(const char *fmt, ...)
{
va_list args;
int count;
int code;
char buf[1024];
	va_start(args,fmt);
	count = vsprintf(buf,fmt,args);
	if (debug)
	    gs_addmess(buf);
	code = gs_execute(buf, count);
	va_end(args);
	return code;
}

/* private funtions */

/* calculate new size then set it */
int
d_resize(void)
{
int i = get_paper_size_index();
int code = 0;
PSDOC *doc;
float	width;		  /* page width in 1/72" */
float	height;		  /* page height in 1/72" */
int	xoffset;	  /* page origin offset in 1/72" */
int	yoffset;	  /* page origin offset in 1/72" */

    /* calculate new size */
    doc = psfile.doc;
    if ( (option.xdpi == 0.0) || (option.ydpi == 0.0) )
	    option.xdpi = option.ydpi = DEFAULT_RESOLUTION;
    display.epsf_clipped = FALSE;

    display.xdpi = option.xdpi;
    display.ydpi = option.ydpi;

    if (zoom && (doc!=(PSDOC *)NULL)) {
	display.xdpi = option.zoom_xdpi;
	display.ydpi = option.zoom_ydpi;
	width = display.width * 72.0 / display.xdpi;
	height = display.height * 72.0 / display.ydpi;
	xoffset = display.zoom_xoffset;
	yoffset = display.zoom_yoffset;
    }
    else if ((doc != (PSDOC *)NULL) && doc->epsf
	&& option.epsf_clip) {
	display.epsf_clipped = TRUE;
	width = doc->boundingbox[URX] - doc->boundingbox[LLX];
	height = doc->boundingbox[URY] - doc->boundingbox[LLY];
	xoffset = doc->boundingbox[LLX];
	yoffset = doc->boundingbox[LLY];
    }
    else if (i < 0) {
	width = option.user_width;
	height = option.user_height;
	xoffset = 0;
	yoffset = 0;
    }
    else {
	width = papersizes[i].width;
	height = papersizes[i].height;
	xoffset = 0;
	yoffset = 0;
    }

    switch (option.orientation) {
	case IDM_LANDSCAPE:
	    if (option.swap_landscape)
		display.orientation = 1;
	    else
		display.orientation = 3;
	    break;
	case IDM_SEASCAPE:
	    if (option.swap_landscape)
		display.orientation = 3;
	    else
		display.orientation = 1;
	    break;
	case IDM_PORTRAIT:
	    display.orientation = 0;
	    break;
	case IDM_UPSIDEDOWN:
	    display.orientation = 2;
	    break;
    }

    display.width  = (unsigned int)(width  * display.xdpi / 72.0 + 0.5);
    display.height = (unsigned int)(height * display.ydpi / 72.0 + 0.5);

    display.xoffset = (unsigned int)(xoffset * display.xdpi / 72.0);
    display.yoffset = (unsigned int)(yoffset * display.ydpi / 72.0);


    /* set new size */

    if (!code)
	code = gs_printf("GSview /HWResolution [%f %f] put\n", display.xdpi, display.ydpi);
    if (!code)   /* need to improve this for PDF - need page bbox */
	code = gs_printf("GSview /PageSize [%f %f] put\n", width, height);
    if ((xoffset!=0) || (yoffset!=0)) {
	if (!code)
	    code = gs_printf("GSview /ImagingBBox [%d %d %f %f] put\n", 
		xoffset, yoffset, xoffset + width, yoffset + height);
    }
    else {
	if (!code)
	    code = gs_printf("GSview /ImagingBBox null put\n"); 
    }
    if (!code)
	code = gs_printf("GSview /Orientation %d put\n", display.orientation);
    if (!code)
	code = gs_printf("GSview /TextAlphaBits %d put\n", option.alpha_text);
    if (!code)
	code = gs_printf("GSview /GraphicsAlphaBits %d put\n", option.alpha_graphics);

    return code;
}


/* save GSview state */
int
d_save(void)
{
int code;
    code = gs_printf("/GSview_Save save def\n");
    display.saved = TRUE;
    return code;
}

/* restore GS */
/* assume restore is quick - don't check message loop */
int
d_restore(void)
{
int code;
    code = gs_printf("userdict /gsview_eps_countcheck known {gsview_eps_countcheck} if flush\n");
    if (!code)
        code = gs_printf("//systemdict /clear get exec //systemdict /cleardictstack get exec\n");
    if (!code)
        code = gs_printf("GSview_Save restore\n");
    display.saved = FALSE;
    return code;
}


/* create GSview dictionary */
int
d_init1(void)
{
    /* create GSview dictionary */
    return gs_printf("/GSview 8 dict def GSview begin\n/PageSize [612 792] def\n\
/ImagingBBox null def\n/Orientation 0 def\n/HWResolution [96.0 96.0] def\n\
/Size PageSize def\n/PageOffset [0 0] def\n\
/TextAlphaBits 1 def /GraphicsAlphaBits 1 def\nend\n");
}


/* open device and install viewer hooks */
int
d_init2(void)
{
int code;
int depth;

    /* calculate depth */
    if (option.depth)
	depth = option.depth;
    else
        depth = display.planes * display.bitcount;
    if (depth > 8)
	depth = 24;
    else if (depth >=8)
	depth = 8;
    else if (depth >=4)
	depth = 4;
    else 
	depth = 1;


    /* set the device depth and size */
    /* single pixel size hides window */
    /* open device */
    code = gs_printf("<< /OutputDevice /%s /BitsPerPixel %d \n",
	    DEVICENAME, depth);
    if (!code)
        code = send_prolog(IDR_VIEWER);
    if (!code)
        code = gs_printf(">> setpagedevice\n");

    if (code) {
	char buf[256];
	sprintf(buf,"Failed to open device or install ViewerPreProcess hook: returns %d\n", code);
	gs_addmess(buf);
	pending.unload = TRUE;
    }
    else
	display.init = TRUE;
    return code;
}


/* input for Ghostscript DLL */
/* get_gs_input() copies/reads from gs_input[] to the supplied buffer */
/* copy/read at most blen bytes */
/* returns number of bytes copied to buffer */
int
get_gs_input(char *buf, int blen)
{
unsigned long len;
GSINPUT *entry;


    if (gsdll.input_index >= gsdll.input_count) {
	return 0; /* no more */
    }

    entry = &gsdll.input[gsdll.input_index];
    len = entry->end - entry->ptr;
    len = min(len, blen);

    if (entry->seek) {
	fseek(psfile.file, entry->ptr, SEEK_SET);
	entry->seek = FALSE;
    }

    len = fread(buf, 1, (int)len, psfile.file);
    entry->ptr += len;

    /* check if we need to move to next item */
    if (entry->ptr >= entry->end) {
        gsdll.input_index++;
	/* do initial seek */
        if (gsdll.input_index < gsdll.input_count) {
	    fseek(psfile.file, gsdll.input[gsdll.input_index].ptr, SEEK_SET);
	    gsdll.input[gsdll.input_index].seek = FALSE;
	}
    }
    return (int)len;
}


/* assume this is quick - don't check message loop */
int
send_trailer(void)
{
char buf[MAXSTR];
unsigned long len;
unsigned long ptr;
int code = 0;
int prevlen;
PSDOC *doc = psfile.doc;
    ptr = doc->begintrailer;
    fseek(psfile.file, ptr, SEEK_SET);
    len = doc->endtrailer - ptr;
    len = min(len, sizeof(buf));
    while (!code && len && (len = fread(buf, 1, (int)len, psfile.file))!=0) {
	code = gs_execute(buf, (int)len);
	prevlen = (int)len;
        ptr += len;
	len = doc->endtrailer - ptr;
	len = min(len, sizeof(buf));
    }
    if (code) {
	gs_addmess("\n---Error in document trailer---\n-------\n");
        gs_addmess_count(buf, prevlen);
	gs_addmess("\n-------\n");
    }
    return code;
}

/* This doesn't actually send anything to Ghostscript */
/* Instead it prepares a list of what will be sent */
int
send_document(void)
{
char filename[MAXSTR];
char buf[MAXSTR];
char *p;
PSDOC *doc = psfile.doc;
    if (psfile.file == (FILE *)NULL)
	return -1;
    strcpy(filename, psfile.name);
    for (p=filename; *p; p++)
	if (*p == '\\')
	    *p = '/';

    if (doc != (PSDOC *)NULL) {
	if (doc->pages != 0) {
	    int i, page;
	    i = 0;
	    page = map_page(psfile.pagenum-1);
	    if (display.need_header) {
		sprintf(buf, "Displaying DSC file %s\n", filename);
		gs_addmess(buf);
		/* add header */
		if (doc->lenheader != 0) {
		    gsdll.input[i].ptr = doc->beginheader;
		    gsdll.input[i].end = doc->endheader;
		    gsdll.input[i].seek = TRUE;
		    i++;
		}
		/* add defaults */
		if (doc->lendefaults != 0) {
		    gsdll.input[i].ptr = doc->begindefaults;
		    gsdll.input[i].end = doc->enddefaults;
		    gsdll.input[i].seek = TRUE;
		    i++;
		}
		/* add prolog */
		if (doc->lenprolog != 0) {
		    gsdll.input[i].ptr = doc->beginprolog;
		    gsdll.input[i].end = doc->endprolog;
		    gsdll.input[i].seek = TRUE;
		    i++;
		}
		/* add setup */
		if (doc->lensetup != 0) {
		    gsdll.input[i].ptr = doc->beginsetup;
		    gsdll.input[i].end = doc->endsetup;
		    gsdll.input[i].seek = TRUE;
		    i++;
		}
		display.need_header = FALSE;
		/* remember if we need trailer later */
		if (doc->lentrailer)
		    display.need_trailer = TRUE;
	    }
	    /* add page */
	    sprintf(buf, "Displaying page %d\n", psfile.pagenum);
	    gs_addmess(buf);
	    gsdll.input[i].ptr = doc->pages[page].begin;
	    gsdll.input[i].end = doc->pages[page].end;
	    gsdll.input[i].seek = TRUE;
	    i++;
	    gsdll.input_count = i;
	    gsdll.input_index = 0;
	}
	else {
	    /* add complete file */
	    sprintf(buf, "Displaying DSC file %s without pages\n", filename);
	    gs_addmess(buf);
	    gsdll.input[0].ptr = doc->beginheader;
	    gsdll.input[0].end = doc->endtrailer;
	    gsdll.input[0].seek = TRUE;
	    gsdll.input_count = 1;
	    gsdll.input_index = 0;
	    display.need_trailer = FALSE;
	}
    }
    else {
	/* add complete file */
        sprintf(buf, "Displaying non DSC file %s\n", filename);
	gs_addmess(buf);
	gsdll.input[0].ptr = 0;
	fseek(psfile.file, 0, SEEK_END);
	gsdll.input[0].end = ftell(psfile.file);
	gsdll.input[0].seek = TRUE;
	gsdll.input_count = 1;
	gsdll.input_index = 0;
        display.need_trailer = FALSE;
    }
/* this is done by get_gs_input
    fseek(psfile.file, gsdll.input[0].ptr, SEEK_SET);
*/
    return 0;
}

BOOL
gs_process_trailer(void)
{
    if (display.need_trailer) {
	/* assume trailer is short, don't check message loop */	
	post_img_message(WM_GSWAIT, IDS_WAIT);
	if (!psfile.ispdf && !dfreopen()) {
	    pending.abort = TRUE;
	    return TRUE;
	}
	if (psfile.ispdf) {
	    if (pdf_trailer())
		return FALSE;
	}
	else {
	    if (send_trailer()) {
		dfclose();
		return FALSE;
	    }
	    dfclose();
	}
	display.need_trailer = FALSE;
    }
    return FALSE;
}

/* process current page or non-DSC file */
/* returns 0 if OK, else GS error code */
int
gs_process_loop2(PENDING *ppend)
{
char buf[MAXSTR];
int len;
int code = 0;
long lsize, ldone;
int pcdone;
int i;

    if (!dfreopen()) {
	request_mutex();
	/* document changed, so force a rescan */
	pending.now = FALSE;
	/* cause a redisplay after abort complete */
	pending.redisplay = TRUE;
	pending.abort = TRUE;
	release_mutex();
	return 0;
    }

    if (!display.init) {
	if (!code)
	    code = d_init1();	/* create GSview dict */
	if (!code)
	    code = d_resize();	/* Set GSview dict entries */
	if (!code)
	    code = d_init2();	/* add ViewerPreProcess then setpagedevice */
    }
    else {
	if (ppend->resize) {
	    if (!code)
		code = d_resize();
	    if (!code)
		code = gs_printf("<< >> //systemdict /setpagedevice get exec\n");
	}
    }

    if (!code)
        if (!display.saved && display.need_header && option.quick_open)
	    code = d_save();
    if (!code && option.epsf_warn)
	code = send_prolog(IDR_EPSFWARN);

    if (code) {
	/* error initializing GS */
	if (!psfile.ispdf)
	    dfclose();
	return code;
    }

    /* move to desired page */
    if (ppend->pagenum > 0)
	psfile.pagenum = ppend->pagenum;
    ppend->pagenum = 0;

    /* highlight search word if needed */
    if (psfile.text_bbox.valid) {
	display.show_find = TRUE;
	psfile.text_bbox.valid = FALSE;	/* don't do it on following pages */
    }
    else
	display.show_find = FALSE;

    if (psfile.ispdf) {
	post_img_message(WM_GSTEXTINDEX, 0);
	if (display.need_header) {
	    gs_addmess("Scanning PDF file\n");
	    if ( (code = pdf_head()) != 0 )
		return code;
	}
	display.need_header = FALSE;
	if (ppend->pagenum > 0)
	    psfile.pagenum = ppend->pagenum;
	ppend->pagenum = 0;
	sprintf(buf, "Displaying PDF page %d\n", psfile.pagenum);
	gs_addmess(buf);
	if ( (code = pdf_page(psfile.pagenum)) != 0 )
	    return code;
    }
    else {
	post_img_message(WM_GSTEXTINDEX, 0);
        /* prepare list of PostScript sections to send */
	if ( (code = send_document()) != 0 ) {
	    dfclose();
	    return code;
	}

        /* calculate document length */
	ldone = 0;
	lsize = 0;
	for (i=0; i<gsdll.input_count; i++)
	    lsize += (gsdll.input[i].end - gsdll.input[i].ptr);
	percent_pending = FALSE;
	percent_done = 0;

	post_img_message(WM_GSWAIT, IDS_WAITDRAW_PC);

	/* send postscript */
	while (!pending.abort && !pending.unload
	    && ((len = get_gs_input(buf, sizeof(buf)))!=0)) {
	    code = gs_execute(buf, len);
	    if (code) {
		dfclose();
		if (pending.abort) {
		    gs_addmess("\n--- Aborted ---\n");
		    return 0;
		}
		else {
		    gs_addmess("\n--- Begin offending input ---\n");
		    gs_addmess_count(buf, len);
		    gs_addmess("\n--- End offending input ---\n");
		    sprintf(buf, "gsdll_execute_cont returns %d\n", code);
		    gs_addmess(buf);
		}
		return code;
	    }
	    ldone += len;
	    pcdone = (int)(ldone * 100 / lsize);
	    if ((pcdone != percent_done) && !percent_pending) {
		percent_done = pcdone;
		percent_pending = TRUE;
		post_img_message(WM_GSPERCENT, 0);
	    }
	    if (!multithread) {
		/* peek message loop */
		peek_message();
	    }
	}
	gs_printf("\n");
	dfclose();
    }

    if (psfile.doc == (PSDOC *)NULL) {
	/* We are at the end of the file */
	/* Do not allow more pages to be displayed */
	request_mutex();
	pending.pagenum = 0;
	pending.restart = TRUE;
	release_mutex();
    }
    if (pending.abort)
	gs_addmess("\n--- Aborted ---\n");

    return 0;
}


/* loop while Ghostscript loaded, document unchanged and no errors */
/* return 0 if OK, or GS error code */
int
gs_process_loop1(void)
{
PENDING lpending;
int code = 0;
    while (!pending.unload) {
	if (!pending.now) {
	    if (szWait[0] != '\0')
	        post_img_message(WM_GSWAIT, IDS_NOWAIT);
	    if (multithread)
		wait_event();	/* wait for event semaphore */
	    else
		get_message();	/* process one message */
	}
	if (pending.now && !pending.unload && !pending.abort) {
	    gsdll.state = BUSY;
	    post_img_message(WM_GSWAIT, IDS_WAIT);

	    /* block other threads while we take a snapshort of pending */
	    request_mutex();
	    lpending = pending;
	    pending.now = FALSE;
	    pending.next = FALSE;
	    pending.abort = FALSE;
	    pending.psfile = NULL;
	    pending.pagenum = 0;
	    pending.resize = FALSE;
	    pending.text = FALSE;
	    pending.restart = FALSE;
	    release_mutex();

	    if (lpending.psfile && display.init)
		lpending.restart = TRUE;

	    if (lpending.resize && display.init && !option.quick_open)
		lpending.restart = TRUE;

	    if (lpending.text && display.init)
		lpending.restart = TRUE;

	    if (lpending.restart || (lpending.resize && !psfile.ispdf) 
		  || lpending.psfile) {
		/* if size or document changed, need to restart document */
		if (display.need_trailer)
		    gs_process_trailer();	/* send trailer to clean up */
		display.need_header = TRUE;
		if (display.saved)
		    code = d_restore();
		if (code)
		    lpending.restart = TRUE;
	    }

	    if (lpending.psfile) {
		request_mutex();
		zoom = FALSE;
		psfile_free(&psfile);
		psfile = *lpending.psfile;
		post_img_message(WM_GSTITLE, 0);
		free(lpending.psfile);
		if (psfile.name[0] == '\0') {
		    /* no new file */
	    	    gsdll.state = IDLE;
		    post_img_message(WM_GSWAIT, IDS_NOWAIT);
		    /* notify in case CLOSE was caused by IDM_SELECT */
		    post_img_message(WM_COMMAND, IDM_CLOSE_DONE);
		    release_mutex();
		    break;
		}
	        release_mutex();
	    }

	    if (lpending.restart && display.init) {
		request_mutex();
		/* restore pending operations */
		pending.pagenum = lpending.pagenum;
		pending.abort = TRUE;		/* ignore errors */
		pending.text = lpending.text;
		pending.now = TRUE;
		release_mutex();
		return 0;	/* restart interpreter */
	    }

	    if (lpending.text) {
		code = gs_process_pstotext();
		gsdll.state = IDLE;
		return code;
	    }

	    if ( (code = gs_process_loop2(&lpending)) != 0 ) {
		gsdll.state = IDLE;
		if (psfile.doc == (PSDOC *)NULL)
		    psfile.pagenum = 1;
		post_img_message(WM_GSWAIT, IDS_NOWAIT);
		return code;
	    }

	    if (pending.abort)
		return 0;

	    if (pending.unload) {
		post_img_message(WM_GSWAIT, IDS_WAITGSCLOSE);
	    }
	    else {
		gsdll.state = IDLE;
		if (psfile.doc == (PSDOC *)NULL)
		    psfile.pagenum = 1;
		post_img_message(WM_GSWAIT, IDS_NOWAIT);
	    }
	    post_img_message(WM_GSSYNC, 0);	/* redraw display */
	}
    }
    return 0;
}



/********************************************************/
/* public functions */

/* In single thread mode, the main message loop is only used 
 * when the GS DLL is unloaded */

/* This is the second message loop, or second thread */
/* It loads the GS DLL and returns when GS DLL is unloaded */
/* The third message loop is in the GS DLL callback gsdll.state = PAGE */

void
gs_process(void)
{
int code;
    /* gsdll.state = UNLOADED; */
    gsdll.state = BUSY;		/* skip IDLE state */
    if (!gs_load_dll()) {
        gsdll.state = UNLOADED;
	request_mutex();
	pending.now = FALSE;
	pending.next = FALSE;
	pending.unload = FALSE;
	pending.abort = FALSE;
	pending.redisplay = FALSE;
	if (pending.psfile) {
	    psfile_free(&psfile);
	    psfile = *pending.psfile;
	    free(pending.psfile);
	    pending.psfile = NULL;
	}
	release_mutex();
	post_img_message(WM_GSWAIT, IDS_NOWAIT);
	return;
    }
/*
    post_img_message(WM_GSWAIT, IDS_WAIT);
*/

    /* send stuff to Ghostscript */
    while (!pending.unload) {
        display.saved = FALSE;
	display.init = FALSE;
	display.need_header = TRUE;
	display.need_trailer = FALSE;
	pending.restart = FALSE;

	if (!pending.now) {
	    if (szWait[0] != '\0')
	        post_img_message(WM_GSWAIT, IDS_NOWAIT);
	    if (multithread)
		wait_event();	/* wait for event semaphore */
	    else
		get_message();	/* process one message */
	}

	if ( (code = gs_dll_init(gsdll_callback, NULL)) != 0 ) {
	    delayed_message_box(IDS_PROCESS_INIT_FAIL, 0);
	    post_img_message(WM_GSSHOWMESS, 0);
	    pending.unload = TRUE;
	    break;
	}
	if (!code) {
	    if (gsdll.execute_begin == NULL)
		break;
	    if ( (code = gsdll.execute_begin()) != 0 ) {
		char buf[256];
		sprintf(buf,"gsdll.execute_begin returns %d\n", code);
		gs_addmess(buf);
		pending.unload = TRUE;
		post_img_message(WM_GSSHOWMESS, 0);
	    }
	}

        if (!code) {
	    code = gs_process_loop1();
	    if (code) {
		if (!pending.abort) {
		    pending.now = FALSE;
		    pending.resize = FALSE;
		    pending.next = FALSE;
		    post_img_message(WM_GSSHOWMESS, 0);
		}
	    }
	}
	if (execute_code >= 0)
	    gsdll.execute_end();
	if (gsdll.exit != NULL) {
	    char buf[256];
	    code = gsdll.exit();
	    if (debug || code) {
	        sprintf(buf,"gsdll_exit returns %d\n", code);
	        gs_addmess(buf);
	    }
	    gs_addmess("\n\n");
	}
	if (pending.redisplay) {
	    pending.redisplay = FALSE;
	    post_img_message(WM_GSREDISPLAY, 0);	/* will set pending.now */
	}
	pending.abort = FALSE;
    }

    post_img_message(WM_GSWAIT, IDS_WAITGSCLOSE);
    gs_free_dll();
    gsdll.state = UNLOADED;
    display.need_header = TRUE;
    display.need_trailer = FALSE;
    display.saved = FALSE;
    pending.now = FALSE;
    pending.next = FALSE;
    pending.unload = FALSE;
    pending.abort = FALSE;
    if (pending.psfile) {
	request_mutex();
	psfile_free(&psfile);
        psfile = *pending.psfile;
	free(pending.psfile);
	pending.psfile = NULL;
	release_mutex();
    }
    /* close document also */
    gsview_unzoom();

    post_img_message(WM_GSWAIT, IDS_NOWAIT);
    post_img_message(WM_GSTITLE, 0);
}


/* next page */
int
gs_page_skip(int skip)
{
PSDOC *doc = psfile.doc;
    if (doc == (PSDOC *)NULL) {
	/* NOT DSC, can only go forward one page at a time */
	if (skip != 1) {
	    play_sound(SOUND_NOPAGE);
	    return FALSE;
	}
	if (gsdll.state == IDLE) {
	    /* at end of file */
	    play_sound(SOUND_NOPAGE);
	    return FALSE;	/* nothing to do */
	}
	else  {
	    request_mutex();
	    gsview_unzoom();	/* should never be zoomed anyway */
	    pending.next = TRUE;
	    release_mutex();
	}
	return TRUE;
    }

    /* Must be DSC */
    if ( (skip == 0) 
        || ((skip > 0) && (psfile.pagenum == doc->numpages))
        || ((skip < 0) && (psfile.pagenum == 1))
	|| (doc->numpages == 0) ) {
	    play_sound(SOUND_NOPAGE);
	    return FALSE;
    }

    request_mutex();
    pending.pagenum = psfile.pagenum + skip;
    if (pending.pagenum > (int)doc->numpages)
	 pending.pagenum = doc->numpages;
    if (pending.pagenum < 1)
	pending.pagenum = 1;
    gsview_unzoom();
    pending.now = TRUE;
    release_mutex();

    return TRUE;
}
     
/* parse argline looking for argument */
/* If argument found, copy to arg and return pointer to next argument */
/* If no argument found, return NULL */
/* To get next argument, call again with return value as argline */
char *
gs_argnext(char *argline, char *arg)
{
    /* quotes may be put around embedded spaces, but are not copied */
    while ((*argline) && (*argline==' '))
	argline++;	/* skip over leading spaces */
    if (*argline=='\0')
	return NULL;	/* no more arguments */

    while (*argline) {
	if (*argline == ' ') {
	    /* end of argument */
	    *arg = '\0';
	    while (*argline && (*argline==' '))
		argline++;
	    return argline;
	}
	else if (*argline == '"') {
	    /* quoted argument */
	    /* copy until closing quote or end of string */
	    argline++;
	    while ((*argline) && (*argline != '"')) {
		*arg++ = *argline;
		argline++;
	    }
	    if ((*argline) && (*argline == '"'))
		argline++;
	}
	else {
	    *arg++ = *argline;
	    argline++;
	}
    }
    *arg = '\0';
    return argline;
}

/* called from gs_load_dll */
int
gs_dll_init(GSDLL_CALLBACK callback, char *devname)
{
char buf[1024];
char *p, *q;
int code;
int depth;
int i, argc;
char **argv;

	/* build options list */
	p = buf;
	buf[0] = '\0';
	buf[1] = '\0';

	/* add include path */
	if (option.gsinclude[0] != '\0') {
	    strcpy(buf, "-I");
	    strcat(buf, option.gsinclude);
	    p += strlen(p)+1;
	    *p = '\0';
	}

	/* add initial device */
	if (devname) {
	    strcpy(p, "-sDEVICE=");
	    p += strlen(p);
	    strcpy(p, devname);
	    p += strlen(p)+1;
	    *p = '\0';
	    /* restrict depth to values supported by Ghostscript */
	    depth = display.planes * display.bitcount;
	    if (depth > 8)
		depth = 24;
	    else if (depth >=8)
		depth = 8;
	    else if (depth >=4)
		depth = 4;
	    else 
		depth = 1;
	    strcpy(p, "-dBitsPerPixel=");
	    p += strlen(p);
	    sprintf(p, "%d", option.depth ? option.depth : depth);
	    p += strlen(p)+1;
	    *p = '\0';
	}
	else {
	    /* no device at all */
	    strcpy(p, "-dNODISPLAY");
	    p += strlen(p)+1;
	    *p = '\0';
	}

	strcpy(p, "-dNOPAUSE");
	p += strlen(p)+1;
	*p = '\0';

/* KLUDGE UNTIL BUG IS FIXED IN GHOSTSCRIPT */
if (!pending.text)
	if (option.safer) {
	    strcpy(p, "-dSAFER");
	    p += strlen(p)+1;
	    *p = '\0';
	}

	if (option.alpha_text > 1) {
	    strcpy(p, "-dNOPLATFONTS");
	    p += strlen(p)+1;
	    *p = '\0';
	}

	if (pending.text) {
	    strcpy(p, "-dDELAYBIND");
	    p += strlen(p)+1;
	    *p = '\0';
 	    strcpy(p, "-dWRITESYSTEMDICT");
	    p += strlen(p)+1;
	    *p = '\0';
	    strcpy(p, "-q");
	    p += strlen(p)+1;
	    *p = '\0';
	}

	/* add other options */
	/* option.gsother must have options separated by one or more spaces */
	/* quotes may be put around embedded spaces, but are not copied */
	q = option.gsother;
	while ((q = gs_argnext(q, p)) != NULL) {
	    p += strlen(p)+1;
	    *p = '\0';
	}

/* convert buf back to argv */
for (argc=1, p=buf; *p; argc++)
	p += strlen(p)+1;
argv = (char **)malloc((argc+2) * sizeof(char *));
argv[0] = option.gsdll;
for (i=1, p=buf; i<=argc; i++) {
    argv[i] = p;
    p+=strlen(p)+1;
}
argv[i] = NULL;

	if (debug) {
	    gs_addmess("Ghostscript options are:\n");
	    for (i=1; i<=argc; i++) {
		gs_addmess("  ");
		gs_addmess(argv[i]);
		gs_addmess("\n");
	    }
	}

	code = gsdll.init(callback, hwndimg, argc, argv);
free((void *)argv);
	if (code) {
	    sprintf(buf,"gsdll_init returns %d\n", code);
	    gs_addmess(buf);
	}
	if (code) {
	    gs_load_dll_cleanup();
	    return code;
	}
/*
	zoom = FALSE;
*/
	display.epsf_clipped = FALSE;
	gsdll.valid = TRUE;
	return code;
}


/******************************************************************/
/* for pstotext */
HMODULE pstotextModule;
FILE *pstotextOutfile;
void *pstotextInstance;
PFN_pstotextInit pstotextInit;
PFN_pstotextFilter pstotextFilter;
PFN_pstotextExit pstotextExit;
char pstotextLine[2048];
int pstotextCount;

int
callback_pstotext(char *str, unsigned long count)
{
    if (pstotextInstance) {
	if (debug)
	    gs_addmess_count(str, count);
	if (pstotextOutfile) {
	    char *d, *e;
	    char *pre, *post;
	    int llx, lly, urx, ury;
	    int status;
	    char ch;
	    if (sizeof(pstotextLine) > count + pstotextCount) { 
		memcpy(pstotextLine+pstotextCount, str, count);
		pstotextCount += count;
		pstotextLine[pstotextCount] = '\0';
		e = strchr(pstotextLine, '\n');
		while ( e != NULL ) {
		    ch = *(++e);	/* save character after \n */
		    *e = '\0';
		    d = pre = post = (char *)NULL;
		    status = pstotextFilter(pstotextInstance, pstotextLine, 
			&pre, &d, &post,
			&llx, &lly, &urx, &ury);
		    if (status)
			return 1;
		    if (d) {
			if (pre) {
			    if (*pre == ' ')
				pre++;
			    fputs(pre, pstotextOutfile);
			}
		        fprintf(pstotextOutfile, "%s %d %d %d %d\n",
			    d, llx, lly, urx, ury);
			if (post)
			    fputs(post, pstotextOutfile);
		    }
		    *e = ch;	/* restore character after \n */
		    memmove(pstotextLine, e, pstotextCount - (e-pstotextLine));
		    pstotextCount -= (e-pstotextLine);
		    pstotextLine[pstotextCount] = '\0';
		    e = strchr(pstotextLine, '\n');
		}
	    }
	    else
		pstotextCount = 0;	/* buffer overflow */
	}
        return 1;
    }
    return 0;
}

/* This handles extracting text from PS file, but not from PDF file */
int 
gs_process_pstotext(void)
{
int code;
char buf[MAXSTR];
int angle;
int len;
long lsize, ldone;
int pcdone;
    if (load_pstotext())
	return 1;

    gs_addmess("Extracting text using pstotext...\n");

    /* open output file */
    if ( (pstotextOutfile = gp_open_scratch_file(szScratch, psfile.text_name, "w")) == (FILE *)NULL) {
	gs_addmess("Can't open temporary file for text extraction\n");
	unload_pstotext();
	return 1;
    }

    percent_pending = FALSE;
    percent_done = 0;
    post_img_message(WM_GSWAIT, IDS_WAITTEXT);

    switch (option.orientation) {
	case IDM_LANDSCAPE:
	    if (option.swap_landscape)
		angle = 90;
	    else
		angle = 270;
	    break;
	case IDM_SEASCAPE:
	    if (option.swap_landscape)
		angle = 270;
	    else
		angle = 90;
	    break;
	case IDM_PORTRAIT:
	    angle = 0;
	    break;
	case IDM_UPSIDEDOWN:
	    angle = 180;
	    break;
    }

    /* send pstotext prolog to GS */
    if ((angle==270) && (code = send_pstotext_prolog(pstotextModule, 2)) != 0 ) {
	gs_addmess("Error processing rot270 prolog\n");
	unload_pstotext();
	return code;
    }
    if ((angle==90) && (code = send_pstotext_prolog(pstotextModule, 3)) != 0 ) {
	gs_addmess("Error processing rot90 prolog\n");
	fclose(pstotextOutfile);
	unload_pstotext();
	return code;
    }

    if ( (code = send_pstotext_prolog(pstotextModule, 1)) != 0 ) {
	gs_addmess("Error processing ocr prolog\n");
	fclose(pstotextOutfile);
	unload_pstotext();
	return code;
    }

    gs_printf("/setpagedevice { pop } def\n");

    if (psfile.ispdf) {
	int i;
	if ( (code = pdf_head()) != 0 ) {
	    gs_addmess("PDF prolog failed\n");
	    unload_pstotext();
	    fclose(pstotextOutfile);
	    return code;
	}
	if ( psfile.doc == (PSDOC *)NULL ) {
	    gs_addmess("Couldn't get PDF page count\n");
	    unload_pstotext();
	    fclose(pstotextOutfile);
	    return 1;
	}
	if ( (code = d_init1()) != 0) {
	    gs_addmess("Error creating GSview dictionary\n");
	    unload_pstotext();
	    fclose(pstotextOutfile);
	    return 1;
	}

	lsize = psfile.doc->numpages;
	i = 1;
	while (!pending.abort && !pending.unload && i <= psfile.doc->numpages) {
	    if ( (code = pdf_page(i)) != 0 ) {
		unload_pstotext();
		fclose(pstotextOutfile);
		return code;
	    }
	    pcdone = (int)(i * 100 / lsize);
	    if ((pcdone != percent_done) && !percent_pending) {
		percent_done = pcdone;
		percent_pending = TRUE;
		post_img_message(WM_GSPERCENT, 0);
	    }
	    i++;
	    if (!multithread) {
		/* peek message loop */
		peek_message();
	    }
	    if (pending.now)
		pending.abort = TRUE;	/* user wants to do something else */
	}
	if ((code = pdf_trailer()) != 0) {
	    gs_addmess("Error in PDF trailer\n");
	    unload_pstotext();
	    fclose(pstotextOutfile);
	    return code;
	}
    }
    else {
	if (!dfreopen()) {
	    gs_addmess("File changed or missing\n");
	    unload_pstotext();
	    fclose(pstotextOutfile);
	    return 1;
	}

	/* get file length */
	fseek(psfile.file, 0L, SEEK_END);
	lsize = ftell(psfile.file);
	if (lsize <= 0)
	    lsize = 1;
	fseek(psfile.file, 0L, SEEK_SET);
	ldone = 0;

	/* send document to GS */
	while (!pending.abort && !pending.unload
	    && ((len = fread(buf, 1, sizeof(buf), psfile.file))!=0)) {
	    code = gs_execute(buf, len);
	    if (code) {
		dfclose();
		fclose(pstotextOutfile);
		unload_pstotext();
		unlink(psfile.text_name);
		psfile.text_name[0] = '\0';
		if (pending.abort) {
		    gs_addmess("\n--- Aborted ---\n");
		    return 0;
		}
		else {
		    gs_addmess("\n--- Begin offending input ---\n");
		    gs_addmess_count(buf, len);
		    gs_addmess("\n--- End offending input ---\n");
		    sprintf(buf, "gsdll_execute_cont returns %d\n", code);
		    gs_addmess(buf);
		}
		return code;
	    }
	    ldone += len;
	    pcdone = (int)(ldone * 100 / lsize);
	    if ((pcdone != percent_done) && !percent_pending) {
		percent_done = pcdone;
		percent_pending = TRUE;
		post_img_message(WM_GSPERCENT, 0);
	    }
	    if (!multithread) {
		/* peek message loop */
		peek_message();
	    }
	    if (pending.now)
		pending.abort = TRUE;	/* user wants to do something else */
	}
	dfclose();
    }

    /* close output file */
    fclose(pstotextOutfile);

    /* unload pstotext DLL */
    unload_pstotext();

    /* if successful, restart extract or find */
    if (!pending.unload && !pending.abort) {
        gs_addmess("\npstotext successful\n");
	if (psfile.text_extract)
	    post_img_message(WM_COMMAND, IDM_TEXTEXTRACT_SLOW);
	else
	    post_img_message(WM_COMMAND, IDM_TEXTFINDNEXT);
    }
    else {
        gs_addmess("\npstotext aborted\n");
	unlink(psfile.text_name);
	psfile.text_name[0] = '\0';
    }
    pending.abort = TRUE;    /* ignore errors */
    return 0;
}
