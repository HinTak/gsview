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

/* gvwmisc.c */
/* Miscellaneous Windows GSview routines */

#include "gvwin.h"

/* SetDlgItemText is a Windows API */
/* PostQuitMessage is a Windows API */

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

/* change menu item checkmark */
void
check_menu_item(int menuid, int itemid, BOOL checked)
{
	menuid = menuid;	/* shut up warning */
        CheckMenuItem(hmenu, itemid, MF_BYCOMMAND | (checked ? MF_CHECKED : MF_UNCHECKED));
}

/* get text of menu item */
int
get_menu_string(int menuid, int itemid, char *str, int len)
{
	menuid = menuid;	/* shut up warning */
	return GetMenuString(hmenu, itemid, str, len, MF_BYCOMMAND);
}

int
load_string(int id, char *str, int len)
{
	return LoadString(phInstance, id, str, len);
}

void
play_sound(int num)
{
	if (strlen(sound[num].file)==0)
		return;
	if (!is_win31 || (strcmp(sound[num].file,BEEP)==0)) {
		MessageBeep(-1);
		return;
	}
	if (is_win31) {
		if (lpfnSndPlaySound != (FPSPS)NULL) 
   		    lpfnSndPlaySound(sound[num].file, SND_SYNC);
		else
		    MessageBeep(-1);
		return;
	}
}


/* display or remove 'wait' message */
void
info_wait(BOOL wait)
{
HWND hwnd;
POINT pt;
	waiting = wait;
	InvalidateRect(hwndimg, (LPRECT)&info_rect, FALSE);
	UpdateWindow(hwndimg);

	if (waiting) {
            GetCursorPos(&pt);
	    hwnd = WindowFromPoint(pt);
	    if ((hwnd == hwndimg) || IsChild(hwndimg,hwnd))
		SetCursor(hcWait);
	}
	else {
	    /* revert to generic text */
	    load_string(IDS_WAIT, szWait, sizeof(szWait));
	    /* set cursor to that of active window */
	    hwnd = GetFocus();
	    if ( (hwndimgchild && IsWindow(hwndimgchild))
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
_chdir(char *dirname)
{
	if (isalpha(dirname[0]) && (dirname[1]==':'))
		(void) setdisk(toupper(dirname[0])-'A');
	if (!((strlen(dirname)==2) && isalpha(dirname[0]) && (dirname[1]==':')))
		chdir(dirname);
	return 0;
}

char * 
_getcwd(char *dirname, int size)
{
	return getcwd(dirname, size);
}


void
send_prolog(FILE *f, int resource)
{  
HGLOBAL hglobal;
LPSTR prolog;
	hglobal = LoadResource(phInstance, 
	    FindResource(phInstance, MAKEINTRESOURCE(resource), RT_RCDATA));
	if ( (prolog = (LPSTR)LockResource(hglobal)) != (LPSTR)NULL) {
	    while (*prolog) {
		if (debug_file != (FILE *)NULL)
	            fputc(*prolog, debug_file);
	        fputc(*prolog++, f);
	    }
	    FreeResource(hglobal);
	}
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
