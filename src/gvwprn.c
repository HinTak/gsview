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

/* gvwprn.c */
/* Printer routines for Windows GSview */
#include "gvwin.h"

/* documented in Device Driver Adaptation Guide */
/* Prototypes taken from print.h */
DECLARE_HANDLE(HPJOB);

HPJOB   WINAPI OpenJob(LPSTR, LPSTR, HPJOB);
int     WINAPI StartSpoolPage(HPJOB);
int     WINAPI EndSpoolPage(HPJOB);
int     WINAPI WriteSpool(HPJOB, LPSTR, int);
int     WINAPI CloseJob(HPJOB);
int     WINAPI DeleteJob(HPJOB, int);
int     WINAPI WriteDialog(HPJOB, LPSTR, int);
int     WINAPI DeleteSpoolPage(HPJOB);

static char pcfname[MAXSTR];	/* name of temporary command file for printing */
static char pfname[MAXSTR];	/* name of temporary file for printing options */
char not_defined[] = "[Not defined]";

/* dialog box for selecting printer properties */
BOOL CALLBACK _export
PropDlgProc(HWND hDlg, UINT wmsg, WPARAM wParam, LPARAM lParam)
{
	char buf[128];
	int iprop;
	int ivalue;
	WORD notify_message;
	char *p;
	char *value;
	static char device[MAXSTR];	/* contains printer device name */
	static struct prop_item_s* propitem;
	char section[MAXSTR];

	switch (wmsg) {
	    case WM_INITDIALOG:
		lstrcpy(device, (LPSTR)lParam);	/* initialise device name */
		propitem = get_properties(device);
		if (propitem == (struct prop_item_s *)NULL) {
		    EndDialog(hDlg, FALSE);
		    return TRUE;
		}
		for (iprop=0; propitem[iprop].name[0]; iprop++) {
		    SendDlgItemMessage(hDlg, PROP_NAME, CB_ADDSTRING, 0, 
			(LPARAM)((LPSTR)propitem[iprop].name+1));
		}
		SendDlgItemMessage(hDlg, PROP_NAME, CB_SETCURSEL, 0, 0L);
		/* force update of PROP_VALUE */
		SendDlgNotification(hDlg, PROP_NAME, CBN_SELCHANGE);
		return TRUE;
	    case WM_COMMAND:
		notify_message = GetNotification(wParam,lParam);
		switch (LOWORD(wParam)) {
		    case ID_HELP:
		        SendMessage(hwndimg, help_message, 0, 0L);
		        return(FALSE);
		    case PROP_NAME:
			if (notify_message != CBN_SELCHANGE) {
				return FALSE;
			}
			iprop = (int)SendDlgItemMessage(hDlg, PROP_NAME, CB_GETCURSEL, 0, 0L);
			if (iprop == CB_ERR) {
			    return FALSE;
			}
			/* now look up entry in gsview.ini */
			/* and update PROP_VALUE list box */
			strcpy(section, device);
			strcat(section, " values");
			GetPrivateProfileString(section, propitem[iprop].name, "", buf, sizeof(buf)-2, INIFILE);
			buf[strlen(buf)+1] = '\0';	/* put double NULL at end */
		    	SendDlgItemMessage(hDlg, PROP_VALUE, CB_RESETCONTENT, 0, 0L);
		    	SendDlgItemMessage(hDlg, PROP_VALUE, CB_ADDSTRING, 0, 
			    (LPARAM)((LPSTR)not_defined));
			p = buf;
			if (*p != '\0') {
			  EnableWindow(GetDlgItem(hDlg, PROP_VALUE), TRUE);
			  while (*p!='\0') {
			    value = p;
			    while ((*p!='\0') && (*p!=','))
				p++;
			    *p++ = '\0';
		    	    SendDlgItemMessage(hDlg, PROP_VALUE, CB_ADDSTRING, 0, 
			        (LPARAM)((LPSTR)value));
			  }
			}
			SendDlgItemMessage(hDlg, PROP_VALUE, CB_SELECTSTRING, -1, (LPARAM)(LPSTR)propitem[iprop].value);
		        SetDlgItemText(hDlg, PROP_VALUE, propitem[iprop].value);
			return FALSE;
		    case PROP_VALUE:
			if (notify_message == CBN_SELCHANGE) {
			    iprop = (int)SendDlgItemMessage(hDlg, PROP_NAME, CB_GETCURSEL, 0, 0L);
			    if (iprop == CB_ERR)
			        return FALSE;
			    ivalue = (int)SendDlgItemMessage(hDlg, PROP_VALUE, CB_GETCURSEL, 0, 0L);
			    if (ivalue == CB_ERR)
			        return FALSE;
			    SendDlgItemMessage(hDlg, PROP_VALUE, CB_GETLBTEXT, ivalue, (LPARAM)(LPSTR)propitem[iprop].value);
			}
			if (notify_message == CBN_EDITCHANGE) {
			    iprop = (int)SendDlgItemMessage(hDlg, PROP_NAME, CB_GETCURSEL, 0, 0L);
			    if (iprop == CB_ERR)
			        return FALSE;
		            GetDlgItemText(hDlg, PROP_VALUE, (LPSTR)propitem[iprop].value, sizeof(propitem->value));
			}
			return FALSE;
		    case IDOK:
			for (iprop=0; propitem[iprop].name[0]; iprop++) {
			    WritePrivateProfileString(device, propitem[iprop].name, propitem[iprop].value, INIFILE);
			}
			free((char *)propitem);
			EndDialog(hDlg, TRUE);
			return TRUE;
		    case IDCANCEL:
			free((char *)propitem);
			EndDialog(hDlg, FALSE);
			return TRUE;
		}
		break;
	}
	return FALSE;
}


/* dialog box for selecting printer device and resolution */
BOOL CALLBACK _export
DeviceDlgProc(HWND hDlg, UINT wmsg, WPARAM wParam, LPARAM lParam)
{
	char buf[128];
	int idevice;
	WORD notify_message;
	char *p;
	char *res;
	int numentry;
	char entry[MAXSTR];
	struct prop_item_s *proplist;

	switch (wmsg) {
	    case WM_INITDIALOG:
		p = get_devices();
		res = p;	/* save for free() */
		for (numentry=0; p!=(char *)NULL && strlen(p)!=0; numentry++) {
		    SendDlgItemMessage(hDlg, DEVICE_NAME, CB_ADDSTRING, 0, 
			(LPARAM)((LPSTR)p));
		    p += strlen(p) + 1;
		}
		free(res);
		if (SendDlgItemMessage(hDlg, DEVICE_NAME, CB_SELECTSTRING, 0, (LPARAM)(LPSTR)option.device_name)
		    == CB_ERR)
		    SendDlgItemMessage(hDlg, DEVICE_NAME, CB_SETCURSEL, 0, 0L);
		/* force update of DEVICE_RES */
		SendDlgNotification(hDlg, DEVICE_NAME, CBN_SELCHANGE);
		if (SendDlgItemMessage(hDlg, DEVICE_RES, CB_SELECTSTRING, 0, (LPARAM)(LPSTR)option.device_resolution)
		    == CB_ERR)
		    SendDlgItemMessage(hDlg, DEVICE_RES, CB_SETCURSEL, 0, 0L);
		return TRUE;
	    case WM_COMMAND:
		notify_message = GetNotification(wParam,lParam);
		switch (LOWORD(wParam)) {
		    case ID_HELP:
		        SendMessage(hwndimg, help_message, 0, 0L);
		        return(FALSE);
		    case DEVICE_NAME:
			if (notify_message != CBN_SELCHANGE) {
				return FALSE;
			}
			idevice = (int)SendDlgItemMessage(hDlg, DEVICE_NAME, CB_GETCURSEL, 0, 0L);
			if (idevice == CB_ERR) {
			    return FALSE;
			}
			SendDlgItemMessage(hDlg, DEVICE_NAME, CB_GETLBTEXT, idevice, (LPARAM)(LPSTR)entry);
			if ( (proplist = get_properties(entry)) != (struct prop_item_s *)NULL ) {
	    		    free((char *)proplist);
			    EnableWindow(GetDlgItem(hDlg, DEVICE_PROP), TRUE);
			}
			else
			    EnableWindow(GetDlgItem(hDlg, DEVICE_PROP), FALSE);
			/* now look up entry in gsview.ini */
			/* and update DEVICE_RES list box */
			GetPrivateProfileString(DEVSECTION, entry, "", buf, sizeof(buf)-2, INIFILE);
			buf[strlen(buf)+1] = '\0';	/* double NULL at end */
		    	SendDlgItemMessage(hDlg, DEVICE_RES, CB_RESETCONTENT, 0, 0L);
			p = buf;
			if (*p == '\0') {
			    /* no resolutions can be set */
			    EnableWindow(GetDlgItem(hDlg, DEVICE_RES), FALSE);
			    EnableWindow(GetDlgItem(hDlg, DEVICE_RESTEXT), FALSE);
			}
			else {
			  EnableWindow(GetDlgItem(hDlg, DEVICE_RES), TRUE);
			  EnableWindow(GetDlgItem(hDlg, DEVICE_RESTEXT), TRUE);
			  while (*p!='\0') {
			    res = p;
			    while ((*p!='\0') && (*p!=','))
				p++;
			    *p++ = '\0';
		    	    SendDlgItemMessage(hDlg, DEVICE_RES, CB_ADDSTRING, 0, 
			        (LPARAM)((LPSTR)res));
			  }
			}
			SendDlgItemMessage(hDlg, DEVICE_RES, CB_SETCURSEL, 0, 0L);
			if (SendDlgItemMessage(hDlg, DEVICE_RES, CB_GETLBTEXT, 0, (LPARAM)(LPSTR)buf)
			    != CB_ERR)
		            SetDlgItemText(hDlg, DEVICE_RES, buf);
			return FALSE;
		    case DEVICE_RES:
			/* don't have anything to do */
			return FALSE;
		    case DEVICE_PROP:
			idevice = (int)SendDlgItemMessage(hDlg, DEVICE_NAME, CB_GETCURSEL, 0, 0L);
			if (idevice == CB_ERR) {
			    return FALSE;
			}
			SendDlgItemMessage(hDlg, DEVICE_NAME, CB_GETLBTEXT, idevice, (LPARAM)(LPSTR)entry);
			if ( (proplist = get_properties(entry)) != (struct prop_item_s *)NULL ) {
#ifndef __WIN32__
	    		    DLGPROC lpProcProp;
#endif
	    		    free((char *)proplist);
			    LoadString(phInstance, IDS_TOPICPRINT, szHelpTopic, sizeof(szHelpTopic));
#ifdef __WIN32__
			    DialogBoxParam( phInstance, "PropDlgBox", hDlg, PropDlgProc, (LPARAM)entry);
#else
			    lpProcProp = (DLGPROC)MakeProcInstance((FARPROC)PropDlgProc, phInstance);
			    DialogBoxParam( phInstance, "PropDlgBox", hDlg, lpProcProp, (LPARAM)entry);
			    FreeProcInstance((FARPROC)lpProcProp);
#endif
			}
			else
			    play_sound(SOUND_ERROR);
			return FALSE;
		    case IDOK:
			/* save device name and resolution */
		        GetDlgItemText(hDlg, DEVICE_NAME, option.device_name, sizeof(option.device_name));
		        GetDlgItemText(hDlg, DEVICE_RES, option.device_resolution, sizeof(option.device_resolution));
			EndDialog(hDlg, TRUE);
			return TRUE;
		    case IDCANCEL:
			EndDialog(hDlg, FALSE);
			return TRUE;
		}
		break;
	}
	return FALSE;
}



/* Modeless dialog box - Cancel printing */
BOOL CALLBACK _export
CancelDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
	case WM_INITDIALOG:
	    SetWindowText(hDlg, szAppName);
	    return TRUE;
	case WM_COMMAND:
	    switch(LOWORD(wParam)) {
		case IDCANCEL:
		    DestroyWindow(hDlg);
		    hDlgModeless = 0;
		    EndDialog(hDlg, 0);
		    return TRUE;
	    }
    }
    return FALSE;
}

/* Dialog box to select printer port */
BOOL CALLBACK _export
SpoolDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
LPSTR entry;
    switch(message) {
	case WM_INITDIALOG:
	    entry = (LPSTR)lParam;
	    while (*entry) {
		SendDlgItemMessage(hDlg, SPOOL_PORT, LB_ADDSTRING, 0, (LPARAM)entry);
		entry += lstrlen(entry)+1;
	    }
	    if ( (*option.printer_port=='\0') ||
		(SendDlgItemMessage(hDlg, SPOOL_PORT, LB_SELECTSTRING, 0, (LPARAM)(LPSTR)option.printer_port)
		    == LB_ERR) )
	        SendDlgItemMessage(hDlg, SPOOL_PORT, LB_SETCURSEL, 0, (LPARAM)0);
	    return TRUE;
	case WM_COMMAND:
	    switch(LOWORD(wParam)) {
		case SPOOL_PORT:
#ifdef __WIN32__
		    if (HIWORD(wParam)
#else
		    if (HIWORD(lParam)
#endif
			               == LBN_DBLCLK)
			PostMessage(hDlg, WM_COMMAND, IDOK, 0L);
		    return FALSE;
		case IDOK:
		    SendDlgItemMessage(hDlg, SPOOL_PORT, LB_GETTEXT, 
			(int)SendDlgItemMessage(hDlg, SPOOL_PORT, LB_GETCURSEL, 0, 0L),
			(LPARAM)(LPSTR)option.printer_port);
		    EndDialog(hDlg, 1+(int)SendDlgItemMessage(hDlg, SPOOL_PORT, LB_GETCURSEL, 0, 0L));
		    return TRUE;
		case IDCANCEL:
		    EndDialog(hDlg, 0);
		    return TRUE;
	    }
    }
    return FALSE;
}

#ifdef __WIN32__
/* Win32s can't get access to OpenJob etc., so we try to sneak the */
/* data through the Windows printer driver unchanged */
BOOL CALLBACK _export
PrintAbortProc(HDC hdcPrn, int code)
{
    MSG msg;
    while (hDlgModeless && PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
	if (hDlgModeless || !IsDialogMessage(hDlgModeless,&msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
	}
    }
    return(hDlgModeless!=0);
}

/* Print File to port */
/* port==NULL means prompt for port with dialog box */
/* Win32s ignores the port parameter */
int
gp_printfile(char *filename, char *port)
{
HDC printer;
PRINTDLG pd;
DOCINFO di;
char *buf;
int *bufcount, count;
FILE *f;
long lsize;
long ldone;
char fmt[MAXSTR];
char pcdone[10];
int error = TRUE;
	memset(&pd, 0, sizeof(PRINTDLG));
	pd.lStructSize = sizeof(PRINTDLG);
	pd.hwndOwner = hwndimg;
	pd.Flags = PD_PRINTSETUP | PD_RETURNDC;

	if ((f = fopen(filename, "rb")) == (FILE *)NULL)
	    return FALSE;
	fseek(f, 0L, SEEK_END);
	lsize = ftell(f);
	if (lsize <= 0)
	    lsize = 1;
	fseek(f, 0L, SEEK_SET);
	ldone = 0;
	LoadString(phInstance, IDS_CANCELDONE, fmt, sizeof(fmt));

	if (PrintDlg(&pd)) {
	printer = pd.hDC;
	if (printer != (HDC)NULL) {
	    if ( (buf = malloc(4096+2)) != (char *)NULL ) {
	    	bufcount = (int *)buf;
		EnableWindow(hwndimg,FALSE);
		hDlgModeless = CreateDialogParam(phInstance,"CancelDlgBox",hwndimg,CancelDlgProc,(LPARAM)szAppName);
		SetAbortProc(printer, PrintAbortProc);
		di.cbSize = sizeof(DOCINFO);
		di.lpszDocName = szAppName;
		di.lpszOutput = NULL;
		if (StartDoc(printer, &di) > 0) {
		    while ( hDlgModeless &&  ((count = fread(buf+2, 1, 4096, f)) != 0) ) {
			*bufcount = count;
			Escape(printer, PASSTHROUGH, count+2, (LPSTR)buf, NULL);
	    		ldone += count;
	    		sprintf(pcdone, fmt, (int)(ldone * 100 / lsize));
	    		SetWindowText(GetDlgItem(hDlgModeless, CANCEL_PCDONE), pcdone);
		    }
		    if (hDlgModeless)
			EndDoc(printer);
		    else
			AbortDoc(printer);
		}
		if (hDlgModeless) {
		    DestroyWindow(hDlgModeless);
		    error = FALSE;
		    hDlgModeless = (HWND)0;
		}
	        EnableWindow(hwndimg,TRUE);
	        free(buf);
	    }
	  }
	  DeleteDC(printer);
	}
	fclose(f);
	return !error;
}
#else /* !__WIN32__ */
/* Print File to port */
/* port==NULL means prompt for port with dialog box */
int
gp_printfile(char *filename, char *port)
{
#define PRINT_BUF_SIZE 16384u
char *buffer;
char *portname;
int i, iport;
DLGPROC lpfnSpoolProc;
HPJOB hJob;
UINT count;
FILE *f;
int error = FALSE;
DLGPROC lpfnCancelProc;
long lsize;
long ldone;
char fmt[MAXSTR];
char pcdone[10];
MSG msg;

	if ((buffer = malloc(PRINT_BUF_SIZE)) == (char *)NULL)
	    return FALSE;
	if (port == (char *)NULL) {
	    /* get list of ports */
	    GetProfileString("ports", NULL, "", buffer, PRINT_BUF_SIZE);
	    /* select a port */
	    lpfnSpoolProc = (DLGPROC)MakeProcInstance((FARPROC)SpoolDlgProc, phInstance);
	    iport = DialogBoxParam(phInstance, "SpoolDlgBox", hwndtext, lpfnSpoolProc, (LPARAM)buffer);
	    FreeProcInstance((FARPROC)lpfnSpoolProc);
	    if (!iport) {
		free(buffer);
		return FALSE;
	    }
	    portname = buffer;
	    for (i=1; i<iport && strlen(portname)!=0; i++)
		portname += lstrlen(portname)+1;
	}
	else
	    portname = port;
	
	if ((f = fopen(filename, "rb")) == (FILE *)NULL) {
	    free(buffer);
	    return FALSE;
	}
	fseek(f, 0L, SEEK_END);
	lsize = ftell(f);
	if (lsize <= 0)
	    lsize = 1;
	fseek(f, 0L, SEEK_SET);

	hJob = OpenJob(portname, filename, (HDC)NULL);
	switch ((int)hJob) {
	    case SP_APPABORT:
	    case SP_ERROR:
	    case SP_OUTOFDISK:
	    case SP_OUTOFMEMORY:
	    case SP_USERABORT:
	        fclose(f);
		free(buffer);
	        return FALSE;
	}
	if (StartSpoolPage(hJob) < 0)
	    error = TRUE;

	lpfnCancelProc = (DLGPROC)MakeProcInstance((FARPROC)CancelDlgProc, phInstance);
	hDlgModeless = CreateDialog(phInstance, "CancelDlgBox", hwndimg, lpfnCancelProc);
	ldone = 0;
	LoadString(phInstance, IDS_CANCELDONE, fmt, sizeof(fmt));

	while (!error && hDlgModeless 
	  && (count = fread(buffer, 1, PRINT_BUF_SIZE, f)) != 0 ) {
	    if (WriteSpool(hJob, buffer, count) < 0)
		error = TRUE;
	    ldone += count;
	    sprintf(pcdone, fmt, (int)(ldone * 100 / lsize));
	    SetWindowText(GetDlgItem(hDlgModeless, CANCEL_PCDONE), pcdone);
	    while (PeekMessage(&msg, hDlgModeless, 0, 0, PM_REMOVE)) {
	        if ((hDlgModeless == 0) || !IsDialogMessage(hDlgModeless, &msg)) {
		    TranslateMessage(&msg);
		    DispatchMessage(&msg);
		}
	    }
	}
	free(buffer);
	fclose(f);

	if (!hDlgModeless)
	    error=TRUE;
	DestroyWindow(hDlgModeless);
	hDlgModeless = 0;
	FreeProcInstance((FARPROC)lpfnCancelProc);
	EndSpoolPage(hJob);
	if (error)
	    DeleteJob(hJob, 0);
	else
	    CloseJob(hJob);
	return !error;
}
#endif /* !__WIN32__ */


/* cleanup print temporary files */
void
print_cleanup(void)
{
	if ((pcfname[0] != '\0') && !debug)
		unlink(pcfname);
	pcfname[0] = '\0';
	if ((pfname[0] != '\0') && !debug)
		unlink(pfname);
	pfname[0] = '\0';
}


/* print a range of pages using a Ghostscript device */
void
gsview_print(BOOL to_file)
{
	BOOL flag;
#ifndef __WIN32__
	DLGPROC lpProcDevice;
#endif
	char command[256];

	if (psfile.name[0] == '\0') {
		gserror(IDS_NOTOPEN, NULL, MB_ICONEXCLAMATION, SOUND_NOTOPEN);
		return;
	}
	
	LoadString(phInstance, IDS_TOPICPRINT, szHelpTopic, sizeof(szHelpTopic));
#ifdef __WIN32__
	flag = DialogBoxParam( phInstance, "DeviceDlgBox", hwndimg, DeviceDlgProc, (LPARAM)NULL);
#else
	lpProcDevice = (DLGPROC)MakeProcInstance((FARPROC)DeviceDlgProc, phInstance);
	flag = DialogBoxParam( phInstance, "DeviceDlgBox", hwndimg, lpProcDevice, (LPARAM)NULL);
	FreeProcInstance((FARPROC)lpProcDevice);
#endif
	if (!flag)
	    return;

	load_string(IDS_WAITGSCLOSE, szWait, sizeof(szWait));
	info_wait(TRUE);
	gs_close();	/* we need a new Ghostscript */
	info_wait(FALSE);

	if (!gsview_cprint(to_file, pcfname, pfname))
	    return;

	sprintf(command,"%s -sGSVIEW=%u @%s", option.gscommand,
		(unsigned int)hwndimg, pfname);

	if (strlen(command) > 126) {
		/* command line too long */
		gserror(IDS_TOOLONG, command, MB_ICONSTOP, SOUND_ERROR);
		unlink(pfname);
		pfname[0] = '\0';
		gsprog.hinst = (HINSTANCE)NULL;
		return;
	}
	load_string(IDS_WAITGSOPEN, szWait, sizeof(szWait));
	info_wait(TRUE);
	gsprog.hinst = (HINSTANCE)WinExec(command, SW_SHOWMINNOACTIVE);

#ifdef __WIN32__
	if (gsprog.hinst == NULL)
#else
	if (gsprog.hinst < HINSTANCE_ERROR)
#endif
	{
		gserror(IDS_CANNOTRUN, command, MB_ICONSTOP, SOUND_ERROR);
		unlink(pfname);
		pfname[0] = '\0';
		info_wait(FALSE);
		gsprog.hinst = (HINSTANCE)NULL;
		return;
	}

/*
	set_timer(timeout*pages);
*/
	load_string(IDS_WAITPRINT, szWait, sizeof(szWait));
	info_wait(TRUE);
	return;
}


