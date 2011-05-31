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

/* gvpmisc.c */
/* Miscellaneous PM GSview routines */

#include "gvpm.h"

BOOL
SetDlgItemText(HWND hwnd, int id, char *str)
{
	WinSetWindowText(WinWindowFromID(hwnd, id), str);
	return TRUE;
}

void
get_help()
{
    WinPostMsg(hwnd_frame, WM_HELP,
	MPFROMLONG(0), MPFROM2SHORT(CMDSRC_OTHER, FALSE));
}

/* display message */
int
message_box(char *str, int icon)
{
  	return WinMessageBox(HWND_DESKTOP, hwnd_frame ? hwnd_frame : HWND_DESKTOP, 
		str, szAppName, 0, icon | MB_MOVEABLE | MB_OK);
}

/* delayed message box, usually because we are on the other thread */
void
delayed_message_box(int id, int icon)
{
    WinPostMsg(hwnd_bmp, WM_GSMESSBOX, MPFROMLONG(id), MPFROMLONG(icon));
}

/* change menu item checkmark */
void
check_menu_item(int menuid, int itemid, BOOL checked)
{
HWND hwndMenu;
MENUITEM mi;
	hwndMenu = WinWindowFromID(hwnd_frame, FID_MENU);
	WinSendMsg(hwndMenu, MM_QUERYITEM, 
		MPFROM2SHORT(menuid, TRUE), MPFROMP(&mi));
	WinSendMsg(mi.hwndSubMenu, MM_SETITEMATTR, MPFROMLONG(itemid),
		MPFROM2SHORT(MIA_CHECKED, checked ? MIA_CHECKED : 0));
}

/* get text of menu item */
int
get_menu_string(int menuid, int itemid, char *str, int len)
{
HWND hwndMenu;
MENUITEM mi;
	hwndMenu = WinWindowFromID(hwnd_frame, FID_MENU);
	WinSendMsg(hwndMenu, MM_QUERYITEM, 
		MPFROM2SHORT(menuid, TRUE), MPFROMP(&mi));
	return (int)WinSendMsg(mi.hwndSubMenu, MM_QUERYITEMTEXT, 
		MPFROM2SHORT(itemid, len), MPFROMP(str));
}

int
load_string(int id, char *str, int len)
{
	return WinLoadString(hab, hlanguage, id, len, str);
}


void
play_system_sound(char *id)
{
HINI hini;
char buf[MAXSTR];
char *p;
	if ( (hini = PrfOpenProfile(hab, szMMini)) == NULLHANDLE )
	    return;
	PrfQueryProfileString(hini, "MMPM2_AlarmSounds", id, "##", buf, sizeof(buf));
	PrfCloseProfile(hini);
    	p = strchr(buf,'#');
    	if (p != (char *)NULL) {
	    *p = '\0';
	    (*pfnMciPlayFile)(hwnd_frame, buf, 0, 0, 0);
	}
	return;
}

void
play_sound(int num)
{
	if (strlen(sound[num].file)==0)
	    return;
	if (!(pfnMciPlayFile) || (strcmp(sound[num].file,BEEP)==0)) {
	    DosBeep(200,200);
	    return;
	}
	if (isdigit(*sound[num].file))
	    play_system_sound(sound[num].file);
	else {
	    char buf[MAXSTR];
	    buf[0] = '\042';
	    strcpy(buf+1, sound[num].file);
	    strcat(buf, "\042");
	    if ((*pfnMciPlayFile)(hwnd_frame, buf, 0, 0, 0))
	        DosBeep(200,200);
	}
}


void
info_wait(int id)
{
POINTL pt;
	if (id)
    	    load_string(id, szWait, sizeof(szWait));
	else 
	    szWait[0] = '\0';
	
	if (hwnd_status) {
	    WinInvalidateRect(hwnd_status, (PRECTL)NULL, TRUE);
  	    WinUpdateWindow(hwnd_status);
	}
	/* find out if cursor over hwnd_bmp */
	if (!WinQueryPointerPos(HWND_DESKTOP, &pt))
	    return;
	if (hwnd_bmp == WinWindowFromPoint(HWND_DESKTOP, &pt, TRUE))
	    WinSendMsg(hwnd_bmp, WM_MOUSEMOVE, MPFROM2SHORT(pt.x, pt.y), (MPARAM)0);
}


int 
gs_chdir(char *dirname)
{
#ifdef __BORLANDC__
	if (isalpha(dirname[0]) && (dirname[1]==':'))
		(void) setdisk(toupper(dirname[0])-'A');
	if (!((strlen(dirname)==2) && isalpha(dirname[0]) && (dirname[1]==':')))
		chdir(dirname);
	return TRUE;
#else
#ifdef __IBMC__
	if (isalpha(dirname[0]) && (dirname[1]==':'))
	    if (_chdrive(toupper(dirname[0])-'A'+1))
		return -1;
	if (!((strlen(dirname)==2) && isalpha(dirname[0]) && (dirname[1]==':')))
	    return _chdir(dirname);
#else
	if (isalpha(dirname[0]) && (dirname[1]==':'))
	    if (_chdrive(dirname[0]))
		return -1;
	return _chdir2(dirname);
#endif
#endif
}

#ifndef __IBMC__
char * 
gs_getcwd(char *dirname, int size)
{
#ifdef __BORLANDC__
	return getcwd(dirname, size);
#else
	return _getcwd2(dirname, size);
#endif
}
#endif


int
send_prolog(int resource)
{  
char *prolog, *p;
APIRET rc;
int code = -1;
	rc = DosGetResource(0, RT_RCDATA, resource, (PPVOID)&prolog);
	if (!rc && (prolog != (char *)NULL) ) {
	    code = 0;
	    p = prolog;
	    while (*p) {
		if (!code)
	            code = gs_execute(p, strlen(p));
		p += strlen(p)+1;;
	    }
	    DosFreeResource(prolog);
	}
	return code;
}

char tempbuf[8192];

void
profile_create_section(PROFILE *prf, char *section, int id)
{
char *rcdata, *entry, *value;
char name[MAXSTR];
    if (!DosGetResource(0, RT_RCDATA, id, (PPVOID)&rcdata)) {
	if (rcdata == (char *)NULL) {
	    profile_close(prf);
	    return;
	}
	entry = rcdata;
	while (strlen(entry)!=0) {
	    for ( value = entry; 
		  (*value!='\0') && (*value!=',') && (*value!='='); 
		  value++)
		/* nothing */;
	    strncpy(name, entry, value-entry);
	    name[value-entry] = '\0';
	    value++;
	    profile_write_string(prf, section, name, value);
	    entry = value + strlen(value) + 1;
	}
   }
   DosFreeResource(rcdata);
}

