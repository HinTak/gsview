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
char temp[MAXSTR];
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
			_chdir(temp);
		    }
		}
	    }
	    else {
		*p = '\0';
		_chdir(temp);
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
	flag = DialogBoxParam( phInstance, "InputDlgBox", hwndimg, InputDlgProc, (LPARAM)&param);
#else
	lpProcInput = (DLGPROC)MakeProcInstance((FARPROC)InputDlgProc, phInstance);
	flag = DialogBoxParam( phInstance, "InputDlgBox", hwndimg, lpProcInput, (LPARAM)&param);
	FreeProcInstance((FARPROC)lpProcInput);
#endif
	return flag;
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
	    rect.top    = 146 * HIWORD(dwUnit) / 8;
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

void
show_about()
{
#ifdef __WIN32__
	DialogBoxParam( phInstance, "AboutDlgBox", hwndimg, AboutDlgProc, (LPARAM)NULL);
#else
	DLGPROC lpProcAbout;
	lpProcAbout = (DLGPROC)MakeProcInstance((FARPROC)AboutDlgProc, phInstance);
	DialogBoxParam( phInstance, "AboutDlgBox", hwndimg, lpProcAbout, (LPARAM)NULL);
	FreeProcInstance((FARPROC)lpProcAbout);
#endif
}

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
	DialogBoxParam( phInstance, "InfoDlgBox", hwndimg, InfoDlgProc, (LPARAM)NULL);
#else
	DLGPROC lpProcInfo;
	lpProcInfo = (DLGPROC)MakeProcInstance((FARPROC)InfoDlgProc, phInstance);
	DialogBoxParam( phInstance, "InfoDlgBox", hwndimg, lpProcInfo, (LPARAM)NULL);
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
	LoadString(phInstance, IDS_NONE, szNone, sizeof(szNone));
	LoadString(phInstance, IDS_SPKR, szSpeaker, sizeof(szSpeaker));
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
	LoadString(phInstance, IDS_TOPICSOUND, szHelpTopic, sizeof(szHelpTopic));
	DialogBoxParam( phInstance, "SoundDlgBox", hwndimg, SoundDlgProc, (LPARAM)NULL);
#else
	DLGPROC lpProcSound;
	LoadString(phInstance, IDS_TOPICSOUND, szHelpTopic, sizeof(szHelpTopic));
	lpProcSound = (DLGPROC)MakeProcInstance((FARPROC)SoundDlgProc, phInstance);
	DialogBoxParam( phInstance, "SoundDlgBox", hwndimg, lpProcSound, (LPARAM)NULL);
	FreeProcInstance((FARPROC)lpProcSound);
#endif
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

/* Get page number from dialog box and store in ppage */
BOOL
get_page(int *ppage, BOOL multiple)
{
#ifndef __WIN32__
DLGPROC lpProcPage;
#endif
BOOL flag;
	if (doc == (PSDOC *)NULL)
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
#ifdef __WIN32__
	flag = DialogBoxParam( phInstance, "PageDlgBox", hwndimg, PageDlgProc, (LPARAM)NULL);
#else
	lpProcPage = (DLGPROC)MakeProcInstance((FARPROC)PageDlgProc, phInstance);
	flag = DialogBoxParam( phInstance, "PageDlgBox", hwndimg, lpProcPage, (LPARAM)NULL);
	FreeProcInstance((FARPROC)lpProcPage);
#endif
	if (flag && (page_list.current >= 0))
		*ppage = page_list.current + 1;
	return flag;
}

BOOL CALLBACK _export
BoundingBoxDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
static int bboxindex;
float x, y;
char buf[MAXSTR];
    switch(message) {
	case WM_INITDIALOG:
	    bboxindex = 0;
	    LoadString(phInstance, IDS_BBPROMPT, buf, sizeof(buf));
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
	    	        LoadString(phInstance, IDS_BBPROMPT+bboxindex, buf, sizeof(buf));
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
	if (!display.page) {
	    gserror(IDS_EPSNOBBOX, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    return FALSE;
	}
#ifdef __WIN32__
	hDlgModeless = CreateDialogParam(phInstance, "BoundingBoxDlgBox", hwndimg, BoundingBoxDlgProc, (LPARAM)NULL);
#else
	lpfnBoundingBoxProc = (DLGPROC)MakeProcInstance((FARPROC)BoundingBoxDlgProc, phInstance);
	hDlgModeless = CreateDialogParam(phInstance, "BoundingBoxDlgBox", hwndimg, lpfnBoundingBoxProc, (LPARAM)NULL);
#endif
	while (hDlgModeless) {
	    do_message();	/* wait for bounding box to be obtained */
	}
#ifndef __WIN32__
	FreeProcInstance((FARPROC)lpfnBoundingBoxProc);
#endif
	return bbox.valid;
}


