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

/* gvwgs.c */
/* Ghostscript DLL interface for GSview */
#include "gvwgs.h"

GSDLL gsdll;
HINSTANCE phInstance;
POINT char_size;
LPSTR szAppName = "GSview Print";
HWND hwnd_client, hwnd_text;
int status_height;


#define WINDOWWIDTH	(80*char_size.x)
#define WINDOWHEIGHT	(24*char_size.y)
HFONT hfont;


#define TWLENGTH 16384
#define TWSCROLL 1024
char twbuf[TWLENGTH];
int twend;

int multithread = FALSE;
BOOL quitnow = FALSE;

char gsarg[2048];
unsigned long gstid;
GSDLL gsdll;

/* command line options */
int debug = 0;	/* don't shut down or delete files if debugging */
char *gsdllname;
char *option;
char *filename;

FILE *infile;

unsigned long lsize, ldone;
int pcdone;
char title[64];

/* forward declarations */
void show_about(void);
int init_window(void);
void gs_addmess(LPSTR str);
int get_args(LPSTR lpszCmdLine, int *pargc, char **pargv[]);
int parse_args(int argc, char *argv[]);
void text_update(void);
LRESULT CALLBACK _export ClientWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
int message_box(char *str, int icon);
void saveas(void);
void gs_thread(void *arg);

#ifdef __WIN32__
#define GetNotification(wParam,lParam) (HIWORD(wParam))
#define SendDlgNotification(hwnd, id, notice) \
    SendMessage((hwnd), WM_COMMAND, MAKELONG((id),(notice)), (LPARAM)GetDlgItem((hwnd),(id)))
#else
#define GetNotification(wParam,lParam) (HIWORD(lParam))
#define SendDlgNotification(hwnd, id, notice) \
    SendMessage((hwnd), WM_COMMAND, id, MAKELPARAM(GetDlgItem((hwnd),(id)),(notice)))
#endif

int PASCAL 
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int cmdShow)
{
    int i, argc;
    char **argv;
    MSG msg;
    phInstance = hInstance;

    if (init_window())
	return 1;
    ShowWindow(hwnd_client, cmdShow);

    get_args(lpszCmdLine, &argc, &argv);
    if (parse_args(argc, argv)) {
#ifdef __WIN32__
	if (multithread)
	    gstid = _beginthread(gs_thread, 65536, NULL);
	else 
#endif
        {
	    /* process messages for window creation */
	    while ((PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE)) != 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	    }
	    gs_thread(0);    /* this will call PeekMessage */
	}
    }

    while (!quitnow && GetMessage(&msg, (HWND)NULL, 0, 0)) {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }

    if (!debug) {
	if (option)
	    unlink(option);
	if (filename)
	    unlink(filename);
    }

    for (i=0; i<argc; i++)
	free(argv[i]);
    free(argv);

    DestroyWindow(hwnd_client);
    return 0;
}

int
init_window(void)
{
WNDCLASS wndclass;
HDC hdc;
TEXTMETRIC tm;
LOGFONT lf;
HMENU hmenu;
RECT rect;

	/* figure out which version of Windows */
#ifdef __WIN32__
DWORD version = GetVersion();
	/* Win32s: bit 15 HIWORD is 1 and bit 14 is 0 */
	/* Win95:  bit 15 HIWORD is 1 and bit 14 is 1 */
	/* WinNT:  bit 15 HIWORD is 0 and bit 14 is 0 */
	/* WinNT with Win95 shell recognised by WinNT + LOBYTE(LOWORD) >= 4 */
	/* check if Windows NT */
	if ((HIWORD(version) & 0x8000)==0)
	    multithread = TRUE;
	/* check if Windows 95 (Windows 4.0) */
	if ( ((HIWORD(version) & 0x8000)!=0) && ((HIWORD(version) & 0x4000)!=0) )
	    multithread = TRUE;
	/* Win32s */
	if ( ((HIWORD(version) & 0x8000)!=0) && ((HIWORD(version) & 0x4000)==0) )
	    multithread = FALSE;
#endif

	/* register the window class */
	wndclass.style = 0;
	wndclass.lpfnWndProc = ClientWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = phInstance;
	wndclass.hIcon = LoadIcon(phInstance, MAKEINTRESOURCE(ID_GSVIEW));
	wndclass.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
	wndclass.hbrBackground =  CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	RegisterClass(&wndclass);

	hdc = GetDC(NULL);  /* desktop */
	memset(&lf, 0, sizeof(lf));
	lf.lfHeight = 8;
	strcpy(lf.lfFaceName, "Helv"); 
	hfont = CreateFontIndirect(&lf);
	SelectObject(hdc, hfont);
	GetTextMetrics(hdc, (LPTEXTMETRIC)&tm);
	ReleaseDC(NULL, hdc);
	char_size.x = tm.tmAveCharWidth;
	char_size.y = tm.tmHeight;
        status_height = 0 /* 6*char_size.y/4 */;

	hwnd_client = CreateWindow(szAppName, (LPSTR)szAppName,
		  WS_OVERLAPPEDWINDOW,
		  CW_USEDEFAULT, CW_USEDEFAULT, 
		  WINDOWWIDTH, WINDOWHEIGHT,
		  NULL, NULL, phInstance, (void FAR *)NULL);

	hmenu = LoadMenu(phInstance, MAKEINTRESOURCE(ID_GSVIEW));
	SetMenu(hwnd_client, hmenu);
	

	
        GetClientRect(hwnd_client, &rect);
	hwnd_text = CreateWindow("EDIT", "",
		WS_CHILD | /* WS_VISIBLE | */ ES_MULTILINE | ES_READONLY | WS_BORDER | WS_VSCROLL | WS_HSCROLL | DS_3DLOOK, 
		rect.left, rect.top,
		rect.right-rect.left, rect.bottom-rect.left-status_height,
		hwnd_client, (HMENU)TEXTWIN_MLE, phInstance, NULL);
	SendMessage(hwnd_text, WM_SETFONT, (WPARAM)hfont, MAKELPARAM(TRUE, 0));
	ShowWindow(hwnd_text, SW_SHOWNA);

	return 0;
}



/* Window Procedure */
LRESULT CALLBACK _export
ClientWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
/*	case WM_CLOSE:   */
	case WM_DESTROY:
		PostQuitMessage(0);
		quitnow = TRUE;
		break;
	case WM_SIZE:
	    if ( (wParam == SIZE_RESTORED) || (wParam == SIZE_MAXIMIZED) ) {
		SetWindowPos(hwnd_text, HWND_TOP, 0, 0, 
		    LOWORD(lParam), HIWORD(lParam)-status_height,
		    SWP_SHOWWINDOW);
	    }
	case WM_TEXTUPDATE:
	    text_update();
	    break;
	case WM_PCUPDATE:
	    sprintf(title, "%d%% - %s", 
		(int)(wParam) > 100 ? 100 : (int)(wParam) , szAppName);
	    SetWindowText(hwnd, title);
	    return 0;
	case WM_COMMAND:
	    switch(LOWORD(wParam)) {
		case IDM_EXIT:
		    PostQuitMessage(0);
		    return 0;
		case IDM_SAVEAS:
		    saveas();
		    return 0;
		case IDM_COPYCLIP:
		    {HGLOBAL hglobal;
		    LPSTR p;
		    DWORD start, end;
		    SendMessage(hwnd_text, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
		    if (start == end) {
			start = 0;
			end = twend;
		    }
		    hglobal = GlobalAlloc(GHND | GMEM_SHARE, end-start+1);
		    if (hglobal == (HGLOBAL)NULL) {
			MessageBeep(-1);
			return(FALSE);
		    }
		    p = GlobalLock(hglobal);
		    if (p == (LPSTR)NULL) {
			MessageBeep(-1);
			return(FALSE);
		    }
#ifdef __WIN32__
		    strncpy(p, twbuf+start, end-start);
#else
		    lstrcpyn(p, twbuf+(int)start, (int)(end-start));
#endif
		    GlobalUnlock(hglobal);
		    OpenClipboard(hwnd_client);
		    EmptyClipboard();
		    SetClipboardData(CF_TEXT, hglobal);
		    CloseClipboard();
		    }
		    return 0;
		case IDM_ABOUT:
		    show_about();
		    return 0;
	    }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

/* Text Window for Ghostscript Messages */
/* uses MS-Windows multiline edit field */

void
text_update(void)
{
    DWORD linecount;
    SendMessage(hwnd_text, WM_SETREDRAW, FALSE, 0);
    SetWindowText(hwnd_text, twbuf);
#ifdef __WIN32__
    /* EM_SETSEL, followed by EM_SCROLLCARET doesn't work */
    linecount = SendMessage(hwnd_text, EM_GETLINECOUNT, (WPARAM)0, (LPARAM)0);
    SendMessage(hwnd_text, EM_LINESCROLL, (WPARAM)0, (LPARAM)linecount-17);
#else
    linecount = SendMessage(hwnd_text, EM_GETLINECOUNT, (WPARAM)0, (LPARAM)0);
    SendMessage(hwnd_text, EM_LINESCROLL, (WPARAM)0, MAKELPARAM(linecount-17, 0));
#endif
    SendMessage(hwnd_text, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(hwnd_text, (LPRECT)NULL, TRUE);
    UpdateWindow(hwnd_text);
}



/* Add string for Ghostscript message window */
void
gs_addmess_count(LPSTR str, int count)
{
char *p;
int i, lfcount;
    /* we need to add \r after each \n, so count the \n's */
    lfcount = 0;
    p = str;
    for (i=0; i<count; i++) {
	if (*p == '\n')
	    lfcount++;
	p++;
    }
    if (count + lfcount >= TWSCROLL)
	return;		/* too large */
    if (count + lfcount + twend >= TWLENGTH-1) {
	/* scroll buffer */
	twend -= TWSCROLL;
	memmove(twbuf, twbuf+TWSCROLL, twend);
    }
    p = twbuf+twend;
    for (i=0; i<count; i++) {
	if (*str == '\n') {
	    *p++ = '\r';
	}
	*p++ = *str++;
    }
    twend += (count + lfcount);
    *(twbuf+twend) = '\0';
    /* tell main thread to update the MLE */
    PostMessage(hwnd_client, WM_TEXTUPDATE, 0, 0);
}

void
gs_addmess(LPSTR str)
{
    gs_addmess_count(str, lstrlen(str));
}


#define MAX_ARGC 10
/* Scan command line and produce argc and argv. */
/* Quotes must be used around arguments which have embedded spaces. */
/* We use this because the inbuilt Borland scanner gives incorrect */
/* results for quoted arguments with embedded spaces. */
int
get_args(LPSTR lpszCmdLine, int *pargc, char **pargv[])
{
int argc = 0;
char **argv;
char *start, *end;
int inquote;
int length;
    argv = (char **)malloc( (MAX_ARGC+1) * sizeof(char *) );
    if (argv == (char **)NULL)
	return 0;
    memset(argv, 0, (MAX_ARGC+1) * sizeof(char *));
    argv[argc] = malloc(MAXSTR+1);
    if (argv[argc] == NULL) {
	free(argv);
	return 0;
    }
    GetModuleFileName(phInstance, argv[argc], MAXSTR);
    argc++;
    start = lpszCmdLine;
    /* skip leading spaces */
    while (*start && (*start == ' '))
	start++;
    end = start;
    while (*end) {
	/* found new argument */
	inquote = FALSE;
	if (*end == '\042') {
	    /* skip leading quote */
	    start++;
	    end++;
	    inquote = TRUE;
	}
	while (*end) {
	    if (!inquote && (*end==' ')) {
		break;
	    }
	    else if (inquote && (*end=='\042')) {
		break;
	    }
	    else
		end++;
	}
	length = (int)(end-start);
	argv[argc] = malloc(length+1);
	if (argv[argc] == NULL) {
	    free(argv);
	    return 0;
	}
	strncpy(argv[argc], start, length);
	argv[argc][length] = '\0';
	if (*end)
	   end++;	/* skip trailing quote or space */
	start = end;
        /* skip leading spaces */
	while (*start && (*start == ' '))
	    start++;
	end = start;
	argc++;
	if (argc == MAX_ARGC)
	    break;
    }
    *pargc = argc;
    *pargv = argv;
    return argc;
}
/* Parse command line arguments */
/* Return 0 if error */
int
parse_args(int argc, char *argv[])
{
char buf[MAXSTR];
char *usage="Usage: gvwgs [/d] dllpath optionfile inputfile\n\
optionfile and inputfile will be deleted on exit\n\
It is intended that gvwgs be called with temporary files\n";
    if (argc >= 2) {
	if ( ((argv[1][0] == '/') || (argv[1][0] == '-')) &&
	     ((argv[1][1] == 'd') || (argv[1][0] == 'D')) ) {
	    debug = 1;
	    sprintf(buf, "argv[0]=\042%s\042\n", argv[0]);
	    gs_addmess(buf);
	    sprintf(buf, "argv[1]=\042%s\042\n", argv[1]);
	    gs_addmess(buf);
	    sprintf(buf, "Shifting arguments\n");
	    gs_addmess(buf);
	    argv++;
	    argc--;
	}
    }

    if (debug || (argc != 4)) {
	int i;
	sprintf(buf, "argc=%d\n", argc);
        gs_addmess(buf);
	for (i=0; i<argc; i++) {
	    sprintf(buf, "argv[%d]=\042%s\042\n", i, argv[i]);
	    gs_addmess(buf);
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
	sprintf(buf, "multithread=%d\n", multithread);
	gs_addmess(buf);
	sprintf(buf, "gsdllname=\042%s\042\n", gsdllname);
	gs_addmess(buf);
	sprintf(buf, "option file=\042%s\042\n", option);
	gs_addmess(buf);
	sprintf(buf, "input file=\042%s\042\n", filename);
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


/* copyright dialog box */
BOOL CALLBACK _export
AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_INITDIALOG:
            SetDlgItemText(hDlg, ABOUT_VERSION, GSVIEW_VERSION);
            return( TRUE);
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDOK:
                    EndDialog(hDlg, TRUE);
                    return(TRUE);
                default:
                    return(FALSE);
            }
        default:
            return(FALSE);
    }
}

void
show_about(void)
{
#ifdef __WIN32__
	DialogBoxParam( phInstance, "AboutDlgBox", hwnd_client, AboutDlgProc, (LPARAM)NULL);
#else
	DLGPROC lpProcAbout;
	lpProcAbout = (DLGPROC)MakeProcInstance((FARPROC)AboutDlgProc, phInstance);
	DialogBoxParam( phInstance, "AboutDlgBox", hwnd_client, lpProcAbout, (LPARAM)NULL);
	FreeProcInstance((FARPROC)lpProcAbout);
#endif
}


/* display message */
int 
message_box(char *str, int icon)
{
	return MessageBox(hwnd_client, str, szAppName, icon | MB_OK);
}

void
saveas(void)
{
FILE *f;
OPENFILENAME ofn;
char szOFilename[256];	/* filename for OFN */

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd_client;
	ofn.lpstrFile = szOFilename;
	ofn.nMaxFile = sizeof(szOFilename);
	ofn.Flags = OFN_PATHMUSTEXIST;

	if (GetSaveFileName(&ofn)) {
	    if ((f = fopen(szOFilename, "wb")) == (FILE *)NULL) {
		message_box("Can't create file", 0);
		return;
	    }
	    fwrite(twbuf, 1, twend, f);
	    fclose(f);
	}
	return;
}

void
gs_clear_gsdll(void)
{
    gsdll.hmodule = (HMODULE)NULL;
    gsdll.revision = NULL;
    gsdll.init = NULL;
    gsdll.execute_begin = NULL;
    gsdll.execute_cont = NULL;
    gsdll.execute_end = NULL;
    gsdll.exit = NULL;
#ifndef __WIN32__
    if (gsdll.callback)
	FreeProcInstance((FARPROC)gsdll.callback);
#endif
    gsdll.callback = NULL;
}

/* free GS DLL */
/* This should only be called when gsdll_execute has returned */
/* TRUE means no error */
BOOL
gs_free_dll(void)
{
char buf[MAXSTR];
int code;
	if (gsdll.hmodule == (HINSTANCE)NULL)
	    return TRUE;
	if (gsdll.exit != NULL) {
	    code = (*gsdll.exit)();
	    sprintf(buf,"gsdll_exit returns %d\n", code);
	    gs_addmess(buf);
	}
	FreeLibrary(gsdll.hmodule);
	sprintf(buf,"Unloaded GSDLL\n");
	gs_addmess(buf);
	gs_clear_gsdll();
	return TRUE;
}

void
gs_load_dll_cleanup(void)
{
char buf[MAXSTR];
    gs_free_dll();
    sprintf(buf, "Can't load Ghostscript DLL %s", gsdllname);
    message_box(buf, 0);
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
	    sprintf(buf,"Callback: STDIN %p %d   stdin not supported\n", str, count);
	    gs_addmess(buf);
	    message_box("GSDLL_CALLBACK: stdin not supported\n",0);
	    return 0;
	case GSDLL_STDOUT:
	    if (str != (char *)NULL)
		gs_addmess_count(str, (int)count);
	    return (int)count;
	case GSDLL_DEVICE:
	case GSDLL_SYNC:
	case GSDLL_PAGE:
	case GSDLL_SIZE:
	    message_box("GSDLL_CALLBACK: display device not supported", 0);
	    break;
	case GSDLL_POLL:
	    if (!multithread) {
		MSG msg;
	        if ((PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE)) != 0) {
		    TranslateMessage(&msg);
		    DispatchMessage(&msg);
	        }
	    }
	    return 0;
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
char buf[MAXSTR];
long revision;
int code;
int gs_argc;
char *gs_argv[3];
	if (gsdll.hmodule)
	    return TRUE;
/* change this to search szExePath first, then system directories */
	gs_clear_gsdll();
	sprintf(buf, "Loading %s\n", gsdllname);
	gs_addmess(buf);
	gsdll.hmodule = LoadLibrary(gsdllname);
 	if (gsdll.hmodule >= (HINSTANCE)HINSTANCE_ERROR) {
	    gs_addmess("Loaded Ghostscript DLL\n");
	    gsdll.revision = (PFN_gsdll_revision) GetProcAddress(gsdll.hmodule, "gsdll_revision");
	    if (gsdll.revision == NULL) {
	        sprintf(buf, "Can't find gsdll_revision\n");
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

	gs_argv[0] = gsdllname;
	gs_argv[1] = gsarg;
	gs_argv[2] = NULL;
	gs_argc = 2;

	code = gsdll.init(gsdll.callback, hwnd_client, gs_argc, gs_argv);
	if (debug) {
	    sprintf(buf,"gsdll_init returns %d\n", code);
	    gs_addmess(buf);
	}
	if (code) {
	    gs_load_dll_cleanup();
	    return !code;
	}
	return !code;
}


/* Thread which loads Ghostscript DLL and sends input file to DLL */
#ifdef __BORLANDC__
#pragma argsused
#endif
void 
gs_thread(void *arg)
{
char buf[MAXSTR];
int len;
int code;

    if (!gs_load_dll())
	return;
    gsdll.execute_begin();

    while ((len = fread(buf, 1, sizeof(buf), infile)) != 0) {
	code = gsdll.execute_cont(buf, len);
	ldone += len;
	if (pcdone != (int)(ldone / lsize)) {
	    pcdone = (int)(ldone / lsize);
	    PostMessage(hwnd_client, WM_PCUPDATE, (WPARAM)pcdone, 0);
	}
	if (code) {
	    sprintf(buf, "gsdll_execute_cont returns %d\n", code);
	    gs_addmess(buf);
	    break;
	}
	if (!multithread) {
	    MSG msg;
	    while ((PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE)) != 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	    }
	}
    }
    fclose(infile);

    gsdll.execute_end();
    gs_free_dll();


    /* tell main thread to shut down */
    if ((code == 0) && (!debug))
	PostMessage(hwnd_client, WM_QUIT, 0, 0);
    else 
	ShowWindow(hwnd_client, SW_SHOWNORMAL);
/*
*/
}

