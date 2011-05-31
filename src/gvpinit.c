/* Copyright (C) 1993-1996, Russell Lang.  All rights reserved.
  
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

/* gvpinit.c */
/* Initialisation routines for PM GSview */
/* by Russell Lang */
#include "gvpm.h"

HELPINIT hi_help;
APIRET init1(void); 
APIRET init2(void); 
APIRET restore_window_position(SWP *pswp);
ULONG frame_flags = 
	    FCF_TITLEBAR |	/* have a title bar */
            FCF_SIZEBORDER |	/* have a sizeable window */
            FCF_MINMAX |	/* have a min and max button */
            FCF_SYSMENU | 	/* include a system menu */
            FCF_VERTSCROLL |	/* vertical scroll bar */
            FCF_HORZSCROLL |	/* horizontal scroll bar */
	    FCF_TASKLIST |	/* show it in window list */
	    FCF_ICON;		/* Load icon from resources */
#ifdef UNUSED
/* need to load these from a different module */
	    FCF_MENU |		/* Load menu from resources */
	    FCF_ACCELTABLE;	/* Load accelerator table from resources */
#endif

/* returns TRUE if language change successful */
BOOL
load_language(int language)
{   /* load language dependent resources */
char langdll[MAXSTR];
char buf[MAXSTR];
HMODULE hmodule;
APIRET rc;
    /* load language dependent resources */
    strcpy(langdll, szExePath);
    strcat(langdll, "gvpm");
    switch (language) {
	case IDM_LANGDE:
	    strcat(langdll, "de");
	    break;
	default:
	    strcat(langdll, "en");
    }
    strcat(langdll, ".dll");
    rc = DosLoadModule(buf, sizeof(buf), langdll, &hmodule);
    if (!rc) {
	if (hlanguage)
	    DosFreeModule(hlanguage);
	hlanguage = hmodule;

	load_string(IDS_GSVIEWVERSION, langdll, sizeof(langdll));
	if (strcmp(GSVIEW_VERSION, langdll) != 0)
	    message_box("Language resources version doesn't match GSview EXE", 0);

	return TRUE;
    }

    return FALSE;
}

void
change_language(void)
{
char *p;
char helptitle[MAXSTR];
    WinEnableWindowUpdate(hwnd_frame, FALSE);
    if (haccel)
	WinDestroyAccelTable(haccel);
    if (hwnd_menu)
	WinDestroyWindow(hwnd_menu);
    hwnd_menu = WinLoadMenu(hwnd_frame, hlanguage, ID_GSVIEW);
    haccel = WinLoadAccelTable(hab, hlanguage, ID_GSVIEW);
    WinSetAccelTable(hab, haccel, hwnd_frame);
    frame_flags |= (FCF_MENU | FCF_ACCELTABLE);
    WinSendMsg(hwnd_frame, WM_UPDATEFRAME, (MPARAM)(frame_flags), (MPARAM)0);

    /* get path to help file */
    strcpy(szHelpName, szExePath);
    p = szHelpName + strlen(szHelpName);
    load_string(IDS_HELPFILE, p, sizeof(szHelpName) - (int)(p-szHelpName));
    load_string(IDS_HELPTITLE, helptitle, sizeof(helptitle));

    if (hwnd_help)
	WinDestroyHelpInstance(hwnd_help);
    /* create help window */
    hi_help.cb = sizeof(HELPINIT);
    hi_help.ulReturnCode = 0;
    hi_help.pszTutorialName = NULL;
    hi_help.phtHelpTable = NULL;
    hi_help.hmodAccelActionBarModule = 0;
    hi_help.idAccelTable = 0;
    hi_help.idActionBar = 0;
    hi_help.pszHelpWindowTitle=(PSZ)helptitle;
    hi_help.hmodHelpTableModule = 0;
    hi_help.fShowPanelId = 0;
    hi_help.pszHelpLibraryName = (PSZ)szHelpName;
    hwnd_help = WinCreateHelpInstance(hab, &hi_help);
    if (!hwnd_help || hi_help.ulReturnCode) {
	char buf[512];
	sprintf(buf, "WinCreateHelpInstance helpfile=%s handle=%ld, rc=%ld",  
	    szHelpName, hwnd_help, hi_help.ulReturnCode);
	message_box(buf, 0);
    }
    if (hwnd_help)
	WinAssociateHelpInstance(hwnd_help, hwnd_frame);

    load_string(IDS_TOPICROOT, szHelpTopic, sizeof(szHelpTopic));
    init_check_menu();

    WinEnableWindowUpdate(hwnd_frame, TRUE);
    WinShowWindow(hwnd_frame, TRUE);
}

MRESULT EXPENTRY 
LanguageDlgProc(HWND hwnd, ULONG mess, MPARAM mp1, MPARAM mp2)
{
    switch(mess) {
        case WM_COMMAND:
            switch(SHORT1FROMMP(mp1)) {
		case DID_CANCEL:
                case DID_OK:
                    WinDismissDlg(hwnd, 0);
                    break;
		case IDM_LANGEN:
		case IDM_LANGDE:
                    WinDismissDlg(hwnd, SHORT1FROMMP(mp1));
            }
            break;
    }
    return WinDefDlgProc(hwnd, mess, mp1, mp2);
}

void 
check_language(void)
{
int language;
COUNTRYCODE pcc;
COUNTRYINFO pci;
ULONG pcbActual;
    pcc.country = 0;	/* ask about default country */
    pcc.codepage = 0;
    if (DosQueryCtryInfo(sizeof(pci), &pcc, &pci, &pcbActual) != 0)
	return;	/* give up */

   if (pcbActual == 0)
	return;

    if (  ((option.language == IDM_LANGEN) &&
	  !((pci.country == 99) || (pci.country == 61) ||
	    (pci.country == 44) || (pci.country == 1)))
	   ||
	  ((option.language == IDM_LANGDE) && (pci.country != 49))
	)
    {	/* GSview language doesn't match country code */
	language = WinDlgBox(HWND_DESKTOP, hwnd_frame, LanguageDlgProc, hlanguage, IDD_LANG, NULL);
	switch (language) {
	    case IDM_LANGEN:
	    case IDM_LANGDE:
		gsview_language(language);
	}
    }
}

APIRET
gsview_init(int argc, char *argv[])
{
  unsigned char class[] = "gvBmpClass";  /* class name */
  unsigned char status_class[] = "gvStatusClass";
  unsigned char button_class[] = "gvButtonClass";
  APIRET rc = 0;
  SWP swp;
  char *cmd, *cmdbase;
  char **argp;
  char filedir[MAXSTR];
  char workdir[MAXSTR];
  FONTMETRICS fm;
  POINTL char_size;
  RECTL rect;
  HPS hps;
  PVOID pResource;
  int i;
  short *pButtonID;
  ULONG version[3];

    gs_getcwd(workdir, sizeof(workdir));	/* remember the working directory */
    if (DosQuerySysInfo(QSV_VERSION_MAJOR, QSV_VERSION_REVISION, &version, sizeof(version)))
	os_version = 201000;  /* a guess */
    else {
	os_version = version[0]*10000 + version[1]*100 + version[2];
    }

    multithread = TRUE;

    rc = init1();
    if (rc)
	return rc;

    /* save SHELL default size and location */
    rc = WinQueryTaskSizePos(hab, 0, &swp);
    if (rc)
	return rc;

    load_string(IDS_WAIT, szWait, sizeof(szWait));

    init_options();
    read_profile(szIniFile);

    if (!load_language(option.language)) {
	message_box("Couldn't load language specific resources.  Resetting to English.", 0);
	option.language = IDM_LANGEN;
	if (!load_language(option.language)) {
	    message_box("Couldn't load English resources.  Please reinstall GSview", 0);
	    return -1;
	}
    }

    if (!WinRegisterClass(	/* register this window class */
  	hab,			/* anchor block */
  	(PSZ)class,		/* class name */
  	(PFNWP) ClientWndProc,	/* window function */
  	CS_SIZEREDRAW |		/* window style */
	CS_MOVENOTIFY,		
  	0))			/* no storage */
	return -1;

    hwnd_frame = WinCreateStdWindow(
  	HWND_DESKTOP,		/* window type */
	0,			/* frame style is not WS_VISIBLE */
  	&frame_flags,		/* definitions */
  	(PSZ)class,		/* client class */
  	(PSZ)szAppName,		/* title */
  	WS_VISIBLE,		/* client style */
  	0,			/* resource module */
  	ID_GSVIEW,		/* resource identifier */
  	&hwnd_bmp);		/* pointer to client */

    change_language();

    if (!WinRegisterClass(	/* register this window class */
  	hab,			/* anchor block */
  	(PSZ)status_class,	/* class name */
  	(PFNWP) StatusWndProc,	/* window function */
  	CS_SIZEREDRAW,		/* window style */
  	0))			/* no storage */
	return -1;

    if (!WinRegisterClass(	/* register this window class */
  	hab,			/* anchor block */
  	(PSZ)button_class,	/* class name */
  	(PFNWP) ButtonWndProc,	/* window function */
  	CS_SIZEREDRAW | CS_MOVENOTIFY,		/* window style */
  	0))			/* no storage */
	return -1;

    hptr_crosshair = WinLoadPointer(HWND_DESKTOP, 0, IDP_CROSSHAIR);
    hptr_hand = WinLoadPointer(HWND_DESKTOP, 0, IDP_HAND);

    /* get initial size and position of status window */
    WinQueryWindowRect(hwnd_bmp, &rect);
    statusbar.x = rect.xRight - rect.xLeft;
    hps = WinGetPS(hwnd_bmp);
    GpiQueryFontMetrics(hps, sizeof(FONTMETRICS), &fm);
    char_size.y = fm.lMaxAscender + fm.lMaxDescender + fm.lExternalLeading + 2;
    char_size.x = fm.lAveCharWidth;
    statusbar.y = char_size.y;
    WinReleasePS(hwnd_bmp);

    hwnd_status = WinCreateWindow(
    	hwnd_frame,
    	status_class,
    	"status bar",
	WS_VISIBLE | WS_SYNCPAINT,
	rect.xLeft, rect.yTop, rect.xRight, statusbar.y,
	hwnd_frame,
	HWND_TOP,
	ID_STATUSBAR,
	NULL,
	NULL);

    button_size.x = 24;
    button_size.y = 24;
    button_shift.x = 0;
    button_shift.y = button_size.y;
    buttonbar.x = button_size.x;
    hwnd_button = WinCreateWindow(
    	hwnd_frame,
    	button_class,
    	"button bar",
	(option.button_show ? WS_VISIBLE : 0) | WS_SYNCPAINT,
	rect.xLeft, rect.yBottom, rect.xLeft+buttonbar.x, rect.yTop - rect.yBottom,
	hwnd_frame,
	hwnd_bmp,
	ID_BUTTONBAR,
	NULL,
	NULL);
    
    rc = DosGetResource((HMODULE)0, RT_RCDATA, IDR_BUTTON ,&pResource);
    if ( rc || (pResource == NULL) )
	return rc;
    pButtonID = (short *)pResource;
    WinQueryWindowRect(hwnd_button, &rect);
    hps = WinGetPS(hwnd_button);
    for (i=0; pButtonID[i]; i++) {
        char title[16];
	HBITMAP hbm;
	title[0] = '\0';
	hbm = GpiLoadBitmap(hps, 0, pButtonID[i], 0, 0);
	if (!hbm) {
	    load_string(pButtonID[i], title, sizeof(title));
	}
        if (buttonhead == (struct button *)NULL)
                buttontail = buttonhead = (struct button *)malloc(sizeof(struct button));
        else {
                buttontail->next = (struct button *)malloc(sizeof(struct button));
                buttontail = buttontail->next;
        }
	buttontail->id = pButtonID[i];
        buttontail->hbitmap = hbm;
	buttontail->rect.xLeft = rect.xLeft + i * button_shift.x;
	buttontail->rect.yBottom = rect.yTop - button_size.y - i * button_shift.y;
	buttontail->rect.xRight = buttontail->rect.xLeft + button_size.x;
	buttontail->rect.yTop = buttontail->rect.yBottom + button_size.y;
	strcpy(buttontail->str, title);
        buttontail->next = NULL;
    }
    WinReleasePS(hps);
    DosFreeResource(pResource);

    OldFrameWndProc = WinSubclassWindow(hwnd_frame, (PFNWP)FrameWndProc);

    rc = restore_window_position(&swp);
    if (rc)
	return rc;

    init2();

#ifdef __EMX__
    if (_emx_vcmp < 0x302e3962) {
	char buf[MAXSTR];
	char mess[MAXSTR];
	load_string(IDS_WRONGEMX, mess, sizeof(mess)-1);
	sprintf(buf, mess, _emx_vprt, EMX_NEEDED);
	message_box(buf, MB_ICONEXCLAMATION);
    }
#endif

    if (argc == 1)
	return rc;

    if ( (cmdbase = malloc(MAXSTR)) == (char *)NULL )
	return rc;

    cmd = cmdbase;
    cmd[0] = '\0';
    for (argp = &argv[1]; argc > 1; argc--, argp++) {
        if ( (strlen(*argp) >= 2)
	     &&  (((*argp)[0] == '/') || ((*argp)[0] == '-'))
	     &&  (((*argp)[1] == 'D') || ((*argp)[1] == 'd')) ) {
	    /* debug option */
	    debug = TRUE;
	}
	else if ( (strlen(*argp) >= 2)
	     &&  (((*argp)[0] == '/') || ((*argp)[0] == '-'))
	     &&  (((*argp)[1] == 'T') || ((*argp)[1] == 't')) ) {
	    /* multithread option */
	    if ((*argp)[2] == '\0')
		multithread = !multithread;
	    else if ((*argp)[2] == '0')
		multithread = FALSE;
	    else 
		multithread = TRUE;
	}
	else {
	    /* defer these options */
	    if ( strlen(*argp) + strlen(cmd) > MAXSTR-1 )
		break;
	    strcat(cmd, *argp);
	    if (argc > 2)
	    strcat(cmd, " ");
	}
    }

    if (!rc)
       rc = DosCreateMutexSem(NULL, &hmutex_ps, 0, FALSE);
    if (rc)
	return rc;
    gsview_initc(cmdbase);


    if (strlen(cmd) == 0) {
	/* no deferred options given */
	free(cmdbase);
	return rc;
    }

    if (strlen(cmd) > 2) {
	/* file given on command line, so use current directory not saved one */
	gs_chdir(workdir);
	/* skip commands /F /P or /S */
	if ((cmd[0] == '/') || (cmd[0] == '-')) {
	    while (*cmd && (*cmd != ' '))
	        cmd++;	/* skip over option */
	    while (*cmd && (*cmd == ' '))
	        cmd++;	/* skip over spaces */
	}
	/* build full path to file */
        if (cmd[0]=='\\') {
	    /* fully specified path on current drive */
	    /* need to prefix with current drive */
	    strcpy(filedir, cmd);	/* temporary save path */
	    strncpy(cmd, workdir, 2);	/* copy current drive */
	    strcpy(cmd+2, filedir);	/* append path */
	}
	else if (cmd[0] && isalpha(cmd[0]) && cmd[1]==':') {
	    /* drive code specified */
	    if ( (strlen(cmd) >= 3) && (cmd[2]=='\\') ) {
		/* fully specified path and drive */
		/* no action */
	    }
	    else if ( (strlen(cmd) >= 3) && (cmd[2]=='/') ) {
		/* fully specified path and drive */
		/* no action */
	    }
	    else {
		/* relative path on specified drive */
		/* change to drive, get directory, append relative path */
		strncpy(filedir, cmd, 2); /* copy drive code */
		filedir[2] = '\0';
		i = gs_chdir(filedir);
		gs_getcwd(filedir, sizeof(filedir));	/* get path on specified drive */
		if (filedir[0] && (filedir[strlen(filedir)-1]!='/')
		    && (filedir[strlen(filedir)-1]!='\\'))
		    strcat(filedir, "\\");
		strcat(filedir, cmd+2);	/* append relative path */
		strcpy(cmd, filedir);
		i = gs_chdir(workdir);
	    }
	}
	else {
	    /* relative path on current drive */
	    /* construct full path */
	    strcpy(filedir, cmd);	/* temporary save */
	    strcpy(cmd, workdir);	/* copy current drive */
	    if (cmd[0] && (cmd[strlen(cmd)-1]!='/')
		    && (cmd[strlen(cmd)-1]!='\\'))
		    strcat(cmd, "\\");
	    strcat(cmd, filedir);	/* append relative path */
	}
	/* load given file */
	WinPostMsg(hwnd_bmp, WM_COMMAND, (MPARAM)IDM_DROP, MPFROMP(cmdbase));	/* mp2 is not strictly correct */
    }
    return rc;
}

APIRET
restore_window_position(SWP *pswp)
{
    SWP swp;
    swp.fl = SWP_ACTIVATE | SWP_MOVE | SWP_SIZE | SWP_SHOW;

    if ((option.img_size.x   != CW_USEDEFAULT) &&
	(option.img_size.y   != CW_USEDEFAULT) &&
	(option.img_origin.y != CW_USEDEFAULT) &&
	(option.img_origin.y != CW_USEDEFAULT)) {
        LONG cxClientMax ;
        LONG cyClientMax ;
        LONG cyTitleBar ;
        LONG cxSizeBorder ;
        LONG cySizeBorder ;

        /* get maximum client window size */
        cxClientMax = WinQuerySysValue (HWND_DESKTOP, SV_CXFULLSCREEN) ;
        cyClientMax = WinQuerySysValue (HWND_DESKTOP, SV_CYFULLSCREEN) ;
        cyTitleBar = WinQuerySysValue (HWND_DESKTOP, SV_CYTITLEBAR) ;
        cxSizeBorder = WinQuerySysValue (HWND_DESKTOP, SV_CXSIZEBORDER) ;
        cySizeBorder = WinQuerySysValue (HWND_DESKTOP, SV_CYSIZEBORDER) ;
        cyClientMax += cyTitleBar ;

        if (option.img_max)
            swp.fl |= SWP_MAXIMIZE;

        /* Make sure x origin is within display boundaries */
        swp.x = option.img_origin.x ;
        if (swp.x < -cxSizeBorder)
            swp.x = 0 ;

        /* Make sure window isn't too wide, or negative value */
        swp.cx = option.img_size.x ;
        if (swp.cx >= cxClientMax || swp.cx < 0) {
            swp.cx = cxClientMax ;
            swp.x = 0 ;
        }

        if ((swp.x + swp.cx) > (cxClientMax + cxSizeBorder))
            swp.x = cxClientMax + cxSizeBorder - swp.cx ;

        /* Make sure y origin is within display boundaries */
        swp.y = option.img_origin.y ;
        if (swp.y < -cySizeBorder)
            swp.y = 0 ;

        /* Make sure window isn't too high, or negative value */
        swp.cy = option.img_size.y ;
        if (swp.cy > cyClientMax || swp.cy < 0) {
            swp.cy = cyClientMax ;
            swp.y = 0 ;
        }

        if ((swp.y + swp.cy) > (cyClientMax + cySizeBorder))
            swp.y = cyClientMax + cySizeBorder - swp.cy ;
    }
    else { /* No saved position -- use supplied position */
	swp = *pswp;
        swp.fl = SWP_ACTIVATE | SWP_MOVE | SWP_SIZE | SWP_SHOW;
    }

    /* Position and size this frame window */
    if (!WinSetWindowPos(hwnd_frame, HWND_TOP,
	swp.x, swp.y, swp.cx, swp.cy, swp.fl))
	return 1;

    return 0;
}

APIRET init1() 
{
    char buf[MAXSTR];
    char *tail, *env;
    APIRET rc = 0;
    char *p;

    PTIB pptib;
    PPIB pppib;

    if ( (rc = DosGetInfoBlocks(&pptib, &pppib)) != 0 ) {
	sprintf(buf,"init1: Couldn't get pid, rc = %ld\n", rc);
	error_message(buf);
	return rc;
    }

    /* get path to EXE */
    if ( (rc = DosQueryModuleName(pppib->pib_hmte, sizeof(szExePath), szExePath)) != 0 ) {
	sprintf(buf,"init1: Couldn't get module name, rc = %ld\n", rc);
	error_message(buf);
	return FALSE;
    }
    if ((tail = strrchr(szExePath,'\\')) != (PCHAR)NULL) {
	tail++;
	*tail = '\0';
    }
    strcpy(szHelpName, szExePath);
    p = szHelpName + strlen(szHelpName);
    load_string(IDS_HELPFILE, p, sizeof(szHelpName) - (int)(p-szHelpName));

    /* get path to INI directory */
    if ((env = getenv("SYSTEM_INI")) != (char *)NULL) {
	strcpy(szIniFile, env);
        if ((tail = strrchr(szIniFile,'\\')) != (PCHAR)NULL) {
		tail++;
		*tail = '\0';
	}
    }
    else
        strcpy(szIniFile, szExePath);
    strcat(szIniFile, INIFILE);

    if ((env = getenv("MMBASE")) != (char *)NULL) {
	char buf[MAXSTR];
	HMODULE hmodMCIAPI;
        strcpy(szMMini, env);
	strtok(szMMini, ";");
	strcat(szMMini, "\\MMPM.INI");
	/* attempt to load MCIAPI.DLL ourselves so gvpm will work without MMOS2 */
	if (!DosLoadModule(buf, sizeof(buf), "MCIAPI", &hmodMCIAPI)) {
	    DosQueryProcAddr(hmodMCIAPI, 0, "mciPlayFile", (PFN *)(&pfnMciPlayFile));
	}
    }


    /* local stuff */
    if (!rc) {
	rc = DosCreateEventSem(NULL, &display.event, 0, FALSE);
  	if (rc)
		error_message("Failed to create display.event semaphore");
    }

    if (!rc) {
	HPS ps = WinGetPS(HWND_DESKTOP);
	HDC hdc = GpiQueryDevice(ps);
	DevQueryCaps(hdc, CAPS_COLOR_PLANES, 1, &display.planes);
	DevQueryCaps(hdc, CAPS_COLOR_BITCOUNT, 1, &display.bitcount);
	DevQueryCaps(hdc, CAPS_ADDITIONAL_GRAPHICS, 1, &display.hasPalMan);
	display.hasPalMan &= CAPS_PALETTE_MANAGER;
	WinReleasePS(ps);
/*
	{ char buf[MAXSTR];
	sprintf(buf,"main: display planes = %d, bitcount = %d",display.planes,display.bitcount);
	message_box(buf, 0);
	}
*/
    }
    bitmap.valid = FALSE;
    return rc;
}

APIRET
init2(void)
{
	init_check_menu();
	update_scroll_bars();
	return 0;
}


void
show_buttons(void)
{
	WinSendMsg(hwnd_frame, WM_UPDATEFRAME, (MPARAM)frame_flags, (MPARAM)0);
}

int
gsview_create_objects(void)
{
char buf[MAXSTR];
char setup[MAXSTR];
APIRET rc;
    strcpy(buf, szExePath);
    strcat(buf, "\\gvpm.exe");
    sprintf(setup, "EXENAME=%s;ASSOCFILTER=*.ps,*.eps,*.pdf", buf);
    rc = !WinCreateObject("WPProgram", "GSview", setup, "<WP_DESKTOP>",
	CO_REPLACEIFEXISTS);
    if (rc)
        gserror(IDS_PROGRAMOBJECTFAILED, NULL, 0, SOUND_ERROR);
    return rc;
}
