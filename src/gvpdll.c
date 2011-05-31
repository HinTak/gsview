/*  Copyright (C) 1993-1998, Ghostgum Software Pty Ltd.  All rights reserved.

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

/* gvpdll.c */
/* GS DLL associated routines for PM */
#include "gvpm.h"

GSDLL gsdll;	/* the main DLL structure */
PENDING pending;


void 
post_img_message(int message, int param)
{
    WinPostMsg(hwnd_bmp, message, (MPARAM)param, (MPARAM)0);
}

void
gs_clear_gsdll(void)
{
    gsdll.hmodule = (HMODULE)NULL;
    gsdll.valid = FALSE;
    gsdll.device = NULL;
    gsdll.revision_number = 0;
    gsdll.revision = NULL;
    gsdll.init = NULL;
    gsdll.execute_begin = NULL;
    gsdll.execute_cont = NULL;
    gsdll.execute_end = NULL;
    gsdll.exit = NULL;
    gsdll.get_bitmap = NULL;
    gsdll.lock_device = NULL;
    gsdll.callback = NULL;
}

void
gs_load_dll_cleanup(void)
{
    gs_free_dll();
    delayed_message_box(IDS_PROCESS_GSLOAD_FAIL, 0);
    post_img_message(WM_GSSHOWMESS, 0);
}

/* load GS DLL if not already loaded */
/* return TRUE if OK */
BOOL
gs_load_dll(void)
{
APIRET rc;
char buf[MAXSTR+40];
char fullname[1024];
const char *shortname;
char *p;
const char *dllname;
	if (gsdll.hmodule)
	    return TRUE;
	post_img_message(WM_GSWAIT, IDS_WAITGSOPEN);
	gs_clear_gsdll();
	dllname = option.gsdll;
	sprintf(buf, "Trying to load %s\n", dllname);
	if (debug)
	    gs_addmess(buf);
	memset(buf, 0, sizeof(buf));
	rc = DosLoadModule(buf, sizeof(buf), dllname, &gsdll.hmodule);
	if (rc) {
	    /* failed */
	    /* try once more - which bug are we dodging? */
	    rc = DosLoadModule(buf, sizeof(buf), dllname, &gsdll.hmodule);
	}
	if (rc) {
	    /* failed */
	    /* try again, with path of EXE */
	    if ((shortname = strrchr((char *)option.gsdll, '\\')) == (const char *)NULL)
		shortname = option.gsdll;
	    strcpy(fullname, szExePath);
	    if ((p = strrchr(fullname,'\\')) != (char *)NULL)
		p++;
	    else
		p = fullname;
	    *p = '\0';
	    strcat(fullname, shortname);
	    dllname = fullname;
	    sprintf(buf, "Trying to load %s\n", dllname);
	    if (debug)
		gs_addmess(buf);
	    rc = DosLoadModule(buf, sizeof(buf), dllname, &gsdll.hmodule);
	    if (rc) {
		/* failed again */
		/* try once more, this time on system search path */
		dllname = shortname;
		sprintf(buf, "Trying to load %s\n", dllname);
		if (debug)
		    gs_addmess(buf);
	        rc = DosLoadModule(buf, sizeof(buf), dllname, &gsdll.hmodule);
	    }
	}



	if (rc == 0) {
	    gs_addmess("Loaded Ghostscript DLL\n");
	    if ((rc = DosQueryProcAddr(gsdll.hmodule, 0, "GSDLL_REVISION", (PFN *)(&gsdll.revision)))!=0) {
	        sprintf(buf, "Can't find GSDLL_REVISION, rc = %ld\n", rc);
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    /* check DLL version */
	    gsdll.revision(NULL, NULL, &gsdll.revision_number, NULL);
	    if ( (gsdll.revision_number < GS_REVISION_MIN) || (gsdll.revision_number > GS_REVISION_MAX) ) {
		sprintf(buf, "Wrong version of DLL found.\n  Found version %ld\n  Need version  %ld - %ld\n", 
			gsdll.revision_number, (long)GS_REVISION_MIN, (long)GS_REVISION_MAX);
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    if ( (gsdll.revision_number == 500) || (gsdll.revision_number == 500) ) {
		gs_addmess("\
**********************************************************************\n\
GSview warning: Ghostscript 5.0 and 5.01 do not work well with GSview.\n\
Please upgrade to a later version.\n\
**********************************************************************\n\
");
	    }
	    if ((rc = DosQueryProcAddr(gsdll.hmodule, 0, "GSDLL_INIT", (PFN *)(&gsdll.init)))!=0) {
	        sprintf(buf, "Can't find GSDLL_INIT, rc = %ld\n", rc);
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    if ((rc = DosQueryProcAddr(gsdll.hmodule, 0, "GSDLL_EXECUTE_BEGIN", (PFN *)(&gsdll.execute_begin)))!=0) {
	        sprintf(buf, "Can't find GSDLL_EXECUTE_BEGIN, rc = %ld\n", rc);
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    if ((rc = DosQueryProcAddr(gsdll.hmodule, 0, "GSDLL_EXECUTE_CONT", (PFN *)(&gsdll.execute_cont)))!=0) {
	        sprintf(buf, "Can't find GSDLL_EXECUTE_CONT, rc = %ld\n", rc);
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    if ((rc = DosQueryProcAddr(gsdll.hmodule, 0, "GSDLL_EXECUTE_END", (PFN *)(&gsdll.execute_end)))!=0) {
	        sprintf(buf, "Can't find GSDLL_EXECUTE_END, rc = %ld\n", rc);
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    if ((rc = DosQueryProcAddr(gsdll.hmodule, 0, "GSDLL_EXIT", (PFN *)(&gsdll.exit)))!=0) {
	        sprintf(buf, "Can't find GSDLL_EXIT, rc = %ld\n", rc);
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    if ((rc = DosQueryProcAddr(gsdll.hmodule, 0, "GSDLL_GET_BITMAP", (PFN *)(&gsdll.get_bitmap)))!=0) {
	        sprintf(buf, "Can't find GSDLL_GET_BITMAP, rc = %ld\n", rc);
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    if ((rc = DosQueryProcAddr(gsdll.hmodule, 0, "GSDLL_LOCK_DEVICE", (PFN *)(&gsdll.lock_device)))!=0) {
	        sprintf(buf, "Can't find GSDLL_LOCK_DEVICE, rc = %ld\n", rc);
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	}
	else {
	    sprintf(buf, "Can't load Ghostscript DLL %s \nDosLoadModule rc = %ld\n", option.gsdll, rc);
	    gs_addmess(buf);
	    gs_load_dll_cleanup();
	    return FALSE;
	}
	gsdll.callback = gsdll_callback;
	return TRUE;
}


/* free GS DLL */
/* This should only be called when gsdll_execute has returned */
/* TRUE means no error */
BOOL
gs_free_dll(void)
{
char buf[MAXSTR];
APIRET rc;
	if (gsdll.hmodule == (HMODULE)NULL)
	    return TRUE;
	display.epsf_clipped = FALSE;
	rc = DosFreeModule(gsdll.hmodule);
	if (debug) {
	    sprintf(buf,"DosFreeModule returns %ld\n", rc);
	    gs_addmess(buf);
	}
	sprintf(buf,"Unloaded GSDLL\n\n");
	gs_addmess(buf);
	gs_clear_gsdll();
	/* display bitmap is also invalid */
	bitmap.valid = FALSE;
        WinInvalidateRect(hwnd_bmp, (PRECTL)NULL, FALSE);
	return !rc;
}

/* terminate DLL */
BOOL
gsdll_close()
{
	pending.unload = TRUE;
	pending.abort = TRUE;
	pending.now = TRUE;
	pending.next = TRUE;
	/* must now wait until gsdll.state = UNLOADED */
	return TRUE;
}


/* callback routine for GS DLL */
int 
gsdll_callback(int message, char *str, unsigned long count)
{
char buf[MAXSTR];
    switch (message) {
	case GSDLL_STDIN:
	    sprintf(buf,"Callback: STDIN %p %ld - stdin not supported\n", str, count);
	    gs_addmess(buf);
	    return (int)0;
	case GSDLL_STDOUT:
	    if (callback_pstotext(str, count))
		return (int)count;
	    pdf_checktag(str, count);
	    if (str != (char *)NULL)
		gs_addmess_count(str, count);
	    return count;
	case GSDLL_DEVICE:
	    if (debug) {
		sprintf(buf,"Callback: DEVICE %p %s\n", str,
		    count ? "open" : "close");
		gs_addmess(buf);
	    }
	    if (gsdll.device && count) {
	        gs_addmess("GSDLL_CALLBACK: multiple display devices not supported");
		break;
	    }
	    if (gsdll.device && (gsdll.device != (unsigned char *)str)) {
		gs_addmess("GSDLL_CALLBACK: not using that device");
		break;
	    }
	    gsdll.device = count ? str : NULL;
/*
	    if (gsdll.lock_device && gsdll.device) {
		(*gsdll.lock_device)(gsdll.device, 1);
		bitmap.width = bitmap.height = 0;
		bitmap.changed = TRUE;
		(*gsdll.lock_device)(gsdll.device, 0);
	    }
*/
	    /* allow window resize when document first displayed */
	    if (gsdll.device)
		fit_page_enabled = option.fit_page;

	    WinPostMsg(hwnd_frame, WM_GSDEVICE, (MPARAM)gsdll.device, (MPARAM)0);
	    break;
	case GSDLL_SYNC:
	    if (debug) {
		sprintf(buf,"Callback: SYNC %p%s\n", str, ignore_sync ? " ignored" : "");
		gs_addmess(buf);
	    }
	    if (gsdll.device != (unsigned char *)str)
	        break;
	    if (ignore_sync) {
		/* ignore this sync, but not the next */
		ignore_sync = FALSE;
		break;
	    }
	    WinPostMsg(hwnd_frame, WM_GSSYNC, (MPARAM)0, (MPARAM)0);
	    break;
	case GSDLL_PAGE:
	    if (debug) {
		sprintf(buf,"Callback: PAGE %p\n", str);
		gs_addmess(buf);
	    }
	    if (gsdll.device != (unsigned char *)str)
	        break;
	    gsdll.state = PAGE;
	    post_img_message(WM_GSWAIT, IDS_NOWAIT);
	    WinPostMsg(hwnd_frame, WM_GSPAGE, (MPARAM)0, (MPARAM)0);
	    if (!psfile.ispdf) {
		dfclose();
		if (gsdll.input_index < gsdll.input_count)
		    gsdll.input[gsdll.input_index].seek = TRUE;	/* must reseek on reopen */
	    }
	    /* wait until freed */
	    if (multithread) {
		/* wait for semaphore */
		if (!pending.next && !pending.now && !pending.unload)
		    wait_event();
	    }
	    else {
	      /* process messsages until pending changes */
	      int rc;
	      QMSG q_mess;		/* queue message */
	      while (!pending.next && !pending.now && !pending.unload &&
		  ((rc = WinGetMsg(hab, &q_mess, 0L, 0, 0)) != 0))
		  WinDispatchMsg(hab, &q_mess);
	    }
	    if (!psfile.ispdf && !dfreopen()) {
		gs_addmess("Callback: PAGE Can't reopen input file.  File has changed or is missing.\n");
		/* document changed or missing, so force a rescan */
		request_mutex();
		pending.abort = TRUE;
		pending.now = FALSE;
		pending.redisplay = TRUE;
		release_mutex();
	    }
	    pending.next = FALSE;
	    if (psfile.doc == (PSDOC *)NULL) {
		if (pending.abort)
		    psfile.pagenum = 1;
		else
		    psfile.pagenum++;
	    }
	    gsdll.state = BUSY;
	    post_img_message(WM_GSWAIT, IDS_WAITDRAW);
	    break;
	case GSDLL_SIZE:
/*
	    bitmap.width = (count & 0xffff);
	    bitmap.height = ((count>>16) & 0xffff);
	    bitmap.changed = TRUE;
*/

	    /* allow window to be resized without user control */
	    fit_page_enabled = option.fit_page;

	    if (debug) {
		sprintf(buf,"Callback: SIZE %p width=%d height=%d\n", str,
		    (int)(count & 0xffff), (int)((count>>16) & 0xffff) );
		gs_addmess(buf);
	    }
	    break;
	case GSDLL_POLL:
	    if (!multithread)
		peek_message();
	    if (pending.abort)
		return -100;	/* signal an error if we want to abort */
	    break;
	default:
	    sprintf(buf,"Callback: Unknown message=%d\n",message);
	    gs_addmess(buf);
	    break;
    }
    return 0;
}

/* DUMMY TESTING FUNCTIONS */
int
get_message(void)
{
int rc;
  QMSG q_mess;		/* queue message */
  if ( (rc = WinGetMsg(hab, &q_mess, 0L, 0, 0)) != 0)
      WinDispatchMsg(hab, &q_mess);
    return rc;
}


int
peek_message()
{
int rc;
  QMSG q_mess;		/* queue message */
  while ( (rc = WinPeekMsg(hab, &q_mess, 0L, 0, 0, PM_REMOVE)) != 0)
      WinDispatchMsg(hab, &q_mess);
    return rc;
}

void
begin_crit_section(void)
{
    if (multithread)
	DosEnterCritSec();
}

void
end_crit_section(void)
{
    if (multithread)
	DosExitCritSec();
}

void
wait_event(void)
{
ULONG count;
    DosResetEventSem(display.event, &count);
    DosWaitEventSem(display.event, -1);
}

void 
request_mutex(void)
{
    if (multithread)
	DosRequestMutexSem(hmutex_ps, 120000);
}

void 
release_mutex(void)
{
    if (multithread)
	DosReleaseMutexSem(hmutex_ps);
}

/* for pstotext */

int
load_pstotext(void)
{
char dllname[MAXSTR];
char buf[MAXSTR];
APIRET rc;
    /* load pstotext DLL */
    strcpy(dllname, szExePath);
    strcat(dllname, "pstotxt2.dll");
    if (DosLoadModule(buf, sizeof(buf), dllname, &pstotextModule)) {
	gs_addmess("Can't load ");
        gs_addmess(dllname);
        gs_addmess("\n");
	gs_addmess("Please select Options | PStoText | Disable\n");
	return 1;
    }
    if ((rc = DosQueryProcAddr(pstotextModule, 0, "pstotextInit", (PFN *)(&pstotextInit))) !=0) {
	sprintf(buf, "Can't find pstotextInit in %s, rc = %ld\n", dllname, rc);
        gs_addmess(buf);
	DosFreeModule(pstotextModule);
	return 1;
    }
    if ((rc = DosQueryProcAddr(pstotextModule, 0, "pstotextFilter", (PFN *)(&pstotextFilter))) !=0) {
	sprintf(buf, "Can't find pstotextFilter in %s, rc = %ld\n", dllname, rc);
        gs_addmess(buf);
	DosFreeModule(pstotextModule);
	return 1;
    }
    if ((rc = DosQueryProcAddr(pstotextModule, 0, "pstotextExit", (PFN *)(&pstotextExit))) !=0) {
	sprintf(buf, "Can't find pstotextExit in %s, rc = %ld\n", dllname, rc);
        gs_addmess(buf);
	DosFreeModule(pstotextModule);
	return 1;
    }
    if ((rc = DosQueryProcAddr(pstotextModule, 0, "pstotextSetCork", (PFN *)(&pstotextSetCork))) !=0) {
	sprintf(buf, "Can't find pstotextSetCork in %s, rc = %ld\n", dllname, rc);
        gs_addmess(buf);
	DosFreeModule(pstotextModule);
	return 1;
    }

    pstotextInit(&pstotextInstance);
    pstotextCount = 0;

    return 0;
}

int
unload_pstotext(void)
{
    if (pstotextOutfile)
	fclose(pstotextOutfile);
    pstotextOutfile = (FILE *)NULL;
    if (pstotextInstance)
        pstotextExit(pstotextInstance);
    pstotextInstance = NULL;
    DosFreeModule(pstotextModule);
    pstotextModule = (HMODULE)NULL;
    pstotextCount = 0;
    return 0;
}


int
send_pstotext_prolog(HINSTANCE hmodule, int resource)
{  
char *prolog, *p;
APIRET rc;
int code = -1;
	rc = DosGetResource(hmodule, RT_RCDATA, resource, (PPVOID)&prolog);
	if (!rc && (prolog != (char *)NULL) ) {
	    code = 0;
	    p = prolog;
	    while (*p) {
		if (!code)
	            code = gs_execute(p, strlen(p));
		p += strlen(p)+1;
	    }
	    DosFreeResource(prolog);
	}
	else {
	    gs_addmess("error: failed to load pstotext resource\n");
	}
	return code;
}

