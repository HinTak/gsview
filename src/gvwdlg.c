/* Copyright (C) 1993-1998, Russell Lang.  All rights reserved.
  
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

/* gvwdlg.c */
/* Dialog boxes for Windows GSview */
#include "gvwin.h"

BOOL
get_filename(char *filename, BOOL save, int filter, int title, int help)
{
LPSTR old_lpstrFile;
LPCSTR old_lpstrTitle;
char szTitle[MAXSTR];
BOOL flag;
LPCSTR old_lpstrFilter;
char szFilter[256];		/* filter for OFN */
int i;
char cReplace;

	if (help)
	    load_string(help, szHelpTopic, sizeof(szHelpTopic));
	old_lpstrTitle = ofn.lpstrTitle;
	if (title) {
	    load_string(title, szTitle, sizeof(szTitle));
	    ofn.lpstrTitle = (LPCSTR)szTitle;
	}
	old_lpstrFile = ofn.lpstrFile;
	if (filename != (LPSTR)NULL)
		ofn.lpstrFile = filename;
	/* Get filter types */
	old_lpstrFilter = ofn.lpstrFilter;
	ofn.nFilterIndex = 0;
	if (load_string(IDS_FILTER_BASE+filter, szFilter, sizeof(szFilter)-1)) {
	    cReplace = szFilter[strlen(szFilter)-1];
	    for (i=0; szFilter[i] != '\0'; i++)
	        if (szFilter[i] == cReplace)
		    szFilter[i] = '\0';
	    ofn.lpstrFilter = szFilter;
	    ofn.nFilterIndex = 0;
	}
	/* call the common dialog box */
	if (save)
	    flag = GetSaveFileName(&ofn);
	else
	    flag = GetOpenFileName(&ofn);
	ofn.lpstrTitle = old_lpstrTitle;
	ofn.lpstrFile = old_lpstrFile;
	ofn.lpstrFilter = old_lpstrFilter;
	ofn.nFilterIndex = 0;
	if ( save && flag && 
	        (psfile.name[0]!='\0') && (lstrcmp(filename, psfile.name) == 0) ) {
	    gserror(IDS_NOTDFNAME, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    flag = FALSE;
	}
#if defined(__WIN32__)
	/* GetOpenFileName() should change current directory */
	/* Deal with broken Win32 that doesn't do this */
	if (flag) {
	    char *p;
	    char temp[MAXSTR];
	    strcpy(temp, filename);
	    p = strrchr(temp, '\\');
	    if (p == NULL) {
		if (strlen(temp) > 2) {
		    if (isalpha(temp[0]) && (temp[1]==':')) {
			temp[2] = '\0';
			gs_chdir(temp);
		    }
		}
	    }
	    else {
		*p = '\0';
		gs_chdir(temp);
	    }
	}
#endif
	return flag;
}

/* Input Dialog Box structures */
LPSTR input_prop = "input_prop";
struct input_param {
	LPSTR prompt;
	LPSTR answer;
};


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
		    get_help();
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

BOOL
get_string(char *prompt, char *answer)
{
struct input_param param;
BOOL flag;
#ifndef __WIN32__
DLGPROC lpProcInput;
#endif
	param.answer = answer;
	param.prompt = prompt;
#ifdef __WIN32__
	flag = DialogBoxParam(hlanguage, "InputDlgBox", hwndimg, InputDlgProc, (LPARAM)&param);
#else
	lpProcInput = (DLGPROC)MakeProcInstance((FARPROC)InputDlgProc, phInstance);
	flag = DialogBoxParam(hlanguage, "InputDlgBox", hwndimg, lpProcInput, (LPARAM)&param);
	FreeProcInstance((FARPROC)lpProcInput);
#endif
	return flag;
}


void
wwait(void)
{
MSG msg;
DWORD start = GetTickCount();
DWORD end = start + 70;
	while ( (GetTickCount() <= end) && (GetTickCount() >= start) ) {
		while (PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE)) {
			 TranslateMessage(&msg);
			 DispatchMessage(&msg);
		}
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
	  case WM_LBUTTONDOWN:
	    {
	    HWND hwicon = GetDlgItem(hDlg, ABOUT_ICON);
	    HICON hicon1 = LoadIcon(phInstance, MAKEINTRESOURCE(ID_GSVIEW));
	    HICON hicon2 = LoadIcon(phInstance, MAKEINTRESOURCE(ID_GSVIEW2));
	    HICON hicon3 = LoadIcon(phInstance, MAKEINTRESOURCE(ID_GSVIEW3));
	    HDC hdc = GetDC(hwicon);
	    RECT rect; POINT pt;
		pt.x = LOWORD(lParam); pt.y = HIWORD(lParam);
		ClientToScreen(hDlg, &pt);
		GetWindowRect(hwicon, &rect);
		if (PtInRect(&rect,pt)) {
			DrawIcon(hdc, 0, 0, hicon2);
			wwait();
			DrawIcon(hdc, 0, 0, hicon3);
			wwait();
			wwait();
			DrawIcon(hdc, 0, 0, hicon2);
			wwait();
			DrawIcon(hdc, 0, 0, hicon1);
		}
		DestroyIcon(hicon1);
		DestroyIcon(hicon2);
		DestroyIcon(hicon3);
		ReleaseDC(hwicon, hdc);
	    }
	    return FALSE;
	case WM_LBUTTONDBLCLK:
	    {DWORD dwUnit = GetDialogBaseUnits();
	    RECT rect; POINT pt;
	    pt.x = LOWORD(lParam); pt.y = HIWORD(lParam);
		/* this is for 8pt dialog fonts */
		rect.left   =   8 * LOWORD(dwUnit) / 5;
		rect.top    = 166 * HIWORD(dwUnit) / 10;
		rect.right  = 240 * LOWORD(dwUnit) / 5 + rect.left;
		rect.bottom =  10 * HIWORD(dwUnit) / 10 + rect.top;
#ifdef NOTUSED
		/* this is for 10pt dialog fonts */
		rect.left   =   8 * LOWORD(dwUnit) / 4;
		rect.top    = 166 * HIWORD(dwUnit) / 8;
		rect.right  = 240 * LOWORD(dwUnit) / 4 + rect.left;
		rect.bottom =   8 * HIWORD(dwUnit) / 8 + rect.top;
#endif

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
		case IDCANCEL:
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

void
show_about()
{
#ifdef __WIN32__
	DialogBoxParam(hlanguage, "AboutDlgBox", hwndimg, AboutDlgProc, (LPARAM)NULL);
#else
	DLGPROC lpProcAbout;
	lpProcAbout = (DLGPROC)MakeProcInstance((FARPROC)AboutDlgProc, phInstance);
	DialogBoxParam(hlanguage, "AboutDlgBox", hwndimg, lpProcAbout, (LPARAM)NULL);
	FreeProcInstance((FARPROC)lpProcAbout);
#endif
}

#ifdef __BORLANDC__
#pragma argsused
#endif
/* information about document dialog box */
BOOL CALLBACK _export
InfoDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_INITDIALOG:
	    info_init(hDlg);
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

/* show info about ps file */
void
show_info()
{
#ifdef __WIN32__
	DialogBoxParam(hlanguage, "InfoDlgBox", hwndimg, InfoDlgProc, (LPARAM)NULL);
#else
	DLGPROC lpProcInfo;
	lpProcInfo = (DLGPROC)MakeProcInstance((FARPROC)InfoDlgProc, phInstance);
	DialogBoxParam(hlanguage, "InfoDlgBox", hwndimg, lpProcInfo, (LPARAM)NULL);
	FreeProcInstance((FARPROC)lpProcInfo);
#endif
}



#define MAX_SYSTEM_SOUND 16
char *system_sounds;
char *sound_entry[MAX_SYSTEM_SOUND];
char szNone[32];
char szSpeaker[32];
int system_num;
BOOL CALLBACK _export SoundDlgProc(HWND hDlg, UINT wmsg, WPARAM wParam, LPARAM lParam);

int
load_sounds(void)
{
	char *p;
	int j;

	/* add our two sounds */
	sound_entry[0] = "";
	sound_entry[1] = BEEP;
	load_string(IDS_NONE, szNone, sizeof(szNone));
	load_string(IDS_SPKR, szSpeaker, sizeof(szSpeaker));
	/* get list of system sounds */
	system_sounds = malloc(PROFILE_SIZE);
	if (system_sounds != (char *)NULL) {
	    GetProfileString("sounds", NULL, "", system_sounds, PROFILE_SIZE);
	}
	p = system_sounds;
	for (j=2; p!=(char *)NULL && j<MAX_SYSTEM_SOUND && strlen(p)!=0; j++) {
	    /* Windows NT uses "Enable=1" in the sounds section */
	    /* We need to prevent this from appearing in the list of sounds */
	    if (strcmp(p, "Enable") == 0)
		j--;
	    else
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
char *p;
	for (ifile=system_num-1; ifile>=0; ifile--) {
	    p = get_sound_name(ifile);
	    if (p != (char *)NULL)
	        SendDlgItemMessage(hDlg, SOUND_FILE, LB_INSERTSTRING, 0,
	            (LPARAM)(LPSTR)p);
	}
}

void
free_sounds(void)
{
	if (system_sounds != (char *)NULL)
		free(system_sounds);
}

void
change_sounds(void)
{
#ifdef __WIN32__
	load_string(IDS_TOPICSOUND, szHelpTopic, sizeof(szHelpTopic));
	DialogBoxParam(hlanguage, "SoundDlgBox", hwndimg, SoundDlgProc, (LPARAM)NULL);
#else
	DLGPROC lpProcSound;
	load_string(IDS_TOPICSOUND, szHelpTopic, sizeof(szHelpTopic));
	lpProcSound = (DLGPROC)MakeProcInstance((FARPROC)SoundDlgProc, phInstance);
	DialogBoxParam(hlanguage, "SoundDlgBox", hwndimg, lpProcSound, (LPARAM)NULL);
	FreeProcInstance((FARPROC)lpProcSound);
#endif
}

#ifdef __BORLANDC__
#pragma argsused
#endif
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
		    load_string(sound[ievent].title, buf, sizeof(buf));
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
			get_help();
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
		                    DlgDirSelectEx(hDlg, dsound[ievent].file + i, sizeof(dsound[ievent].file), SOUND_FILE);
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
		                DlgDirSelectEx(hDlg, szPath, sizeof(szPath), SOUND_FILE);
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
			if (strcmp(dsound[ievent].file,BEEP)==0) {
				MessageBeep(-1);
				return FALSE;
			}
			if (lpfnSndPlaySound != (FPSPS)NULL) 
			    lpfnSndPlaySound(dsound[ievent].file, SND_SYNC);
			else
			    MessageBeep(-1);
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

/* Return TRUE if a contiguous block of pages (>2) is found */
/* Store start and end of this range in first and last */
/* If not contiguous, store entire page range in first and last */
BOOL
contiguous_range(HWND hDlg, int *first, int *last)
{
int i;
BOOL gap = FALSE;	/* TRUE is gap found after block */
BOOL contiguous = TRUE;
int block = 0;		/* number of pages set in a contiguous block */
BOOL selected;
    for (i=0; i<psfile.doc->numpages; i++) {
	selected = (int)SendDlgItemMessage(hDlg, PAGE_LIST, LB_GETSEL, i, 0L);
	if (selected && contiguous) {
	    if (gap)
		contiguous = FALSE;
	    else {
		if (block == 0)
		    *first = i;
		else
		    *last = i+1;
		block++;
	    }
	}
	else if (!selected && block)
	    gap = TRUE;
    }
    if (block < 2)
	contiguous = FALSE;
    if (!contiguous) {
	*first = 0;
	*last = psfile.doc->numpages;
    }
    return contiguous;
}


#ifdef __BORLANDC__
#pragma argsused
#endif
BOOL CALLBACK _export
PageDlgProc(HWND hDlg, UINT wmsg, WPARAM wParam, LPARAM lParam)
{
	int i;
	WORD notify_message;
	static BOOL ecdisable;
	switch (wmsg) {
	    case WM_INITDIALOG:
		{char buf[MAXSTR];
		for (i=0; i<psfile.doc->numpages; i++) {
		    SendDlgItemMessage(hDlg, PAGE_LIST, LB_ADDSTRING, 0, 
			(LPARAM)((LPSTR)psfile.doc->pages[map_page(i)].label));
		}
		SendDlgItemMessage(hDlg, PAGE_LIST, LB_SETCURSEL, 
		    psfile.page_list.current, 0L);
		SendDlgItemMessage(hDlg, PAGE_LIST, LB_GETTEXT, 
		    psfile.page_list.current, (LPARAM)buf);
		ecdisable = TRUE;
		SetDlgItemText(hDlg, PAGE_EDIT, buf);
		ecdisable = FALSE;
		}
		return TRUE;
	    case WM_COMMAND:
		notify_message = GetNotification(wParam,lParam);
		switch (LOWORD(wParam)) {
		    case PAGE_EDIT:
			if (!ecdisable && (notify_message == EN_CHANGE)) {
			    char buf[MAXSTR];
			    GetDlgItemText(hDlg, PAGE_EDIT, buf, sizeof(buf));
			    if ((i = (int)SendDlgItemMessage(hDlg, PAGE_LIST, 
				    LB_FINDSTRINGEXACT, -1, (LPARAM)buf))
				!= LB_ERR) {
				SendDlgItemMessage(hDlg, PAGE_LIST, 
				    LB_SETCURSEL, i, 0L);
			    }
			}
			return TRUE;
		    case PAGE_LIST:
			if (notify_message == LBN_DBLCLK)
				PostMessage(hDlg, WM_COMMAND, IDOK, 0L);
		        else if (notify_message == LBN_SELCHANGE) {
			    char buf[MAXSTR];
			    i = (int)SendDlgItemMessage(hDlg, PAGE_LIST, 
				LB_GETCURSEL, 0, 0L);
			    if (i != LB_ERR) {
				SendDlgItemMessage(hDlg, PAGE_LIST, LB_GETTEXT, 
				    i, (LPARAM)buf);
				/* Update edit field, but stop edit field from
			         * from altering list box selection */
				ecdisable = TRUE;
				SetDlgItemText(hDlg, PAGE_EDIT, buf);
				ecdisable = FALSE;
			    }
			}
			return FALSE;
		    case IDOK:
			i = (int)SendDlgItemMessage(hDlg, PAGE_LIST, 
				LB_GETCURSEL, 0, 0L);
			if (i == LB_ERR)
			    EndDialog(hDlg, FALSE);
			else {
			    psfile.page_list.current = i;
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


#ifdef __BORLANDC__
#pragma argsused
#endif
BOOL CALLBACK _export
PageMultiDlgProc(HWND hDlg, UINT wmsg, WPARAM wParam, LPARAM lParam)
{
	int i;
	WORD notify_message;
	switch (wmsg) {
	    case WM_INITDIALOG:
		if (psfile.page_list.reverse)
		    SendDlgItemMessage(hDlg, PAGE_REVERSE, BM_SETCHECK, 1, 0);
		for (i=0; i<psfile.doc->numpages; i++) {
		    SendDlgItemMessage(hDlg, PAGE_LIST, LB_ADDSTRING, 0, 
			(LPARAM)((LPSTR)psfile.doc->pages[map_page(i)].label));
		}
		if (psfile.page_list.multiple) {
		    /* multiple selection list box */
		    for (i=0; i<psfile.doc->numpages; i++)
			if (psfile.page_list.select[i]) 
			    SendDlgItemMessage(hDlg, PAGE_LIST, LB_SETSEL, TRUE, MAKELPARAM(i,0));
		    SendDlgItemMessage(hDlg, PAGE_LIST, LB_SETSEL, TRUE, MAKELPARAM(psfile.page_list.current, 0));
		}
		else {
		    /* single selection list box */
		    SendDlgItemMessage(hDlg, PAGE_LIST, LB_SETSEL, TRUE, MAKELPARAM(psfile.page_list.current, 0));
		    SendDlgItemMessage(hDlg, PAGE_LIST, LB_SETCURSEL, psfile.page_list.current, 0L);
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
				MAKELPARAM(0,psfile.doc->numpages-1));
			return FALSE;
		    case PAGE_ODD:
			{
			int first, last;
			contiguous_range(hDlg, &first, &last);
			for (i=(int)SendDlgItemMessage(hDlg, PAGE_LIST, 
				LB_GETCOUNT, 0, 0L)-1; i>=0; i--)
			    if (i >= first && i < last)
			        SendDlgItemMessage(hDlg, PAGE_LIST, LB_SETSEL, 
				    !(i&1), MAKELPARAM(i,0));
			SendDlgItemMessage(hDlg, PAGE_LIST, LB_SETTOPINDEX, 
			    first, 0L);
			}
			return FALSE;
		    case PAGE_EVEN:
			{
			int first, last;
			contiguous_range(hDlg, &first, &last);
			for (i=(int)SendDlgItemMessage(hDlg, PAGE_LIST, 
				LB_GETCOUNT, 0, 0L)-1; i>=0; i--)
			    if (i >= first && i < last)
			        SendDlgItemMessage(hDlg, PAGE_LIST, LB_SETSEL, 
				    (i&1), MAKELPARAM(i,0));
			SendDlgItemMessage(hDlg, PAGE_LIST, LB_SETTOPINDEX, 
			    first, 0L);
			}
			return FALSE;
		    case IDOK:
			psfile.page_list.reverse = 
			    (int)SendDlgItemMessage(hDlg, PAGE_REVERSE, 
				BM_GETCHECK, 0, 0);
			i = (int)SendDlgItemMessage(hDlg, PAGE_LIST, LB_GETCURSEL, 0, 0L);
			psfile.page_list.current = (i == LB_ERR) ? -1 : i;
			for (i=0; i<psfile.doc->numpages; i++) {
			  psfile.page_list.select[i] =
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

/* Get page number from dialog box and store in ppage */
/* multiple is TRUE if multiple pages may be selected */
/* allpages is TRUE if all pages should be initially selected */
BOOL
get_page(int *ppage, BOOL multiple, BOOL allpages)
{
#ifndef __WIN32__
DLGPROC lpProcPage;
#endif
DLGPROC lpProc;
BOOL flag;
LPSTR dlgname;
int i;
	if (psfile.doc == (PSDOC *)NULL)
		return FALSE;
	if (psfile.doc->numpages == 0) {
		gserror(IDS_NOPAGE, NULL, MB_ICONEXCLAMATION, SOUND_NONUMBER);
		return FALSE;
	}
	// Make psfile.page_list.reverse sticky
        // psfile.page_list.reverse = FALSE;
	psfile.page_list.current = *ppage - 1;
	psfile.page_list.multiple = multiple;
	if (psfile.page_list.select == (BOOL *)NULL)
		return FALSE;

	memset(psfile.page_list.select, 0, psfile.doc->numpages * sizeof(BOOL) );
	if (multiple) {
	    for (i=0; i< psfile.doc->numpages; i++)
		psfile.page_list.select[i] = allpages;
	}
	psfile.page_list.select[psfile.page_list.current] = TRUE;

	if (psfile.page_list.multiple) {
	    dlgname = "PageMultiDlgBox";
	    lpProc = (DLGPROC)PageMultiDlgProc;
	}
	else {
	    dlgname = "PageDlgBox";
	    lpProc = (DLGPROC)PageDlgProc;
	}
#ifdef __WIN32__
	flag = DialogBoxParam(hlanguage, dlgname, hwndimg, lpProc, (LPARAM)NULL);
#else
	lpProcPage = (DLGPROC)MakeProcInstance((FARPROC)lpProc, phInstance);
	flag = DialogBoxParam(hlanguage, dlgname, hwndimg, lpProcPage, (LPARAM)NULL);
	FreeProcInstance((FARPROC)lpProcPage);
#endif
	if (flag && (psfile.page_list.current >= 0))
		*ppage = psfile.page_list.current + 1;
	return flag;
}


#ifdef __BORLANDC__
#pragma argsused
#endif
BOOL CALLBACK _export
BoundingBoxDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
static int bboxindex;
float x, y;
char buf[MAXSTR];
    switch(message) {
	case WM_INITDIALOG:
	    bboxindex = 0;
	    load_string(IDS_BBPROMPT, buf, sizeof(buf));
	    SetDlgItemText(hDlg, BB_PROMPT, buf);
	    return TRUE;
	case WM_COMMAND:
	    switch(LOWORD(wParam)) {
		case BB_CLICK:
		    if (!get_cursorpos(&x, &y)) {
			DestroyWindow(hDlg);
			hDlgModeless = 0;
		    }
		    switch(bboxindex) {
			case 0:
			    bbox.llx = x;
			    break;
			case 1:
			    bbox.lly = y;
			    break;
			case 2:
			    bbox.urx = x;
			    break;
			case 3:
			    bbox.ury = y;
			    bbox.valid = TRUE;
			    break;
		    }
		    bboxindex++;
		    if (bboxindex <= 3) {
	    	        load_string(IDS_BBPROMPT+bboxindex, buf, sizeof(buf));
	    	        SetDlgItemText(hDlg, BB_PROMPT, buf);
			return FALSE;
		    }
		case IDCANCEL:
		    DestroyWindow(hDlg);
		    hDlgModeless = 0;
		    return TRUE;
	    }
	case WM_CLOSE:
	    DestroyWindow(hDlg);
	    hDlgModeless = 0;
	    return TRUE;
    }
    return FALSE;
}

BOOL
get_bbox(void)
{
#ifndef __WIN32__
DLGPROC lpfnBoundingBoxProc;
#endif
	bbox.valid = FALSE;
	bbox.llx = bbox.lly = bbox.urx = bbox.ury = 0;
	if ((gsdll.state != PAGE) && (gsdll.state != IDLE)) {
	    gserror(IDS_EPSNOBBOX, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    return FALSE;
	}
#ifdef __WIN32__
	hDlgModeless = CreateDialogParam(hlanguage, "BoundingBoxDlgBox", hwndimg, BoundingBoxDlgProc, (LPARAM)NULL);
#else
	lpfnBoundingBoxProc = (DLGPROC)MakeProcInstance((FARPROC)BoundingBoxDlgProc, phInstance);
	hDlgModeless = CreateDialogParam(hlanguage, "BoundingBoxDlgBox", hwndimg, lpfnBoundingBoxProc, (LPARAM)NULL);
#endif
	while (hDlgModeless) {
	    do_message();	/* wait for bounding box to be obtained */
	}
#ifndef __WIN32__
	FreeProcInstance((FARPROC)lpfnBoundingBoxProc);
#endif
	return bbox.valid;
}

/* dialog box for warning PSTOEPS warning and auto/manual bbox selection */
#ifdef __BORLANDC__
#pragma argsused
#endif
BOOL CALLBACK _export
PSTOEPSDlgProc(HWND hDlg, UINT wmsg, WPARAM wParam, LPARAM lParam)
{
	switch (wmsg) {
	    case WM_INITDIALOG:
		if (option.auto_bbox)
		    SendDlgItemMessage(hDlg, PSTOEPS_AUTOBBOX, BM_SETCHECK, 1, 0);
		return TRUE;
	    case WM_COMMAND:
		switch (LOWORD(wParam)) {
		    case ID_HELP:
			get_help();
		        return FALSE;
		    case IDOK:
		    case IDYES:
			/* get Print to File status */
			option.auto_bbox = (int)SendDlgItemMessage(hDlg, PSTOEPS_AUTOBBOX, BM_GETCHECK, 0, 0);
			EndDialog(hDlg, IDYES);
			return TRUE;
		    case IDNO:
			get_help();
		    case IDCANCEL:
			EndDialog(hDlg, FALSE);
			return TRUE;
		}
		break;
	}
	return FALSE;
}


BOOL
pstoeps_warn(void)
{
int flag;
#ifndef __WIN32__
DLGPROC lpProcPSTOEPS;
#endif
    load_string(IDS_TOPICPSTOEPS, szHelpTopic, sizeof(szHelpTopic));
#ifdef __WIN32__
    flag = DialogBoxParam(hlanguage, "PSTOEPSDlgBox", hwndimg, PSTOEPSDlgProc, (LPARAM)NULL);
#else
    lpProcPSTOEPS = (DLGPROC)MakeProcInstance((FARPROC)PSTOEPSDlgProc, phInstance);
    flag = DialogBoxParam(hlanguage, "PSTOEPSDlgBox", hwndimg, lpProcPSTOEPS, (LPARAM)NULL);
    FreeProcInstance((FARPROC)lpProcPSTOEPS);
#endif
    return (flag == IDYES);
}



#ifdef __BORLANDC__
#pragma argsused
#endif
/* input string dialog box */
BOOL CALLBACK _export
InstallDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_INITDIALOG:
	    SetDlgItemText(hDlg, INSTALL_DLL, option.gsdll);
	    SetDlgItemText(hDlg, INSTALL_INCLUDE, option.gsinclude);
	    SetDlgItemText(hDlg, INSTALL_OTHER, option.gsother);
            return( TRUE);
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
		case ID_DEFAULT:
		    install_default(hDlg);
		    return(FALSE);
		case ID_HELP:
		    get_help();
		    return(FALSE);
		case IDOK:
		    /* do sanity check on the following strings */
		    GetDlgItemText(hDlg, INSTALL_DLL, option.gsdll, MAXSTR);
		    GetDlgItemText(hDlg, INSTALL_INCLUDE, option.gsinclude, MAXSTR);
		    GetDlgItemText(hDlg, INSTALL_OTHER, option.gsother, MAXSTR);
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

BOOL
install_gsdll(void)
{
BOOL flag;
#ifndef __WIN32__
DLGPROC lpProcInstall;
#endif
	load_string(IDS_TOPICINSTALL, szHelpTopic, sizeof(szHelpTopic));
#ifdef __WIN32__
	flag = DialogBoxParam(hlanguage, "InstallDlgBox", hwndimg, InstallDlgProc, (LPARAM)NULL);
#else
	lpProcInstall = (DLGPROC)MakeProcInstance((FARPROC)InstallDlgProc, phInstance);
	flag = DialogBoxParam(hlanguage, "InstallDlgBox", hwndimg, lpProcInstall, (LPARAM)NULL);
	FreeProcInstance((FARPROC)lpProcInstall);
#endif
	if (flag)
	    option.configured = TRUE;
	return flag;
}


char *depthlist[] =    {"Default", "1", "4", "8", "24"};
int index_to_depth[] = { 0,         1,   4,   8,   24};
int depth_to_index(int depth)
{
int i;
    for (i=0; i<sizeof(index_to_depth)/sizeof(int); i++)
	if (index_to_depth[i] == depth)
	    return i;
    return 0;
}

char *alphalist[] =    {"1", "2", "4"};
int index_to_alpha[] = { 1,   2,   4};
int alpha_to_index(int alpha)
{
int i;
    for (i=0; i<sizeof(index_to_alpha)/sizeof(int); i++)
	if (index_to_alpha[i] == alpha)
	    return i;
    return 0;
}

void
enable_alpha(HWND hDlg)
{
    int i;
    i = (int)SendDlgItemMessage(hDlg, DSET_DEPTH, CB_GETCURSEL, 0, 0L);
    if (i == CB_ERR)
	return;
    i = real_depth(index_to_depth[i]);
    i = (i >= 8);
    EnableWindow(GetDlgItem(hDlg, DSET_TALPHA), i);
    EnableWindow(GetDlgItem(hDlg, DSET_GALPHA), i);
}


/* dialog box for display settings */
#ifdef __BORLANDC__
#pragma argsused
#endif
BOOL CALLBACK _export
DisplaySettingsDlgProc(HWND hDlg, UINT wmsg, WPARAM wParam, LPARAM lParam)
{
    char buf[128];
    int i;
    WORD notify_message;

    switch (wmsg) {
	case WM_INITDIALOG:
	    if (option.xdpi == option.ydpi)
		sprintf(buf,"%g", option.xdpi);
	    else 
		sprintf(buf,"%g %g", option.xdpi, option.ydpi);
	    SetDlgItemText(hDlg, DSET_RES, buf);
	    if (option.zoom_xdpi == option.zoom_ydpi)
		sprintf(buf,"%g", option.zoom_xdpi);
	    else 
		sprintf(buf,"%g %g", option.zoom_xdpi, option.zoom_ydpi);
	    SetDlgItemText(hDlg, DSET_ZOOMRES, buf);
	    SendDlgItemMessage(hDlg, DSET_DEPTH, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	    for (i=0; i<sizeof(depthlist)/sizeof(char *); i++) {
		strcpy(buf, depthlist[i]);
		if (strcmp(buf, "Default")==0)
		    load_string(IDS_DEFAULT, buf, sizeof(buf)-1);
	        SendDlgItemMessage(hDlg, DSET_DEPTH, CB_ADDSTRING, 0, 
		    (LPARAM)((LPSTR)buf));
	    }
    	    SendDlgItemMessage(hDlg, DSET_DEPTH, CB_SETCURSEL, depth_to_index(option.depth), (LPARAM)0);
	    SendDlgItemMessage(hDlg, DSET_TALPHA, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	    SendDlgItemMessage(hDlg, DSET_GALPHA, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	    for (i=0; i<sizeof(alphalist)/sizeof(char *); i++) {
	        SendDlgItemMessage(hDlg, DSET_TALPHA, CB_ADDSTRING, 0, 
		    (LPARAM)((LPSTR)alphalist[i]));
	        SendDlgItemMessage(hDlg, DSET_GALPHA, CB_ADDSTRING, 0, 
		    (LPARAM)((LPSTR)alphalist[i]));
	    }
	    enable_alpha(hDlg);
    	    SendDlgItemMessage(hDlg, DSET_TALPHA, CB_SETCURSEL, alpha_to_index(option.alpha_text), (LPARAM)0);
    	    SendDlgItemMessage(hDlg, DSET_GALPHA, CB_SETCURSEL, alpha_to_index(option.alpha_graphics), (LPARAM)0);
	    return TRUE;
	case WM_COMMAND:
	    notify_message = GetNotification(wParam,lParam);
	    switch (LOWORD(wParam)) {
		case ID_HELP:
		    load_string(IDS_TOPICDSET, szHelpTopic, sizeof(szHelpTopic));
		    get_help();
		    return FALSE;
		case DSET_DEPTH:
		    if (notify_message == CBN_SELCHANGE)
			enable_alpha(hDlg);
		    return FALSE;
		case IDOK:
		    {
		    BOOL unzoom = FALSE;
		    BOOL resize = FALSE;
	            BOOL restart = FALSE;
		    float x, y;
		    GetDlgItemText(hDlg, DSET_RES, buf, sizeof(buf)-2);
		    switch (sscanf(buf,"%f %f", &x, &y)) {
		      case EOF:
		      case 0:
			break;
		      case 1:
			y = x;
		      case 2:
			if (x==0.0)
			    x= DEFAULT_RESOLUTION;
			if (y==0.0)
			    y= DEFAULT_RESOLUTION;
			if ( (x != option.xdpi) || (y != option.ydpi) ) {
			    option.xdpi = x;
			    option.ydpi = y;
			    resize = TRUE; 
			    unzoom = TRUE;
			}
		    }
		    GetDlgItemText(hDlg, DSET_ZOOMRES, buf, sizeof(buf)-2);
		    switch (sscanf(buf,"%f %f", &x, &y)) {
		      case EOF:
		      case 0:
			break;
		      case 1:
			y = x;
		      case 2:
			if (x==0.0)
			    x= DEFAULT_RESOLUTION;
			if (y==0.0)
			    y= DEFAULT_RESOLUTION;
			if ( (x != option.zoom_xdpi) || (y != option.zoom_ydpi) ) {
			    option.zoom_xdpi = x;
			    option.zoom_ydpi = y;
			    resize = TRUE; 
			    unzoom = TRUE;
			}
		    }
    		    i = (int)SendDlgItemMessage(hDlg, DSET_DEPTH, CB_GETCURSEL, 0, 0L);
		    i = index_to_depth[i];
		    if (i != option.depth) {
			option.depth = i;
			restart = TRUE;
			resize = TRUE; 
			unzoom = TRUE;
		    }
    		    i = (int)SendDlgItemMessage(hDlg, DSET_TALPHA, CB_GETCURSEL, 0, 0L);
		    i = index_to_alpha[i];
		    if (i != option.alpha_text) {
			option.alpha_text = i;
			restart = TRUE;
			resize = TRUE; 
			unzoom = TRUE;
		    }
    		    i = (int)SendDlgItemMessage(hDlg, DSET_GALPHA, CB_GETCURSEL, 0, 0L);
		    i = index_to_alpha[i];
		    if (i != option.alpha_graphics) {
			option.alpha_graphics = i;
			/* restart = TRUE; */
			resize = TRUE; 
			unzoom = TRUE;
		    }
		    if (resize) {
			if (unzoom)
			    gsview_unzoom();
			if (gsdll.state != UNLOADED) {
/* gs_resize has this check 
			    if (option.redisplay && (gsdll.state == PAGE) && (psfile.doc != (PSDOC *)NULL))
*/
				gs_resize();
			    /* for those that can't be changed with a */
			    /* postscript command so must close gs */
			    if (restart)
				pending.restart = TRUE;
			}
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

void
display_settings()
{
#ifdef __WIN32__
	DialogBoxParam(hlanguage, "DisplaySettingsDlgBox", hwndimg, DisplaySettingsDlgProc, (LPARAM)NULL);
#else
	DLGPROC lpProcDisplaySettings;
	lpProcDisplaySettings = (DLGPROC)MakeProcInstance((FARPROC)DisplaySettingsDlgProc, phInstance);
	DialogBoxParam(hlanguage, "DisplaySettingsDlgBox", hwndimg, lpProcDisplaySettings, (LPARAM)NULL);
	FreeProcInstance((FARPROC)lpProcDisplaySettings);
#endif
}

/* dialog box for selecting PDF2PS options */
#ifdef __BORLANDC__
#pragma argsused
#endif
BOOL CALLBACK _export
PDF2PSDlgProc(HWND hDlg, UINT wmsg, WPARAM wParam, LPARAM lParam)
{
	int i;
	switch (wmsg) {
	    case WM_INITDIALOG:
		/* set Print to File check box */
		if (option.pdf2ps & OPTION_PDF2PS_BINARYOK)
		    SendDlgItemMessage(hDlg, PDF2PS_BINARYOK, BM_SETCHECK, 1, 0);
		if (option.pdf2ps & OPTION_PDF2PS_LEVEL1)
		    SendDlgItemMessage(hDlg, PDF2PS_LEVEL1, BM_SETCHECK, 1, 0);
		if (option.pdf2ps & OPTION_PDF2PS_NOPROCSET)
		    SendDlgItemMessage(hDlg, PDF2PS_NOPROCSET, BM_SETCHECK, 1, 0);
		return TRUE;
	    case WM_COMMAND:
		switch (LOWORD(wParam)) {
		    case ID_HELP:
			get_help();
		        return FALSE;
		    case IDOK:
			/* get Print to File status */
			i = (int)SendDlgItemMessage(hDlg, PDF2PS_BINARYOK, BM_GETCHECK, 0, 0);
			option.pdf2ps = (option.pdf2ps & (~OPTION_PDF2PS_BINARYOK)) | (i ? OPTION_PDF2PS_BINARYOK : 0);
			i = (int)SendDlgItemMessage(hDlg, PDF2PS_LEVEL1, BM_GETCHECK, 0, 0);
			option.pdf2ps = (option.pdf2ps & (~OPTION_PDF2PS_LEVEL1)) | (i ? OPTION_PDF2PS_LEVEL1 : 0);
			i = (int)SendDlgItemMessage(hDlg, PDF2PS_NOPROCSET, BM_GETCHECK, 0, 0);
			option.pdf2ps = (option.pdf2ps & (~OPTION_PDF2PS_NOPROCSET)) | (i ? OPTION_PDF2PS_NOPROCSET : 0);
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


BOOL
get_pdf2ps_options(void)
{
int flag;
#ifndef __WIN32__
DLGPROC lpProcPDF2PS;
#endif
    load_string(IDS_TOPICOPEN, szHelpTopic, sizeof(szHelpTopic));
#ifdef __WIN32__
    flag = DialogBoxParam(hlanguage, "PDF2PSDlgBox", hwndimg, PDF2PSDlgProc, (LPARAM)NULL);
#else
    lpProcPDF2PS = (DLGPROC)MakeProcInstance((FARPROC)PDF2PSDlgProc, phInstance);
    flag = DialogBoxParam(hlanguage, "PDF2PSDlgBox", hwndimg, lpProcPDF2PS, (LPARAM)NULL);
    FreeProcInstance((FARPROC)lpProcPDF2PS);
#endif
    return flag;
}



/* Text Window for Ghostscript Messages */
/* uses MS-Windows multiline edit field */


#ifdef __WIN32__
#define TWLENGTH 65536
#else
#define TWLENGTH 16384
#endif
#define TWSCROLL 1024
char twbuf[TWLENGTH];
int twend;


#ifdef __BORLANDC__
#pragma argsused
#endif
BOOL CALLBACK _export
TextDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_INITDIALOG:
            SetDlgItemText(hDlg, TEXTWIN_MLE, twbuf);
	    {
	    DWORD linecount;
#ifdef __WIN32__
	    /* EM_SETSEL, followed by EM_SCROLLCARET doesn't work */
	    linecount = SendDlgItemMessage(hDlg, TEXTWIN_MLE, EM_GETLINECOUNT, (WPARAM)0, (LPARAM)0);
	    SendDlgItemMessage(hDlg, TEXTWIN_MLE, EM_LINESCROLL, (WPARAM)0, (LPARAM)linecount-18);
#else
	    linecount = SendDlgItemMessage(hDlg, TEXTWIN_MLE, EM_GETLINECOUNT, (WPARAM)0, (LPARAM)0);
	    SendDlgItemMessage(hDlg, TEXTWIN_MLE, EM_LINESCROLL, (WPARAM)0, MAKELPARAM(linecount-18, 0));
#endif
	    }
            return(TRUE);
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
		case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE);
                case IDOK:
                    EndDialog(hDlg, TRUE);
                    return(TRUE);
		case ID_HELP:
		    get_help();
		    return(FALSE);
		case TEXTWIN_COPY:
		    {HGLOBAL hglobal;
		    LPSTR p;
		    DWORD result;
		    int start, end;
		    result = SendDlgItemMessage(hDlg, TEXTWIN_MLE, EM_GETSEL, (WPARAM)0, (LPARAM)0);
		    start = LOWORD(result);
		    end   = HIWORD(result);
		    if (start == end) {
			start = 0;
			end = twend;
		    }
		    hglobal = GlobalAlloc(GHND | GMEM_SHARE, end-start+1);
		    if (hglobal == (HGLOBAL)NULL) {
			MessageBeep(-1);
			return(FALSE);
		    }
		    p = GlobalLock(hglobal);
		    if (p == (LPSTR)NULL) {
			MessageBeep(-1);
			return(FALSE);
		    }
#ifdef __WIN32__
		    strncpy(p, twbuf+start, end-start);
#else
		    lstrcpyn(p, twbuf+start, end-start);
#endif
		    GlobalUnlock(hglobal);
		    OpenClipboard(hwndimg);
		    EmptyClipboard();
		    SetClipboardData(CF_TEXT, hglobal);
		    CloseClipboard();
		    }
                default:
                    return(FALSE);
            }
        default:
            return(FALSE);
    }
}

/* display dialog box with multiline edit control */
void 
gs_showmess(void)
{
#ifdef __WIN32__
    load_string(IDS_TOPICMESS, szHelpTopic, sizeof(szHelpTopic));
    DialogBoxParam(hlanguage, "TextDlgBox", hwndimg, TextDlgProc, (LPARAM)NULL);
#else
    DLGPROC lpProcText;
    load_string(IDS_TOPICMESS, szHelpTopic, sizeof(szHelpTopic));
    lpProcText = (DLGPROC)MakeProcInstance((FARPROC)TextDlgProc, phInstance);
    DialogBoxParam(hlanguage, "TextDlgBox", hwndimg, lpProcText, (LPARAM)NULL);
    FreeProcInstance((FARPROC)lpProcText);
#endif
}


/* Add string for Ghostscript message window */
void
gs_addmess_count(char GVFAR *str, int count)
{
LPSTR p;
int i, lfcount;
    /* we need to add \r after each \n, so count the \n's */
    lfcount = 0;
    p = str;
    for (i=0; i<count; i++) {
	if (*p == '\n')
	    lfcount++;
	p++;
    }
    if (count + lfcount >= TWSCROLL)
	return;		/* too large */
    if (count + lfcount + twend >= TWLENGTH-1) {
	/* scroll buffer */
	twend -= TWSCROLL;
	memmove(twbuf, twbuf+TWSCROLL, twend);
    }
    p = twbuf+twend;
    for (i=0; i<count; i++) {
	if (*str == '\n') {
	    *p++ = '\r';
	}
	if (*str == '\0') {
	    *p++ = ' ';	/* ignore null characters */
	    str++;
	}
	else
	    *p++ = *str++;
    }
    twend += (count + lfcount);
    *(twbuf+twend) = '\0';
}

void
gs_addmess(char GVFAR *str)
{
    gs_addmess_count(str, lstrlen(str));
}
