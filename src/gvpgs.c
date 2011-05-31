/* Copyright (C) 1996-1998, Ghostgum Software Pty Ltd.  All rights reserved.
  
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

/* gvpgs.c */
/* Ghostscript DLL interface for GSview */

/* Usage: gvpgs [/d] dllname option filename
 *  dllname   pathname of GS DLL, e.g. c:\gs3.33\gsdll2.dll
 *  option    name of file containing options, one per line
 *            This will be given to GS as @option
 *  filename  input file for GS
 *
 * WARNING:  option and filename WILL BE DELETED on exit!
 *           gvpgs is meant to be called with temporary files
 */

#define DEBUG

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_DOSERRORS
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "gvcrc.h"
#include "gsdll.h"
#include "gvpgs.h"

MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG mess, MPARAM mp1, MPARAM mp2);
void text_update(void);
void gs_addmess(char *str);
void gs_addmess_count(char *str, int count);
int parse_arg(int argc, char *argv[]);
void gs_thread(void *arg);
void show_about(void);
void saveas(void);
int init_window(HAB hab);
int message_box(char *str, int icon);

#define TWLENGTH 16384
#define TWSCROLL 1024
char twbuf[TWLENGTH];
int twend;
char stdinbuf[MAXSTR];

char *szAppName = "GSview Print";
unsigned long lsize, ldone;
int pcdone;
char title[64];

HWND hwnd_frame;
HWND hwnd_client;
HWND hwnd_text;
POINTL char_size;
int status_height;

char gsarg[2048];
TID gstid;
GSDLL gsdll;
HMTX text_mutex;

/* command line options */
int debug = 0;	/* don't shut down or delete files if debugging */
char *gsdllname;
char *option;
char *filename;

FILE *infile;
char *class = "GSPGS_CLASS";

ULONG frame_flags = 
	    FCF_TITLEBAR |	/* have a title bar */
            FCF_SIZEBORDER |	/* have a sizeable window */
            FCF_MINMAX |	/* have a min and max button */
            FCF_SYSMENU | 	/* include a system menu */
	    FCF_TASKLIST |	/* show it in window list */
	    FCF_ICON |		/* Load icon from resources */
	    FCF_MENU;		/* Load menu from resources */

int 
main(int argc, char *argv[])
{
HAB hab;
HMQ hmq;
QMSG qmsg;

    hab = WinInitialize(0);
    hmq = WinCreateMsgQueue(hab, 0);

    if (!init_window(hab)) {

	if (parse_arg(argc, argv)) {
	    gstid = _beginthread(gs_thread, NULL, 131072, NULL);
	}

	text_update();
    }

    while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
	WinDispatchMsg(hab, &qmsg);

    if (!debug) {
	if (option)
	    unlink(option);
	if (filename)
	    unlink(filename);
    }

    if (gstid)
	DosKillThread(gstid);
    DosCloseMutexSem(text_mutex);
    WinDestroyWindow(hwnd_frame);
    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);
    return 0;
} 


/* Create windows */
int
init_window(HAB hab)
{
HPS hps;
FONTMETRICS fm;
SWP swp;
RECTL rect;

    if (DosCreateMutexSem(NULL, &text_mutex, 0, FALSE) != 0)
	return FALSE;

    if (!WinRegisterClass(	/* register this window class */
  	hab,			/* anchor block */
  	(PSZ)class,		/* class name */
  	(PFNWP) ClientWndProc,	/* window function */
  	CS_SIZEREDRAW,			/* window style */
  	0))			/* no storage */
	{
	message_box("Can't register window class", 0);
	return -1;
    }

    WinRegisterClass(hab, "GVPGS", ClientWndProc, 0L, 0);

    hwnd_frame = WinCreateStdWindow(
  	HWND_DESKTOP,		/* window type */
	0,			/* frame style is not WS_VISIBLE */
	&frame_flags,
  	(PSZ)class,		/* client class */
  	(PSZ)szAppName,		/* title */
  	WS_VISIBLE,		/* client style */
  	0,			/* resource module */
  	ID_GSVIEW,		/* resource identifier */
  	&hwnd_client);		/* pointer to client */

    /* find out default text size */
    hps = WinGetPS(hwnd_client);
    GpiQueryFontMetrics(hps, sizeof(FONTMETRICS), &fm);
    char_size.y = fm.lMaxAscender + fm.lMaxDescender + fm.lExternalLeading + 2;
    char_size.x = fm.lAveCharWidth;
    WinReleasePS(hwnd_client);

    /* position window */
    WinQueryTaskSizePos(hab, 0, &swp);
    swp.y += swp.cy;	/* keep top left in suggest position */
    swp.cx = 70 * char_size.x;
    swp.cy = 15 * char_size.y;
    swp.y -= swp.cy;
    swp.fl = SWP_MOVE | SWP_SIZE;
    WinSetWindowPos(hwnd_frame, HWND_TOP,
	swp.x, swp.y, swp.cx, swp.cy, swp.fl);

    WinQueryWindowRect(hwnd_client, &rect);

    status_height = 0 /* char_size.y */;
    hwnd_text = WinCreateWindow(
    	hwnd_client,
    	WC_MLE,
    	"",
        MLS_BORDER | MLS_READONLY | MLS_HSCROLL | MLS_VSCROLL |
	WS_VISIBLE | WS_TABSTOP,
	0, status_height, 
        rect.xRight-rect.xLeft, rect.yTop-rect.yBottom - status_height,
	hwnd_client,
	HWND_TOP,
	TEXTWIN_MLE,
	NULL,
	NULL);

    /* show window */
    swp.fl = SWP_ACTIVATE | SWP_SHOW;
    WinSetWindowPos(hwnd_frame, HWND_TOP,
	swp.x, swp.y, swp.cx, swp.cy, swp.fl);

    WinSetFocus(HWND_DESKTOP, hwnd_client);

    return 0;
}

/* display message box */
int
message_box(char *str, int icon)
{
  	return WinMessageBox(HWND_DESKTOP, hwnd_client ? hwnd_client : HWND_DESKTOP, 
		str, "GVPGS", 0, icon | MB_MOVEABLE | MB_OK);
}

/* Update MLE to match twbuf */
void
text_update()
{
IPT ipt = 0;
    if (hwnd_text == (HWND)NULL)
	return;
    if (DosRequestMutexSem(text_mutex, 10000) == ERROR_TIMEOUT)
	DosBeep(100, 100);
    WinEnableWindowUpdate(hwnd_text, FALSE);
    WinSendMsg( hwnd_text, MLM_DISABLEREFRESH, 0, 0);
    /* delete current contents */
    ipt = (IPT)WinSendMsg( hwnd_text, MLM_QUERYTEXTLENGTH, 0, 0); /* get current length */
    WinSendMsg( hwnd_text, MLM_SETSEL, (MPARAM)0, (MPARAM)ipt);
    WinSendMsg( hwnd_text, MLM_CLEAR, (MPARAM)0, (MPARAM)0);
    /* add udpated buffer */
    ipt = 0;
    WinSendMsg(hwnd_text, MLM_SETIMPORTEXPORT, (MPARAM)twbuf, (MPARAM)sizeof(twbuf));
    WinSendMsg( hwnd_text, MLM_IMPORT, (MPARAM)&ipt, (MPARAM)twend);
    WinSendMsg( hwnd_text, MLM_SETSEL, (MPARAM)twend, (MPARAM)twend);
    WinSendMsg( hwnd_text, MLM_ENABLEREFRESH, 0, 0);
    WinEnableWindowUpdate(hwnd_text, TRUE);
    DosReleaseMutexSem(text_mutex);
    WinInvalidateRect(hwnd_text, (PRECTL)NULL, TRUE);
    WinUpdateWindow(hwnd_text);
}


/* The main WndProc */
MRESULT EXPENTRY 
ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch(msg) {
	case WM_CREATE:
	    /* should create child windows here */
    	    break;
	case WM_TEXTUPDATE:
	    text_update();
	    break;
	case WM_PCUPDATE:
	    sprintf(title, "%d%% - %s", 
		(int)(mp1) > 100 ? 100 : (int)(mp1) , szAppName);
	    WinSetWindowText( hwnd_frame, title);
	    return 0;
	case WM_SIZE:
	    {SWP swp;
	    swp.y = status_height;
	    swp.x = 0;
	    swp.cx = SHORT1FROMMP(mp2);
	    swp.cy = SHORT2FROMMP(mp2) - status_height;
	    swp.fl = SWP_MOVE | SWP_SIZE;
	    WinSetWindowPos(hwnd_text, HWND_TOP,
	    swp.x, swp.y, swp.cx, swp.cy, swp.fl);
	    }
	    break;
        case WM_COMMAND:
            switch(SHORT1FROMMP(mp1)) {
                case IDM_SAVEAS:
		    saveas();
                    break;
		case IDM_EXIT:
		    WinPostMsg(hwnd, WM_CLOSE, 0, 0);
                    break;
		case IDM_ABOUT:
		    show_about();
		    return (MRESULT)TRUE;
		case IDM_COPYCLIP:
		    {MRESULT mr; 
	    	    mr = WinSendMsg( WinWindowFromID(hwnd, TEXTWIN_MLE),
	    		MLM_QUERYSEL, (MPARAM)MLFQS_MINMAXSEL, (MPARAM)0);
		    if (SHORT1FROMMP(mr) == SHORT2FROMMP(mr)) /* no selection so select all */
	    		WinSendMsg( WinWindowFromID(hwnd, TEXTWIN_MLE),
	    		    MLM_SETSEL, 0, (MPARAM)strlen(twbuf));
	    	    WinSendMsg( WinWindowFromID(hwnd, TEXTWIN_MLE),
	    		MLM_COPY, (MPARAM)0, (MPARAM)0);
		    WinSendMsg( WinWindowFromID(hwnd, TEXTWIN_MLE),
		        MLM_SETSEL, (MPARAM)strlen(twbuf), (MPARAM)strlen(twbuf));
		    }
		    return (MRESULT)TRUE;
            }
            break;
	case WM_ERASEBACKGROUND:
	    WinFillRect((HPS)mp1, (PRECTL)mp2, SYSCLR_DIALOGBACKGROUND);
	    return (MRESULT)FALSE;	/* we have erased it */
    }
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}


/* Add string for Ghostscript message window */
void
gs_addmess_count(char *str, int count)
{
    if (DosRequestMutexSem(text_mutex, 10000) == ERROR_TIMEOUT)
	DosBeep(100, 100);
    if (count >= TWSCROLL)
	return;		/* too large */
    if (count + twend >= TWLENGTH-1) {
	/* scroll buffer */
	twend -= TWSCROLL;
	memmove(twbuf, twbuf+TWSCROLL, twend);
    }
    memmove(twbuf+twend, str, count);
    twend += count;
    *(twbuf+twend) = '\0';
    DosReleaseMutexSem(text_mutex);
    /* tell main thread to update the MLE */
    WinPostMsg(hwnd_client, WM_TEXTUPDATE, 0, 0);
}

void
gs_addmess(char *str)
{
    gs_addmess_count(str, strlen(str));
}

/* Parse command line arguments */
/* Return 0 if error */
int
parse_arg(int argc, char *argv[])
{
char buf[MAXSTR];
char *usage="Usage: gvpgs [/d] dllpath optionfile inputfile\n\
optionfile and inputfile will be deleted on exit\n\
It is intended that gvpgs be called with temporary files\n";
    if (argc == 5) {
	if ( ((argv[1][0] == '/') || (argv[1][0] == '-')) &&
	     ((argv[1][1] == 'd') || (argv[1][0] == 'D')) ) {
	    debug = 1;
	    argv++;
	    argc--;
	}
	else {
	    gs_addmess(usage);
	    return FALSE;
	}
    }

    if (argc != 4) {
	gs_addmess(usage);
	return FALSE;
    }

    gsdllname = argv[1];
    option = argv[2];
    filename = argv[3]; 
    if (debug) {
	sprintf(buf, "gsdllname=%s\n", gsdllname);
	gs_addmess(buf);
	sprintf(buf, "option file=%s\n", option);
	gs_addmess(buf);
	sprintf(buf, "input file=%s\n", filename);
	gs_addmess(buf);
    }
    infile = fopen(filename, "rb");
    if (infile == (FILE *)NULL) {
	gs_addmess(usage);
	sprintf(gsarg, "Can't find file '%s'\n", filename);
	gs_addmess(gsarg);
	return FALSE;
    }

    /* find length of file */
    fseek(infile, 0L, SEEK_END);
    lsize = ftell(infile);
    lsize = lsize / 100;	/* to get percent values */
    if (lsize <= 0)
	lsize = 1;
    fseek(infile, 0L, SEEK_SET);
    ldone = 0;

    /* build options list */
    strcpy(gsarg, "@");
    strcat(gsarg, option);
    gsarg[strlen(gsarg) + 1] = '\0';

    return TRUE;
}

/* free GS DLL */
/* This should only be called when gsdll_execute_cont has returned */
/* TRUE means no error */
BOOL
gs_free_dll(void)
{
char buf[MAXSTR];
APIRET rc = 0;
int code;

	if (gsdll.hmodule == (HMODULE)NULL)
	    return TRUE;
	if (gsdll.exit != NULL) {
	    code = (*gsdll.exit)();
#ifdef DEBUG
	    sprintf(buf,"gsdll_exit returns %d\n", code);
	    gs_addmess(buf);
#endif
	}
	if (gsdll.hmodule) {
	    rc = DosFreeModule(gsdll.hmodule);
#ifdef DEBUG
	    sprintf(buf,"DosFreeModule returns %ld\n", rc);
	    gs_addmess(buf);
#endif
	}
	gsdll.hmodule = (HMODULE)NULL;
	/* be extra safe */
	gsdll.revision = NULL;
	gsdll.init = NULL;
	gsdll.execute_begin = NULL;
	gsdll.execute_cont = NULL;
	gsdll.execute_end = NULL;
	gsdll.exit = NULL;
	return !rc;
}

/* Cleanup when DLL couldn't be loaded */
void
gs_load_dll_cleanup(void)
{
char buf[MAXSTR];
    gs_free_dll();
    sprintf(buf, "Can't load DLL %s", gsdllname);
    message_box(buf, 0);
}


/* callback routine for GS DLL */
int 
gsdll_callback(int message, char *str, unsigned long count)
{
char buf[MAXSTR];
    switch (message) {
	case GSDLL_STDIN:
	    sprintf(buf,"Callback: STDIN %p %ld   stdin not supported\n", str, count);
	    gs_addmess(buf);
	    message_box("GSDLL_CALLBACK: stdin not supported\n",0);
	    return 0;
	case GSDLL_STDOUT:
	    if (str != (char *)NULL)
		gs_addmess_count(str, count);
	    return count;
	case GSDLL_DEVICE:
	case GSDLL_SYNC:
	case GSDLL_PAGE:
	case GSDLL_SIZE:
	    message_box("GSDLL_CALLBACK: os2dll device not supported", 0);
	    break;
	default:
	    sprintf(buf,"Callback: Unknown message=%d\n",message);
	    gs_addmess(buf);
	    break;
    }
    return 0;
}


/* load GS DLL if not already loaded */
/* return TRUE if OK */
BOOL
gs_load_dll(void)
{
APIRET rc;
char buf[MAXSTR+40];
long revision;
const char *shortname;
const char *dllname;
	if (gsdll.hmodule)
	    return TRUE;
	dllname = gsdllname;
	sprintf(buf, "Trying to load %s\n", dllname);
	if (debug)
	    gs_addmess(buf);
	memset(buf, 0, sizeof(buf));
	rc = DosLoadModule(buf, sizeof(buf), dllname, &gsdll.hmodule);
	if (rc) {
	    /* failed */
	    /* try once more, this time on system search path */
	    if ((shortname = strrchr((char *)gsdllname, '\\')) == (const char *)NULL)
		shortname = gsdllname;
	    dllname = shortname;
	    sprintf(buf, "Trying to load %s\n", dllname);
	    if (debug)
		gs_addmess(buf);
	    rc = DosLoadModule(buf, sizeof(buf), dllname, &gsdll.hmodule);
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
	    gsdll.revision(NULL, NULL, &revision, NULL);
	    if ( (revision < GS_REVISION_MIN) || (revision > GS_REVISION_MAX) ) {
		sprintf(buf, "Wrong version of DLL found.\n  Found version %ld\n  Need version  %ld - %ld\n", 
			revision, (long)GS_REVISION_MIN, (long)GS_REVISION_MAX);
		gs_addmess(buf);
		gs_load_dll_cleanup();
		return FALSE;
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
	}
	else {
	    sprintf(buf, "Can't load Ghostscript DLL %s \nDosLoadModule rc = %ld\n", gsdllname, rc);
	    gs_addmess(buf);
	    gs_load_dll_cleanup();
	    return FALSE;
	}
	return TRUE;
}


/* Thread which loads Ghostscript DLL and sends input file to DLL */
void 
gs_thread(void *arg)
{
char buf[MAXSTR];
int len;
int code;
HAB hab;
int gs_argc;
char *gs_argv[3];

    hab = WinInitialize(0);

    if (!gs_load_dll())
	return;

    gs_argv[0] = gsdllname;
    gs_argv[1] = gsarg;
    gs_argv[2] = NULL;
    gs_argc = 2;

    code = gsdll.init(gsdll_callback, hwnd_client, gs_argc, gs_argv);
    if (debug) {
	sprintf(buf,"gsdll_init returns %d\n", code);
	gs_addmess(buf);
    }
    if (!code) {
	code = gsdll.execute_begin();

	if (!code) {
	  while ((len = fread(buf, 1, sizeof(buf), infile)) != 0) {
	    code = gsdll.execute_cont(buf, len);
	    ldone += len;
	    if (pcdone != (int)(ldone / lsize)) {
		pcdone = (int)(ldone / lsize);
		WinPostMsg(hwnd_client, WM_PCUPDATE, (MPARAM)pcdone, 0);
	    }
	    if (code) {
		sprintf(buf, "gsdll_execute_cont returns %d\n", code);
		gs_addmess(buf);
		break;
	    }
	  }
	  if (!code)
	      gsdll.execute_end();
	}
	fclose(infile);

	gsdll.execute_end();
    }
    gs_free_dll();

    WinTerminate(hab);

    /* tell main thread to shut down */
    if ((code == 0) && (!debug))
	WinPostMsg(hwnd_client, WM_QUIT, 0, 0);
    else {
        WinSetWindowPos(hwnd_frame, HWND_TOP, 0, 0, 0, 0, 
		SWP_ACTIVATE | SWP_SHOW | SWP_RESTORE);
    }
    gstid = 0;
}


/* About Dialog Box */
MRESULT EXPENTRY AboutDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch(msg) {
    case WM_INITDLG:
	WinSetWindowText( WinWindowFromID(hwnd, ABOUT_VERSION),
	    	GSVIEW_DOT_VERSION );
	break;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_OK:
          WinDismissDlg(hwnd, TRUE);
          return (MRESULT)TRUE;
      }
      break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}	

void
show_about(void)
{
	WinDlgBox(HWND_DESKTOP, hwnd_frame, AboutDlgProc, 0, IDD_ABOUT, 0);
}

void
saveas(void)
{
FILEDLG FileDlg;
FILE *f;
	memset(&FileDlg, 0, sizeof(FILEDLG));
	FileDlg.cbSize = sizeof(FILEDLG);
	FileDlg.fl = FDS_CENTER | FDS_SAVEAS_DIALOG;
	WinFileDlg(HWND_DESKTOP, hwnd_frame, &FileDlg);
	if (FileDlg.lReturn == DID_OK) {
	    if ((f = fopen(FileDlg.szFullFile, "wb")) == (FILE *)NULL) {
		message_box("Can't create file", 0);
		return;
	    }
	    fwrite(twbuf, 1, twend, f);
	    fclose(f);
	}
	return;
}

