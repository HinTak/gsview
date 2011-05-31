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

/* gsgrab.h */

#define GSGRAB_VERSION	"0.2alpha 1994-04-10"

#define GRABICON	100

#define IDC_SETUP	201
#define IDC_CANCEL	202
#define IDC_ABOUT	203
#define IDC_HELP	204
#define IDHELP		204
#define NBUTTON		4


#define GRABDLG		500
#define IDC_GSCOMMAND	501
#define IDC_PRINTER	502
#define IDC_RESTEXT	503
#define IDC_RES		504
#define IDC_PORT	505
#define IDC_INTERVAL	506

#define ABOUTDLG	520
#define IDC_VERSION	521

#define IDR_DEVICES 	600

#ifndef RC_INVOKED
extern char szAppName[];
extern char szCommand[256];
extern char szGrabFile[128];

extern char szGhostscript[256];
extern char szPrinter[64];
extern char szResolution[64];
extern char szPort[64];
extern char szInterval[20];

extern char szHelpName[];
extern char szIniName[];
extern char szOptionSection[];
extern char szDevSection[];
extern char szDefCommand[];
extern char szUnknown[];

extern HWND hwndgrab;
extern HICON hicongrab;
extern BOOL printing;
extern HINSTANCE phInstance;
extern POINT char_size;
extern HWND hbutton[NBUTTON];
extern WNDPROC lpfnButtonWndProc;
extern int interval;

/* gsgrab.c */

BOOL start_timer(void);
void stop_timer(void);
BOOL call_gs(void);
LRESULT CALLBACK _export WndGSgrabProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK _export ButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

/* gsgdlg.c */
void show_about(void);
BOOL setup(void);

/* gsginit.c */
void init_window(void);
void init_profile(void);

#ifdef __WIN32__
#define GetNotification(wParam,lParam) (HIWORD(wParam))
#define SendDlgNotification(hwnd, id, notice) \
    SendMessage((hwnd), WM_COMMAND, MAKELONG((id),(notice)), (LPARAM)GetDlgItem((hwnd),(id)))
#else
#define GetNotification(wParam,lParam) (HIWORD(lParam))
#define SendDlgNotification(hwnd, id, notice) \
    SendMessage((hwnd), WM_COMMAND, id, MAKELPARAM(GetDlgItem((hwnd),(id)),(notice)))
#endif

#endif
