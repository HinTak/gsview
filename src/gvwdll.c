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

/* gvwdll.c */
/* GS DLL associated routines for MS-Windows */
#include "gvwin.h"

/* forward references */
int get_gs_input(char FAR *buf, int blen);

FARPROC lpfnCallback;

void
gs_clear_gsdll(void)
{
    gsdll.hmodule = (HMODULE)NULL;
    gsdll.valid = FALSE;
    gsdll.device = NULL;
    gsdll.revision = NULL;
    gsdll.init = NULL;
    gsdll.execute_begin = NULL;
    gsdll.execute_cont = NULL;
    gsdll.execute_end = NULL;
    gsdll.exit = NULL;
    gsdll.lock_device = NULL;
    gsdll.copy_dib = NULL;
    gsdll.copy_palette = NULL;
    gsdll.draw = NULL;
    gsdll.get_bitmap_row = NULL;
#ifndef __WIN32__
    if (gsdll.callback)
	FreeProcInstance((FARPROC)gsdll.callback);
#endif
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
char buf[MAXSTR+40];
long revision;
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
	/* Try to load DLL first with given path */
	gsdll.hmodule = LoadLibrary(dllname);
	if (gsdll.hmodule < (HINSTANCE)HINSTANCE_ERROR) {
	    /* failed */
	    /* try again, with path of EXE */
	    if ((shortname = strrchr((char *)option.gsdll, '\\')) == (const char *)NULL)
		shortname = option.gsdll;
	    GetModuleFileName(phInstance, fullname, sizeof(fullname));
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
	    gsdll.hmodule = LoadLibrary(dllname);
	    if (gsdll.hmodule < (HINSTANCE)HINSTANCE_ERROR) {
		/* failed again */
		/* try once more, this time on system search path */
		dllname = shortname;
		sprintf(buf, "Trying to load %s\n", dllname);
		if (debug)
		    gs_addmess(buf);
		gsdll.hmodule = LoadLibrary(dllname);
	    }
	}

 	if (gsdll.hmodule >= (HINSTANCE)HINSTANCE_ERROR) {
	    sprintf(buf, "Loaded Ghostscript DLL %s\n", dllname);
	    gs_addmess(buf);
	    gsdll.revision = (PFN_gsdll_revision) GetProcAddress(gsdll.hmodule, "gsdll_revision");
	    if (gsdll.revision == NULL) {
		sprintf(buf, "Can't find gsdll_revision\n");
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    /* check DLL version */
	    gsdll.revision(NULL, NULL, &revision, NULL);
	    if ( (revision < GS_REVISION) || (revision > GS_REVISION_MAX) ) {
		sprintf(buf, "Wrong version of DLL found.\n  Found version %ld\n  Need version  %ld\n", revision, (long)GS_REVISION);
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    /* continue loading other functions */
	    gsdll.init = (PFN_gsdll_init) GetProcAddress(gsdll.hmodule, "gsdll_init");
	    if (gsdll.init == NULL) {
	        sprintf(buf, "Can't find gsdll_init\n");
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    gsdll.execute_begin = (PFN_gsdll_execute_begin) GetProcAddress(gsdll.hmodule, "gsdll_execute_begin");
	    if (gsdll.execute_begin == NULL) {
	        sprintf(buf, "Can't find gsdll_execute_begin\n");
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    gsdll.execute_cont = (PFN_gsdll_execute_cont) GetProcAddress(gsdll.hmodule, "gsdll_execute_cont");
	    if (gsdll.execute_cont == NULL) {
	        sprintf(buf, "Can't find gsdll_execute_cont\n");
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    gsdll.execute_end = (PFN_gsdll_execute_end) GetProcAddress(gsdll.hmodule, "gsdll_execute_end");
	    if (gsdll.execute_end == NULL) {
	        sprintf(buf, "Can't find gsdll_execute_end\n");
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    gsdll.lock_device = (PFN_gsdll_lock_device) GetProcAddress(gsdll.hmodule, "gsdll_lock_device");
	    if (gsdll.lock_device == NULL) {
	        sprintf(buf, "Can't find gsdll_lock_device\n");
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    gsdll.copy_dib = (PFN_gsdll_copy_dib) GetProcAddress(gsdll.hmodule, "gsdll_copy_dib");
	    if (gsdll.copy_dib == NULL) {
	        sprintf(buf, "Can't find gsdll_copy_dib\n");
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    gsdll.copy_palette = (PFN_gsdll_copy_palette) GetProcAddress(gsdll.hmodule, "gsdll_copy_palette");
	    if (gsdll.copy_palette == NULL) {
	        sprintf(buf, "Can't find gsdll_copy_palette\n");
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    gsdll.draw = (PFN_gsdll_draw) GetProcAddress(gsdll.hmodule, "gsdll_draw");
	    if (gsdll.draw == NULL) {
	        sprintf(buf, "Can't find gsdll_draw\n");
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    gsdll.get_bitmap_row = (PFN_gsdll_get_bitmap_row) GetProcAddress(gsdll.hmodule, "gsdll_get_bitmap_row");
	    if (gsdll.get_bitmap_row == NULL) {
	        sprintf(buf, "Can't find gsdll_get_bitmap_row\n");
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	    gsdll.exit = (PFN_gsdll_exit) GetProcAddress(gsdll.hmodule, "gsdll_exit");
	    if (gsdll.exit == NULL) {
	        sprintf(buf, "Can't find gsdll_exit\n");
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
	    }
	}
	else {
	    gs_addmess("Can't load Ghostscript DLL\n");
	    gs_load_dll_cleanup();
	    return FALSE;
	}
#ifdef __WIN32__
	gsdll.callback = gsdll_callback;
#else
	gsdll.callback = (GSDLL_CALLBACK)MakeProcInstance((FARPROC)gsdll_callback, phInstance);
#endif

    return TRUE;
}


/* free GS DLL */
/* This should only be called when gsdll_execute has returned */
/* TRUE means no error */
BOOL
gs_free_dll(void)
{
char buf[MAXSTR];
	if (gsdll.hmodule == (HINSTANCE)NULL)
	    return TRUE;
	begin_crit_section();
	display.epsf_clipped = FALSE;
	FreeLibrary(gsdll.hmodule);
	gs_clear_gsdll();
	/* no bitmap exists */
        bitmap.width = 0;
        bitmap.height = 0;
	bitmap.changed = 0;
	end_crit_section();
	sprintf(buf,"Unloaded GSDLL\n\n");
	gs_addmess(buf);
	return TRUE;
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
#ifdef __WIN32__
int _export 
gsdll_callback(int message, char *str, unsigned long count)
#else
int _far _export
gsdll_callback(int message, char FAR *str, unsigned long count)
#endif
{
char buf[MAXSTR];
    switch (message) {
	case GSDLL_STDIN:
	    sprintf(buf,"Callback: STDIN %p %d - stdin not supported\n", str, count);
	    gs_addmess(buf);
	    return (int)0;
	case GSDLL_STDOUT:
	    if (callback_pstotext(str, count))
		return (int)count;
	    pdf_checktag(str, (int)count);
	    if (str != (char *)NULL)
		gs_addmess_count(str, (int)count);
	    return (int)count;
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
	    gsdll.device = count ? (unsigned char *)str : NULL;
	    bitmap.width = bitmap.height = 0;
	    bitmap.changed = TRUE;
	    PostMessage(hwndimg, WM_GSDEVICE, (WPARAM)0, (LPARAM)0);
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
	    PostMessage(hwndimg, WM_GSSYNC, (WPARAM)0, (LPARAM)0);
	    break;
	case GSDLL_PAGE:
	    {MSG msg;
	    gsdll.state = PAGE;
	    if (debug) {
		sprintf(buf,"Callback: PAGE %p\n", str);
		gs_addmess(buf);
	    }
	    if (gsdll.device != (unsigned char *)str)
	        break;
	    PostMessage(hwndimg, WM_GSPAGE, (WPARAM)0, (LPARAM)0);
	    if (!psfile.ispdf) {
		dfclose();
		if (gsdll.input_index < gsdll.input_count)
		    gsdll.input[gsdll.input_index].seek = TRUE;	/* must reseek on reopen */
	    }
	    post_img_message(WM_GSWAIT, IDS_NOWAIT);
	    if (multithread) {
		/* wait for semaphore */
		if (!pending.next && !pending.now && !pending.unload)
		    wait_event();
	    }
	    else {
		/* process message queue until something is pending */
		while (!pending.next && !pending.now && !pending.unload && GetMessage(&msg, (HWND)NULL, 0, 0)) {
		    if ((hDlgModeless == 0) || !IsDialogMessage(hDlgModeless, &msg)) {
			if (!TranslateAccelerator(hwndimg, haccel, &msg)) {
			    TranslateMessage(&msg);
			    DispatchMessage(&msg);
			}
		    }
		}
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
	    }
	    break;
	case GSDLL_SIZE:
	    bitmap.width = ((WORD)count & 0xffff);
	    bitmap.height = ((WORD)((count)>>16) & 0xffff);
	    bitmap.changed = TRUE;
	    if (debug) {
		sprintf(buf,"Callback: SIZE %p width=%d height=%d\n", str,
		    (count & 0xffff), ((count>>16) & 0xffff) );
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

int
get_message(void)
{
MSG msg;
int status;
    if ((status = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0) {
	if ((hDlgModeless == 0) || !IsDialogMessage(hDlgModeless, &msg)) {
	    if (!TranslateAccelerator(hwndimg, haccel, &msg)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	    }
	}
    }
    return status;
}


int
peek_message()
{
MSG msg;
int status;
    while ((status = PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE)) != 0) {
	if ((hDlgModeless == 0) || !IsDialogMessage(hDlgModeless, &msg)) {
	    if (!TranslateAccelerator(hwndimg, haccel, &msg)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	    }
	}
    }
    return status;
}


static int crit_count = 0;
void
begin_crit_section(void)
{
    crit_count++;
#ifdef __WIN32__
    if (multithread)
	EnterCriticalSection(&crit_sec);
#endif
}

void
end_crit_section(void)
{
    crit_count--;
#ifdef __WIN32__
    if (multithread)
	LeaveCriticalSection(&crit_sec);
#endif
}

void
wait_event(void)
{
#ifdef __WIN32__
    if (multithread) {
	ResetEvent(display.event);
	WaitForSingleObject(display.event, INFINITE);
    }
#endif
}

void 
request_mutex(void)
{
#ifdef __WIN32__
    if (multithread)
	WaitForSingleObject(hmutex_ps, 120000);
#endif
}

void 
release_mutex(void)
{
#ifdef __WIN32__
    if (multithread)
	ReleaseMutex(hmutex_ps);
#endif
}

/* for pstotext */

int
load_pstotext(void)
{
char dllname[MAXSTR];
    /* load pstotext DLL */
    strcpy(dllname, szExePath);
#ifdef __WIN32__
    strcat(dllname, "pstotxt3.dll");
#else
    strcat(dllname, "pstotxt1.dll");
#endif
    pstotextModule = LoadLibrary(dllname);
    if (pstotextModule < (HINSTANCE)HINSTANCE_ERROR) {
	gs_addmess("Can't load ");
        gs_addmess(dllname);
        gs_addmess("\n");
	gs_addmess("Please select Options | PStoText | Disable\n");
	return 1;
    }
    pstotextInit = (PFN_pstotextInit) GetProcAddress(pstotextModule, "pstotextInit");
    if (pstotextInit == (PFN_pstotextInit)NULL) {
	gs_addmess("Can't find pstotextInit() in ");
	gs_addmess(dllname);
        gs_addmess("\n");
	FreeLibrary(pstotextModule);
	return 1;
    }
    pstotextFilter = (PFN_pstotextFilter) GetProcAddress(pstotextModule, "pstotextFilter");
    if (pstotextFilter == (PFN_pstotextFilter)NULL) {
	gs_addmess("Can't find pstotextFilter() in ");
	gs_addmess(dllname);
        gs_addmess("\n");
	FreeLibrary(pstotextModule);
	return 1;
    }
    pstotextExit = (PFN_pstotextExit) GetProcAddress(pstotextModule, "pstotextExit");
    if (pstotextExit == (PFN_pstotextExit)NULL) {
	gs_addmess("Can't find pstotextExit() in ");
	gs_addmess(dllname);
        gs_addmess("\n");
	FreeLibrary(pstotextModule);
	return 1;
    }
    pstotextSetCork = (PFN_pstotextSetCork) GetProcAddress(pstotextModule, "pstotextSetCork");
    if (pstotextSetCork == (PFN_pstotextSetCork)NULL) {
	gs_addmess("Can't find pstotextSetCork() in ");
	gs_addmess(dllname);
        gs_addmess("\n");
	FreeLibrary(pstotextModule);
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
    FreeLibrary(pstotextModule);
    pstotextModule = NULL;
    pstotextCount = 0;
    return 0;
}

int
send_pstotext_prolog(HINSTANCE hmodule, int resource)
{  
HGLOBAL hglobal;
LPSTR prolog;
int code = -1;
	hglobal = LoadResource(hmodule, 
	    FindResource(hmodule, (LPSTR)resource, RT_RCDATA));
	if ( (prolog = (LPSTR)LockResource(hglobal)) != (LPSTR)NULL) {
	    code = gs_execute(prolog, lstrlen(prolog));
	    FreeResource(hglobal);
	}
	return code;
}

