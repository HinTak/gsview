/*
 * dialog.c -- General dialog boxes for GSVIEW.EXE, 
 *              a graphical interface for MS-Windows Ghostscript
 * Copyright (C) 1993  Russell Lang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Author: Russell Lang
 * Internet: rjl@monu1.cc.monash.edu.au
 */

#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dir.h>
#include <io.h>
#define NeedFunctionPrototypes 1
#include "ps.h"
#include "gsview.h"


BOOL
getfilename(LPSTR filename, BOOL save, UINT filter, UINT title, UINT help)
{
LPSTR old_lpstrFile;
LPCSTR old_lpstrTitle;
char szTitle[MAXSTR];
BOOL flag;
	if (help)
	    LoadString(phInstance, help, szHelpTopic, sizeof(szHelpTopic));
	old_lpstrTitle = ofn.lpstrTitle;
	if (title) {
	    LoadString(phInstance, title, szTitle, sizeof(szTitle));
	    ofn.lpstrTitle = (LPCSTR)szTitle;
	}
	old_lpstrFile = ofn.lpstrFile;
	if (filename != (LPSTR)NULL)
		ofn.lpstrFile = filename;
	ofn.nFilterIndex = filter;
	if (save)
	    flag = GetSaveFileName(&ofn);
	else
	    flag = GetOpenFileName(&ofn);
	ofn.lpstrTitle = old_lpstrTitle;
	ofn.lpstrFile = old_lpstrFile;
	ofn.nFilterIndex = FILTER_PS;
	if ( save && flag && 
	        (dfname[0]!='\0') && (lstrcmp(filename, dfname) == 0) ) {
	    gserror(IDS_NOTDFNAME, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    flag = FALSE;
	}
	return flag;
}

/* Input Dialog Box structures */
LPSTR input_prop = "input_prop";
struct input_param {
	LPSTR prompt;
	LPSTR answer;
};

BOOL
get_string(char *prompt, char *answer)
{
struct input_param param;
DLGPROC lpProcInput;
BOOL flag;
	param.answer = answer;
	param.prompt = prompt;
	lpProcInput = (DLGPROC)MakeProcInstance((FARPROC)InputDlgProc, phInstance);
	flag = DialogBoxParam( phInstance, "InputDlgBox", hwndimg, lpProcInput, (LPARAM)&param);
	FreeProcInstance((FARPROC)lpProcInput);
	return flag;
}


/* input string dialog box */
BOOL CALLBACK _export
InputDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_INITDIALOG:
	    {
	      HLOCAL hlocal;
	      LPSTR *panswer;
	      struct input_param *pparam = (struct input_param *)lParam;
	      SetDlgItemText(hDlg, ID_PROMPT, pparam->prompt);
	      SetDlgItemText(hDlg, ID_ANSWER, pparam->answer);
	      /* save address of answer string in property list */
	      hlocal = LocalAlloc(LHND, sizeof(pparam->answer));
	      panswer = (LPSTR *)LocalLock(hlocal);
	      if (panswer != (LPSTR *)NULL) {
	        *panswer = pparam->answer;
		LocalUnlock(hlocal);
	        SetProp(hDlg, input_prop, (HANDLE)hlocal);
	      }
	    }
            return( TRUE);
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
		case ID_HELP:
		    SendMessage(hwndimg, help_message, 0, 0L);
		    return(FALSE);
                case ID_ANSWER:
                    return(TRUE);
		case IDOK:
		    {
		      HLOCAL hlocal = (HLOCAL)GetProp(hDlg, input_prop); 
		      LPSTR *panswer;
	              panswer = (LPSTR *)LocalLock(hlocal);
	              if (panswer != (LPSTR *)NULL) {
		        GetDlgItemText(hDlg, ID_ANSWER, *panswer, MAXSTR);
			LocalUnlock(hlocal);
		      }
		      LocalFree(hlocal);
		      RemoveProp(hDlg, input_prop);
		    }
                    EndDialog(hDlg, TRUE);
                    return(TRUE);
                case IDCANCEL:
		    {
		      HLOCAL hlocal = (HLOCAL)GetProp(hDlg, input_prop); 
		      LocalFree(hlocal);
		      RemoveProp(hDlg, input_prop);
		    }
                    EndDialog(hDlg, FALSE);
                    return(TRUE);
                default:
                    return(FALSE);
            }
        default:
            return(FALSE);
    }
}


/* copyright dialog box */
BOOL CALLBACK _export
AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_INITDIALOG:
            SetDlgItemText(hDlg, ABOUT_VERSION, GSVIEW_VERSION);
            return( TRUE);
	case WM_LBUTTONDBLCLK:
	    {DWORD dwUnit = GetDialogBaseUnits();
	    RECT rect; POINT pt;
	    pt.x = LOWORD(lParam); pt.y = HIWORD(lParam);
	    rect.left   =   8 * LOWORD(dwUnit) / 4;
	    rect.top    = 138 * HIWORD(dwUnit) / 8;
	    rect.right  = 240 * LOWORD(dwUnit) / 4 + rect.left;
	    rect.bottom =   8 * HIWORD(dwUnit) / 8 + rect.top;
	    if (PtInRect(&rect,pt)) {
		BITMAP bm;
		HBITMAP hbitmap_old;
		HBITMAP hbitmap = LoadBitmap(phInstance,"gsview_bitmap");
		HDC hdc = GetDC(hDlg);
		HDC hdcsrc = CreateCompatibleDC(hdc);
		hbitmap_old = SelectObject(hdcsrc,hbitmap);
		GetObject(hbitmap, sizeof(BITMAP),&bm);
		BitBlt(hdc, rect.right-bm.bmWidth,rect.bottom-bm.bmHeight,
		   bm.bmWidth,bm.bmHeight,hdcsrc,0,0,SRCCOPY);
		SelectObject(hdcsrc,hbitmap_old);
		DeleteObject(hbitmap);
		DeleteDC(hdcsrc);
		ReleaseDC(hDlg,hdc);
	    }
	    }
	    return FALSE;
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDOK:
                    EndDialog(hDlg, TRUE);
                    return(TRUE);
                default:
                    return(FALSE);
            }
        default:
            return(FALSE);
    }
}

/* information about document dialog box */
BOOL CALLBACK _export
InfoDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_INITDIALOG:
	  { char buf[MAXSTR];
	    int n;
	    if (dfname[0] != '\0') {
            	SetDlgItemText(hDlg, INFO_FILE, dfname);
		if (doc) {
		    if (is_ctrld)
			LoadString(phInstance, IDS_NOTDSC, buf, sizeof(buf));
		    else  {
			if (doc->epsf) {
			    switch (preview) {
				case IDS_EPSI:
				  LoadString(phInstance, IDS_EPSI, buf, sizeof(buf));
				  break;
				case IDS_EPST:
				  LoadString(phInstance, IDS_EPST, buf, sizeof(buf));
				  break;
				case IDS_EPSW:
				  LoadString(phInstance, IDS_EPSW, buf, sizeof(buf));
				  break;
				default:
				  LoadString(phInstance, IDS_EPSF, buf, sizeof(buf));
			    }
			}
			else
			    LoadString(phInstance, IDS_DSC, buf, sizeof(buf));
		    }
            	    SetDlgItemText(hDlg, INFO_TYPE, buf);
		    SetDlgItemText(hDlg, INFO_TITLE, doc->title ? doc->title : "");
		    SetDlgItemText(hDlg, INFO_DATE, doc->date ? doc->date : "");
		    sprintf(buf, "%d %d %d %d", doc->boundingbox[LLX], doc->boundingbox[LLY], 
			doc->boundingbox[URX], doc->boundingbox[URY]);
		    SetDlgItemText(hDlg, INFO_BBOX, buf);
		    switch(doc->orientation) {
			case LANDSCAPE:
	    			LoadString(phInstance, IDS_LANDSCAPE, buf, sizeof(buf));
				break;
			case PORTRAIT:
	    			LoadString(phInstance, IDS_PORTRAIT, buf, sizeof(buf));
				break;
			default:
				buf[0] = '\0';
		    }
		    SetDlgItemText(hDlg, INFO_ORIENT, buf);
		    switch(doc->pageorder) {
			case ASCEND: 
	    			LoadString(phInstance, IDS_ASCEND, buf, sizeof(buf));
				break;
			case DESCEND: 
	    			LoadString(phInstance, IDS_DESCEND, buf, sizeof(buf));
				break;
			case SPECIAL:
	    			LoadString(phInstance, IDS_SPECIAL, buf, sizeof(buf));
				break;
			default:
				buf[0] = '\0';
		    }
		    SetDlgItemText(hDlg, INFO_ORDER, buf);
		    if (doc->default_page_media && doc->default_page_media->name) {
		        sprintf(buf,"%s %d %d",doc->default_page_media->name,
		            doc->default_page_media->width,
		            doc->default_page_media->height);
		    }
		    else {
			buf[0] = '\0';
		    }
		    SetDlgItemText(hDlg, INFO_DEFMEDIA, buf);
		    sprintf(buf, "%d", doc->numpages);
		    SetDlgItemText(hDlg, INFO_NUMPAGES, buf);
		    n = map_page(pagenum - 1);
		    if (doc->pages)
			    sprintf(buf, "\"%s\"   %d", doc->pages[n].label ? doc->pages[n].label : "", pagenum);
		    else
			    buf[0] = '\0';
		    SetDlgItemText(hDlg, INFO_PAGE, buf);
		
		}
		else {
		    LoadString(phInstance, IDS_NOTDSC, buf, sizeof(buf));
            	    SetDlgItemText(hDlg, INFO_TYPE, buf);
		}
		sprintf(buf, "%d \327 %d", bitmap_width, bitmap_height);
		SetDlgItemText(hDlg, INFO_BITMAP, buf);
	    }
	    else {
	    	LoadString(phInstance, IDS_NOFILE, buf, sizeof(buf));
            	SetDlgItemText(hDlg, INFO_FILE, buf);
	   }
	  }
          return( TRUE);
	case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDOK:
                    EndDialog(hDlg, TRUE);
                    return(TRUE);
                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE);
                default:
                    return(FALSE);
            }
        default:
            return(FALSE);
    }
}


#define MAX_SYSTEM_SOUND 16
char *system_sounds;
char *sound_entry[MAX_SYSTEM_SOUND];
char szNone[32];
char szSpeaker[32];
int system_num;

int
load_sounds(void)
{
	char *p;
	int j;

	/* add our two sounds */
	sound_entry[0] = "";
	sound_entry[1] = BEEP;
	LoadString(phInstance, IDS_NONE, szNone, sizeof(szNone));
	LoadString(phInstance, IDS_SPKR, szSpeaker, sizeof(szSpeaker));
	/* get list of system sounds */
	system_sounds = malloc(PROFILE_SIZE);
	if (system_sounds != (char *)NULL) {
	    GetProfileString("sounds", NULL, "", system_sounds, PROFILE_SIZE);
	}
	p = system_sounds;
	for (j=2; p!=(char *)NULL && j<MAX_SYSTEM_SOUND && strlen(p)!=0; j++) {
	    sound_entry[j] = p;	
	    p += strlen(p) + 1;
	}
	system_num = j;
	return system_num;
}

char *
get_sound_entry(int index)
{
	return (sound_entry[index]);
}

char *
get_sound_name(int index)
{
static char buf[64];
char *p;
	if (index==0)
		return szNone;
	if (index==1)
		return szSpeaker;
        GetProfileString("sounds", sound_entry[index], ",", buf, sizeof(buf));
    	p = strchr(buf,',');
    	if (p != (char *)NULL)
		return p+1;
	return (char *)NULL;
}

int 
find_sound_name(char *entry)
{
int i;
	for (i=0; i<system_num; i++) {
	    if (strcmp(entry, sound_entry[i])==0)
	        return i;
	}
	return -1;	/* no find */
}

void
add_sounds(HWND hDlg)
{
int ifile;
	for (ifile=system_num-1; ifile>=0; ifile--)
	    SendDlgItemMessage(hDlg, SOUND_FILE, LB_INSERTSTRING, 0,
	         (LPARAM)(LPSTR)get_sound_name(ifile));
}

void
free_sounds(void)
{
	if (system_sounds != (char *)NULL)
		free(system_sounds);
}

BOOL CALLBACK _export
SoundDlgProc(HWND hDlg, UINT wmsg, WPARAM wParam, LPARAM lParam)
{
	char buf[MAXSTR];
	int ievent, ifile;
	static struct sound_s dsound[NUMSOUND];	/* copy of sound[] */
	WORD notify_message;
	char *szWaveFilter = "*.wav";
	static char szPath[MAXSTR];
	static int file_start;
	char *p;

	switch (wmsg) {
	    case WM_INITDIALOG:
		file_start = load_sounds();
		for (ievent=0; ievent<NUMSOUND; ievent++) {
		    strcpy(dsound[ievent].file, sound[ievent].file);
		    LoadString(phInstance, sound[ievent].title, buf, sizeof(buf));
		    SendDlgItemMessage(hDlg, SOUND_EVENT, LB_ADDSTRING, 0, 
			(LPARAM)((LPSTR)buf));
		}
		ievent = 0;
		SendDlgItemMessage(hDlg, SOUND_EVENT, LB_SETCURSEL, ievent, 0L);
		/* force update of SOUND_FILE */
		SendDlgNotification(hDlg, SOUND_EVENT, LBN_SELCHANGE);
		return TRUE;
	    case WM_COMMAND:
		notify_message = GetNotification(wParam,lParam);
		switch (LOWORD(wParam)) {
		    case ID_HELP:
		        SendMessage(hwndimg, help_message, 0, 0L);
		        return(FALSE);
		    case SOUND_EVENT:
			if (notify_message != LBN_SELCHANGE) {
				return FALSE;
			}
			ievent = (int)SendDlgItemMessage(hDlg, SOUND_EVENT, LB_GETCURSEL, 0, 0L);
			if (ievent == LB_ERR) {
			    EnableWindow(GetDlgItem(hDlg, SOUND_TEST), FALSE);
			    return FALSE;
			}
			ifile = find_sound_name(dsound[ievent].file);
			if (ifile >= 0) {
			    strcpy(buf, get_sound_name(ifile));
			    szPath[0] = '\0';
			    EnableWindow(GetDlgItem(hDlg, SOUND_TEST), ifile!=0);
			}
			else {
			    /* must be WAVE file */
			    strcpy(szPath, dsound[ievent].file);
			    p = strrchr(szPath, '\\');
			    if (p != (char *)NULL) {
			        strcpy(buf,++p);
			        *p = '\0';
			    }
			    else {
			        strcpy(buf, szPath);
			    }
			    EnableWindow(GetDlgItem(hDlg, SOUND_TEST), TRUE);
			}
			strcat(szPath, szWaveFilter);
			DlgDirList(hDlg, szPath, SOUND_FILE, SOUND_PATH, DDL_DRIVES | DDL_DIRECTORY);
			add_sounds(hDlg);
			SendDlgItemMessage(hDlg, SOUND_FILE, LB_SELECTSTRING, file_start, (LPARAM)(LPSTR)buf);
			return FALSE;
		    case SOUND_FILE:
			if (notify_message == LBN_SELCHANGE) {
			    ifile = (int)SendDlgItemMessage(hDlg, SOUND_FILE, LB_GETCURSEL, 0, 0L);
			    SendDlgItemMessage(hDlg, SOUND_FILE, LB_GETTEXT, ifile, (LPARAM)(LPSTR)buf);
			    ievent = (int)SendDlgItemMessage(hDlg, SOUND_EVENT, LB_GETCURSEL, 0, 0L);
			    if (ifile >= file_start) {
				if (buf[0] == '[') { /* selected a directory */
				    EnableWindow(GetDlgItem(hDlg, SOUND_TEST), FALSE);
			        }
				else { /* selected a WAVE file */
		                    int i = GetDlgItemText(hDlg, SOUND_PATH, dsound[ievent].file, MAXSTR);
			            if (dsound[ievent].file[i-1] != '\\')
			                dsound[ievent].file[i++] = '\\';
		                    DlgDirSelect(hDlg, dsound[ievent].file + i, SOUND_FILE);
				    EnableWindow(GetDlgItem(hDlg, SOUND_TEST), TRUE);
				}
			    }
			    else {
				EnableWindow(GetDlgItem(hDlg, SOUND_TEST), ifile!=0);
				strcpy(dsound[ievent].file,get_sound_entry(ifile));
			    }
			}
			if (notify_message == LBN_DBLCLK) {
			    ifile = (int)SendDlgItemMessage(hDlg, SOUND_FILE, LB_GETCURSEL, 0, 0L);
			    SendDlgItemMessage(hDlg, SOUND_FILE, LB_GETTEXT, ifile, (LPARAM)(LPSTR)buf);
			    if (buf[0] == '[') {
		                DlgDirSelect(hDlg, szPath, SOUND_FILE);
			        lstrcat(szPath, szWaveFilter);
		                DlgDirList(hDlg, szPath, SOUND_FILE, SOUND_PATH, DDL_DRIVES | DDL_DIRECTORY);
				add_sounds(hDlg);
			    }
			    else {
				SendDlgNotification(hDlg, SOUND_TEST, BN_CLICKED);
			    }
			}
			return FALSE;
		    case SOUND_TEST:
			ievent = (int)SendDlgItemMessage(hDlg, SOUND_EVENT, LB_GETCURSEL, 0, 0L);
			if (strlen(dsound[ievent].file)==0)
				return FALSE;
			if (!is_win31 || (strcmp(dsound[ievent].file,BEEP)==0)) {
				MessageBeep(-1);
				return FALSE;
			}
			if (is_win31) {
				if (lpfnSndPlaySound != (FPSPS)NULL) 
	    			    lpfnSndPlaySound(dsound[ievent].file, SND_SYNC);
				else
				    MessageBeep(-1);
				return FALSE;
			}
			return FALSE;
		    case IDOK:
			for (ievent=0; ievent<NUMSOUND; ievent++)
				strcpy(sound[ievent].file, dsound[ievent].file);
			free_sounds();
			EndDialog(hDlg, TRUE);
			return TRUE;
		    case IDCANCEL:
			free_sounds();
			EndDialog(hDlg, FALSE);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/* Get page number from dialog box and store in ppage */
BOOL
get_page(int *ppage, BOOL multiple)
{
DLGPROC lpProcPage;
BOOL flag;
	if (doc == (struct document *)NULL)
		return FALSE;
	if (doc->numpages == 0) {
		gserror(IDS_NOPAGE, NULL, MB_ICONEXCLAMATION, SOUND_NONUMBER);
		return FALSE;
	}
	page_list.current = *ppage - 1;
	page_list.multiple = multiple;
	if (page_list.select == (BOOL *)NULL)
		return FALSE;
	memset(page_list.select, 0, doc->numpages * sizeof(BOOL) );
	lpProcPage = (DLGPROC)MakeProcInstance((FARPROC)PageDlgProc, phInstance);
	flag = DialogBoxParam( phInstance, "PageDlgBox", hwndimg, lpProcPage, (LPARAM)NULL);
	FreeProcInstance((FARPROC)lpProcPage);
	if (flag && (page_list.current >= 0))
		*ppage = page_list.current + 1;
	return flag;
}

BOOL CALLBACK _export
PageDlgProc(HWND hDlg, UINT wmsg, WPARAM wParam, LPARAM lParam)
{
	char buf[40];
	int i;
	WORD notify_message;
	switch (wmsg) {
	    case WM_INITDIALOG:
		if (page_list.multiple)
		    LoadString(phInstance, IDS_SELECTPAGES, buf, sizeof(buf));
		else
		    LoadString(phInstance, IDS_SELECTPAGE, buf, sizeof(buf));
		SetWindowText(hDlg, buf);
		for (i=0; i<doc->numpages; i++) {
		    SendDlgItemMessage(hDlg, PAGE_LIST, LB_ADDSTRING, 0, 
			(LPARAM)((LPSTR)doc->pages[map_page(i)].label));
		}
		SendDlgItemMessage(hDlg, PAGE_LIST, LB_SETSEL, TRUE, MAKELPARAM(page_list.current,0));
		SendDlgItemMessage(hDlg, PAGE_LIST, LB_SETCURSEL, page_list.current, 0L);
		if (!page_list.multiple) {
			EnableWindow(GetDlgItem(hDlg, PAGE_ALL), FALSE);
			EnableWindow(GetDlgItem(hDlg, PAGE_ODD), FALSE);
			EnableWindow(GetDlgItem(hDlg, PAGE_EVEN), FALSE);
		}
		return TRUE;
	    case WM_COMMAND:
		notify_message = GetNotification(wParam,lParam);
		switch (LOWORD(wParam)) {
		    case PAGE_LIST:
			if (notify_message == LBN_DBLCLK)
				PostMessage(hDlg, WM_COMMAND, IDOK, 0L);
			return FALSE;
		    case PAGE_ALL:
			SendDlgItemMessage(hDlg, PAGE_LIST, LB_SELITEMRANGE, TRUE, 
				MAKELPARAM(0,doc->numpages-1));
			return FALSE;
		    case PAGE_ODD:
			for (i=(int)SendDlgItemMessage(hDlg, PAGE_LIST, LB_GETCOUNT, 0, 0L)-1; i>=0; i--)
			    SendDlgItemMessage(hDlg, PAGE_LIST, LB_SETSEL, !(i&1), MAKELPARAM(i,0));
			return FALSE;
		    case PAGE_EVEN:
			for (i=(int)SendDlgItemMessage(hDlg, PAGE_LIST, LB_GETCOUNT, 0, 0L); i>=0; i--)
			    SendDlgItemMessage(hDlg, PAGE_LIST, LB_SETSEL, (i&1), MAKELPARAM(i,0));
			SendDlgItemMessage(hDlg, PAGE_LIST, LB_SETTOPINDEX, 0, 0L);
			return FALSE;
		    case IDOK:
			i = (int)SendDlgItemMessage(hDlg, PAGE_LIST, LB_GETCURSEL, 0, 0L);
			page_list.current = (i == LB_ERR) ? -1 : i;
			for (i=0; i<doc->numpages; i++) {
			  page_list.select[i] =
			    (int)SendDlgItemMessage(hDlg, PAGE_LIST, LB_GETSEL, i, 0L);
			}
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



/* get user defined size */
BOOL
gsview_usersize()
{
char prompt[MAXSTR];
char answer[MAXSTR];
	LoadString(phInstance, IDS_TOPICMEDIA, szHelpTopic, sizeof(szHelpTopic));
	LoadString(phInstance, IDS_USERWIDTH, prompt, sizeof(prompt));
	sprintf(answer,"%d", user_width);
	if (!get_string(prompt,answer) || atoi(answer)==0)
		return FALSE;
	user_width = atoi(answer);
	LoadString(phInstance, IDS_USERHEIGHT, prompt, sizeof(prompt));
	sprintf(answer,"%d", user_height);
	if (!get_string(prompt,answer) || atoi(answer)==0)
		return FALSE;
	user_height = atoi(answer);
	return TRUE;
}

