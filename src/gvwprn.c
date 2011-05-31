/* Copyright (C) 1993-1998, Ghostgum Software Pty Ltd.  All rights reserved.
  
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

char not_defined[] = "[Not defined]";
char * get_ports(void);
char * get_queues(void);
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
		GetPrivateProfileString(section, editpropname, "", buf, sizeof(buf)-2, szIniFile);
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
		    WritePrivateProfileString(section, name, NULL, szIniFile);
		    WritePrivateProfileString(device, name, NULL, szIniFile);
		}
		EndDialog(hDlg, TRUE);
		}
		return TRUE;
	    case ID_HELP:
		get_help();
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
		    WritePrivateProfileString(section, name, value, szIniFile);
		    strtok(value, ",");
		    WritePrivateProfileString(device, name, value, szIniFile);
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
    static char notdef[128];
    static char device[MAXSTR];	/* contains printer device name */
    static struct prop_item_s* propitem;
    char section[MAXSTR];

    switch (wmsg) {
	case WM_INITDIALOG:
	    lstrcpy(device, (LPSTR)lParam);	/* initialise device name */
	    load_string(IDS_NOTDEFTAG, notdef, sizeof(notdef));
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

	    strcpy(section, device);
	    strcat(section, " Options");
	    GetPrivateProfileString(section, "Xoffset", "0", buf, sizeof(buf)-2, szIniFile);
	    SetDlgItemText(hDlg, PROP_XOFFSET, buf);
	    GetPrivateProfileString(section, "Yoffset", "0", buf, sizeof(buf)-2, szIniFile);
	    SetDlgItemText(hDlg, PROP_YOFFSET, buf);

	    SetFocus(GetDlgItem(hDlg, IDOK));
	    return TRUE;
	case WM_COMMAND:
	    notify_message = GetNotification(wParam,lParam);
	    switch (LOWORD(wParam)) {
		case ID_HELP:
		    load_string(IDS_TOPICPROP, szHelpTopic, sizeof(szHelpTopic));
		    get_help();
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
		    SendDlgItemMessage(hDlg, PROP_VALUE, CB_RESETCONTENT, 0, 0L);
		    SendDlgItemMessage(hDlg, PROP_VALUE, CB_ADDSTRING, 0, 
			(LPARAM)((LPSTR)notdef));
		    strcpy(section, device);
		    strcat(section, " values");
		    GetPrivateProfileString(section, propitem[iprop].name, "", buf, sizeof(buf)-2, szIniFile);
		    buf[strlen(buf)+1] = '\0';	/* put double NULL at end */
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
		    strcpy(buf, propitem[iprop].value);
		    if (strcmp(buf, not_defined)==0)
			strcpy(buf, notdef);
		    SendDlgItemMessage(hDlg, PROP_VALUE, CB_SELECTSTRING, -1, (LPARAM)(LPSTR)buf);
		    SetDlgItemText(hDlg, PROP_VALUE, buf);
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
			if (strcmp(propitem[iprop].value, notdef)==0)
			    strcpy(propitem[iprop].value, not_defined);
		    }
		    if (notify_message == CBN_EDITCHANGE) {
			iprop = (int)SendDlgItemMessage(hDlg, PROP_NAME, CB_GETCURSEL, 0, 0L);
			if (iprop == CB_ERR)
			    return FALSE;
			GetDlgItemText(hDlg, PROP_VALUE, (LPSTR)propitem[iprop].value, sizeof(propitem->value));
			if (strcmp(propitem[iprop].value, notdef)==0)
			    strcpy(propitem[iprop].value, not_defined);
		    }
		    return FALSE;
		case PROP_EDIT:
		    load_string(IDS_TOPICEDITPROP, szHelpTopic, sizeof(szHelpTopic));
		    iprop = (int)SendDlgItemMessage(hDlg, PROP_NAME, CB_GETCURSEL, 0, 0L);
		    editpropname[0] = '\0';
		    if (iprop != CB_ERR)
			strcpy(editpropname, propitem[iprop].name);
#ifdef __WIN32__
		    DialogBoxParam(hlanguage, "EditPropDlgBox", hDlg, EditPropDlgProc, (LPARAM)device);
#else
		    {DLGPROC lpProcProp;
			lpProcProp = (DLGPROC)MakeProcInstance((FARPROC)EditPropDlgProc, phInstance);
			DialogBoxParam(hlanguage, "EditPropDlgBox", hDlg, lpProcProp, (LPARAM)device);
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
		    DialogBoxParam(hlanguage, "EditPropDlgBox", hDlg, EditPropDlgProc, (LPARAM)device);
#else
		    {DLGPROC lpProcProp;
			lpProcProp = (DLGPROC)MakeProcInstance((FARPROC)EditPropDlgProc, phInstance);
			DialogBoxParam(hlanguage, "EditPropDlgBox", hDlg, lpProcProp, (LPARAM)device);
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
			WritePrivateProfileString(device, propitem[iprop].name, propitem[iprop].value, szIniFile);
		    }
		    strcpy(section, device);
		    strcat(section, " Options");
		    GetDlgItemText(hDlg, PROP_XOFFSET, buf, sizeof(buf)-2);
		    WritePrivateProfileString(section, "Xoffset", buf, szIniFile);
		    GetDlgItemText(hDlg, PROP_YOFFSET, buf, sizeof(buf)-2);
		    WritePrivateProfileString(section, "Yoffset", buf, szIniFile);
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


LPSTR
GetUPPname(HWND hwnd, LPSTR uppname, int upplen)
{
    LPSTR p, q;
    uppname[0] = '\0';
    GetDlgItemText(GetParent(hwnd), 
	    DEVICE_OPTIONS, (LPSTR)uppname, upplen);
    p = uppname;
    if (p[0] == '"') {
	/* remove quotes around configuration file */
	p++;
	q = strchr(p, '"');
	if (q != (LPSTR)NULL)
	    *q = '\0';
    }
    if (p[0] == '@')
	p++;
    if (strlen(p))
        memmove(uppname, p, strlen(p)+1);
    return uppname;
}

/* dialog box for selecting uniprint configuration file */
#ifdef __BORLANDC__
#pragma argsused
#endif
BOOL CALLBACK _export
UniDlgProc(HWND hDlg, UINT wmsg, WPARAM wParam, LPARAM lParam)
{
    WORD notify_message;
    LPSTR p, q;
    char uppname[MAXSTR];	/* contains printer device name */
    static LPSTR ubuf;
    LPSTR desc;
    int i;

    switch (wmsg) {
	case WM_INITDIALOG:
	    ubuf = NULL;
	    /* Delay initialization of the list box until 
	     * after it is displayed, because searching for 
	     * configuration files takes several seconds.
	     */
	    load_string(IDS_WAIT, uppname, sizeof(uppname));
	    SendDlgItemMessage(hDlg, UPP_LIST, LB_RESETCONTENT, 0, (LPARAM)0);
	    SendDlgItemMessage(hDlg, UPP_LIST, LB_ADDSTRING, 
		0, (LPARAM)uppname);
	    EnableWindow(GetDlgItem(hDlg, UPP_LIST), FALSE);
	    uppname[0] = uppname[1] = '\0';
	    GetUPPname(hDlg, uppname, sizeof(uppname));
	    SetDlgItemText(hDlg, UPP_NAME, uppname);
	    PostMessage(hDlg, WM_COMMAND, WM_USER, 0L);
	    return TRUE;
	case WM_COMMAND:
	    notify_message = GetNotification(wParam,lParam);
	    switch (LOWORD(wParam)) {
		case WM_USER:
		    /* time consuming initialization */
		    ShowWindow(hDlg, SW_SHOW);
		    InvalidateRect(hDlg, NULL, FALSE);
		    UpdateWindow(hDlg);
		    desc = NULL;
		    uppname[0] = uppname[1] = '\0';
		    i = enum_upp_path(option.gsinclude, NULL, 0);
		    if ((ubuf = malloc(i)) == (char *)NULL) {
			play_sound(SOUND_ERROR);
			PostMessage(hDlg, WM_COMMAND, IDCANCEL, 0L);
			return TRUE;	/* no memory */
		    }
		    enum_upp_path(option.gsinclude, ubuf, i);
		    GetUPPname(hDlg, uppname, sizeof(uppname));
		    SendDlgItemMessage(hDlg, UPP_LIST, LB_RESETCONTENT, 
			0, (LPARAM)0);
		    for (p = ubuf; *p; p += lstrlen(p) + 1) {
			q = p + lstrlen(p) + 1;
			if (lstrcmp(p, uppname) == 0)
			    desc = q;
			SendDlgItemMessage(hDlg, UPP_LIST, LB_ADDSTRING, 
				0, (LPARAM)q);
			p = q;
		    }
		    if (desc != (LPSTR)NULL)
			SendDlgItemMessage(hDlg, UPP_LIST, LB_SELECTSTRING, 
			    0, (LPARAM)desc);
		    SetDlgItemText(hDlg, UPP_NAME, uppname);
		    EnableWindow(GetDlgItem(hDlg, UPP_LIST), TRUE);
		    return TRUE;	/* we processed the message */
		case ID_HELP:
		    get_help();
		    return FALSE;
		case UPP_LIST:
		    if (notify_message == LBN_SELCHANGE) {
			char dname[MAXSTR];
			if (ubuf == NULL)
			    return FALSE;
			i = (int)SendDlgItemMessage(hDlg, UPP_LIST, 
			    LB_GETCURSEL, 0, 0L);
			if (i == LB_ERR)
			    return FALSE;
			if (SendDlgItemMessage(hDlg, UPP_LIST, LB_GETTEXTLEN, 
			      i, (LPARAM)0) + 1 > sizeof(dname))
			    return FALSE;
			SendDlgItemMessage(hDlg, UPP_LIST, LB_GETTEXT, 
			    i, (LPARAM)(LPSTR)dname);
			SetDlgItemText(hDlg, UPP_NAME, 
			    uppmodel_to_name(ubuf, dname));
		    }
		    else if (notify_message == LBN_DBLCLK) {
			PostMessage(hDlg, WM_COMMAND, IDOK, (LPARAM)0);
		    }
		    return FALSE;
		case IDOK:
		    if (GetDlgItemText(hDlg, UPP_NAME, uppname+2, 
		      sizeof(uppname)-3) != 0) {
		      uppname[0] = '"';
		      uppname[1] = '@';
		      strcat(uppname, "\042");
		      SetDlgItemText(GetParent(hDlg), 
			    DEVICE_OPTIONS, uppname);
		    }
		    if (ubuf)
		 	free(ubuf);
		    EndDialog(hDlg, TRUE);
		    return TRUE;
		case IDCANCEL:
		    if (ubuf)
		 	free(ubuf);
		    EndDialog(hDlg, FALSE);
		    return TRUE;
	    }
	    break;
    }
    return FALSE;
}

/* dialog box for selecting PostScript prolog/epilog and Ctrl+D */
#ifdef __BORLANDC__
#pragma argsused
#endif
BOOL CALLBACK _export
AdvPSDlgProc(HWND hDlg, UINT wmsg, WPARAM wParam, LPARAM lParam)
{

    switch (wmsg) {
	case WM_INITDIALOG:
	    {
	    /* for PostScript printers, provide options for sending
	     * Ctrl+D before and after job, and sending a prolog
	     * and epilog file.
	     * These are set using the Advanced button on the Printer
	     * Setup dialog, only enabled for PostScript printer.
	     */
	    PROFILE *prf;
	    char buf[MAXSTR];
	    char section[MAXSTR];
	    int prectrld=0;
	    int postctrld=0;
	    SendDlgItemMessage(GetParent(hDlg), SPOOL_PORT, LB_GETTEXT, 
		(int)SendDlgItemMessage(GetParent(hDlg), SPOOL_PORT, 
			LB_GETCURSEL, 0, 0L),
		(LPARAM)(LPSTR)section);
	    if ( (prf = profile_open(szIniFile)) != (PROFILE *)NULL ) {
		profile_read_string(prf, section, "PreCtrlD", "0", buf, 
		    sizeof(buf)-2);
		if (sscanf(buf, "%d", &prectrld) != 1)
		    prectrld = 0;
		SendDlgItemMessage(hDlg, ADVPS_PRECTRLD, BM_SETCHECK, 
		    prectrld ? 1 : 0, 0);
		profile_read_string(prf, section, "PostCtrlD", "0", buf, 
		    sizeof(buf)-2);
		if (sscanf(buf, "%d", &postctrld) != 1)
		    postctrld = 0;
		SendDlgItemMessage(hDlg, ADVPS_POSTCTRLD, BM_SETCHECK, 
		    postctrld ? 1 : 0, 0);
		profile_read_string(prf, section, "Prolog", "", buf, 
		   sizeof(buf)-2);
		SetDlgItemText(hDlg, ADVPS_PROLOG, buf);
		profile_read_string(prf, section, "Epilog", "", buf, 
		   sizeof(buf)-2);
		SetDlgItemText(hDlg, ADVPS_EPILOG, buf);
		profile_close(prf);
	    }
	    }
	    return TRUE;
	case WM_COMMAND:
	    switch (LOWORD(wParam)) {
		case ADVPS_PROLOGBROWSE:
		    {   char buf[MAXSTR];
			GetDlgItemText(hDlg, ADVPS_PROLOG, buf, sizeof(buf)-1);
			if (get_filename(buf, FALSE, FILTER_ALL, 
			       0, IDS_TOPICPRINT)) {
			    SetDlgItemText(hDlg, ADVPS_PROLOG, buf);
			}
		    }
		    return FALSE;
		case ADVPS_EPILOGBROWSE:
		    {   char buf[MAXSTR];
			GetDlgItemText(hDlg, ADVPS_EPILOG, buf, sizeof(buf)-1);
			if (get_filename(buf, FALSE, FILTER_ALL, 
			       0, IDS_TOPICPRINT)) {
			    SetDlgItemText(hDlg, ADVPS_EPILOG, buf);
			}
		    }
		    return FALSE;
		case ID_HELP:
		    get_help();
		    return FALSE;
		case IDOK:
		    {
		    char buf[MAXSTR];
		    char section[MAXSTR];
		    int prectrld;
		    int postctrld;
		    /* save settings */
		    SendDlgItemMessage(GetParent(hDlg), SPOOL_PORT, LB_GETTEXT, 
			(int)SendDlgItemMessage(GetParent(hDlg), SPOOL_PORT, 
				LB_GETCURSEL, 0, 0L),
			(LPARAM)(LPSTR)section);
		    prectrld = (int)SendDlgItemMessage(hDlg, ADVPS_PRECTRLD, 
			BM_GETCHECK, 0, 0);
		    WritePrivateProfileString(section, "PreCtrlD", 
			prectrld ? "1" : "0", szIniFile);
		    postctrld = (int)SendDlgItemMessage(hDlg, ADVPS_POSTCTRLD, 
			BM_GETCHECK, 0, 0);
		    WritePrivateProfileString(section, "PostCtrlD", 
			postctrld ? "1" : "0", szIniFile);
		    GetDlgItemText(hDlg, ADVPS_PROLOG, buf, sizeof(buf)-1);
		    WritePrivateProfileString(section, "Prolog", 
			buf, szIniFile);
		    GetDlgItemText(hDlg, ADVPS_EPILOG, buf, sizeof(buf)-1);
		    WritePrivateProfileString(section, "Epilog", 
			buf, szIniFile);
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


char *device_queue_list;
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
		lpProcPage = (DLGPROC)MakeProcInstance((FARPROC)PageMultiDlgProc, phInstance);
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
		if (strlen(p)==0) {
		    /* no printers, so force Print to File */
		    option.print_to_file = TRUE;
		    option.psprinter = FALSE;
		    SendDlgItemMessage(hDlg, SPOOL_TOFILE, BM_SETCHECK, 1, 0);
		    EnableWindow(GetDlgItem(hDlg, SPOOL_TOFILE), FALSE);
		    EnableWindow(GetDlgItem(hDlg, SPOOL_PORT), FALSE);
		    EnableWindow(GetDlgItem(hDlg, SPOOL_PORTTEXT), FALSE);
		    EnableWindow(GetDlgItem(hDlg, DEVICE_PSPRINT), FALSE);
		}
		while (p && *p) {
		    if ( strcmp(p, option.printer_queue) == 0 )
		        device_queue_index = idevice;
		    SendDlgItemMessage(hDlg, SPOOL_PORT, LB_ADDSTRING, 0, (LPARAM)p);
		    p += lstrlen(p)+1;
		    idevice++;
		}
	        SendDlgItemMessage(hDlg, SPOOL_PORT, LB_SETCURSEL, device_queue_index, (LPARAM)0);
		/* fill in page list box */
		if ( (psfile.doc != (PSDOC *)NULL) && (psfile.doc->numpages != 0)) {
		    psfile.page_list.current = psfile.pagenum-1;
		    psfile.page_list.multiple = TRUE;
		    for (i=0; i< psfile.doc->numpages; i++)
			psfile.page_list.select[i] = TRUE;
		    psfile.page_list.select[psfile.page_list.current] = TRUE;
		    psfile.page_list.reverse = option.print_reverse;
#ifdef __WIN32__
		    PageMultiDlgProc(hDlg, wmsg, wParam, lParam);
#else
		    CallWindowProc((WNDPROC)lpProcPage, hDlg, wmsg, wParam, lParam);
#endif
		}
		else {
		    psfile.page_list.multiple = FALSE;
		    EnableWindow(GetDlgItem(hDlg, PAGE_ALL), FALSE);
		    EnableWindow(GetDlgItem(hDlg, PAGE_ODD), FALSE);
		    EnableWindow(GetDlgItem(hDlg, PAGE_EVEN), FALSE);
		    EnableWindow(GetDlgItem(hDlg, PAGE_REVERSE), FALSE);
		    SendDlgItemMessage(hDlg, PAGE_LIST, LB_ADDSTRING, 0, 
			(LPARAM)((LPSTR)"All"));
		    EnableWindow(GetDlgItem(hDlg, PAGE_LISTTEXT), FALSE);
		    EnableWindow(GetDlgItem(hDlg, PAGE_LIST), FALSE);
		}
		/* set PostScript Printer check box */
		if (option.psprinter) {
		    SendDlgItemMessage(hDlg, DEVICE_PSPRINT, BM_SETCHECK, 1, 0);
		    EnableWindow(GetDlgItem(hDlg, DEVICE_NAMETEXT), FALSE);
		    EnableWindow(GetDlgItem(hDlg, DEVICE_NAME), FALSE);
		    EnableWindow(GetDlgItem(hDlg, DEVICE_RESTEXT), FALSE);
		    EnableWindow(GetDlgItem(hDlg, DEVICE_RES), FALSE);
		    EnableWindow(GetDlgItem(hDlg, DEVICE_PROP), FALSE);
		    EnableWindow(GetDlgItem(hDlg, DEVICE_OPTIONS), FALSE);
		    EnableWindow(GetDlgItem(hDlg, DEVICE_UNIPRINT), FALSE);
		    EnableWindow(GetDlgItem(hDlg, SPOOL_TOFILE), FALSE);
		}
		else {
		    EnableWindow(GetDlgItem(hDlg, DEVICE_ADVPS), FALSE);
		    /* set Print to File check box */
		    if (option.print_to_file) {
			SendDlgItemMessage(hDlg, SPOOL_TOFILE, BM_SETCHECK, 1, 0);
			EnableWindow(GetDlgItem(hDlg, SPOOL_PORT), FALSE);
			EnableWindow(GetDlgItem(hDlg, SPOOL_PORTTEXT), FALSE);
		    }
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
		    case DEVICE_PSPRINT:
			if (notify_message == BN_CLICKED) {
			    int enable;
		    	    i = (int)SendDlgItemMessage(hDlg, DEVICE_PSPRINT, BM_GETCHECK, 0, 0);
			    /* toggle state */
			    i = (i == 0) ? 1 : 0;
			    SendDlgItemMessage(hDlg, DEVICE_PSPRINT, BM_SETCHECK, i, 0);
			    enable = !i;
			    EnableWindow(GetDlgItem(hDlg, DEVICE_NAMETEXT), enable);
			    EnableWindow(GetDlgItem(hDlg, DEVICE_NAME), enable);
			    EnableWindow(GetDlgItem(hDlg, DEVICE_OPTIONSTEXT), enable);
			    EnableWindow(GetDlgItem(hDlg, DEVICE_OPTIONS), enable);
			    if (i) {
				EnableWindow(GetDlgItem(hDlg, DEVICE_RESTEXT), FALSE);
				EnableWindow(GetDlgItem(hDlg, DEVICE_RES), FALSE);
			    }
			    else
				SendDlgNotification(hDlg, DEVICE_NAME, CBN_SELCHANGE);
			    EnableWindow(GetDlgItem(hDlg, DEVICE_PROP), enable);
			    if (i && (int)SendDlgItemMessage(hDlg, SPOOL_TOFILE, BM_GETCHECK, 0, 0))
				SendDlgNotification(hDlg, SPOOL_TOFILE, BN_CLICKED);
			    EnableWindow(GetDlgItem(hDlg, SPOOL_TOFILE), enable);
			    EnableWindow(GetDlgItem(hDlg, DEVICE_UNIPRINT), enable);
			    EnableWindow(GetDlgItem(hDlg, DEVICE_ADVPS), !enable);
			}
			return FALSE;
		    case DEVICE_ADVPS:
			{
#ifndef __WIN32__
	    		DLGPROC lpProcAdv;
#endif
			load_string(IDS_TOPICPRINT, szHelpTopic, 
				sizeof(szHelpTopic));
#ifdef __WIN32__
			DialogBoxParam(hlanguage, "AdvancedDlgBox", hDlg, 
				AdvPSDlgProc, (LPARAM)NULL);
#else
			lpProcAdv = (DLGPROC)MakeProcInstance(
				(FARPROC)AdvPSDlgProc, phInstance);
			DialogBoxParam(hlanguage, "AdvancedDlgBox", hDlg, 
				lpProcAdv, (LPARAM)NULL);
			FreeProcInstance((FARPROC)lpProcAdv);
#endif
			}
			return FALSE;
		    case ID_HELP:
		        get_help();
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
			GetPrivateProfileString(DEVSECTION, entry, "", buf, sizeof(buf)-2, szIniFile);
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
			/* update printer options */
			{ char section[MAXSTR];
			strcpy(section, entry);
			strcat(section, " Options");
			GetPrivateProfileString(section, "Options", "", buf, sizeof(buf)-2, szIniFile);
			if (buf[0] == '@') {
			    /* STUPID Windows *sometimes* removes the quotes.
			     * If the profile string contains quotes at the
			     * the start *and* end, Windows will remove them.
			     * Otherwise, quotes will be copied intact.
			     * The quotes are important, so we have to put
			     * them back in.
			     */
			    memmove(buf+1, buf, strlen(buf)+1);
			    buf[0] = '\042';
			    strcat(buf, "\042");
			}
			SetDlgItemText(hDlg, DEVICE_OPTIONS, buf);
			}
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
			    load_string(IDS_TOPICPRINT, szHelpTopic, sizeof(szHelpTopic));
#ifdef __WIN32__
			    DialogBoxParam(hlanguage, "PropDlgBox", hDlg, PropDlgProc, (LPARAM)entry);
#else
			    lpProcProp = (DLGPROC)MakeProcInstance((FARPROC)PropDlgProc, phInstance);
			    DialogBoxParam(hlanguage, "PropDlgBox", hDlg, lpProcProp, (LPARAM)entry);
			    FreeProcInstance((FARPROC)lpProcProp);
#endif
			}
			else
			    play_sound(SOUND_ERROR);
			return FALSE;
		    case DEVICE_UNIPRINT:
			{
#ifndef __WIN32__
	    		DLGPROC lpProcUni;
#endif
		        GetDlgItemText(hDlg, DEVICE_NAME, buf, sizeof(buf));
			if (strcmp(buf, "uniprint") != 0) {
			  /* select uniprint device */
			  if (SendDlgItemMessage(hDlg, DEVICE_NAME, 
			      CB_SELECTSTRING, 0, 
			      (LPARAM)(LPSTR)"uniprint")
			        == CB_ERR) {
			    play_sound(SOUND_ERROR);
			    return FALSE;	/* can't select uniprint */
			  }
			  SendDlgNotification(hDlg, DEVICE_NAME, CBN_SELCHANGE);
			}
			load_string(IDS_TOPICPRINT, szHelpTopic, 
				sizeof(szHelpTopic));
#ifdef __WIN32__
			DialogBoxParam(hlanguage, "UniDlgBox", hDlg, 
				UniDlgProc, (LPARAM)NULL);
#else
			lpProcUni = (DLGPROC)MakeProcInstance((FARPROC)UniDlgProc, phInstance);
			DialogBoxParam(hlanguage, "UniDlgBox", hDlg, lpProcUni, (LPARAM)entry);
			FreeProcInstance((FARPROC)lpProcUni);
#endif
			}
			return FALSE;
		    case PAGE_ALL:
		    case PAGE_EVEN:
		    case PAGE_ODD:
#ifdef __WIN32__
		    	PageMultiDlgProc(hDlg, wmsg, wParam, lParam);
#else
			CallWindowProc((WNDPROC)lpProcPage, hDlg, wmsg, wParam, lParam);
#endif
			return FALSE;
		    case IDOK:
			/* save device name and resolution */
		        GetDlgItemText(hDlg, DEVICE_NAME, option.device_name, sizeof(option.device_name));
		        GetDlgItemText(hDlg, DEVICE_RES, option.device_resolution, sizeof(option.device_resolution));
		 	option.psprinter = (int)SendDlgItemMessage(hDlg, DEVICE_PSPRINT, BM_GETCHECK, 0, 0);
			/* get Print to File status */
		 	option.print_to_file = (int)SendDlgItemMessage(hDlg, 
			    SPOOL_TOFILE, BM_GETCHECK, 0, 0);
			if (!option.print_to_file) {
			    /* save queue name */
			    SendDlgItemMessage(hDlg, SPOOL_PORT, LB_GETTEXT, 
				(int)SendDlgItemMessage(hDlg, SPOOL_PORT, LB_GETCURSEL, 0, 0L),
				(LPARAM)(LPSTR)option.printer_queue);
			}
			/* get pages */
			if ((psfile.doc != (PSDOC *)NULL) 
				&& (psfile.doc->numpages != 0)) {
#ifdef __WIN32__
			    PageMultiDlgProc(hDlg, wmsg, wParam, lParam);
#else
			    CallWindowProc((WNDPROC)lpProcPage, hDlg, 
				wmsg, wParam, lParam);
#endif
		            option.print_reverse = psfile.page_list.reverse;
			}
#ifndef __WIN32__
			FreeProcInstance((FARPROC)lpProcPage);
#endif
			/* get options */
			{ char section[MAXSTR];
			strcpy(section, option.device_name);
			strcat(section, " Options");
		        GetDlgItemText(hDlg, DEVICE_OPTIONS, buf, sizeof(buf)-2);
			WritePrivateProfileString(section, "Options", buf, szIniFile);
			}
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
get_device(void)
{
int result;
#ifndef __WIN32__
DLGPROC lpProcDevice;
#endif
#define DEVICE_BUF_SIZE 4096
    device_queue_list = get_queues();
    if (device_queue_list == (char *)NULL)
	return FALSE;

    load_string(IDS_TOPICPRINT, szHelpTopic, sizeof(szHelpTopic));
#ifdef __WIN32__
    result = DialogBoxParam(hlanguage, "DeviceDlgBox", hwndimg, DeviceDlgProc, (LPARAM)NULL);
#else
    lpProcDevice = (DLGPROC)MakeProcInstance((FARPROC)DeviceDlgProc, phInstance);
    result = DialogBoxParam(hlanguage, "DeviceDlgBox", hwndimg, lpProcDevice, (LPARAM)NULL);
    FreeProcInstance((FARPROC)lpProcDevice);
#endif
    free(device_queue_list);
    if (result != IDOK)
	return FALSE;
    return TRUE;
}




#ifdef __BORLANDC__
#pragma argsused
#endif
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
/* For Win32s this selects a port */
/* For Win95 or WinNT, this selects a queue */
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
	    if ( (is_win32s ? (*option.printer_port=='\0') : (*option.printer_queue=='\0') ) ||
		(SendDlgItemMessage(hDlg, SPOOL_PORT, LB_SELECTSTRING, 0, (LPARAM)(LPSTR)(is_win32s ? option.printer_port : option.printer_queue))
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
			(LPARAM)(LPSTR)(is_win32s ? option.printer_port : option.printer_queue));
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
    if (is_win32s) {
	if ((buffer = malloc(PORT_BUF_SIZE)) == (char *)NULL)
	    return NULL;
	GetProfileString("Devices", NULL, "", buffer, PORT_BUF_SIZE);
	return buffer;
    }

    /* enumerate all available printers */
    EnumPrinters(PRINTER_ENUM_CONNECTIONS | PRINTER_ENUM_LOCAL, NULL, 1, NULL, 0, &needed, &count);
    if (needed == 0) {
	/* no printers */
	enumbuffer = malloc(4);
	if (enumbuffer == (char *)NULL)
	    return NULL;
	memset(enumbuffer, 0, 4);
	return enumbuffer;	
    }
    enumbuffer = malloc(needed);
    if (enumbuffer == (char *)NULL)
	return NULL;
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
    if (buffer == (char *)NULL) 
	return FALSE;
    if ( (queue == (char *)NULL) || (strlen(queue)==0) ) {
	/* select a queue */
#ifdef __WIN32__
	iport = DialogBoxParam(hlanguage, "QueueDlgBox", hwndimg, SpoolDlgProc, (LPARAM)buffer);
#else
	DLGPROC lpfnSpoolProc;
	lpfnSpoolProc = (DLGPROC)MakeProcInstance((FARPROC)SpoolDlgProc, phInstance);
	iport = DialogBoxParam(hlanguage, "QueueDlgBox", hwndimg, lpfnSpoolProc, (LPARAM)buffer);
	FreeProcInstance((FARPROC)lpfnSpoolProc);
#endif
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
#else
char *
get_queues(void)
{
char *buffer;
    if ((buffer = malloc(PORT_BUF_SIZE)) == (char *)NULL)
	return NULL;
    GetProfileString("Devices", NULL, "", buffer, PORT_BUF_SIZE);
    return buffer;
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
char device[MAXSTR];
#ifdef __WIN32__
	if (is_win95 || is_winnt)
	    return get_queuename(portname, port);
#endif

	if (port && strlen(port)) {
	    /* check if it is a queue name */
	    GetProfileString("Devices", port, "", device, sizeof(device));
	    if (strlen(device)) {
		/* map it to a port name */
		strtok(device, ",");
		port = strtok(NULL, ",");
	    }
	}

        buffer = get_ports();
	if ( (port == (char *)NULL) || (strlen(port)==0) ) {
	    if (buffer == (char *)NULL)
		return FALSE;
	    /* select a port */
	    iport = DialogBoxParam(hlanguage, "SpoolDlgBox", hwndimg, SpoolDlgProc, (LPARAM)buffer);
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

int gp_printfile(char *filename, char *port)
{
#ifdef __WIN32__
    if (is_win95 || is_winnt)
	return gp_printfile_win32(filename, port);
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
    hDlgModeless = CreateDialog(hlanguage, "CancelDlgBox", hwndimg, lpfnCancelProc);
    ldone = 0;
    load_string(IDS_CANCELDONE, fmt, sizeof(fmt));

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
#ifndef __WIN32__
DLGPROC lpfnCancelProc;
#endif
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
	    gserror(0, "Can't allocate global memory for gsv16spl", 0, SOUND_ERROR);
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
	    gserror(0, "gsv16spl didn't give us a window handle", 0, SOUND_ERROR);
	    return FALSE;	/* gsv16spl didn't SendMessage to us */
	}

	if (!SendMessage(hwndspl, WM_GSV16SPL, 0, (LPARAM)hmem)) {
	    fclose(f);
	    free(buffer);
	    GlobalFree(hmem);
	    gserror(0, "gsv16spl couldn't start printer job", 0, SOUND_ERROR);
	    return FALSE;
	}


#ifdef __WIN32__
	hDlgModeless = CreateDialog(hlanguage, "CancelDlgBox", hwndimg, CancelDlgProc);
#else
        lpfnCancelProc = (DLGPROC)MakeProcInstance((FARPROC)CancelDlgProc, phInstance);
        hDlgModeless = CreateDialog(hlanguage, "CancelDlgBox", hwndimg, lpfnCancelProc);
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

void
start_gvwgs(void)
{
    BOOL flag;
    char progname[MAXSTR];
    char command[MAXSTR+MAXSTR];

#ifdef __WIN32__
    if (!is_win32s)
        sprintf(command,"%s \042%s\042 \042%s\042 \042%s\042", debug ? "/d" : "",
	    option.gsdll, printer.optname, printer.psname);
    else
#endif
        sprintf(command,"%s %s %s %s", debug ? "/d" : "",
	    option.gsdll, printer.optname, printer.psname);

    if (strlen(command) > MAXSTR-1) {
	/* command line too long */
	gserror(IDS_TOOLONG, command, MB_ICONHAND, SOUND_ERROR);
	if (!debug)
	    unlink(printer.psname);
	printer.psname[0] = '\0';
	if (!debug)
	    unlink(printer.optname);
	printer.optname[0] = '\0';
	return;
    }

    info_wait(IDS_WAIT);
    strcpy(progname, szExePath);
#ifdef __WIN32__
#ifdef DECALPHA
    strcat(progname, "gvwgsda.exe");
#else
    strcat(progname, "gvwgs32.exe");
#endif
#else
    strcat(progname, "gvwgs16.exe");
#endif
    flag = exec_pgm(progname, command, &printer.prog);
    if (!flag || !printer.prog.valid) {
	    cleanup_pgm(&printer.prog);
	    gserror(IDS_CANNOTRUN, progname, MB_ICONHAND, SOUND_ERROR);
	    if (!debug)
		unlink(printer.psname);
	    printer.psname[0] = '\0';
	    if (!debug)
		unlink(printer.optname);
	    printer.optname[0] = '\0';
	    info_wait(IDS_NOWAIT);
	    return;
    }

    info_wait(IDS_NOWAIT);
}



/* print a range of pages using a Ghostscript device */
void
gsview_print(void)
{
	if (psfile.name[0] == '\0') {
		gserror(IDS_NOTOPEN, NULL, MB_ICONEXCLAMATION, SOUND_NOTOPEN);
		return;
	}
	
	if (!get_device())
	    return;

	if (!gsview_cprint(printer.psname, printer.optname))
	    return;

#ifdef __WIN32__
	if (is_win32s) 
#endif
	{
	    /* Win16 and Win32s can't load GS DLL twice */
	    /* We must unload the current GS DLL */
	    if (gsdll.valid)
		pending.unload = TRUE;
	    /* printer_pending will cause start_gvwgs() to be run */
	    /* from main message loop, after displaying GS DLL */
	    /* has unloaded */
	    win32s_printer_pending = TRUE;
	    return;
	}

#ifdef __WIN32__
        start_gvwgs();
	return;
#endif
}

/* Convert a range of pages from PDF to PS */
void
gsview_pdf2ps(char *output)
{
	if (psfile.name[0] == '\0') {
		gserror(IDS_NOTOPEN, NULL, MB_ICONEXCLAMATION, SOUND_NOTOPEN);
		return;
	}
	
	if (!gsview_pdf2ps_common(printer.psname, printer.optname, output))
	    return;

#ifdef __WIN32__
	if (is_win32s) 
#endif
	{
	    /* Win16 and Win32s can't load GS DLL twice */
	    /* We must unload the current GS DLL */
	    if (gsdll.valid)
		pending.unload = TRUE;
	    /* printer_pending will cause start_gvwgs() to be run */
	    /* from main message loop, after displaying GS DLL */
	    /* has unloaded */
	    win32s_printer_pending = TRUE;
	    return;
	}

#ifdef __WIN32__
        start_gvwgs();
	return;
#endif
}

