/* Copyright (C) 1993-2000, Ghostgum Software Pty Ltd.  All rights reserved.
  
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
#include "gvc.h"

/* copy printer profiles */
int
gsview_printer_profiles(void)
{
char buf[MAXSTR];
FILE *pf;
char section[MAXSTR];
char *key;
const char *value;
PROFILE *prf;
    /* open an INI file and copy everything to user ini file
     * overwriting anything the user already had
     */
#ifdef UNIX
    strncpy(buf, szEtcPath, MAXSTR-1);
#else
    strncpy(buf, szExePath, MAXSTR-1);
#endif
    strncat(buf, "printer.ini", sizeof(buf)-1-strlen(buf));
    if (debug & DEBUG_GENERAL)
	gs_addmessf("Reading printer profiles from \042%s\042\n", buf);
    pf = fopen(buf, "r");
    if (!pf) {
        strcpy(buf, "printer.ini");
	if (debug & DEBUG_GENERAL)
	    gs_addmessf("Reading printer profiles from \042%s\042\n", buf);
        pf = fopen(buf, "r");
	if (!pf) {
	    gserror(IDS_NOPRINTERINI, NULL, 0, SOUND_ERROR);
	    return 1;
	}
    }
    prf = profile_open(szIniFile);
    if (!prf) {
	gserror(IDS_NOINI, NULL, 0, SOUND_ERROR);
	return 1;
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
    return 0;
}


void
init_options(void)
{
    option.language = IDM_LANGEN;
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
    option.unitfine = 0;
    option.quick_open = TRUE;
    option.pstotext = IDM_PSTOTEXTNORM - IDM_PSTOTEXTMENU - 1;
    option.settings = TRUE;
    option.button_show = TRUE;
    option.fit_page = FALSE;	/* Changed after 2.1 */
    option.safer = TRUE;
    option.media = IDM_A4;
    strcpy(option.medianame, "A4");
    option.media_rotate = FALSE;
    option.user_width = 610;
    option.user_height = 792;
    option.epsf_clip = FALSE;
    option.epsf_warn = FALSE;
    option.ignore_dsc = FALSE;
    option.dsc_warn = IDM_DSC_WARN;
    option.show_bbox = FALSE;
    option.redisplay = TRUE;
    option.auto_orientation = TRUE;	/* Added after 2.1 */
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
    strcpy(option.printer_device, "djet500");
    strcpy(option.printer_resolution, "300");
    option.print_fixed_media = 1;
    option.print_gdi_depth = IDC_MONO;
    option.print_gdi_fixed_media = 1;
    strcpy(option.convert_device, "pdfwrite");
    strcpy(option.convert_resolution, "600");
    option.convert_fixed_media = 1;
    option.print_to_file = FALSE;
#ifdef _Windows
    option.print_method = PRINT_GDI;
#else
    option.print_method = PRINT_GS;
#endif
    option.print_reverse = FALSE;
    option.pdf2ps = 0;
    option.auto_bbox = TRUE;
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
    char thismedia[32];
    if (!stricmp(option.medianame, MEDIA_USERDEFINED)) {
	option.media = IDM_USERSIZE;
    }
    else {
	for (i=IDM_MEDIAFIRST; i<IDM_MEDIALAST; i++) {
	    if (get_menu_string(IDM_MEDIAMENU, i, thismedia, sizeof(thismedia))) {
		if (!stricmp(thismedia, option.medianame)) {
		    option.media = i;
		    strncpy(option.medianame, thismedia, sizeof(option.medianame));
		    break;
		}
	    }
	}
    }

    /* update menus */
    check_menu_item(IDM_UNITMENU, option.unit, TRUE);
    check_menu_item(IDM_UNITMENU, IDM_UNITFINE, option.unitfine);
    check_menu_item(IDM_LANGMENU, option.language, TRUE);
    check_menu_item(IDM_PSTOTEXTMENU, option.pstotext + IDM_PSTOTEXTMENU + 1, TRUE);
    if (option.auto_orientation)
        check_menu_item(IDM_ORIENTMENU, IDM_AUTOORIENT, TRUE);
    else 
        check_menu_item(IDM_ORIENTMENU, option.orientation, TRUE);
    check_menu_item(IDM_ORIENTMENU, IDM_SWAPLANDSCAPE, option.swap_landscape);
    check_menu_item(IDM_MEDIAMENU, option.media, TRUE);
    check_menu_item(IDM_MEDIAMENU, IDM_MEDIAROTATE, option.media_rotate);
#if defined(_Windows) || defined(OS2)
    check_menu_item(IDM_OPTIONMENU, IDM_FITPAGE, option.fit_page);
    check_menu_item(IDM_OPTIONMENU, IDM_QUICK_OPEN, option.quick_open);
#endif
    check_menu_item(IDM_OPTIONMENU, IDM_SAVESETTINGS, option.settings);
    check_menu_item(IDM_OPTIONMENU, IDM_BUTTONSHOW, option.button_show);
    check_menu_item(IDM_OPTIONMENU, IDM_SAFER, option.safer);
    check_menu_item(IDM_OPTIONMENU, IDM_SAVEDIR, option.save_dir);
    check_menu_item(IDM_OPTIONMENU, IDM_AUTOREDISPLAY, option.redisplay);
    check_menu_item(IDM_OPTIONMENU, IDM_EPSFCLIP, option.epsf_clip);
    check_menu_item(IDM_OPTIONMENU, IDM_EPSFWARN, option.epsf_warn);
    check_menu_item(IDM_OPTIONMENU, IDM_IGNOREDSC, option.ignore_dsc);
    check_menu_item(IDM_DSCMENU, option.dsc_warn, TRUE);
    check_menu_item(IDM_OPTIONMENU, IDM_SHOWBBOX, option.show_bbox);
}

#if UNIX
void
default_gsdll(char *buf)
{
    strcpy(buf, "gs");
}

void
default_gsinclude(char *buf)
{
    buf[0] = '\0';
}

#else
void
default_gsdir(char *buf)
{
char *p;
    /* assume that GS is in the adjacent directory */
    strcpy(buf, szExePath);
    p = buf + strlen(buf) - 1;	/* remove trailing slash */
    if (*p == '\\')
	*p = '\0';
    p = strrchr(buf, '\\');	/* remove trailing gsview */
    if (p)
	*(++p) = '\0';
    if (option.gsversion % 100 == 0)
	sprintf(buf+strlen(buf), "gs%d.%01d", 
	    option.gsversion / 100, option.gsversion % 100);
    else
	sprintf(buf+strlen(buf), "gs%d.%02d", 
	    option.gsversion / 100, option.gsversion % 100);
}

void
default_gsdll(char *buf)
{
    if (get_gs_string(option.gsversion, "GS_DLL", buf, MAXSTR))
	return;
    default_gsdir(buf);
    strncat(buf, "\\bin\\", MAXSTR-1-strlen(buf));
    strncat(buf, GS_DLLNAME, MAXSTR-1-strlen(buf));
}

void
default_gsinclude(char *buf)
{
char destdir[MAXSTR];
    if (get_gs_string(option.gsversion, "GS_LIB", buf, MAXSTR))
	return;
    default_gsdir(destdir);
    default_gsinclude_from_path(buf, destdir);
}

/* return GS fontpath in buf, based on Ghostscript path in gspath */
/* Handles different paths pre/post GS 5.93 */
void
default_gsinclude_from_path(char *buf, char *gspath)
{
char temp[MAXSTR];
    strcpy(buf, gspath);
    if (option.gsversion >= 593)
        strncat(buf, "\\lib", MAXSTR-2-strlen(buf));
    strcat(buf, ";");

    strncat(buf, gspath, MAXSTR-1-strlen(buf));
    if (option.gsversion >= 593) {
	char *p;
	p = strrchr(buf, '\\');	/* remove trailing \\gsN.NN */
	if (p)
	    *p = '\0';
    }
    strncat(buf, "\\fonts", MAXSTR-1-strlen(buf));

    gs_getcwd(temp, sizeof(temp)-1);
    if (!gs_chdir("c:\\psfonts")) {
	strncat(buf, ";", MAXSTR-1-strlen(buf));
	strncat(buf, "c:\\psfonts", MAXSTR-1-strlen(buf));
    }
    gs_chdir(temp);
}

void
install_default(HWND hwnd)
{
char buf[MAXSTR];

    default_gsdll(buf);
    SetDlgItemText(hwnd, INSTALL_DLL, buf);

    default_gsinclude(buf);
    SetDlgItemText(hwnd, INSTALL_INCLUDE, buf);

    strcpy(buf, "-dNOPLATFONTS -sFONTPATH=\042c:\\psfonts\042");
    SetDlgItemText(hwnd, INSTALL_OTHER, buf);
}
#endif

int
gsview_changed(void)
{
char sysini[MAXSTR];

#ifndef UNIX
    if (!getenv("TEMP")) {
	gserror(IDS_NEEDTEMP, NULL, 0, 0);
	putenv("TEMP=c:\\");   /* just in case the user ignores us */
    }
#endif

    if (option.configured) {
	if (beta())
	    return 1;	/* don't run if expired */
	post_command_line();
	return 0;
    }

    if (beta_warn())
	return 1;	/* don't run */

    /* check if the system administrator has pre-configured GSview */
#ifdef UNIX
    strncpy(sysini, szEtcPath, MAXSTR-1);
#else
    strncpy(sysini, szExePath, MAXSTR-1);
#endif
    strncat(sysini, INIFILE, MAXSTR-1-strlen(sysini));
    read_profile(sysini);
    if (option.configured) {
	/* pre configured INI file was found */
	gsview_printer_profiles();	/* trust sys admin to have it correct */
	/* don't bother running the configure wizard */
	post_command_line();
	return 0;
    }
    /* revert back to our copy of INI file */
    read_profile(szIniFile);

    check_language();	/* offer to change language if doesn't match WIN.INI */

    config_wizard(FALSE);
    
    return 0;
}

