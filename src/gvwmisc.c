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

/* gvwmisc.c */
/* Miscellaneous Windows GSview routines */

#include "gvwin.h"

/* SetDlgItemText is a Windows API */

void
post_img_message(int message, int param)
{
    PostMessage(hwndimg, message, (WPARAM)param, (LPARAM)0);
}

void
get_help(void)
{
	SendMessage(hwndimg, help_message, 0, 0L);
}

/* display message */
int message_box(char *str, int icon)
{
	return MessageBox(hwndimg, str, szAppName, icon | MB_OK);
}

void
delayed_message_box(int id, int icon)
{
    PostMessage(hwndimg, WM_GSMESSBOX, (WPARAM)id, (LPARAM)icon);
}

#pragma argsused
/* change menu item checkmark */
void
check_menu_item(int menuid, int itemid, BOOL checked)
{
        CheckMenuItem(hmenu, itemid, MF_BYCOMMAND | (checked ? MF_CHECKED : MF_UNCHECKED));
}

#pragma argsused
/* get text of menu item */
int
get_menu_string(int menuid, int itemid, char *str, int len)
{
	return GetMenuString(hmenu, itemid, str, len, MF_BYCOMMAND);
}

int
load_string(int id, char *str, int len)
{
	return LoadString(hlanguage, id, str, len);
}

void
play_sound(int num)
{
	if (strlen(sound[num].file)==0)
		return;
	if (strcmp(sound[num].file,BEEP)==0) {
		MessageBeep(-1);
		return;
	}
	if (lpfnSndPlaySound != (FPSPS)NULL) 
	    lpfnSndPlaySound(sound[num].file, SND_SYNC);
	else
	    MessageBeep(-1);
}


/* display or remove 'wait' message */
void
info_wait(int id)
{
HWND hwnd;
POINT pt;
	if (id)
    	    load_string(id, szWait, sizeof(szWait));
	else 
	    szWait[0] = '\0';

	InvalidateRect(hwndimg, (LPRECT)&info_rect, FALSE);
	UpdateWindow(hwndimg);

	if (szWait[0] != '\0') {
            GetCursorPos(&pt);
	    hwnd = WindowFromPoint(pt);
	    if ((hwnd == hwndimg) || IsChild(hwndimg,hwnd))
		SetCursor(hcWait);
	}
	else {
	    /* set cursor to that of active window */
	    hwnd = GetActiveWindow();
	    if ( (gsdll.device)
	      && ((hwnd == hwndimg) || (hwnd == hwndimgchild)) ) {
		if (in_child_client_area()) {
			SetCursor(GetClassCursor(hwndimgchild));
			return;
		}
	    }
	    SetCursor(GetClassCursor(hwnd));
	}
}

/* change directory and drive */
int
gs_chdir(char *dirname)
{
#ifdef __WIN32__
	return !SetCurrentDirectory(dirname);
#else
	if (isalpha(dirname[0]) && (dirname[1]==':'))
		(void) setdisk(toupper(dirname[0])-'A');
	if (!((strlen(dirname)==2) && isalpha(dirname[0]) && (dirname[1]==':')))
		return chdir(dirname);
	return 0;
#endif
}

char * 
gs_getcwd(char *dirname, int size)
{
#ifdef __WIN32__
	GetCurrentDirectory(size, dirname);
	return dirname;
#else
	return getcwd(dirname, size);
#endif
}


int
send_prolog(int resource)
{  
HGLOBAL hglobal;
LPSTR prolog;
int code = -1;
	hglobal = LoadResource(phInstance, 
	    FindResource(phInstance, MAKEINTRESOURCE(resource), RT_RCDATA));
	if ( (prolog = (LPSTR)LockResource(hglobal)) != (LPSTR)NULL) {
	    code = gs_execute(prolog, lstrlen(prolog));
	    FreeResource(hglobal);
	}
	return code;
}

void
profile_create_section(PROFILE *prf, char *section, int id)
{  
HGLOBAL hglobal;
LPSTR entry, value;
char name[MAXSTR];
char val[256];
	hglobal = LoadResource(phInstance, 
	    FindResource(phInstance, MAKEINTRESOURCE(id), RT_RCDATA));
	if ( (entry = (LPSTR)LockResource(hglobal)) == (LPSTR)NULL)
	    return;
	while (lstrlen(entry)!=0) {
	    for ( value = entry; 
		  (*value!='\0') && (*value!=',') && (*value!='='); 
		  value++)
		/* nothing */;
	    _fstrncpy(name, entry, (int)(value-entry));
	    name[(int)(value-entry)] = '\0';
	    value++;
	    _fstrncpy(val, value, sizeof(val));
	    profile_write_string(prf, section, name, val);
	    entry = value + lstrlen(value) + 1;
	}
	FreeResource(hglobal);
}
