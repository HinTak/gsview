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

/* gvpinit.c */
/* Initialisation routines for PM GSview */
/* by Russell Lang */
#include "gvpm.h"

APIRET init1(void); 
APIRET init2(void); 
APIRET restore_window_position(SWP *pswp);
void init_options(void);
ULONG frame_flags = 
	    FCF_TITLEBAR |	/* have a title bar */
            FCF_SIZEBORDER |	/* have a sizeable window */
            FCF_MINMAX |	/* have a min and max button */
            FCF_SYSMENU | 	/* include a system menu */
            FCF_VERTSCROLL |	/* vertical scroll bar */
            FCF_HORZSCROLL |	/* horizontal scroll bar */
	    FCF_TASKLIST |	/* show it in window list */
	    FCF_ICON |		/* Load icon from resources */
	    FCF_MENU |		/* Load menu from resources */
	    FCF_ACCELTABLE;	/* Load accelerator table from resources */

APIRET
gsview_init(int argc, char *argv[])
{
  unsigned char class[] = "gvBmpClass";  /* class name */
  unsigned char status_class[] = "gvStatusClass";
  unsigned char button_class[] = "gvButtonClass";
  APIRET rc = 0;
  SWP swp;
  char *cmd, *cmdbase, *p;
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

    _getcwd(workdir, sizeof(workdir));	/* remember the working directory */
    if (DosQuerySysInfo(QSV_VERSION_MAJOR, QSV_VERSION_REVISION, &version, sizeof(version)))
	os_version = 201000;  /* a guess */
    else {
	os_version = version[0]*10000 + version[1]*100 + version[2];
    }

    rc = init1();
    if (rc)
	return rc;

    /* save SHELL default size and location */
    rc = WinQueryTaskSizePos(hab, 0, &swp);
    if (rc)
	return rc;

    rc = DosCreateThread(&queue_tid, queue_thread, 0, 0, 8192);
    if (rc) {
        error_message("gsview_init: Failed to create queue thread");
        return rc;
    }

    rc = DosCreateThread(&term_tid, term_thread, 0, 0, 8192);
    if (rc) {
        error_message("gsview_init: Failed to create termination queue thread");
        return rc;
    }
    load_string(IDS_WAIT, szWait, sizeof(szWait));

    init_options();
    read_profile();

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
  	(PSZ)"PM GSview",	/* title */
  	WS_VISIBLE,		/* client style */
  	0,			/* resource module */
  	ID_GSVIEW,		/* resource identifier */
  	&hwnd_bmp);		/* pointer to client */

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

    if (_emx_vcmp < 0x302e3868) {
	char buf[256];
	sprintf(buf, "You have emx %s.\rYou need emx %s or later.\rPM GSview will not run correctly.",
	    _emx_vprt, EMX_NEEDED);
	message_box(buf, MB_ICONEXCLAMATION);
    }
    if (changed_version) {
	message_box("The installed version of GSview has changed.  \
Please read the Installation help and then correctly set\r\
Options | Ghostscript Command", MB_ICONEXCLAMATION);
	load_string(IDS_TOPICINSTALL, szHelpTopic, sizeof(szHelpTopic));
	get_help();
    }

    if (argc == 1)
	return rc;

    if ( (strlen(argv[1]) >= 2)
	 &&  ((argv[1][0] == '/') || (argv[1][0] == '-'))
	 &&  ((argv[1][1] == 'D') || (argv[1][1] == 'd')) ) {
	    debug = TRUE;
	    argv++;
	    argc--;
    }

    if ( (cmdbase = malloc(MAXSTR)) == (char *)NULL )
	return rc;

    cmd = cmdbase;
    cmd[0] = '\0';
    for (argp = &argv[1]; argc > 1; argc--, argp++) {
	if ( strlen(*argp) + strlen(cmd) > MAXSTR-1 )
	    break;
	strcat(cmd, *argp);
	if (argc > 2)
	    strcat(cmd, " ");
    }

    if (strlen(cmd) > 2) {
	/* file given on command line, so use current directory not saved one */
	_chdir(workdir);
	/* skip commands /F /P or /S */
	if ((cmd[0] == '/') || (cmd[0] == '-')) {
	    cmd += 2;
	    while (*cmd && (*cmd == ' '))
	        cmd++;
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
		i = _chdir(filedir);
		_getcwd(filedir, sizeof(filedir));	/* get path on specified drive */
		if (filedir[0] && (filedir[strlen(filedir)-1]!='/')
		    && (filedir[strlen(filedir)-1]!='\\'))
		    strcat(filedir, "\\");
		strcat(filedir, cmd+2);	/* append relative path */
		strcpy(cmd, filedir);
		i = _chdir(workdir);
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
    char buf[256];
    char name[256];
    char *tail, *env;
    APIRET rc = 0;

    PTIB pptib;
    PPIB pppib;

    if ( (rc = DosGetInfoBlocks(&pptib, &pppib)) != 0 ) {
	sprintf(buf,"init1: Couldn't get pid, rc = \n", rc);
	error_message(buf);
	return rc;
    }
    gsview.pid = pppib->pib_ulpid;
    sprintf(gsview.id, "GSVIEW_%u", gsview.pid);

    /* get path to EXE */
    if ( (rc = DosQueryModuleName(pppib->pib_hmte, sizeof(szExePath), szExePath)) != 0 ) {
	sprintf(buf,"init1: Couldn't get module name, rc = %d\n", rc);
	error_message(buf);
	return FALSE;
    }
    if ((tail = strrchr(szExePath,'\\')) != (PCHAR)NULL) {
	tail++;
	*tail = '\0';
    }
    strcat(szHelpFile, szExePath);
    strcat(szHelpFile, HELPFILE);

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


    if (!rc) {
	sprintf(name, NEXT_NAME, gsview.id);
	rc = DosCreateEventSem(name, &gsview.next_event, 0, FALSE);
  	if (rc) {
		sprintf(buf, "Failed to create: next page event semaphore \"%s\" rc = %d", name, rc);
		error_message(buf);
	}
    }
    if (!rc) {
	sprintf(name, MUTEX_NAME, gsview.id);
	rc = DosCreateMutexSem(name, &gsview.bmp_mutex, 0, FALSE);
  	if (rc) {
		sprintf(buf, "Failed to create: bmp mutex semaphore \"%s\" rc = %d", name, rc);
		error_message(buf);
	}
    }
    if (!rc) {
	sprintf(name, QUEUE_NAME, gsview.id);
	if ( (rc = DosCreateQueue(&gsview.queue, QUE_FIFO, name)) != 0 ) {
		sprintf(buf,"Failed to create: \"%s\", rc = %d\n", name, rc);
		error_message(buf);
	}

    }
    if (!rc) {
	sprintf(gsview.term_queue_name, "\\QUEUES\\TERM_%s", gsview.id);
	if ( (rc = DosCreateQueue(&gsview.term_queue, QUE_FIFO, gsview.term_queue_name)) != 0 ) {
		sprintf(buf,"Failed to create: \"%s\", rc = %d\n", gsview.term_queue_name, rc);
		error_message(buf);
	}
    }

    /* local stuff */
    if (!rc) {
	rc = DosCreateEventSem(NULL, &display.done, 0, FALSE);
  	if (rc)
		error_message("Failed to create: display.done semaphore");
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
	{ char buf[256];
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
	int i;
	char thismedia[20];
	for (i=IDM_LETTER; i<IDM_USERSIZE; i++) {
	    get_menu_string(IDM_MEDIAMENU, i, thismedia, sizeof(thismedia));
	    if (!stricmp(thismedia, option.medianame)) {
		break;
	    }
	}
	option.media = i;
	strncpy(option.medianame,thismedia,sizeof(option.medianame));
	/* update menus */
	if (option.quick) check_menu_item(IDM_OPTIONMENU, IDM_QUICK, TRUE);
	if (option.settings) check_menu_item(IDM_OPTIONMENU, IDM_SAVESETTINGS, TRUE);
	if (option.button_show) check_menu_item(IDM_OPTIONMENU, IDM_BUTTONSHOW, TRUE);
	if (option.fit_page) check_menu_item(IDM_OPTIONMENU, IDM_FITPAGE, TRUE);
	if (option.safer) check_menu_item(IDM_OPTIONMENU, IDM_SAFER, TRUE);
	if (option.save_dir) check_menu_item(IDM_OPTIONMENU, IDM_SAVEDIR, TRUE);
	if (option.redisplay) check_menu_item(IDM_OPTIONMENU, IDM_AUTOREDISPLAY, TRUE);
	if (option.epsf_clip) check_menu_item(IDM_OPTIONMENU, IDM_EPSFCLIP, TRUE);
	if (option.epsf_warn) check_menu_item(IDM_OPTIONMENU, IDM_EPSFWARN, TRUE);
	if (option.ignore_dsc) check_menu_item(IDM_OPTIONMENU, IDM_IGNOREDSC, TRUE);
	check_menu_item(IDM_UNITMENU, option.unit, TRUE);
	check_menu_item(IDM_ORIENTMENU, option.orientation, TRUE);
	check_menu_item(IDM_MEDIAMENU, option.media, TRUE);
	check_menu_item(IDM_DEPTHMENU, gsview_depth_to_menu(option.depth), TRUE);
	update_scroll_bars();
	return 0;
}


void
init_options()
{
int i;
	/* make an educated guess about gs command */
	strcpy(option.gscommand, szExePath);
	strcat(option.gscommand, DEFAULT_GSCOMMAND);
	strcat(option.gscommand, " -I");
	strcat(option.gscommand, szExePath);
	strcpy(option.gscommand+strlen(option.gscommand)-1, ";");
	strcat(option.gscommand, szExePath);
	strcat(option.gscommand, "fonts;");
	i = strlen(option.gscommand);
	option.gscommand[i++] = tolower(szIniFile[0]);
	option.gscommand[i] = '\0';
	strcat(option.gscommand, ":\\psfonts");
	option.img_origin.x = CW_USEDEFAULT;
	option.img_origin.y = CW_USEDEFAULT;
	option.img_size.x = CW_USEDEFAULT;
	option.img_size.y = CW_USEDEFAULT;
	option.img_max = FALSE;
	option.unit = IDM_UNITPT;
	option.quick = TRUE;
	option.settings = TRUE;
	option.button_show = TRUE;
	option.fit_page = TRUE;
	option.safer = TRUE;
	option.media = IDM_LETTER;
	strcpy(option.medianame, "letter");
	option.user_width = 610;
	option.user_height = 792;
	option.epsf_clip = FALSE;
	option.epsf_warn = FALSE;
	option.ignore_dsc = FALSE;
	option.redisplay = TRUE;
	option.orientation = IDM_PORTRAIT;
	option.swap_landscape = FALSE;
	option.xdpi = DEFAULT_RESOLUTION;
	option.ydpi = DEFAULT_RESOLUTION;
	option.save_dir = TRUE;
	strcpy(option.device_name, "deskjet");
	strcpy(option.device_resolution, "300");
}

void
show_buttons(void)
{
	WinSendMsg(hwnd_frame, WM_UPDATEFRAME, (MPARAM)frame_flags, (MPARAM)0);
}

