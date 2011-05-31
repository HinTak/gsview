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

/* gvcinit.c */
/* Initialisation routines for PM and Windows GSview */
#ifdef _Windows
#include "gvwin.h"
#else
#include "gvpm.h"
#endif


/* copy printer profiles */
void
gsview_printer_profiles(void)
{
char buf[MAXSTR];
FILE *pf;
char section[MAXSTR];
char *key, *value;
PROFILE *prf;
    if (message_box("Update GSview printer list, overwriting any existing entries?", 
	MB_YESNO) != IDYES)
	return;

    /* open an INI file and copy everything to user ini file
     * overwriting anything the user already had
     */
    strcpy(buf, szExePath);
    strcat(buf, "printer.ini");
    pf = fopen(buf, "r");
    if (!pf) {
	message_box("Can't open printer.ini", 0);
	return;
    }
    prf = profile_open(szIniFile);
    if (!prf) {
	message_box("Can't open INI file", 0);
	return;
    }
    while (fgets(buf, sizeof(buf)-1, pf)) {
	if (buf[0] == '[') {
	    /* new section */
	    strcpy(section, buf+1);
	    strtok(section, "[]");
	    /* delete old section */
	    profile_write_string(prf, section, NULL, NULL);
	}
	else if ((buf[0] != '\n') && (buf[0] != ';')) {
	    key = strtok(buf, "=");
	    value = strtok(NULL, "\n");
	    if (value == (char *)NULL)
		value = "";
	    /* don't copy devices from wrong OS */
#ifdef _Windows
	    if ((strcmp(section, "Devices")==0) &&
		(strcmp(key, "os2prn") == 0))
		    key = NULL;
#else
	    if ((strcmp(section, "Devices")==0) &&
		(strncmp(key, "mswinpr", 7) == 0))
		    key = NULL;
#endif
	    if (key)
		profile_write_string(prf, section, key, value);
	}
    }
    profile_close(prf);
    fclose(pf);
}


void
init_options(void)
{
    default_gsdll(option.gsdll);
    default_gsinclude(option.gsinclude);
    option.gsother[0] = '\0';

    option.img_origin.x = CW_USEDEFAULT;
    option.img_origin.y = CW_USEDEFAULT;
    option.img_size.x = CW_USEDEFAULT;
    option.img_size.y = CW_USEDEFAULT;
    option.img_max = FALSE;
    option.drawmethod = IDM_DRAWDEF;
    option.unit = IDM_UNITPT;
    option.quick_open = TRUE;
    option.quick_text = FALSE;
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
    option.show_bbox = FALSE;
    option.redisplay = TRUE;
    option.orientation = IDM_PORTRAIT;
    option.swap_landscape = FALSE;
    option.xdpi = DEFAULT_RESOLUTION;
    option.ydpi = DEFAULT_RESOLUTION;
    option.zoom_xdpi = 300;
    option.zoom_ydpi = 300;
    option.depth = 0;
    option.alpha_text = 1;
    option.alpha_graphics = 1;
    option.save_dir = TRUE;
    strcpy(option.device_name, "deskjet");
    strcpy(option.device_resolution, "300");
    option.configured = FALSE;
}

void
gsview_initc(LPSTR cmdline)
{
    if (debug) {
	gs_addmess("Debugging ON\n");
	if (multithread)
	   gs_addmess("Multi Threaded\n");
	else
	   gs_addmess("Single Threaded\n");
	gs_addmess("Command line: ");
	gs_addmess(cmdline);
	gs_addmess("\n");
    }

    play_sound(SOUND_START);
}


void
init_check_menu(void)
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
    check_menu_item(IDM_UNITMENU, option.unit, TRUE);
    check_menu_item(IDM_ORIENTMENU, option.orientation, TRUE);
    check_menu_item(IDM_ORIENTMENU, IDM_SWAPLANDSCAPE, option.swap_landscape);
    check_menu_item(IDM_MEDIAMENU, option.media, TRUE);
    check_menu_item(IDM_OPTIONMENU, IDM_QUICK_OPEN, option.quick_open);
    check_menu_item(IDM_OPTIONMENU, IDM_QUICK_TEXT, option.quick_text);
    check_menu_item(IDM_OPTIONMENU, IDM_SAVESETTINGS, option.settings);
    check_menu_item(IDM_OPTIONMENU, IDM_BUTTONSHOW, option.button_show);
    check_menu_item(IDM_OPTIONMENU, IDM_FITPAGE, option.fit_page);
    check_menu_item(IDM_OPTIONMENU, IDM_SAFER, option.safer);
    check_menu_item(IDM_OPTIONMENU, IDM_SAVEDIR, option.save_dir);
    check_menu_item(IDM_OPTIONMENU, IDM_AUTOREDISPLAY, option.redisplay);
    check_menu_item(IDM_OPTIONMENU, IDM_EPSFCLIP, option.epsf_clip);
    check_menu_item(IDM_OPTIONMENU, IDM_EPSFWARN, option.epsf_warn);
    check_menu_item(IDM_OPTIONMENU, IDM_IGNOREDSC, option.ignore_dsc);
    check_menu_item(IDM_OPTIONMENU, IDM_SHOWBBOX, option.show_bbox);
}


void
default_gsdll(char *buf)
{
char destdir[MAXSTR];
char *p;
    /* assume that GS is in the adjacent directory */
    strcpy(destdir, szExePath);
    p = strrchr(destdir, '\\');	/* remove trailing \ */
    if (p)
	*p = '\0';
    p = strrchr(destdir, '\\');	/* remove trailing gsview */
    if (p)
	*(++p) = '\0';
    strcat(destdir, GS_BASEDIR);

    strcpy(buf, destdir);
    strcat(buf, "\\");
    strcat(buf, GS_DLLNAME);
}

void
default_gsinclude(char *buf)
{
char destdir[MAXSTR];
char *p;
    /* assume that GS is in the adjacent directory */
    strcpy(destdir, szExePath);
    p = strrchr(destdir, '\\');	/* remove trailing \ */
    if (p)
	*p = '\0';
    p = strrchr(destdir, '\\');	/* remove trailing gsview */
    if (p)
	*(++p) = '\0';
    strcat(destdir, GS_BASEDIR);

    strcpy(buf, destdir);

    strcat(buf, ";");
    strcat(buf, destdir);
    strcat(buf, "\\fonts");

    gs_getcwd(destdir, sizeof(destdir)-1);
    if (!gs_chdir("c:\\psfonts")) {
	strcat(buf, ";");
	strcat(buf, "c:\\psfonts");
    }
    gs_chdir(destdir);
}

void
install_default(HWND hwnd)
{
char buf[MAXSTR];

    default_gsdll(buf);
    SetDlgItemText(hwnd, INSTALL_DLL, buf);

    default_gsinclude(buf);
    SetDlgItemText(hwnd, INSTALL_INCLUDE, buf);

    buf[0]='\0';
    SetDlgItemText(hwnd, INSTALL_OTHER, buf);
}

int
gsview_changed(void)
{
FILE *f;
char buf[MAXSTR];
char *p;
    if (!getenv("TEMP")) {
	message_box("You must set the environment variable TEMP to a valid \
writeable directory.  Without this, GSview and Aladdin Ghostscript \
will not be able to print", 0);
	putenv("TEMP=c:\\");   /* just in case the user ignores us */
    }

    if (option.configured)
	return beta();	/* don't run if expired */

    if (beta_warn())
	return 1;	/* don't run */

    if (message_box("The installed version of GSview has changed. \
Proceed with configuration of GSview?", MB_YESNO) != IDYES) {
	message_box("GSview has not been configured correctly.  \
Please read the Installation help and then correctly set\
\042Options | Configure Ghostscript\042", 0);
	return 0;
    }
    
    default_gsdll(option.gsdll);
    default_gsinclude(option.gsinclude);
    option.gsother[0] = '\0';

    /* check if Ghostscript really has been installed */
    /* first look for the DLL */
    if ( (f = fopen(option.gsdll, "rb")) == (FILE *)NULL ) {
	message_box("You have not installed Ghostscript.  \
Please read the GSview README.TXT file.", 0);
	return 0;
    }
    fclose(f);

    /* next look for gs_init.ps */
    strcpy(buf, option.gsdll);
    p = strrchr(buf, '\\');	/* remove trailing DLLNAME */
    if (p)
	*(++p) = '\0';
    strcat(buf, "gs_init.ps");
    if ( (f = fopen(buf, "rb")) == (FILE *)NULL ) {
	message_box("You have not installed the Ghostscript library files.  \
Please read the GSview README.TXT file.", 0);
	return 0;
    }
    fclose(f);
    /* at this stage we don't look for fonts, but maybe we should */
    
    option.configured = TRUE;

    /* save the current options */
    write_profile();

    /* copy printer profiles */
    gsview_printer_profiles();

    /* update for platform, e.g. registry, progman, object */
    gsview_create_objects();

    message_box("Configuration complete", 0);
    return 0;
}

