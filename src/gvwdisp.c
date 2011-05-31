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

/* gvwdisp.c */
/* Display GSview routines for Windows */
#include "gvwin.h"

/* handle messages while we are waiting */
void
do_message(void)
{
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
	if ((hDlgModeless == 0) || !IsDialogMessage(hDlgModeless, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

/* run Ghostscript for previewing document */
/* return TRUE if ok, FALSE if error */
BOOL
gs_open(void)
{
#ifdef WIN32
char command[256];
#else
char command[512];
#endif
FILE *optfile;
int depth;
	/* return if already open */
	if ((gsprog.valid) && IsWindow(hwndimgchild))
		return TRUE;

	display.do_endfile = FALSE;
	display.do_resize = FALSE;
	zoom = FALSE;
	pipeinit();		/* so we wait for first request */
	gs_size();
#ifdef OLD
	sprintf(command,"\042%s\042 %s -I\042%s\042 -dBitsPerPixel=%d %s -r%gx%g -g%ux%u -sGSVIEW=%u -",
		option.gsexe, option.gsother, option.gsinclude,
		(option.depth ? option.depth : display.bitcount*display.planes),
		option.safer ? "-dSAFER" : "", 
		option.xdpi, option.ydpi, 
                display.width, display.height, (unsigned int)hwndimg);
#else
	/* new version uses temporary file to reduce command line length */
	/* avoids setting FIXEDMEDIA and does sanity check on BitsPerPixel */

	/* restrict depth to values supported by Ghostscript */
	depth = display.planes * display.bitcount;
	if (depth >= 24)
	    depth = 24;
	else if (depth >=15)
	    depth = 16;
	else if (depth >=8)
	    depth = 8;
	else if (depth >=4)
	    depth = 4;
	else 
	    depth = 1;

	if ((display.optname[0] != '\0') && !debug)
	    unlink(display.optname);
	display.optname[0] = '\0';
	if ( (optfile = gp_open_scratch_file(szScratch, display.optname, "w")) == (FILE *)NULL) {
	    play_sound(SOUND_ERROR);
	    return FALSE;
	}

	if (option.gsversion == IDM_GS351)
	    fprintf(optfile, "-I\042%s\042\n", option.gsinclude);
	else
	    fprintf(optfile, "-I%s\n", option.gsinclude);
	if (option.safer)
	    fprintf(optfile, "-dSAFER\n");
	fprintf(optfile, "-dBitsPerPixel=%d\n", option.depth ? option.depth : depth);
	fprintf(optfile, "-dDEVICEXRESOLUTION=%g\n", (double)option.xdpi);
	fprintf(optfile, "-dDEVICEYRESOLUTION=%g\n", (double)option.ydpi);
	fprintf(optfile, "-dDEVICEWIDTH=%u\n", display.width);
	fprintf(optfile, "-dDEVICEHEIGHT=%u\n", display.height);
	fclose(optfile);
	sprintf(command,"%s %s @%s -sGSVIEW=%u -", option.gsexe, 
	    option.gsother, display.optname, (unsigned int)hwndimg);
#endif

	if ( ((strlen(command) > 126) && !(is_winnt || is_win95))
	  ||  (strlen(command) > 256) ) {
		display.do_display = FALSE;
		gserror(IDS_TOOLONG, command, MB_ICONSTOP, SOUND_ERROR);
		return FALSE;
	}
	info_wait(IDS_WAITGSOPEN);
	gsprog.hinst = (HINSTANCE)WinExec(command, SW_SHOWMINNOACTIVE);

#ifdef __WIN32__
	if ( (is_win95 && ((int)gsprog.hinst < HINSTANCE_ERROR))
	  || (is_winnt && ((int)gsprog.hinst < HINSTANCE_ERROR))
          || (gsprog.hinst == NULL) ) {	/* Win32s WinExec returned buggy value */
#else
	if (gsprog.hinst < HINSTANCE_ERROR) {
#endif
		info_wait(IDS_NOWAIT);
		display.do_display = FALSE;
		gserror(IDS_CANNOTRUN, command, MB_ICONSTOP, SOUND_ERROR);
		load_string(IDS_TOPICINSTALL, szHelpTopic, sizeof(szHelpTopic));
		get_help();
		return FALSE;
	}
	{   /* TEMPORARY KLUDGE */
	int i;
	for (i=0; i<10; i++)
	    do_message();	/* wait for gswin to establish with debugger running */
	}
#ifdef NOTUSED
#endif
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
		gsprog.hinst = (HINSTANCE)NULL;
		clear_timer();
		info_wait(IDS_NOWAIT);
		gserror(IDS_WRONGGS, NULL, MB_ICONSTOP, SOUND_ERROR);
		return FALSE;
	}

	/* wait for gswin to initialise */
	if (set_timer(CLOSE_TIMEOUT))
	    EnableWindow(hwndimg, FALSE);
	while (!is_pipe_done()&&  !bTimeout)
		do_message();	/* wait for pipe data request from gswin */
	clear_timer();
	EnableWindow(hwndimg, TRUE);

	display.saved = FALSE;
	display.page = FALSE;
	display.sync = FALSE;
	gsprog.input = pipeopen();	/* open pipe to gswin */
	if (gsprog.input == (FILE *)NULL) {
	    gs_close();
	    return FALSE;
	}
	gsprog.valid = TRUE;
	BringWindowToTop(hwndimg);
	SetFocus(hwndimg);	/* kludge: without this desktop gets focus */
	return TRUE;
}

/* close Ghostscript */
BOOL
gs_close(void)
{
BOOL force = FALSE;
	if (!gsprog.valid)
	    return TRUE;

	if (doc == (PSDOC *)NULL) {
	    /* we don't know how many pages remain so we must force an exit */
	    if (!is_pipe_done())
		force = TRUE;
	}
	else {
	    if (display.page)
		next_page();
	}

	if (!force) {
	    /* try to close Ghostscript cleanly */
	    pipeclose();
	    if (set_timer(CLOSE_TIMEOUT))
		EnableWindow(hwndimg, FALSE);
#ifdef __WIN32__
	    {   /* TEMPORARY KLUDGE */
		int i;
		for (i=0; i<10; i++) {
		    do_message();	/* wait for gswin to close */
		    Sleep(50);
		}
	    }
#else
	    while (GetModuleUsage(gsprog.hinst) &&  !bTimeout)
		do_message();	/* wait for gswin to close */
#endif
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
	gsprog.hinst = (HINSTANCE)NULL;
	hwndimgchild = (HWND)NULL;
	hwndtext = (HWND)NULL;
	bitmap_scrollx = bitmap_scrolly = 0;
	display.saved = FALSE;
	display.epsf_clipped = FALSE;
	display.page = FALSE;
	display.sync = FALSE;
	pipeclose();

	if ((display.optname[0] != '\0') && !debug)
	    unlink(display.optname);
	display.optname[0] = '\0';
	return TRUE;
}

/* send a NEXT_PAGE message to Ghostscript */
void
next_page(void)
{
int i;
	if (hwndimgchild && IsWindow(hwndimgchild)) {
		SendMessage(hwndimgchild, WM_GSVIEW, NEXT_PAGE, 0L);
		display.page = FALSE;
	}
	do_message();	/* wait for Ghostscript to process message */
	for (i=0; i<32; i++) {
	   /* Wait a bit for pipe contents after showpage to be read */
	   do_message();
	   if (is_pipe_done())
		break;
	}
}



/* return TRUE if file length or modification time changed */
BOOL
psfile_changed(void)
{
struct ftime thisftime;
long thisflength;
	getftime(fileno(psfile.file), &thisftime);
	thisflength = filelength(fileno(psfile.file));
	return ( (thisflength != psfile.length) ||
		memcmp(&thisftime, &psfile.datetime, sizeof(thisftime)) );
}

void
psfile_savestat(void)
{
	getftime(fileno(psfile.file), &psfile.datetime);
	psfile.length = filelength(fileno(psfile.file));
}
