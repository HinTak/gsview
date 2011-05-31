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

/* gsgdlg.c */
/* Dialog boxes for GSgrab */

#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <stdlib.h>
#include "gsgrab.h"

/* copyright dialog box */
BOOL CALLBACK _export
AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_INITDIALOG:
            SetDlgItemText(hDlg, IDC_VERSION, GSGRAB_VERSION);
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
	DialogBoxParam( phInstance, (LPSTR)ABOUTDLG, hwndgrab, AboutDlgProc, (LPARAM)NULL);
#else
	DLGPROC lpProcAbout;
	lpProcAbout = (DLGPROC)MakeProcInstance((FARPROC)AboutDlgProc, phInstance);
	DialogBoxParam( phInstance, (LPSTR)ABOUTDLG, hwndgrab, lpProcAbout, (LPARAM)NULL);
	FreeProcInstance((FARPROC)lpProcAbout);
#endif
}


void
profile_create_section(char *section, int id)
{  
HGLOBAL hglobal;
LPSTR entry, value;
char name[128];
char val[256];
	hglobal = LoadResource(phInstance, 
	    FindResource(phInstance, MAKEINTRESOURCE(id), RT_RCDATA));
	if ( (entry = (LPSTR)LockResource(hglobal)) == (LPSTR)NULL)
	    return;
	while (lstrlen(entry)!=0) {
	    for ( value = entry; 
		  (*value!='\0') && (*value!=',') && (*value!='='); 
		  value++)
		/* nothing */;
	    _fstrncpy(name, entry, (int)(value-entry));
	    name[(int)(value-entry)] = '\0';
	    value++;
	    _fstrncpy(val, value, sizeof(val));
	    WritePrivateProfileString(section, name, val, szIniName);
	    entry = value + lstrlen(value) + 1;
	}
	FreeResource(hglobal);
}

#define PROFILE_SIZE  2048

/* dialog box for GSgrab setup */
BOOL CALLBACK _export
SetupDlgProc(HWND hDlg, UINT wmsg, WPARAM wParam, LPARAM lParam)
{
	char buf[128];
	int idevice;
	WORD notify_message;
	char *p;
	char *res;
	int numentry;
	char entry[128];

	switch (wmsg) {
	    case WM_INITDIALOG:
		/* initialise Ghostscript command and Interval */
		SetDlgItemText(hDlg, IDC_GSCOMMAND, szGhostscript);
		SetDlgItemText(hDlg, IDC_INTERVAL, szInterval);
		/* allocate buffer for profile items */
		if ( (res = malloc(PROFILE_SIZE)) == (char *)NULL)
		    return TRUE;
		/* initialise printer and resolution items */
		p = res;
		GetPrivateProfileString(szDevSection, NULL, "", res, PROFILE_SIZE, szIniName);
		if (strlen(p) == 0) {
		    /* [Devices] section doesn't exist.  Initialise from resources */
		    profile_create_section(szDevSection, IDR_DEVICES);
		}
		GetPrivateProfileString(szDevSection, NULL, "", res, PROFILE_SIZE, szIniName);
		for (numentry=0; p!=(char *)NULL && strlen(p)!=0; numentry++) {
		    SendDlgItemMessage(hDlg, IDC_PRINTER, CB_ADDSTRING, 0, 
			(LPARAM)((LPSTR)p));
		    p += strlen(p) + 1;
		}
		/* initialise ports list */
	        SendDlgItemMessage(hDlg, IDC_PORT, CB_ADDSTRING, 0, 
		    (LPARAM)(szUnknown));
		GetProfileString("Ports", NULL, "", res, PROFILE_SIZE);
		p = res;
		for (numentry=0; p!=(char *)NULL && strlen(p)!=0; numentry++) {
		    if (strcmp(p, "GSGRAB"))	/* don't let user cause infinite loop */
		    SendDlgItemMessage(hDlg, IDC_PORT, CB_ADDSTRING, 0, 
			(LPARAM)((LPSTR)p));
		    p += strlen(p) + 1;
		}
		/* free profile buffer */
		free(res);
		/* select initial printer */
		if (SendDlgItemMessage(hDlg, IDC_PRINTER, CB_SELECTSTRING, 0, (LPARAM)(LPSTR)szPrinter)
		    == CB_ERR)
		    SetWindowText(GetDlgItem(hDlg, IDC_PRINTER), szPrinter);
/*
		    SendDlgItemMessage(hDlg, IDC_PRINTER, CB_SETCURSEL, 0, 0L);
*/
		/* force update of IDC_RES */
		SendDlgNotification(hDlg, IDC_PRINTER, CBN_SELCHANGE);
		if (SendDlgItemMessage(hDlg, IDC_RES, CB_SELECTSTRING, 0, (LPARAM)(LPSTR)szResolution)
		    == CB_ERR)
		    SetWindowText(GetDlgItem(hDlg, IDC_RES), szResolution);
/*
		    SendDlgItemMessage(hDlg, IDC_RES, CB_SETCURSEL, 0, 0L);
*/
		/* select initial port */
		if (SendDlgItemMessage(hDlg, IDC_PORT, CB_SELECTSTRING, 0, (LPARAM)(LPSTR)szPort)
		    == CB_ERR)
		    SendDlgItemMessage(hDlg, IDC_PRINTER, CB_SETCURSEL, 0, 0L);
		return TRUE;
	    case WM_COMMAND:
		notify_message = GetNotification(wParam,lParam);
		switch (LOWORD(wParam)) {
		    case IDC_GSCOMMAND:
			/* don't have anything to do */
			return FALSE;
		    case IDC_PRINTER:
			if (notify_message != CBN_SELCHANGE) {
				return FALSE;
			}
			idevice = (int)SendDlgItemMessage(hDlg, IDC_PRINTER, CB_GETCURSEL, 0, 0L);
			if (idevice == CB_ERR) {
			    return FALSE;
			}
			SendDlgItemMessage(hDlg, IDC_PRINTER, CB_GETLBTEXT, idevice, (LPARAM)(LPSTR)entry);
			/* now look up entry in gsgrab.ini */
			/* and update IDC_RES list box */
			GetPrivateProfileString(szDevSection, entry, "", buf, sizeof(buf)-2, szIniName);
			buf[strlen(buf)+1] = '\0';	/* double NULL at end */
		    	SendDlgItemMessage(hDlg, IDC_RES, CB_RESETCONTENT, 0, 0L);
			p = buf;
			if (*p == '\0') {
			    /* no resolutions can be set */
			    EnableWindow(GetDlgItem(hDlg, IDC_RES), FALSE);
			    EnableWindow(GetDlgItem(hDlg, IDC_RESTEXT), FALSE);
			}
			else {
			  EnableWindow(GetDlgItem(hDlg, IDC_RES), TRUE);
			  EnableWindow(GetDlgItem(hDlg, IDC_RESTEXT), TRUE);
			  while (*p!='\0') {
			    res = p;
			    while ((*p!='\0') && (*p!=','))
				p++;
			    *p++ = '\0';
		    	    SendDlgItemMessage(hDlg, IDC_RES, CB_ADDSTRING, 0, 
			        (LPARAM)((LPSTR)res));
			  }
			}
			SendDlgItemMessage(hDlg, IDC_RES, CB_SETCURSEL, 0, 0L);
			if (SendDlgItemMessage(hDlg, IDC_RES, CB_GETLBTEXT, 0, (LPARAM)(LPSTR)buf)
			    != CB_ERR)
		            SetDlgItemText(hDlg, IDC_RES, buf);
			return FALSE;
		    case IDC_RES:
			/* don't have anything to do */
			return FALSE;
		    case IDC_PORT:
			/* don't have anything to do */
			return FALSE;
		    case IDC_INTERVAL:
			/* don't have anything to do */
			return FALSE;
		    case IDOK:
			/* save dialog items */
		        GetDlgItemText(hDlg, IDC_GSCOMMAND, szGhostscript, sizeof(szGhostscript));
		        GetDlgItemText(hDlg, IDC_PRINTER, szPrinter, sizeof(szPrinter));
		        GetDlgItemText(hDlg, IDC_RES, szResolution, sizeof(szResolution));
		        GetDlgItemText(hDlg, IDC_PORT, szPort, sizeof(szPort));
		        GetDlgItemText(hDlg, IDC_INTERVAL, szInterval, sizeof(szInterval));
			EndDialog(hDlg, TRUE);
			return TRUE;
		    case IDCANCEL:
			EndDialog(hDlg, FALSE);
			return TRUE;
		    case IDC_HELP:
			WinHelp(hDlg,szHelpName,HELP_KEY,(DWORD)"Setup");
			return FALSE;
		}
		break;
	}
	return FALSE;
}



/* Set up GSgrab options */
BOOL
setup(void)
{
	BOOL flag;

#ifdef __WIN32__
	flag = DialogBoxParam( phInstance, GRABDLG, hwndgrab, SetupDlgProc, (LPARAM)NULL);
#else
	DLGPROC lpProcSetup;
	lpProcSetup = (DLGPROC)MakeProcInstance((FARPROC)SetupDlgProc, phInstance);
	flag = DialogBoxParam( phInstance, (LPSTR)GRABDLG, hwndgrab, lpProcSetup, (LPARAM)NULL);
	FreeProcInstance((FARPROC)lpProcSetup);
#endif
	if (!flag)
	    return FALSE;
	interval = atoi(szInterval);
	if (interval > 60) {
	    interval = 60;
	    strcpy(szInterval, "60");
	}
	/* save to INI file */
	WritePrivateProfileString(szOptionSection, "Ghostscript",  szGhostscript,  szIniName);
	WritePrivateProfileString(szOptionSection, "Printer",  szPrinter,  szIniName);
	WritePrivateProfileString(szOptionSection, "Resolution", szResolution,  szIniName);
	WritePrivateProfileString(szOptionSection, "Port", szPort,  szIniName);
	WritePrivateProfileString(szOptionSection, "Interval", szInterval,  szIniName);
        WriteProfileString("ports", szGrabFile, "");
	WritePrivateProfileString(szOptionSection, "Version", GSGRAB_VERSION,  szIniName);
	return TRUE;
}

