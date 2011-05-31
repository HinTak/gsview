/*
 * print.c --   Printing operations for GSVIEW.EXE, 
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
#include <io.h>
#define NeedFunctionPrototypes 1
#include "ps.h"
#include "gsview.h"

static char pcfname[MAXSTR];	/* name of temporary command file for printing */
static char pfname[MAXSTR];	/* name of temporary file for printing options */

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

struct prop_item_s {
	char	name[MAXSTR];
	char	value[MAXSTR];
};

char not_defined[] = "[Not defined]";

struct prop_item_s *
get_properties(char *device)
{
char *entries, *p;
int i, numentry;
struct prop_item_s *proplist;
	entries = malloc(PROFILE_SIZE);
	if (entries == (char *)NULL)
	   return NULL;
	GetPrivateProfileString(device, NULL, "", entries, PROFILE_SIZE, INIFILE);
	if (strlen(entries) == 0) {
	    free(entries);
	    return NULL;
	}
	p = entries;
	for (numentry=0; p!=(char *)NULL && strlen(p)!=0; numentry++)
	    p += strlen(p) + 1;
	proplist = (struct prop_item_s *)malloc((numentry+1) * sizeof(struct prop_item_s));
	if (proplist == (struct prop_item_s *)NULL) {
	    free(entries);
	    return NULL;
	}
	p = entries;
	for (i=0; i<numentry; i++) {
	    strcpy(proplist[i].name, p);
	    GetPrivateProfileString(device, p, "", proplist[i].value, sizeof(proplist->value), INIFILE);
	    p += strlen(p) + 1;
	}
	proplist[numentry].name[0] = '\0';
	proplist[numentry].value[0] = '\0';
	free(entries);
	return proplist;
}

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
		if (SendDlgItemMessage(hDlg, DEVICE_NAME, CB_SELECTSTRING, 0, (LPARAM)(LPSTR)device_name)
		    == CB_ERR)
		    SendDlgItemMessage(hDlg, DEVICE_NAME, CB_SETCURSEL, 0, 0L);
		/* force update of DEVICE_RES */
		SendDlgNotification(hDlg, DEVICE_NAME, CBN_SELCHANGE);
		if (SendDlgItemMessage(hDlg, DEVICE_RES, CB_SELECTSTRING, 0, (LPARAM)(LPSTR)device_resolution)
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
	    		    DLGPROC lpProcProp;
	    		    free((char *)proplist);
			    LoadString(phInstance, IDS_TOPICPRINT, szHelpTopic, sizeof(szHelpTopic));
			    lpProcProp = (DLGPROC)MakeProcInstance((FARPROC)PropDlgProc, phInstance);
			    DialogBoxParam( phInstance, "PropDlgBox", hDlg, lpProcProp, (LPARAM)entry);
			    FreeProcInstance((FARPROC)lpProcProp);
			}
			else
			    play_sound(SOUND_ERROR);
			return FALSE;
		    case IDOK:
			/* save device name and resolution */
		        GetDlgItemText(hDlg, DEVICE_NAME, device_name, sizeof(device_name));
		        GetDlgItemText(hDlg, DEVICE_RES, device_resolution, sizeof(device_resolution));
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
	    SendDlgItemMessage(hDlg, SPOOL_PORT, LB_SETCURSEL, 0, (LPARAM)0);
	    return TRUE;
	case WM_COMMAND:
	    switch(LOWORD(wParam)) {
		case SPOOL_PORT:
#ifdef WIN32
		    if (HIWORD(wParam)
#else
		    if (HIWORD(lParam)
#endif
			               == LBN_DBLCLK)
			PostMessage(hDlg, WM_COMMAND, IDOK, 0L);
		    return FALSE;
		case IDOK:
		    EndDialog(hDlg, 1+(int)SendDlgItemMessage(hDlg, SPOOL_PORT, LB_GETCURSEL, 0, 0L));
		    return TRUE;
		case IDCANCEL:
		    EndDialog(hDlg, 0);
		    return TRUE;
	    }
    }
    return FALSE;
}

/* Print File to port */
int
gp_printfile(char *filename)
{
#define PRINT_BUF_SIZE 16384u
char *buffer;
char *portname;
DLGPROC lpfnSpoolProc;
int i, port;
HPJOB hJob;
WORD count;
FILE *f;
int error = FALSE;
DLGPROC lpfnCancelProc;
long lsize;
long ldone;
char fmt[MAXSTR];
char pcdone[10];
MSG msg;

	/* get list of ports */
	if ((buffer = malloc(PRINT_BUF_SIZE)) == (char *)NULL)
	    return FALSE;
	GetProfileString("ports", NULL, "", buffer, PRINT_BUF_SIZE);
	/* select a port */
	lpfnSpoolProc = (DLGPROC)MakeProcInstance((FARPROC)SpoolDlgProc, phInstance);
	port = DialogBoxParam(phInstance, "SpoolDlgBox", hwndtext, lpfnSpoolProc, (LPARAM)buffer);
	FreeProcInstance((FARPROC)lpfnSpoolProc);
	if (!port) {
	    free(buffer);
	    return FALSE;
	}
	portname = buffer;
	for (i=1; i<port && strlen(portname)!=0; i++)
	    portname += lstrlen(portname)+1;
	
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


/* get a filename and spool it for printing */
void
gsview_spool()
{
	static char filename[MAXSTR];

	if (!getfilename(filename, OPEN, FILTER_ALL, IDS_PRINTFILE, IDS_TOPICPRINT))
		return;

	if (!gp_printfile(filename)) {
		play_sound(SOUND_ERROR);
		return;
	}
}


char *
get_devices()
{
HGLOBAL hglobal;
LPSTR device;
LPSTR lp;
char *p;
	p = malloc(PROFILE_SIZE);
	if (p == (char *)NULL)
	    return (char *)NULL;

	GetPrivateProfileString(DEVSECTION, NULL, "", p, PROFILE_SIZE, INIFILE);
	if (strlen(p) == 0) {
	    /* [Devices] section doesn't exist.  Initialise from resources */
	    hglobal = LoadResource(phInstance, FindResource(phInstance, "gsview_devices", RT_RCDATA));
	    if ( (device = (LPSTR)LockResource(hglobal)) == (LPSTR)NULL)
		return (char *)NULL;
	    while (lstrlen(device)!=0) {
	    	for (lp = device; (*lp!='\0') && (*lp!=','); lp++)
	    		/* nothing */;
		*lp++ = '\0';
		WritePrivateProfileString(DEVSECTION, device, lp, INIFILE);
		device = lp + lstrlen(lp) + 1;
	    }
	    FreeResource(hglobal);
	}
	GetPrivateProfileString(DEVSECTION, NULL, "", p, PROFILE_SIZE, INIFILE);
	return p;
}

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
	int i;
	BOOL flag;
	DLGPROC lpProcDevice;
	float print_xdpi, print_ydpi;
	int width, height;

	struct prop_item_s *proplist;
	
	static char output[MAXSTR];	/* output filename for printing */
	char *fname;			/* filename to print */
	char command[256];
	FILE *pcfile;
	FILE *optfile;
	int pages;
	int thispage = pagenum;

	if (dfname[0] == '\0') {
		gserror(IDS_NOTOPEN, NULL, MB_ICONEXCLAMATION, SOUND_NOTOPEN);
		return;
	}
	
	LoadString(phInstance, IDS_TOPICPRINT, szHelpTopic, sizeof(szHelpTopic));
	lpProcDevice = (DLGPROC)MakeProcInstance((FARPROC)DeviceDlgProc, phInstance);
	flag = DialogBoxParam( phInstance, "DeviceDlgBox", hwndimg, lpProcDevice, (LPARAM)NULL);
	FreeProcInstance((FARPROC)lpProcDevice);
	if (!flag) {
	    return;
	}

	info_wait(TRUE);
	gswin_close();	/* we need a new Ghostscript */
	info_wait(FALSE);

	fname = (char *)NULL;
	if (doc == (struct document *)NULL) {
		play_sound(SOUND_NONUMBER);
	    	LoadString(phInstance, IDS_PRINTINGALL, command, sizeof(command));
		if (MessageBox(hwndimg, command, szAppName, MB_OKCANCEL | MB_ICONINFORMATION) == IDCANCEL)
			return;
		fname = dfname;
		pages = 1;
	}
	else {
	    pages = 1;
	    if (doc->numpages != 0) {
		if (!get_page(&thispage, TRUE))
		    return;
	        pages = 0;
	        for (i=0; i< doc->numpages; i++) {
	            if (page_list.select[i]) pages++;
	        }
	    }

	    if ((pcfname[0] != '\0') && !debug)
		unlink(pcfname);
	    pcfname[0] = '\0';
	    if ( (pcfile = gp_open_scratch_file(szScratch, pcfname, "w")) == (FILE *)NULL) {
		play_sound(SOUND_ERROR);
		return;
	    }
	    pscopydoc(pcfile);
	    fclose(pcfile);
	    fname = pcfname;
	}
	
	if (to_file) {
	    if (!getfilename(output, SAVE, FILTER_ALL, IDS_OUTPUTFILE, IDS_TOPICPRINT))
		return;
	}

	/* calculate image size */
	switch (sscanf(device_resolution,"%fx%f", &print_xdpi, &print_ydpi)) {
	    case EOF:
	    case 0:
	        print_xdpi = print_ydpi = DEFAULT_RESOLUTION;
	        break;
	    case 1:
	        print_ydpi = print_xdpi;
	}
	i = get_papersizes_index();
	if (i < 0) {
	    width = user_width;
	    width = user_height;
	}
	else {
	    width = papersizes[i].width;
	    height = papersizes[i].height;
	}
	width  = (unsigned int)(width  / 72.0 * print_xdpi);
	height = (unsigned int)(height / 72.0 * print_ydpi);


	if ((pfname[0] != '\0') && !debug)
		unlink(pfname);
	pfname[0] = '\0';
	if ( (optfile = gp_open_scratch_file(szScratch, pfname, "w")) == (FILE *)NULL) {
		play_sound(SOUND_ERROR);
		return;
	}
	fprintf(optfile, "-dNOPAUSE\n");
	fprintf(optfile, "-sDEVICE=%s\n",device_name);
	fprintf(optfile, "-r%gx%g\n", (double)print_xdpi, (double)print_ydpi);
	fprintf(optfile, "-g%ux%u\n",width,height);
	if (to_file) {
	    char *p;
	    fprintf(optfile, "-sOutputFile=");
	    for (p=output; *p != '\0'; p++)
	        if (*p == '\\')
	            fputc('/',optfile);
	        else
	            fputc(*p,optfile);
	    fputc('\n',optfile);
	}
	if ((proplist = get_properties(device_name)) != (struct prop_item_s *)NULL) {
	    /* output current property selections */
	    for (i=0; proplist[i].name[0]; i++) {
		if (strcmp(proplist[i].value, not_defined) != 0)
		    fprintf(optfile,"-%s=%s\n", proplist[i].name, proplist[i].value);
	    }
	    free((char *)proplist);
	}
	fclose(optfile);

	sprintf(command,"%s -sGSVIEW=%u @%s %s quit.ps", szGSwin,
		(unsigned int)hwndimg, pfname, fname);

	if (strlen(command) > 126) {
		/* command line too long */
		gserror(IDS_TOOLONG, command, MB_ICONSTOP, SOUND_ERROR);
		unlink(pfname);
		pfname[0] = '\0';
		gswin_hinst = (HINSTANCE)NULL;
		return;
	}
	info_wait(TRUE);
	gswin_hinst = (HINSTANCE)WinExec(command, SW_SHOWMINNOACTIVE);

	if (gswin_hinst < HINSTANCE_ERROR) {
		gserror(IDS_CANNOTRUN, command, MB_ICONSTOP, SOUND_ERROR);
		unlink(pfname);
		pfname[0] = '\0';
		info_wait(FALSE);
		gswin_hinst = (HINSTANCE)NULL;
		return;
	}

	set_timer(timeout*pages);
	info_wait(TRUE);
	return;
}

/* extract a range of pages for later printing */
void
gsview_extract()
{
	FILE *f;
	static char output[MAXSTR];
	int thispage = pagenum;

	if (dfname[0] == '\0') {
		gserror(IDS_NOTOPEN, NULL, MB_ICONEXCLAMATION, SOUND_NOTOPEN);
		return;
	}

	if (doc == (struct document *)NULL) {
		gserror(IDS_NOPAGE, NULL, MB_ICONEXCLAMATION, SOUND_NONUMBER);
		return;
	}
	
	if (doc->numpages != 0)
	    if (!get_page(&thispage, TRUE))
	        return;

	if (!getfilename(output, SAVE, FILTER_PS, NULL, IDS_TOPICPRINT))
		return;

	if ((f = fopen(output, "wb")) == (FILE *)NULL) {
		return;
	}

	info_wait(TRUE);
	if (doc->numpages != 0)
	    pscopydoc(f);
	else
	    pscopy(dfile, f, doc->beginheader, doc->endtrailer);

	fclose(f);

	info_wait(FALSE);
	return;
}

/* pscopydoc is copied (with modifications) from ghostview misc.c */
/* Copyright (C) 1992  Timothy O. Theisen */
/* length calculates string length at compile time */
/* can only be used with character constants */
#define length(a) (sizeof(a)-1)

/* Copy the headers, marked pages, and trailer to fp */
void
pscopydoc(FILE *fp)
{
    char text[PSLINELENGTH];
    char *comment;
    BOOL pages_written = FALSE;
    BOOL pages_atend = FALSE;
    int pages = 0;
    int page = 1;
    int i;
    long here;

    for (i=0; i< doc->numpages; i++) {
	    if (page_list.select[i]) pages++;
    }

    here = doc->beginheader;
    while ( (comment = pscopyuntil(dfile, fp, here,
			   doc->endheader, "%%Pages:")) != (char *)NULL ) {
	here = ftell(dfile);
	if (pages_written || pages_atend) {
	    free(comment);
	    continue;
	}
	sscanf(comment+length("%%Pages:"), "%s", text);
	if (strcmp(text, "(atend)") == 0) {
	    fputs(comment, fp);
	    pages_atend = TRUE;
	} else {
	    switch (sscanf(comment+length("%%Pages:"), "%*d %d", &i)) {
		case 1:
		    fprintf(fp, "%%%%Pages: %d %d\r\n", pages, i);
		    break;
		default:
		    fprintf(fp, "%%%%Pages: %d\r\n", pages);
		    break;
	    }
	    pages_written = TRUE;
	}
	free(comment);
    }
    pscopy(dfile, fp, doc->beginpreview, doc->endpreview);
    pscopy(dfile, fp, doc->begindefaults, doc->enddefaults);
    pscopy(dfile, fp, doc->beginprolog, doc->endprolog);
    pscopy(dfile, fp, doc->beginsetup, doc->endsetup);

    page = 1;
    for (i = 0; i < doc->numpages; i++) {
	if (page_list.select[map_page(i)])  {
	    comment = pscopyuntil(dfile, fp, doc->pages[i].begin,
				  doc->pages[i].end, "%%Page:");
	    fprintf(fp, "%%%%Page: %s %d\r\n",
		    doc->pages[i].label, page++);
	    free(comment);
	    pscopy(dfile, fp, -1, doc->pages[i].end);
	}
    }

    here = doc->begintrailer;
    while ( (comment = pscopyuntil(dfile, fp, here,
			   doc->endtrailer, "%%Pages:")) != (char *)NULL ) {
	here = ftell(dfile);
	if (pages_written) {
	    free(comment);
	    continue;
	}
	switch (sscanf(comment+length("%%Pages:"), "%*d %d", &i)) {
	    case 1:
		fprintf(fp, "%%%%Pages: %d %d\r\n", pages, i);
		break;
	    default:
		fprintf(fp, "%%%%Pages: %d\r\n", pages);
		break;
	}
	pages_written = TRUE;
	free(comment);
    }
}
#undef length
