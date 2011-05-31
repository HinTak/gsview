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
char * get_ports(void);
#define PORT_BUF_SIZE 4096

void
strip_spaces(char *s)
{
char *d = s;
   while (*s) {
	if (*s != ' ')
	    *d++ = *s;
	s++;
   }
   *d = '\0';
}

char editpropname[MAXSTR];

/* dialog for adding or editing properties */
BOOL CALLBACK _export
EditPropDlgProc(HWND hDlg, UINT wmsg, WPARAM wParam, LPARAM lParam)
{
static char device[MAXSTR];	/* contains printer device name */
    switch (wmsg) {
	case WM_INITDIALOG:
	    lstrcpy(device, (LPSTR)lParam);	/* initialise device name */
	    if (*editpropname) {
		char section[MAXSTR];
		char buf[MAXSTR];
		if (*editpropname == 's')
		    SendDlgItemMessage(hDlg, EDITPROP_STRING, BM_SETCHECK, 
			    (WPARAM)1, 0L);
		else
		    SendDlgItemMessage(hDlg, EDITPROP_NUMBER, BM_SETCHECK, 
			    (WPARAM)1, 0L);
		SetDlgItemText(hDlg, EDITPROP_NAME, editpropname+1);
		strcpy(section, device);
		strcat(section, " values");
		GetPrivateProfileString(section, editpropname, "", buf, sizeof(buf)-2, INIFILE);
		SetDlgItemText(hDlg, EDITPROP_VALUE, buf);
	    }
	    else
	        SendDlgItemMessage(hDlg, EDITPROP_NUMBER, BM_SETCHECK, (WPARAM)1, 0L);
	    return TRUE;
	case WM_COMMAND:
	  switch (LOWORD(wParam)) {
	    case EDITPROP_DEL:
		{
		char name[MAXSTR];
		char section[MAXSTR];
		if ( SendDlgItemMessage(hDlg, EDITPROP_STRING, 
		    BM_GETCHECK, (WPARAM)1, 0L) > 0) {
		    strcpy(name, "s");
		}
		else
		    strcpy(name, "d");
		GetDlgItemText(hDlg, EDITPROP_NAME, name+1, sizeof(name)-2);
		strip_spaces(name);
		if (strlen(name)>1) {
		    strcpy(section, device);
		    strcat(section, " values");
		    WritePrivateProfileString(section, name, NULL, INIFILE);
		    WritePrivateProfileString(device, name, NULL, INIFILE);
		}
		EndDialog(hDlg, TRUE);
		}
		return TRUE;
	    case ID_HELP:
		SendMessage(hwndimg, help_message, 0, 0L);
		return(FALSE);
	    case IDOK:
		{
		char name[MAXSTR];
		char value[MAXSTR];
		char section[MAXSTR];
		if ( SendDlgItemMessage(hDlg, EDITPROP_STRING, 
		    BM_GETCHECK, (WPARAM)1, 0L) > 0) {
		    strcpy(name, "s");
		}
		else
		    strcpy(name, "d");
		GetDlgItemText(hDlg, EDITPROP_NAME, name+1, sizeof(name)-2);
		GetDlgItemText(hDlg, EDITPROP_VALUE, value, sizeof(value)-1);
		strip_spaces(name);
		strip_spaces(value);
		if ((strlen(name)>1) && strlen(value)) {
		    strcpy(section, device);
		    strcat(section, " values");
		    WritePrivateProfileString(section, name, value, INIFILE);
		    strtok(value, ",");
		    WritePrivateProfileString(device, name, value, INIFILE);
		}
		EndDialog(hDlg, TRUE);
		}
		return TRUE;
	    case IDCANCEL:
		EndDialog(hDlg, FALSE);
		return TRUE;
	  }
	  break;
	}
	return FALSE;
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
	    SendDlgItemMessage(hDlg, PROP_NAME, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	    SendDlgItemMessage(hDlg, PROP_VALUE, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	    for (iprop=0; propitem[iprop].name[0]; iprop++) {
		SendDlgItemMessage(hDlg, PROP_NAME, CB_ADDSTRING, 0, 
		    (LPARAM)((LPSTR)propitem[iprop].name+1));
	    }
	    SendDlgItemMessage(hDlg, PROP_NAME, CB_SETCURSEL, 0, 0L);
	    /* force update of PROP_VALUE */
	    SendDlgNotification(hDlg, PROP_NAME, CBN_SELCHANGE);
	    EnableWindow(GetDlgItem(hDlg, PROP_NAME), (iprop != 0));
	    EnableWindow(GetDlgItem(hDlg, PROP_VALUE), (iprop != 0));
	    EnableWindow(GetDlgItem(hDlg, PROP_EDIT), (iprop != 0));

	    if (option.gsversion < IDM_GS351) {
		EnableWindow(GetDlgItem(hDlg, PROP_XOFFSET), FALSE);
		EnableWindow(GetDlgItem(hDlg, PROP_YOFFSET), FALSE);
	    }
	    else {
		strcpy(section, device);
		strcat(section, " PageOffset");
		GetPrivateProfileString(section, "X", "0", buf, sizeof(buf)-2, INIFILE);
		SetDlgItemText(hDlg, PROP_XOFFSET, buf);
		GetPrivateProfileString(section, "Y", "0", buf, sizeof(buf)-2, INIFILE);
		SetDlgItemText(hDlg, PROP_YOFFSET, buf);
	    }

	    SetFocus(GetDlgItem(hDlg, IDOK));
	    return TRUE;
	case WM_COMMAND:
	    notify_message = GetNotification(wParam,lParam);
	    switch (LOWORD(wParam)) {
		case ID_HELP:
		    load_string(IDS_TOPICPROP, szHelpTopic, sizeof(szHelpTopic));
		    SendMessage(hwndimg, help_message, 0, 0L);
		    load_string(IDS_TOPICPRINT, szHelpTopic, sizeof(szHelpTopic));
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
		case PROP_EDIT:
		    load_string(IDS_TOPICEDITPROP, szHelpTopic, sizeof(szHelpTopic));
		    iprop = (int)SendDlgItemMessage(hDlg, PROP_NAME, CB_GETCURSEL, 0, 0L);
		    editpropname[0] = '\0';
		    if (iprop != CB_ERR)
			strcpy(editpropname, propitem[iprop].name);
#ifdef __WIN32__
		    DialogBoxParam( phInstance, "EditPropDlgBox", hDlg, EditPropDlgProc, (LPARAM)device);
#else
		    {DLGPROC lpProcProp;
			lpProcProp = (DLGPROC)MakeProcInstance((FARPROC)EditPropDlgProc, phInstance);
			DialogBoxParam( phInstance, "EditPropDlgBox", hDlg, lpProcProp, (LPARAM)device);
			FreeProcInstance((FARPROC)lpProcProp);
		    }
#endif
		    free((char *)propitem);
		    SendMessage(hDlg, WM_INITDIALOG, (WPARAM)hDlg, (LPARAM)device);
		    load_string(IDS_TOPICPROP, szHelpTopic, sizeof(szHelpTopic));
		    SendDlgItemMessage(hDlg, IDOK, BM_SETSTYLE, 
			    (WPARAM)BS_DEFPUSHBUTTON, MAKELPARAM(TRUE, 0));
		    SendDlgItemMessage(hDlg, PROP_EDIT, BM_SETSTYLE, 
			    (WPARAM)BS_PUSHBUTTON, MAKELPARAM(TRUE, 0));
		    return FALSE;
		case PROP_NEW:
		    load_string(IDS_TOPICEDITPROP, szHelpTopic, sizeof(szHelpTopic));
		    editpropname[0] = '\0';
#ifdef __WIN32__
		    DialogBoxParam( phInstance, "EditPropDlgBox", hDlg, EditPropDlgProc, (LPARAM)device);
#else
		    {DLGPROC lpProcProp;
			lpProcProp = (DLGPROC)MakeProcInstance((FARPROC)EditPropDlgProc, phInstance);
			DialogBoxParam( phInstance, "EditPropDlgBox", hDlg, lpProcProp, (LPARAM)device);
			FreeProcInstance((FARPROC)lpProcProp);
		    }
#endif
		    free((char *)propitem);
		    SendMessage(hDlg, WM_INITDIALOG, (WPARAM)hDlg, (LPARAM)device);
		    load_string(IDS_TOPICPROP, szHelpTopic, sizeof(szHelpTopic));
		    SendDlgItemMessage(hDlg, IDOK, BM_SETSTYLE, 
			    (WPARAM)BS_DEFPUSHBUTTON, MAKELPARAM(TRUE, 0));
		    SendDlgItemMessage(hDlg, PROP_NEW, BM_SETSTYLE, 
			    (WPARAM)BS_PUSHBUTTON, MAKELPARAM(TRUE, 0));
		    return FALSE;
		case IDOK:
		    for (iprop=0; propitem[iprop].name[0]; iprop++) {
			WritePrivateProfileString(device, propitem[iprop].name, propitem[iprop].value, INIFILE);
		    }
		    strcpy(section, device);
		    strcat(section, " PageOffset");
		    GetDlgItemText(hDlg, PROP_XOFFSET, buf, sizeof(buf-2));
		    WritePrivateProfileString(section, "X", buf, INIFILE);
		    GetDlgItemText(hDlg, PROP_YOFFSET, buf, sizeof(buf-2));
		    WritePrivateProfileString(section, "Y", buf, INIFILE);
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


#ifdef OLD
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
#else
char *device_queue_list;
int device_to_file;
int device_queue_index;

/* dialog box for selecting printer device and resolution */
BOOL CALLBACK _export
DeviceDlgProc(HWND hDlg, UINT wmsg, WPARAM wParam, LPARAM lParam)
{
	char buf[128];
	int i, idevice;
	WORD notify_message;
	char *p;
	char *res;
	char entry[MAXSTR];
	struct prop_item_s *proplist;
#ifndef __WIN32__
	static DLGPROC lpProcPage;
#endif

	switch (wmsg) {
	    case WM_INITDIALOG:
#ifndef __WIN32__
		lpProcPage = (DLGPROC)MakeProcInstance((FARPROC)PageDlgProc, phInstance);
#endif
		p = get_devices();
		res = p;	/* save for free() */
		while ( p!=(char *)NULL && strlen(p)!=0) {
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
		/* insert queue list */
		p = device_queue_list;
		device_queue_index = 0;
	        idevice = 0;
		while (*p) {
		    if ( strcmp(p, option.printer_port) == 0 )
		        device_queue_index = idevice;
		    SendDlgItemMessage(hDlg, SPOOL_PORT, LB_ADDSTRING, 0, (LPARAM)p);
		    p += lstrlen(p)+1;
		    idevice++;
		}
	        SendDlgItemMessage(hDlg, SPOOL_PORT, LB_SETCURSEL, device_queue_index, (LPARAM)0);
		/* fill in page list box */
		if ( (doc != (PSDOC *)NULL) && (doc->numpages != 0)) {
		    page_list.current = psfile.pagenum-1;
		    page_list.multiple = TRUE;
#ifdef __WIN32__
		    PageDlgProc(hDlg, wmsg, wParam, lParam);
#else
		    CallWindowProc((WNDPROC)lpProcPage, hDlg, wmsg, wParam, lParam);
#endif
		}
		else {
		    page_list.multiple = FALSE;
		    EnableWindow(GetDlgItem(hDlg, PAGE_ALL), FALSE);
		    EnableWindow(GetDlgItem(hDlg, PAGE_ODD), FALSE);
		    EnableWindow(GetDlgItem(hDlg, PAGE_EVEN), FALSE);
		    SendDlgItemMessage(hDlg, PAGE_LIST, LB_ADDSTRING, 0, 
			(LPARAM)((LPSTR)"All"));
		    EnableWindow(GetDlgItem(hDlg, PAGE_LISTTEXT), FALSE);
		    EnableWindow(GetDlgItem(hDlg, PAGE_LIST), FALSE);
		}
		/* set Print to File check box */
		if (device_to_file) {
		    SendDlgItemMessage(hDlg, SPOOL_TOFILE, BM_SETCHECK, 1, 0);
		    EnableWindow(GetDlgItem(hDlg, SPOOL_PORT), FALSE);
		    EnableWindow(GetDlgItem(hDlg, SPOOL_PORTTEXT), FALSE);
		}
		return TRUE;
	    case WM_COMMAND:
		notify_message = GetNotification(wParam,lParam);
		switch (LOWORD(wParam)) {
		    case PAGE_LIST:
		    case SPOOL_PORT:
			if (notify_message == LBN_DBLCLK)
			    PostMessage(hDlg, WM_COMMAND, IDOK, 0L);
			return FALSE;
		    case SPOOL_TOFILE:
			if (notify_message == BN_CLICKED) {
		    	    i = (int)SendDlgItemMessage(hDlg, SPOOL_TOFILE, BM_GETCHECK, 0, 0);
			    /* toggle state */
			    i = (i == 0) ? 1 : 0;
			    SendDlgItemMessage(hDlg, SPOOL_TOFILE, BM_SETCHECK, i, 0);
			    if (i) {  /* save selection */
				device_queue_index = (int)SendDlgItemMessage(hDlg, SPOOL_PORT, LB_GETCURSEL, 0, 0L);
			    }
			    /* can't clear selection */
			    EnableWindow(GetDlgItem(hDlg, SPOOL_PORT), (i ? FALSE : TRUE));
			    EnableWindow(GetDlgItem(hDlg, SPOOL_PORTTEXT), (i ? FALSE : TRUE));
			}
			return FALSE;
		    case ID_HELP:
		        SendMessage(hwndimg, help_message, 0, 0L);
		        return FALSE;
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
		    case PAGE_ALL:
		    case PAGE_EVEN:
		    case PAGE_ODD:
#ifdef __WIN32__
		    	PageDlgProc(hDlg, wmsg, wParam, lParam);
#else
			CallWindowProc((WNDPROC)lpProcPage, hDlg, wmsg, wParam, lParam);
#endif
			return FALSE;
		    case IDOK:
			/* save device name and resolution */
		        GetDlgItemText(hDlg, DEVICE_NAME, option.device_name, sizeof(option.device_name));
		        GetDlgItemText(hDlg, DEVICE_RES, option.device_resolution, sizeof(option.device_resolution));
			/* get Print to File status */
		 	device_to_file = (int)SendDlgItemMessage(hDlg, 
			    SPOOL_TOFILE, BM_GETCHECK, 0, 0);
			if (!device_to_file) {
			    /* save queue name */
			    SendDlgItemMessage(hDlg, SPOOL_PORT, LB_GETTEXT, 
				(int)SendDlgItemMessage(hDlg, SPOOL_PORT, LB_GETCURSEL, 0, 0L),
				(LPARAM)(LPSTR)option.printer_port);
			}
			/* get pages */
			if ((doc != (PSDOC *)NULL) && (doc->numpages != 0))
#ifdef __WIN32__
			    PageDlgProc(hDlg, wmsg, wParam, lParam);
#else
			    CallWindowProc((WNDPROC)lpProcPage, hDlg, wmsg, wParam, lParam);
#endif
#ifndef __WIN32__
			FreeProcInstance((FARPROC)lpProcPage);
#endif
			EndDialog(hDlg, TRUE);
			return TRUE;
		    case IDCANCEL:
			EndDialog(hDlg, FALSE);
#ifndef __WIN32__
			FreeProcInstance((FARPROC)lpProcPage);
#endif
			return TRUE;
		}
		break;
	}
	return FALSE;
}


int
get_device(int to_file)
{
int result;
#ifndef __WIN32__
DLGPROC lpProcDevice;
#endif
#define DEVICE_BUF_SIZE 4096
    device_to_file = to_file;
    device_queue_list = get_ports();
    if (device_queue_list == (char *)NULL)
	return FALSE;

    load_string(IDS_TOPICPRINT, szHelpTopic, sizeof(szHelpTopic));
#ifdef __WIN32__
    result = DialogBoxParam( phInstance, "DeviceDlgBox", hwndimg, DeviceDlgProc, (LPARAM)NULL);
#else
    lpProcDevice = (DLGPROC)MakeProcInstance((FARPROC)DeviceDlgProc, phInstance);
    result = DialogBoxParam( phInstance, "DeviceDlgBox", hwndimg, lpProcDevice, (LPARAM)NULL);
    FreeProcInstance((FARPROC)lpProcDevice);
#endif
    free(device_queue_list);
    if (result != IDOK)
	return FALSE;
    return TRUE;
}


#endif



#pragma argsused
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
char *
get_queues(void)
{
int i;
DWORD count, needed;
PRINTER_INFO_1 *prinfo;
char *enumbuffer;
char *buffer;
char *p;
    /* enumerate all available printers */
    EnumPrinters(PRINTER_ENUM_CONNECTIONS | PRINTER_ENUM_LOCAL, NULL, 1, NULL, 0, &needed, &count);
    enumbuffer = malloc(needed);
    if (enumbuffer == (char *)NULL)
	return FALSE;
    if (!EnumPrinters(PRINTER_ENUM_CONNECTIONS | PRINTER_ENUM_LOCAL, NULL, 1, (LPBYTE)enumbuffer, needed, &needed, &count)) {
	char buf[256];
	free(enumbuffer);
	sprintf(buf, "EnumPrinters() failed, error code = %d", GetLastError());
	gserror(0, buf, MB_ICONHAND, SOUND_ERROR);
	return NULL;
    }
    prinfo = (PRINTER_INFO_1 *)enumbuffer;
    if ((buffer = malloc(PORT_BUF_SIZE)) == (char *)NULL) {
	free(enumbuffer);
	return NULL;
    }
    /* copy printer names to single buffer */
    p = buffer;
    for (i=0; i<count; i++) {
	if (strlen(prinfo[i].pName) + 1 < (PORT_BUF_SIZE- (p-buffer))) {
	    strcpy(p, prinfo[i].pName);
	    p += strlen(p) + 1;
	}
    }
    *p = '\0';	/* double null at end */
    free(enumbuffer);
    return buffer;
}

/* return TRUE if queuename available */
/* return FALSE if cancelled or error */
/* if queue non-NULL, use as suggested queue */
BOOL
get_queuename(char *portname, char *queue)
{
char *buffer;
char *p;
int i, iport;

    buffer = get_queues();
    if ( (queue == (char *)NULL) || (strlen(queue)==0) ) {
	/* select a queue */
	iport = DialogBoxParam(phInstance, "QueueDlgBox", hwndtext, SpoolDlgProc, (LPARAM)buffer);
	if (!iport) {
	    free(buffer);
	    return FALSE;
	}
	p = buffer;
	for (i=1; i<iport && strlen(p)!=0; i++)
	    p += lstrlen(p)+1;
	/* prepend \\spool\ which is used by Ghostscript to distinguish */
	/* real files from queues */
	strcpy(portname, "\\\\spool\\");
	strcat(portname, p);
    }
    else {
	strcpy(portname, "\\\\spool\\");
	strcat(portname, queue);
    }

    free(buffer);
    return TRUE;
}
#endif


/* return TRUE if portname available */
/* return FALSE if cancelled or error */
/* if port non-NULL, use as suggested port */
BOOL
get_portname(char *portname, char *port)
{
char *buffer;
char *p;
int i, iport;
char filename[MAXSTR];
#ifdef __WIN32__
	if (is_win95 || is_winnt)
	    return get_queuename(portname, port);
#endif

        buffer = get_ports();
	if ( (port == (char *)NULL) || (strlen(port)==0) ) {
	    if (buffer == (char *)NULL)
		return NULL;
	    /* select a port */
	    iport = DialogBoxParam(phInstance, "SpoolDlgBox", hwndtext, SpoolDlgProc, (LPARAM)buffer);
	    if (!iport) {
	        free(buffer);
	        return FALSE;
	    }
	    p = buffer;
	    for (i=1; i<iport && strlen(p)!=0; i++)
	        p += lstrlen(p)+1;
	    strcpy(portname, p);
	}
	else
	    strcpy(portname, port);

	if (strlen(portname) == 0)
	    return FALSE;
	if (strcmp(portname,"FILE:") == 0) {
	    strcpy(filename, "*.prn");
	    if (!get_filename(filename, TRUE, FILTER_ALL, IDS_PRINTFILE, IDS_TOPICPRINT)) {
	        free(buffer);
	        return FALSE;
	    }
	    strcpy(portname, filename);
	}
#ifdef NOTUSED
	else {
	    portname[strlen(portname)-1] = '\0';  /* remove trailing colon */
	}
#endif
	free(buffer);
	return TRUE;
}


char *
get_ports(void)
{
char *buffer;
#ifdef __WIN32__
	if (is_win95 || is_winnt)
	    return get_queues();
#endif

	if ((buffer = malloc(PORT_BUF_SIZE)) == (char *)NULL)
	    return NULL;
	GetProfileString("ports", NULL, "", buffer, PORT_BUF_SIZE);
	return buffer;
}




/******************************************************************/
/* Print File to port or queue */
/* port==NULL means prompt for port or queue with dialog box */
int gp_printfile(char *filename, char *port);

/* This is messy because Microsoft changed the spooler interface */
/* between Window 3.1 and Windows 95/NT */
/* and didn't provide the spooler interface in Win32s */

/* This code requires several different versions */
/* Win16: Use OpenJob etc. */
int gp_printfile_win16(char *filename, char *port);

/* Win95, WinNT: Use OpenPrinter, WritePrinter etc. */
int gp_printfile_win32(char *filename, char *port);

/* Win32s: Pass to Win16 spooler via gsv16spl.exe */
int gp_printfile_gsv16spl(char *filename, char *port);

/* Currently not used, because Universal Thunk example code won't work */
/* Use Win16 OpenJob via Universal Thunk */
int gp_printfile_win32s(char *filename, char *port);

/* Currently not used, because it works on some PCs and not on others */
/* This writes direct to a printer port */
int gp_printfile_port(char *filename, char *port);

/* Currently not used, the following one tries to sneak code */
/* through a Windows printer driver */
/* This usually works for Epson compatible drivers, */
/* but does not work for PostScript or Generic/Text only drivers */
int gp_printfile_gdi32(char *filename, char *port);

int gp_printfile_test(char *filename, char *port);

int gp_printfile(char *filename, char *port)
{
#ifdef __WIN32__
    if (is_win95 || is_winnt)
	return gp_printfile_win32(filename, port);
/*
    return gp_printfile_win32s(filename, port);
    return gp_printfile_port(filename, port);
*/
    return gp_printfile_gsv16spl(filename, port);
#else
    /* Win16 */
    return gp_printfile_win16(filename, port);
#endif
   
}

#define PRINT_BUF_SIZE 16384u

#ifndef __WIN32__
/* Win16 method using OpenJob etc. */
int
gp_printfile_win16(char *filename, char *port)
{
char *buffer;
char portname[MAXSTR];
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

    if (!get_portname(portname, port))
	return FALSE;

    if ((buffer = malloc(PRINT_BUF_SIZE)) == (char *)NULL)
	return FALSE;
    
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

#else /* __WIN32__ */

/* True Win32 method, using OpenPrinter, WritePrinter etc. */
int 
gp_printfile_win32(char *filename, char *port)
{
DWORD count;
char *buffer;
char portname[MAXSTR];
FILE *f;
HANDLE printer;
DOC_INFO_1 di;
DWORD written;

    if (!get_portname(portname, port))
	return FALSE;
    port = portname + 8;	/* skip over \\spool\ */

    if ((buffer = malloc(PRINT_BUF_SIZE)) == (char *)NULL)
        return FALSE;
	
    if ((f = fopen(filename, "rb")) == (FILE *)NULL) {
	free(buffer);
	return FALSE;
    }


    /* open a printer */
    if (!OpenPrinter(port, &printer, NULL)) {
	char buf[256];
	sprintf(buf, "OpenPrinter() failed for \042%s\042, error code = %d", port, GetLastError());
	gserror(0, buf, MB_ICONHAND, SOUND_ERROR);
	free(buffer);
	return FALSE;
    }
    /* from here until ClosePrinter, should AbortPrinter on error */

    di.pDocName = filename;
    di.pOutputFile = NULL;
    di.pDatatype = "RAW";  /* for available types see EnumPrintProcessorDatatypes */
    if (!StartDocPrinter(printer, 1, (LPBYTE)&di)) {
	char buf[256];
	sprintf(buf, "StartDocPrinter() failed, error code = %d", GetLastError());
	gserror(0, buf, MB_ICONHAND, SOUND_ERROR);
	AbortPrinter(printer);
	free(buffer);
	return FALSE;
    }
   

    while ((count = fread(buffer, 1, PRINT_BUF_SIZE, f)) != 0 ) {
	if (!WritePrinter(printer, (LPVOID)buffer, count, &written)) {
	    free(buffer);
	    fclose(f);
	    AbortPrinter(printer);
	    return FALSE;
	}
    }
    fclose(f);
    free(buffer);

    if (!EndDocPrinter(printer)) {
	char buf[256];
	sprintf(buf, "EndDocPrinter() failed, error code = %d", GetLastError());
	gserror(0, buf, MB_ICONHAND, SOUND_ERROR);
	AbortPrinter(printer);
	return FALSE;
    }

    if (!ClosePrinter(printer)) {
	char buf[256];
	sprintf(buf, "ClosePrinter() failed, error code = %d", GetLastError());
	gserror(0, buf, MB_ICONHAND, SOUND_ERROR);
	return FALSE;
    }
    return TRUE;
}


#ifdef NOTUSED
/* Win32s */
/* try to load spool32.dll to get access to Win16 OpenJob etc. */
/* via Universal Thunk */
int
gp_printfile_win32s(char *filename, char *port)
{
HMODULE hlib_spool;
pfnOpenJob OpenJob;
pfnStartSpoolPage StartSpoolPage;
pfnEndSpoolPage EndSpoolPage;
pfnWriteSpool WriteSpool;
pfnCloseJob CloseJob;
pfnDeleteJob DeleteJob;

char *buffer;
char portname[MAXSTR];
HANDLE hJob;
UINT count;
FILE *f;
int error = FALSE;
long lsize;
long ldone;
char fmt[MAXSTR];
char pcdone[10];
MSG msg;

    hlib_spool = LoadLibrary("SPOOL32.DLL");
    if (hlib_spool == NULL)	/* if can't find it, try other method */
	return gp_printfile_port(filename, port);

    /* get pointers to all the routines */
    OpenJob = (pfnOpenJob)GetProcAddress(hlib_spool, "OpenJob");
    StartSpoolPage = (pfnStartSpoolPage)GetProcAddress(hlib_spool, "StartSpoolPage");
    EndSpoolPage = (pfnEndSpoolPage)GetProcAddress(hlib_spool, "EndSpoolPage");
    WriteSpool = (pfnWriteSpool)GetProcAddress(hlib_spool, "WriteSpool");
    CloseJob = (pfnCloseJob)GetProcAddress(hlib_spool, "CloseJob");
    DeleteJob = (pfnDeleteJob)GetProcAddress(hlib_spool, "DeleteJob");

    if (!get_portname(portname, port)) {
	FreeLibrary(hlib_spool);
	return FALSE;
    }

    if ((buffer = malloc(PRINT_BUF_SIZE)) == (char *)NULL) {
	FreeLibrary(hlib_spool);
	return FALSE;
    }
    
    if ((f = fopen(filename, "rb")) == (FILE *)NULL) {
	FreeLibrary(hlib_spool);
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
	    FreeLibrary(hlib_spool);
	    return FALSE;
    }
    if (StartSpoolPage(hJob) < 0)
	error = TRUE;

    hDlgModeless = CreateDialog(phInstance, "CancelDlgBox", hwndimg, CancelDlgProc);
    ldone = 0;
    LoadString(phInstance, IDS_CANCELDONE, fmt, sizeof(fmt));

    while (!error && hDlgModeless 
      && (count = fread(buffer, 1, PRINT_BUF_SIZE, f)) != 0 ) {
	if (WriteSpool(hJob, buffer, (WORD)count) < 0)
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
    EndSpoolPage(hJob);
    if (error)
	DeleteJob(hJob, 0);
    else
	CloseJob(hJob);
    FreeLibrary(hlib_spool);
    return !error;

}


/* Open port as a file */
/* Works under Win16 and Win32, but not needed under Win16 */
int
gp_printfile_port(char *filename, char *port)
{
/* Get printer port list from win.ini, then copy to \\.\port */
char *buffer;
char portname[MAXSTR];
FILE *f;
long count;
int error = FALSE;
long lsize;
long ldone;
char pcdone[20];
MSG msg;
FILE *outfile;

	if (!get_portname(portname, port))
	    return FALSE;

	if ((buffer = malloc(PRINT_BUF_SIZE)) == (char *)NULL)
	    return FALSE;

	if ((f = fopen(filename, "rb")) == (FILE *)NULL) {
	    free(buffer);
	    return FALSE;
	}
	fseek(f, 0L, SEEK_END);
	lsize = ftell(f);
	if (lsize <= 0)
	    lsize = 1;
	fseek(f, 0L, SEEK_SET);

	
	outfile = fopen(portname, "wb");
	if (outfile == (FILE *)NULL) {
		fclose(f);
		free(buffer);
		return FALSE;
	}

	hDlgModeless = CreateDialog(phInstance, "CancelDlgBox", hwndtext, CancelDlgProc);
	ldone = 0;

	while (!error && hDlgModeless 
	  && (count = fread(buffer, 1, PRINT_BUF_SIZE, f)) != 0 ) {
	    if (fwrite(buffer, 1, count, outfile) < count)
		error = TRUE;
	    ldone += count;
	    sprintf(pcdone, "%d %%done", (int)(ldone * 100 / lsize));
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
	fclose(outfile);
	return !error;
}
#endif

/* Start a 16-bit application gsv16spl.exe and pass printer data in */
/* global memory.  gsv16spl.exe then uses 16-bit spooler functions. */
/* Only works under Win16 and Win32s */
/* Intended for Win32s where 16-bit spooler functions are not available */
/* and Win32 spooler functions are not implemented. */
int
gp_printfile_gsv16spl(char *filename, char *port)
{
/* Get printer port list from win.ini */
char *buffer;
char portname[MAXSTR];
FILE *f;
unsigned int count;
int error = FALSE;
long lsize;
long ldone;
char pcdone[20];
MSG msg;
DLGPROC lpfnCancelProc;
HINSTANCE hinst;
char command[MAXSTR];
HGLOBAL hmem;
LPBYTE data;

	if (!get_portname(portname, port))
	    return FALSE;

	if ((buffer = malloc(PRINT_BUF_SIZE)) == (char *)NULL)
	    return FALSE;

	hmem = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, PRINT_BUF_SIZE+sizeof(WORD));
	if (hmem == (HGLOBAL)NULL) {
	    free(buffer);
	    gserror(0, "Can't allocate global memory for gsv16spl", NULL, SOUND_ERROR);
	    return FALSE;
	}

	if ((f = fopen(filename, "rb")) == (FILE *)NULL) {
	    free(buffer);
	    GlobalFree(hmem);
	    return FALSE;
	}
	fseek(f, 0L, SEEK_END);
	lsize = ftell(f);
	if (lsize <= 0)
	    lsize = 1;
	fseek(f, 0L, SEEK_SET);

	data = GlobalLock(hmem);
	lstrcpy(((LPSTR)data)+2, portname);
	*((LPWORD)data) = (WORD)(lstrlen(portname)+1);
	GlobalUnlock(hmem);
	
	strcpy(command, szExePath);
	strcat(command, "gsv16spl.exe");
	sprintf(command+strlen(command), " %lu", (unsigned long)hwndimg);
	hinst = (HINSTANCE)WinExec(command, SW_SHOWMINNOACTIVE);
#ifdef __WIN32__
	if (hinst == NULL)
#else
	if (hinst < HINSTANCE_ERROR)
#endif
	{
	    fclose(f);
	    free(buffer);
	    GlobalFree(hmem);
	    gserror(IDS_CANNOTRUN, command, MB_ICONSTOP, SOUND_ERROR);
	    return FALSE;
	}
	if (hwndspl == (HWND)NULL) {
	    fclose(f);
	    free(buffer);
	    GlobalFree(hmem);
	    gserror(0, "gsv16spl didn't give us a window handle", NULL, SOUND_ERROR);
	    return FALSE;	/* gsv16spl didn't SendMessage to us */
	}

	if (!SendMessage(hwndspl, WM_GSV16SPL, 0, (LPARAM)hmem)) {
	    fclose(f);
	    free(buffer);
	    GlobalFree(hmem);
	    gserror(0, "gsv16spl couldn't start printer job", NULL, SOUND_ERROR);
	    return FALSE;
	}


#ifdef __WIN32__
	hDlgModeless = CreateDialog(phInstance, "CancelDlgBox", hwndtext, CancelDlgProc);
#else
        lpfnCancelProc = (DLGPROC)MakeProcInstance((FARPROC)CancelDlgProc, phInstance);
        hDlgModeless = CreateDialog(phInstance, "CancelDlgBox", hwndimg, lpfnCancelProc);
#endif
	ldone = 0;

	data = GlobalLock(hmem);
	while (!error && hDlgModeless 
	  && (count = fread(buffer, 1, PRINT_BUF_SIZE, f)) != 0 ) {
	    *((LPWORD)data) = (WORD)count;
#ifdef __WIN32__
	    memcpy(((LPSTR)data)+2, buffer, count);
#else
	    _fmemcpy(((LPSTR)data)+2, buffer, count);
#endif
	    GlobalUnlock(hmem);
	    if (!SendMessage(hwndspl, WM_GSV16SPL, 0, (LPARAM)hmem))
		error = TRUE;
	    ldone += count;
	    sprintf(pcdone, "%d %%done", (int)(ldone * 100 / lsize));
	    SetWindowText(GetDlgItem(hDlgModeless, CANCEL_PCDONE), pcdone);
	    while (PeekMessage(&msg, hDlgModeless, 0, 0, PM_REMOVE)) {
		if ((hDlgModeless == 0) || !IsDialogMessage(hDlgModeless, &msg)) {
		    TranslateMessage(&msg);
		    DispatchMessage(&msg);
  		}
  	    }
	    data = GlobalLock(hmem);
  	}
	fclose(f);

	if (!hDlgModeless)
	    error=TRUE;

	if (error)
	    *((LPWORD)data) = 0xffff;	/* abort */
	else
	    *((LPWORD)data) = 0;	/* EOF */
	GlobalUnlock(hmem);
        SendMessage(hwndspl, WM_GSV16SPL, 0, (LPARAM)hmem);
	GlobalFree(hmem);
	free(buffer);

	DestroyWindow(hDlgModeless);
	hDlgModeless = 0;
#ifndef __WIN32__
	FreeProcInstance((FARPROC)lpfnCancelProc);
#endif
	return !error;
}


#endif /* __WIN32__ */


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
	char command[MAXSTR+MAXSTR];
	
	if (psfile.name[0] == '\0') {
		gserror(IDS_NOTOPEN, NULL, MB_ICONEXCLAMATION, SOUND_NOTOPEN);
		return;
	}

	LoadString(phInstance, IDS_TOPICPRINT, szHelpTopic, sizeof(szHelpTopic));
	
	if (!get_device(to_file))
	    return;

	info_wait(IDS_WAITGSCLOSE);
	gs_close();	/* we need a new Ghostscript */
	info_wait(IDS_NOWAIT);

	if (!gsview_cprint(device_to_file, pcfname, pfname))
	    return;

	sprintf(command,"%s %s -sGSVIEW=%u @%s", option.gsexe, option.gsother,
		(unsigned int)hwndimg, pfname);

	if (strlen(command) > 126) {
		/* command line too long */
		gserror(IDS_TOOLONG, command, MB_ICONSTOP, SOUND_ERROR);
		unlink(pfname);
		pfname[0] = '\0';
		gsprog.hinst = (HINSTANCE)NULL;
		return;
	}
	info_wait(IDS_WAITGSOPEN);
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
		info_wait(IDS_NOWAIT);
		gsprog.hinst = (HINSTANCE)NULL;
		return;
	}

/*
	set_timer(timeout*pages);
*/
	info_wait(IDS_WAITPRINT);
	return;
}


