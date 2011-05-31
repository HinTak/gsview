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

/* gvpprn.c */
/* Printer routines for PM GSview */
#include "gvpm.h"

char not_defined[] = "[Not defined]";

MRESULT EXPENTRY
SpoolDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
int notify_message;
int i;
char *entry;
    switch(msg) {
      case WM_INITDLG:
	entry = (char *)mp2;
	while (*entry) {
	    WinSendMsg( WinWindowFromID(hwnd, SPOOL_PORT),
	    	LM_INSERTITEM, MPFROMLONG(LIT_END), MPFROMP(entry));
	    entry += strlen(entry)+1;
	}
	i = (int)WinSendMsg( WinWindowFromID(hwnd, SPOOL_PORT),
	    LM_SEARCHSTRING, MPFROM2SHORT(LSS_CASESENSITIVE, LIT_FIRST),
	    MPFROMP(option.printer_port) );
	if ((i == LIT_ERROR) || (i == LIT_NONE))
	    i = 0;
	WinSendMsg( WinWindowFromID(hwnd, SPOOL_PORT),
	    	LM_SELECTITEM, MPFROMLONG(i), MPFROMLONG(TRUE) );
	break;
    case WM_CONTROL:
	notify_message = SHORT2FROMMP(mp1);
	switch (notify_message) {
	    case LN_ENTER:
	        if (SHORT1FROMMP(mp1) == SPOOL_PORT)
		    WinPostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPFROM2SHORT(CMDSRC_OTHER, TRUE));
		break;
	}
	break;
    case  WM_COMMAND:
      switch(LOUSHORT(mp1)) {
        case DID_OK:
	    i = (int)WinSendMsg(WinWindowFromID(hwnd, SPOOL_PORT), 
			LM_QUERYSELECTION, (MPARAM)0, (MPARAM)0);
	    if (i != LIT_NONE)
		    WinSendMsg(WinWindowFromID(hwnd, SPOOL_PORT), LM_QUERYITEMTEXT,  MPFROM2SHORT(i, sizeof(option.printer_port)), MPFROMP(option.printer_port));
	    WinDismissDlg(hwnd, 1+(int)WinSendMsg(WinWindowFromID(hwnd, SPOOL_PORT), 
		LM_QUERYSELECTION, (MPARAM)0, (MPARAM)0));
            return (MRESULT)TRUE;
      }
      break;
    }
    return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

char *
get_ports(char *p, int len)
{
PROFILE *prf;
#define PORTSECTION "Ports"
	if ( (prf = profile_open(szIniFile)) == (PROFILE *)NULL)
	    return (char *)NULL;

	profile_read_string(prf, PORTSECTION, NULL, "", p, PROFILE_SIZE);
	if (strlen(p) == 0) {
	    /* [Ports] section doesn't exist.  Initialise from resources */
	    profile_create_section(prf, PORTSECTION, IDR_PORTS);
	    profile_read_string(prf, PORTSECTION, NULL, "", p, len);
	}
	profile_close(prf);
	return p;
}


/* Print File to port */
/* port==NULL means prompt for port with dialog box */
int
gp_printfile(char *filename, char *port)
{
#define PRINT_BUF_SIZE 16384u
char *buffer;
char *portname;
int i, iport;
unsigned int count;
FILE *f;
FILE *printer;
int error = FALSE;
long lsize;
long ldone;
char fmt[MAXSTR];
char pcdone[10];

	if ((buffer = malloc(PRINT_BUF_SIZE)) == (char *)NULL)
	    return FALSE;
	if (port == (char *)NULL) {
	    /* get list of ports */
	    get_ports(buffer, PRINT_BUF_SIZE);
	    /* select a port */
	    iport = WinDlgBox(HWND_DESKTOP, hwnd_frame, SpoolDlgProc, 0, IDD_SPOOL, buffer);
	    if (!iport || iport == 65536) {
		free(buffer);
		return FALSE;
	    }
	    portname = buffer;
	    for (i=1; i<iport && strlen(portname)!=0; i++)
		portname += strlen(portname)+1;
	}
	else
	    portname = port;
	if (strcmp(portname,"FILE:") == 0) {
	    strcpy(buffer, "*.prn");
	    if (!get_filename(buffer, TRUE, FILTER_ALL, IDS_PRINTFILE, IDS_TOPICPRINT)) {
	        free(buffer);
	        return FALSE;
	    }
	    portname = buffer;
	}
	else {
	    portname[strlen(portname)-1] = '\0';  /* remove trailing colon */
	}
	
	if ((f = fopen(filename, "rb")) == (FILE *)NULL) {
	    free(buffer);
	    return FALSE;
	}
	fseek(f, 0L, SEEK_END);
	lsize = ftell(f);
	if (lsize <= 0)
	    lsize = 1;
	fseek(f, 0L, SEEK_SET);

	printer = fopen(portname, "wb");
	if (printer == (FILE *)NULL) {
	        fclose(f);
		free(buffer);
	        return FALSE;
	}

/*
	hDlgModeless = CreateDialog(phInstance, "CancelDlgBox", hwndimg, lpfnCancelProc);
*/
	ldone = 0;
	load_string(IDS_CANCELDONE, fmt, sizeof(fmt));

	while (!error /* && hDlgModeless  */
	  && (count = fread(buffer, 1, PRINT_BUF_SIZE, f)) != 0 ) {
	    if (fwrite(buffer, 1, count, printer) != count)
		error = TRUE;
	    ldone += count;
	    sprintf(pcdone, fmt, (int)(ldone * 100 / lsize));
	    /* WinSetWindowText(WinWindowFromID(hwnd_cancel, CANCEL_PCDONE), pcdone); */
/*
	    while (PeekMessage(&msg, hDlgModeless, 0, 0, PM_REMOVE)) {
	        if ((hDlgModeless == 0) || !IsDialogMessage(hDlgModeless, &msg)) {
		    TranslateMessage(&msg);
		    DispatchMessage(&msg);
		}
	    }
*/
	}
	free(buffer);
	fclose(f);
	fclose(printer);

	return !error;
}


/* dialog box for selecting printer properties */
MRESULT EXPENTRY
PropDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
	char buf[128];
	int iprop;
	int ivalue;
	char *p;
	char *value;
	static char device[MAXSTR];	/* contains printer device name */
	static struct prop_item_s* propitem;
	char section[MAXSTR];

    switch (msg) {
      case WM_INITDLG:
	strcpy(device, mp2);	/* initialise device name */
	propitem = get_properties(device);
	if (propitem == (struct prop_item_s *)NULL) {
	    WinDismissDlg(hwnd, FALSE);
	    return (MRESULT)TRUE;
	}
	for (iprop=0; propitem[iprop].name[0]; iprop++) {
	    WinSendMsg( WinWindowFromID(hwnd, PROP_NAME),
	    	LM_INSERTITEM, MPFROMLONG(LIT_END), MPFROMP(propitem[iprop].name+1));
	}
	WinSendMsg( WinWindowFromID(hwnd, PROP_NAME),
	    	LM_SELECTITEM, MPFROMLONG(0), MPFROMLONG(TRUE) );
/*
	WinSendMsg(hwnd, WM_CONTROL, MPFROM2SHORT(PROP_NAME, CBN_LBSELECT),
	    	MPFROMLONG(WinWindowFromID(hwnd, PROP_NAME)));
*/
	break;
      case WM_CONTROL:
	if (mp1 == MPFROM2SHORT(PROP_NAME, CBN_LBSELECT)) {
	    iprop = (int)WinSendMsg(WinWindowFromID(hwnd, PROP_NAME), LM_QUERYSELECTION, (MPARAM)0, (MPARAM)0);
	    if (iprop == LIT_NONE)
	        return FALSE;
	    /* now look up entry in gsview.ini */
	    /* and update PROP_VALUE list box */
	    strcpy(section, device);
	    strcat(section, " values");
	    {PROFILE *prf;
/* need to reopen profile file - this is wasteful */
		if ( (prf = profile_open(szIniFile)) != (PROFILE *)NULL ) {
	    	    profile_read_string(prf, section, propitem[iprop].name, "", buf, sizeof(buf)-2);
		    profile_close(prf);
		}
		else
		    buf[0] = '\0';
	    }
	    while ((*buf) && (buf[strlen(buf)-1]==' '))
		buf[strlen(buf)-1] = '\0';    /* remove trailing spaces */
	    buf[strlen(buf)+1] = '\0';	/* put double NULL at end */
	    WinSendMsg(WinWindowFromID(hwnd, PROP_VALUE), LM_DELETEALL, (MPARAM)0, (MPARAM)0);
	    WinSendMsg( WinWindowFromID(hwnd, PROP_VALUE),
	    	        LM_INSERTITEM, MPFROMLONG(LIT_END), MPFROMP(not_defined) );
	    p = buf;
	    if (*p != '\0') {
	      WinEnableWindow(WinWindowFromID(hwnd, PROP_VALUE), TRUE);
	      while (*p!='\0') {
		value = p;
		while ((*p!='\0') && (*p!=','))
		    p++;
		*p++ = '\0';
	    	WinSendMsg( WinWindowFromID(hwnd, PROP_VALUE),
	    	        LM_INSERTITEM, MPFROMLONG(LIT_END), MPFROMP(value) );
	      }
	    }
	    iprop = (int)WinSendMsg( WinWindowFromID(hwnd, PROP_VALUE),
	    	    LM_SEARCHSTRING, MPFROM2SHORT(LSS_CASESENSITIVE, LIT_FIRST),
		    MPFROMP(propitem[iprop].value) );
	    if ((iprop == LIT_ERROR) || (iprop == LIT_NONE))
		iprop = 0;
	    WinSendMsg( WinWindowFromID(hwnd, PROP_VALUE),
	    	LM_SELECTITEM, MPFROMLONG(iprop), MPFROMLONG(TRUE) );
/*
	        SetDlgItemText(hDlg, PROP_VALUE, propitem[iprop].value);
*/
	}
	if (mp1 == MPFROM2SHORT(PROP_VALUE, CBN_LBSELECT)) {
	    iprop = (int)WinSendMsg(WinWindowFromID(hwnd, PROP_NAME), LM_QUERYSELECTION, (MPARAM)0, (MPARAM)0);
	    if (iprop == LIT_NONE)
	        return FALSE;
	    ivalue = (int)WinSendMsg(WinWindowFromID(hwnd, PROP_VALUE), LM_QUERYSELECTION, (MPARAM)0, (MPARAM)0);
	    if (ivalue == LIT_NONE)
	        return FALSE;
	    WinSendMsg(WinWindowFromID(hwnd, PROP_VALUE), LM_QUERYITEMTEXT,  
		MPFROM2SHORT(ivalue, sizeof(propitem->value)), 
		MPFROMP(propitem[iprop].value));
	}
	if (mp1 == MPFROM2SHORT(PROP_VALUE, CBN_EFCHANGE)) {
	    iprop = (int)WinSendMsg(WinWindowFromID(hwnd, PROP_NAME), LM_QUERYSELECTION, (MPARAM)0, (MPARAM)0);
	    if (iprop == LIT_NONE)
	        return FALSE;
	    WinQueryWindowText(WinWindowFromID(hwnd, PROP_VALUE), 
			 sizeof(propitem->value), propitem[iprop].value);
	}
	break;
    case WM_COMMAND:
	switch(LOUSHORT(mp1)) {
	    case ID_HELP:
		get_help();
		return (MRESULT)TRUE;
	    case DID_OK:
	    	{PROFILE *prf;
/* need to reopen profile file - this is wasteful */
		  if ( (prf = profile_open(szIniFile)) != (PROFILE *)NULL ) {
		    for (iprop=0; propitem[iprop].name[0]; iprop++) {
			profile_write_string(prf, device, propitem[iprop].name, propitem[iprop].value);
		    }
		    profile_close(prf);
		  }
		}
		free((char *)propitem);
		WinDismissDlg(hwnd, DID_OK);
            	return (MRESULT)TRUE;
	    case DID_CANCEL:
		free((char *)propitem);
		WinDismissDlg(hwnd, DID_CANCEL);
		return (MRESULT)TRUE;
	}
	break;
    }
    return WinDefDlgProc(hwnd, msg, mp1, mp2);
}


/* dialog box for selecting printer device and resolution */
MRESULT EXPENTRY
DeviceDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    char buf[128];
    int idevice;
    int i;
    char *p;
    char *res;
    int numentry;
    char entry[MAXSTR];
    struct prop_item_s *proplist;

    switch (msg) {
	case WM_INITDLG:
	    p = get_devices();
	    res = p;	/* save for free() */
	    idevice = 0;
	    for (numentry=0; p!=(char *)NULL && strlen(p)!=0; numentry++) {
	    	WinSendMsg( WinWindowFromID(hwnd, DEVICE_NAME),
	    	    LM_INSERTITEM, MPFROMLONG(LIT_END), MPFROMP(p) );
		if (strcmp(p, option.device_name) == 0)
		    idevice = numentry;
	        p += strlen(p) + 1;
	    }
	    free(res);
	    WinSendMsg( WinWindowFromID(hwnd, DEVICE_NAME),
	    	LM_SELECTITEM, MPFROMLONG(idevice), MPFROMLONG(TRUE) );
	    /* force update of DEVICE_RES */
	    WinSendMsg(hwnd, WM_CONTROL, MPFROM2SHORT(DEVICE_NAME, CBN_LBSELECT),
	    	MPFROMLONG(WinWindowFromID(hwnd, DEVICE_NAME)));
	    i = (int)WinSendMsg( WinWindowFromID(hwnd, DEVICE_RES),
	    	    LM_SEARCHSTRING, MPFROM2SHORT(LSS_CASESENSITIVE, LIT_FIRST),
		    MPFROMP(option.device_resolution) );
	    if ((i == LIT_ERROR) || (i == LIT_NONE))
		i = 0;
	    WinSendMsg( WinWindowFromID(hwnd, DEVICE_RES),
	    	LM_SELECTITEM, MPFROMLONG(i), MPFROMLONG(TRUE) );
	    break;
    	case WM_CONTROL:
	    if (mp1 == MPFROM2SHORT(DEVICE_NAME, CBN_LBSELECT)) {
		idevice = (int)WinSendMsg(WinWindowFromID(hwnd, DEVICE_NAME), LM_QUERYSELECTION, (MPARAM)0, (MPARAM)0);
		if (idevice == LIT_NONE)
		    return FALSE;
		WinSendMsg(WinWindowFromID(hwnd, DEVICE_NAME), LM_QUERYITEMTEXT,  MPFROM2SHORT(idevice, sizeof(entry)), MPFROMP(entry));
		if ( (proplist = get_properties(entry)) != (struct prop_item_s *)NULL ) {
	    	    free((char *)proplist);
		    WinEnableWindow(WinWindowFromID(hwnd, DEVICE_PROP), TRUE);
		}
		else
		    WinEnableWindow(WinWindowFromID(hwnd, DEVICE_PROP), FALSE);
		/* now look up entry in gvpm.ini */
		/* and update DEVICE_RES list box */
		{PROFILE *prf;
/* need to reopen profile file - this is wasteful */
		  if ( (prf = profile_open(szIniFile)) != (PROFILE *)NULL ) {
		    profile_read_string(prf, DEVSECTION, entry, "", buf, sizeof(buf)-2);
		    profile_close(prf);
		  }
		  else
		    buf[0] = '\0';
		}
	    	while ((*buf) && (buf[strlen(buf)-1]==' '))
	    	buf[strlen(buf)-1] = '\0';    /* remove trailing spaces */
		buf[strlen(buf)+1] = '\0';	/* double NULL at end */
		WinSendMsg(WinWindowFromID(hwnd, DEVICE_RES), LM_DELETEALL, (MPARAM)0, (MPARAM)0);
		p = buf;
		if (*p == '\0') {
		    /* no resolutions can be set */
		    WinEnableWindow(WinWindowFromID(hwnd, DEVICE_RES), FALSE);
		    WinEnableWindow(WinWindowFromID(hwnd, DEVICE_RESTEXT), FALSE);
		}
		else {
		  WinEnableWindow(WinWindowFromID(hwnd, DEVICE_RES), TRUE);
		  WinEnableWindow(WinWindowFromID(hwnd, DEVICE_RESTEXT), TRUE);
		  while (*p!='\0') {
		    res = p;
		    while ((*p!='\0') && (*p!=','))
			p++;
		    *p++ = '\0';
	    	    WinSendMsg( WinWindowFromID(hwnd, DEVICE_RES),
	    	        LM_INSERTITEM, MPFROMLONG(LIT_END), MPFROMP(res) );
		  }
		}
	        WinSendMsg( WinWindowFromID(hwnd, DEVICE_RES),
	    	    LM_SELECTITEM, MPFROMLONG(0), MPFROMLONG(TRUE) );
		if ((int)WinSendMsg(WinWindowFromID(hwnd, DEVICE_RES), LM_QUERYSELECTION, (MPARAM)0, (MPARAM)0)
			!= LIT_NONE)
		    WinSetWindowText(WinWindowFromID(hwnd, DEVICE_RES), buf);
	    }
	    break;
	case WM_COMMAND:
	    switch(LOUSHORT(mp1)) {
		case DID_OK:
		    /* save device name and resolution */
		    WinQueryWindowText(WinWindowFromID(hwnd, DEVICE_NAME), 
			 sizeof(option.device_name), option.device_name);
		    WinQueryWindowText(WinWindowFromID(hwnd, DEVICE_RES), 
			sizeof(option.device_resolution), option.device_resolution);
		    WinDismissDlg(hwnd, DID_OK);
            	    return (MRESULT)TRUE;
	        case ID_HELP:
		    get_help();
		    return (MRESULT)TRUE;
		case DEVICE_PROP:
		    idevice = (int)WinSendMsg(WinWindowFromID(hwnd, DEVICE_NAME), 
			LM_QUERYSELECTION, (MPARAM)0, (MPARAM)0);
		    if (idevice == LIT_NONE)
		        return (MRESULT)TRUE;
		    WinSendMsg(WinWindowFromID(hwnd, DEVICE_NAME), LM_QUERYITEMTEXT,  MPFROM2SHORT(idevice, sizeof(entry)), MPFROMP(entry));
		    if ( (proplist = get_properties(entry)) != (struct prop_item_s *)NULL ) {
	    		    free((char *)proplist);
			    load_string(IDS_TOPICPRINT, szHelpTopic, sizeof(szHelpTopic));
	    		    WinDlgBox(HWND_DESKTOP, hwnd, PropDlgProc, 0, IDD_PROP, entry);
		    }
		    else
		        play_sound(SOUND_ERROR);
		    return (MRESULT)TRUE;
	    }
	    break;
    }
    return WinDefDlgProc(hwnd, msg, mp1, mp2);
}


/* print a range of pages using a Ghostscript device */
void
gsview_print(BOOL to_file)
{
	int flag;
	char command[MAXSTR+MAXSTR];
	char progname[256];
	char *args;
	PRINTER *prn = &printer;
	

	if (psfile.name[0] == '\0') {
		gserror(IDS_NOTOPEN, NULL, MB_ICONEXCLAMATION, SOUND_NOTOPEN);
		return;
	}
	
	load_string(IDS_TOPICPRINT, szHelpTopic, sizeof(szHelpTopic));
	if (WinDlgBox(HWND_DESKTOP, hwnd_frame, DeviceDlgProc, 0, IDD_DEVICE, NULL)
		!= DID_OK)
	    return;

	if (!gsview_cprint(to_file, prn->cfname, prn->fname))
	    return;

	args = strchr(option.gscommand, ' ');
	if (args) {
	    strncpy(progname, option.gscommand, (int)(args-option.gscommand));
	    progname[(int)(args-option.gscommand)] = '\0';
	    args++;
	}
	else {
	    strncpy(progname, option.gscommand, MAXSTR);
	    args = "";
	}

	sprintf(command,"%s @%s", args, prn->fname);

	if (strlen(command) > MAXSTR-1) {
		/* command line too long */
		gserror(IDS_TOOLONG, command, MB_ICONHAND, SOUND_ERROR);
		if (!debug)
		    unlink(prn->fname);
		prn->fname[0] = '\0';
	        if ((prn->cfname[0] != '\0') && !debug)
		    unlink(prn->cfname);
	        prn->cfname[0] = '\0';
		return;
	}

	load_string(IDS_WAITPRINT, szWait, sizeof(szWait));
	info_wait(TRUE);

	flag = exec_pgm(progname, command, FALSE, &prn->prog);
	if (!flag || !prn->prog.valid) {
	        cleanup_pgm(&prn->prog);
		gserror(IDS_CANNOTRUN, command, MB_ICONHAND, SOUND_ERROR);
		if (!debug)
		    unlink(prn->fname);
		prn->fname[0] = '\0';
	        if ((prn->cfname[0] != '\0') && !debug)
		    unlink(prn->cfname);
	        prn->cfname[0] = '\0';
		info_wait(FALSE);
		return;
	}
	info_wait(FALSE);
	
	return;
}

