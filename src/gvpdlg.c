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

/* gvpdlg.c */
/* Dialog boxes for PM GSview */
#include "gvpm.h"

BOOL get_string_busy;
char get_string_prompt[MAXSTR];
char get_string_answer[MAXSTR];
void listbox_getpath(HWND hwnd, char *path, int listbox);
void listbox_directory(HWND hwnd, char *path, int listbox, int stext);

MRESULT EXPENTRY 
InputDlgProc(HWND hwnd, ULONG mess, MPARAM mp1, MPARAM mp2)
{
    switch(mess) {
	case WM_INITDLG:
	    WinSendMsg( WinWindowFromID(hwnd, ID_ANSWER),
	    	EM_SETTEXTLIMIT, MPFROM2SHORT(MAXSTR, 0), MPFROMLONG(0) );
	    WinSetWindowText( WinWindowFromID(hwnd, ID_ANSWER),
	    	get_string_answer );
	    WinSetWindowText( WinWindowFromID(hwnd, ID_PROMPT),
	    	get_string_prompt );
	    WinSendMsg( WinWindowFromID(hwnd, ID_ANSWER),
	    	EM_SETSEL, MPFROM2SHORT(0, strlen(get_string_answer)), MPFROMLONG(0) );
	    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, ID_ANSWER));
    	    break;
        case WM_COMMAND:
            switch(SHORT1FROMMP(mp1)) {
                case DID_OK:
                    WinEnableWindow(WinWindowFromID(hwnd, DID_OK), FALSE);
                    WinQueryWindowText(WinWindowFromID(hwnd, ID_ANSWER),
                    	MAXSTR, get_string_answer);
                    WinDismissDlg(hwnd, DID_OK);
                    break;
		case ID_HELP:
		    get_help();
		    return (MRESULT)TRUE;
            }
            break;
    }
    return WinDefDlgProc(hwnd, mess, mp1, mp2);
}

BOOL
get_string(char *prompt, char *answer)
{
	if (get_string_busy) {
	    message_box("get_string is busy",0);
	    return FALSE;
	}
	get_string_busy = TRUE;
	strncpy(get_string_prompt, prompt, MAXSTR);
	strncpy(get_string_answer, answer, MAXSTR);
	
	if (WinDlgBox(HWND_DESKTOP, hwnd_frame, InputDlgProc, 0, IDD_INPUT, NULL)
	   == DID_OK) {
		strncpy(answer, get_string_answer, MAXSTR);
		get_string_busy = FALSE;
		return TRUE;
	}
	get_string_busy = FALSE;
	return FALSE;
}


BOOL 
get_filename(char *filename, BOOL save, int filter, int title, int help)
{
FILEDLG FileDlg;
FILE *f;
char szTitle[MAXSTR];
char *p;
int i;
	memset(&FileDlg, 0, sizeof(FILEDLG));
	FileDlg.cbSize = sizeof(FILEDLG);
	if (save)
	    FileDlg.fl = FDS_CENTER | FDS_SAVEAS_DIALOG;
	else
	    FileDlg.fl = FDS_CENTER | FDS_OPEN_DIALOG;
	if (help) {
	    load_string(help, szHelpTopic, sizeof(szHelpTopic));
	    FileDlg.fl |= FDS_HELPBUTTON;
	}
	if (title) {
	    load_string(title, szTitle, sizeof(szTitle));
	    FileDlg.pszTitle = szTitle;
	}
	_getcwd(FileDlg.szFullFile, sizeof(FileDlg.szFullFile));
	for (p=FileDlg.szFullFile; *p; p++) {
	    if (*p == '/')
		*p = '\\';
	}
	if (filter) {
	    i = strlen(FileDlg.szFullFile);
	    if (i && FileDlg.szFullFile[i-1]!='\\') {
		strcat(FileDlg.szFullFile, "\\");
		i++;
	    }
	    load_string(IDS_FILTER_BASE+filter, FileDlg.szFullFile+i, sizeof(FileDlg.szFullFile)-i);
	}
	WinFileDlg(HWND_DESKTOP, hwnd_frame, &FileDlg);
	if (FileDlg.lReturn == DID_OK) {
	    f = (FILE *)NULL;
	    if ( !save && ((f = fopen(FileDlg.szFullFile, "rb")) == (FILE *)NULL) ) {
		gserror(IDS_FILENOTFOUND, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
		return FALSE;
	    }
	    if (f)
	        fclose(f);
	    strncpy(filename, FileDlg.szFullFile, MAXSTR);
	    p = strrchr(FileDlg.szFullFile, '\\');
	    if (p) {
		*p = '\0';
		_chdir(FileDlg.szFullFile);
	    }
	    return TRUE;
	}
	return FALSE;
}

#ifdef NOTUSED
/* Get page number from dialog box and store in ppage */
BOOL
get_page(int *ppage, BOOL multiple)
{
	char answer[MAXSTR];
	if (doc == (PSDOC *)NULL)
		return FALSE;
	if (doc->numpages == 0) {
		gserror(IDS_NOPAGE, NULL, MB_ICONEXCLAMATION, SOUND_NONUMBER);
		return FALSE;
	}
	load_string(IDS_TOPICOPEN, szHelpTopic, sizeof(szHelpTopic));
	sprintf(answer,"%d", psfile.pagenum);
	if (!get_string("Select Page",answer) || atoi(answer)==0)
		return FALSE;
	*ppage = atoi(answer);
	return TRUE;
}
#endif

/* information about document dialog box */
MRESULT EXPENTRY 
InfoDlgProc(HWND hwnd, ULONG mess, MPARAM mp1, MPARAM mp2)
{
    switch(mess) {
        case WM_INITDLG:
	    info_init(hwnd);
	    break;
        case WM_COMMAND:
            switch(SHORT1FROMMP(mp1)) {
                case DID_OK:
                    WinEnableWindow(WinWindowFromID(hwnd, DID_OK), FALSE);
                    WinQueryWindowText(WinWindowFromID(hwnd, ID_ANSWER),
                    	MAXSTR, get_string_answer);
                    WinDismissDlg(hwnd, DID_OK);
                    break;
            }
            break;
    }
    return WinDefDlgProc(hwnd, mess, mp1, mp2);
}


/* show info about ps file */
void
show_info()
{
	WinDlgBox(HWND_DESKTOP, hwnd_frame, InfoDlgProc, 0, IDD_INFO, NULL);
}


/* About Dialog Box */
MRESULT EXPENTRY AboutDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch(msg) {
    case WM_INITDLG:
	WinSetWindowText( WinWindowFromID(hwnd, ABOUT_VERSION),
	    	GSVIEW_VERSION );
	break;
    case WM_BUTTON1DBLCLK:
	{POINTL pt;
	RECTL rect;
	HPS hps;
	HBITMAP hbm;
	rect.xLeft = 8; rect.xRight = rect.xLeft+216;
	rect.yBottom = 8; rect.yTop = rect.yBottom + 8;
	pt.x = SHORT1FROMMP(mp1);  pt.y = SHORT2FROMMP(mp1);
	WinMapDlgPoints(hwnd, &pt, 1, FALSE);
	if (WinPtInRect(hab, &rect, &pt)) {
	    hps = WinGetPS(hwnd);
	    hbm = GpiLoadBitmap(hps, NULLHANDLE, IDM_MISC, 64, 64);
	    pt.x = 250; pt.y = 12;
	    WinMapDlgPoints(hwnd, &pt, 1, TRUE);
	    WinDrawBitmap(hps, hbm, NULL, &pt, CLR_BLACK, CLR_WHITE, DBM_NORMAL);
	    GpiDeleteBitmap(hbm);
	    WinReleasePS(hps);
	}
	}
	break;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_OK:
          WinDismissDlg(hwnd, TRUE);
          return (MRESULT)TRUE;
      }
      break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}	

void
show_about()
{
		WinDlgBox(HWND_DESKTOP, hwnd_frame, AboutDlgProc, 0, IDD_ABOUT, 0);
}


MRESULT EXPENTRY PageDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
char buf[40];
int i;
int notify_message;
    switch(msg) {
    case  WM_INITDLG:
	if (page_list.multiple)
	    load_string(IDS_SELECTPAGES, buf, sizeof(buf));
	else
	    load_string(IDS_SELECTPAGE, buf, sizeof(buf));
	WinSetWindowText(hwnd, buf);
	for (i=0; i<doc->numpages; i++) {
	    WinSendMsg( WinWindowFromID(hwnd, PAGE_LIST),
	    	LM_INSERTITEM, MPFROMLONG(LIT_END), 
		MPFROMP(doc->pages[map_page(i)].label) );
	}
	WinSendMsg( WinWindowFromID(hwnd, PAGE_LIST),
	    	LM_SELECTITEM, MPFROMLONG(page_list.current), MPFROMLONG(TRUE) );
	if (page_list.current > 5)
	    WinSendMsg( WinWindowFromID(hwnd, PAGE_LIST),
		LM_SETTOPINDEX, MPFROMLONG(page_list.current - 5), (MPARAM)0 );
	if (!page_list.multiple) {
		WinEnableWindow(WinWindowFromID(hwnd, PAGE_ALL), FALSE);
		WinEnableWindow(WinWindowFromID(hwnd, PAGE_ODD), FALSE);
		WinEnableWindow(WinWindowFromID(hwnd, PAGE_EVEN), FALSE);
	}
	break;
    case WM_CONTROL:
	notify_message = SHORT2FROMMP(mp1);
	switch (notify_message) {
	    case LN_ENTER:
	        if (SHORT1FROMMP(mp1) == PAGE_LIST)
		    WinPostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPFROM2SHORT(CMDSRC_OTHER, TRUE));
		break;
	}
	break;
    case  WM_COMMAND:
      switch(LOUSHORT(mp1)) {
        case DID_OK:
	    i = (int)WinSendMsg(WinWindowFromID(hwnd, PAGE_LIST), LM_QUERYSELECTION, (MPARAM)0, (MPARAM)0);
	    page_list.current = (i == LIT_NONE) ? -1 : i;
	    for (i=0; i<doc->numpages; i++) {
	        page_list.select[i] = 0;
	    }
	    if (page_list.multiple) {
	        i = LIT_FIRST;
	        while ( (i = (int)WinSendMsg(WinWindowFromID(hwnd, PAGE_LIST), LM_QUERYSELECTION, (MPARAM)i, (MPARAM)0))
		    != LIT_NONE )
		    page_list.select[i] = TRUE;
	    }
	    else
		page_list.select[page_list.current] = TRUE;
            WinDismissDlg(hwnd, DID_OK);
            return (MRESULT)TRUE;
	case PAGE_ALL:
	    WinEnableWindowUpdate(WinWindowFromID(hwnd, PAGE_LIST), FALSE);
	    for (i=(int)WinSendMsg(WinWindowFromID(hwnd, PAGE_LIST), LM_QUERYITEMCOUNT, (MPARAM)0, (MPARAM)0)-1; i>=0; i--)
		WinSendMsg(WinWindowFromID(hwnd, PAGE_LIST), LM_SELECTITEM, (MPARAM)i, (MPARAM)TRUE);
	    WinEnableWindowUpdate(WinWindowFromID(hwnd, PAGE_LIST), TRUE);
            return (MRESULT)TRUE;
	case PAGE_ODD:
	    WinEnableWindowUpdate(WinWindowFromID(hwnd, PAGE_LIST), FALSE);
	    for (i=(int)WinSendMsg(WinWindowFromID(hwnd, PAGE_LIST), LM_QUERYITEMCOUNT, (MPARAM)0, (MPARAM)0)-1; i>=0; i--)
		WinSendMsg(WinWindowFromID(hwnd, PAGE_LIST), LM_SELECTITEM, (MPARAM)i, (MPARAM)!(i&1));
	    WinEnableWindowUpdate(WinWindowFromID(hwnd, PAGE_LIST), TRUE);
            return (MRESULT)TRUE;
	case PAGE_EVEN:
	    WinEnableWindowUpdate(WinWindowFromID(hwnd, PAGE_LIST), FALSE);
	    for (i=(int)WinSendMsg(WinWindowFromID(hwnd, PAGE_LIST), LM_QUERYITEMCOUNT, (MPARAM)0, (MPARAM)0)-1; i>=0; i--)
		WinSendMsg(WinWindowFromID(hwnd, PAGE_LIST), LM_SELECTITEM, (MPARAM)i, (MPARAM)(i&1));
	    WinEnableWindowUpdate(WinWindowFromID(hwnd, PAGE_LIST), TRUE);
            return (MRESULT)TRUE;
	case ID_HELP:
	    get_help();
	    return (MRESULT)TRUE;
      }
      break;
    }
    return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/* Get page number from dialog box and store in ppage */
BOOL
get_page(int *ppage, BOOL multiple)
{
int flag;
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
	if (page_list.multiple)
	    flag = WinDlgBox(HWND_DESKTOP, hwnd_frame, PageDlgProc, 0, IDD_MULTIPAGE, NULL);
	else
	    flag = WinDlgBox(HWND_DESKTOP, hwnd_frame, PageDlgProc, 0, IDD_PAGE, NULL);
	if ((flag == DID_OK) && (page_list.current >= 0))
		*ppage = page_list.current + 1;
	return (flag == DID_OK);
}

MRESULT EXPENTRY BoundingBoxDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
static int bboxindex;
float x, y;
char buf[MAXSTR];
    switch(msg) {
	case WM_INITDLG:
	    bboxindex = 0;
	    load_string(IDS_BBPROMPT, buf, sizeof(buf));
	    WinSetDlgItemText(hwnd, BB_PROMPT, buf);
	    break;
	case WM_COMMAND:
            switch(SHORT1FROMMP(mp1)) {
		case BB_CLICK:
		    if (!get_cursorpos(&x, &y)) {
			WinDestroyWindow(hwnd);
			hwnd_modeless = 0;
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
	    	        WinSetDlgItemText(hwnd, BB_PROMPT, buf);
			WinSetFocus(HWND_DESKTOP, hwnd_modeless);
			return (MRESULT)FALSE;
		    }
		    /* all corners have been obtained so close dialog box */
		    WinDestroyWindow(hwnd);
		    hwnd_modeless = 0;
		    return (MRESULT)TRUE;
	    }
	case WM_CLOSE:
	    WinDestroyWindow(hwnd);
	    hwnd_modeless = 0;
	    return (MRESULT)TRUE;
    }
    return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

BOOL
get_bbox(void)
{
QMSG q_mess;		/* queue message */
	bbox.valid = FALSE;
	bbox.llx = bbox.lly = bbox.urx = bbox.ury = 0;
	if (!display.page) {
	    gserror(IDS_EPSNOBBOX, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    return FALSE;
	}
	hwnd_modeless = WinLoadDlg(HWND_DESKTOP, hwnd_frame, BoundingBoxDlgProc, (HMODULE)0, IDD_BBOX, NULL);
	WinSetWindowPos(hwnd_modeless, HWND_TOP, 0, 0, 0, 0, SWP_ZORDER | SWP_ACTIVATE);
	while (hwnd_modeless) {
	    /* wait for bounding box to be obtained */
  	    if (WinGetMsg(hab, &q_mess, 0L, 0, 0))
	        WinDispatchMsg(hab, &q_mess);
	}
	return bbox.valid;
}

/* sounds stuff */

#define MAX_SYSTEM_SOUND 16
char *system_sounds;
char *sound_entry[MAX_SYSTEM_SOUND];
char szNone[32];
char szSpeaker[32];
int system_num;
MRESULT EXPENTRY SoundDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

int
load_sounds(void)
{
char *p;
int j;
HINI hini;
	/* add our two sounds */
	sound_entry[0] = "";
	sound_entry[1] = BEEP;
	load_string(IDS_NONE, szNone, sizeof(szNone));
	load_string(IDS_SPKR, szSpeaker, sizeof(szSpeaker));
	/* get list of system sounds */
	system_sounds = malloc(PROFILE_SIZE);
	memset(system_sounds, 0, PROFILE_SIZE);
	if (system_sounds != (char *)NULL) {
 	    if ( (hini = PrfOpenProfile(hab, szMMini)) != NULLHANDLE ) {
	        PrfQueryProfileString(hini, "MMPM2_AlarmSounds", NULL, "\0\0", system_sounds, PROFILE_SIZE-1);
	        PrfCloseProfile(hini);
	    }
	}
	p = system_sounds;
	for (j=2; p!=(char *)NULL && j<MAX_SYSTEM_SOUND && strlen(p)!=0; j++) {
	    if (atoi(p) > 100)
	        p += strlen(p) + 1;	/* ignore <Try it> */
	    if (strlen(p)==0)
		j--;
	    else {
	        sound_entry[j] = p;	
	        p += strlen(p) + 1;
	    }
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
char *p, *q;
HINI hini;
	if (index==0)
		return szNone;
	if (index==1)
		return szSpeaker;
	/* get human readable name for system sound */
	if ( (hini = PrfOpenProfile(hab, szMMini)) == NULLHANDLE )
	    return "";
	PrfQueryProfileString(hini, "MMPM2_AlarmSounds", sound_entry[index], "##", buf, sizeof(buf));
	PrfCloseProfile(hini);
    	p = strchr(buf,'#');
    	if (p != (char *)NULL) {
    	    q = strchr(++p,'#');
	    if (q != (char *)NULL) {
	        *q = '\0';
		return p;
	    }
	}
	return "";
}

int 
find_sound_name(char *entry)
{
int i;
	for (i=0; i<system_num; i++) {
	    if (strcmp(entry, get_sound_entry(i))==0)
	        return i;
	}
	return -1;	/* no find */
}

void
add_sounds(HWND hwnd)
{
int ifile;
	for (ifile=system_num-1; ifile>=0; ifile--)
	    WinSendDlgItemMsg(hwnd, SOUND_FILE, LM_INSERTITEM, MPFROMSHORT(0),
	         MPFROMP(get_sound_name(ifile)));
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
	load_string(IDS_TOPICSOUND, szHelpTopic, sizeof(szHelpTopic));
	WinDlgBox(HWND_DESKTOP, hwnd_frame, SoundDlgProc, 0, IDD_SOUND, NULL);
}

MRESULT EXPENTRY SoundDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
	char buf[MAXSTR];
	int ievent, ifile;
	static struct sound_s dsound[NUMSOUND];	/* copy of sound[] */
	char *szWaveFilter = "*.wav";
	static char szPath[MAXSTR];
	static int file_start;
	char *p;

	switch (msg) {
	    case WM_INITDLG:
		file_start = load_sounds();
		for (ievent=0; ievent<NUMSOUND; ievent++) {
		    strcpy(dsound[ievent].file, sound[ievent].file);
		    load_string(sound[ievent].title, buf, sizeof(buf));
		    WinSendDlgItemMsg(hwnd, SOUND_EVENT, LM_INSERTITEM, 
			MPFROMLONG(LIT_END), MPFROMP(buf));
		}
		ievent = 0;
		WinSendDlgItemMsg(hwnd, SOUND_EVENT, LM_SELECTITEM, MPFROMLONG(ievent), MPFROMLONG(TRUE));
		/* force update of SOUND_FILE */
		WinSendMsg(WinWindowFromID(hwnd, SOUND_EVENT), WM_CONTROL,
			MPFROM2SHORT(SOUND_EVENT, LN_SELECT),  MPFROMLONG(0));
		break;
	    case WM_CONTROL:
		if (mp1 == MPFROM2SHORT(SOUND_EVENT, LN_SELECT)) {
			ievent = (int)WinSendDlgItemMsg(hwnd, SOUND_EVENT, LM_QUERYSELECTION,
				MPFROMSHORT(LIT_FIRST), MPFROMLONG(0));
			if (ievent == LIT_NONE) {
			    WinEnableWindow(WinWindowFromID(hwnd, SOUND_TEST), FALSE);
			    break;
			}
			ifile = find_sound_name(dsound[ievent].file);
			if (ifile >= 0) {
			    strcpy(buf, get_sound_name(ifile));
			    szPath[0] = '\0';
			    WinEnableWindow(WinWindowFromID(hwnd, SOUND_TEST), ifile!=0);
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
			    WinEnableWindow(WinWindowFromID(hwnd, SOUND_TEST), TRUE);
			}
			strcat(szPath, szWaveFilter);
			WinEnableWindowUpdate(WinWindowFromID(hwnd, SOUND_FILE), FALSE);
			WinSendDlgItemMsg(hwnd, SOUND_FILE, LM_DELETEALL, MPFROMLONG(0), MPFROMLONG(0));
			listbox_directory(hwnd, szPath, SOUND_FILE, SOUND_PATH);
			add_sounds(hwnd);
			WinEnableWindowUpdate(WinWindowFromID(hwnd, SOUND_FILE), TRUE);
			WinShowWindow(WinWindowFromID(hwnd, SOUND_FILE), TRUE);
			ifile = (int)WinSendDlgItemMsg(hwnd, SOUND_FILE, LM_SEARCHSTRING, 
				MPFROM2SHORT(0,LIT_FIRST), MPFROMP(buf));
			if (ifile != LIT_NONE)
			    WinSendDlgItemMsg(hwnd, SOUND_FILE, LM_SELECTITEM, MPFROMLONG(ifile), MPFROMLONG(TRUE));
			else
			    WinSendDlgItemMsg(hwnd, SOUND_FILE, LM_SELECTITEM, MPFROMLONG(0), MPFROMLONG(TRUE));
		}
		if (mp1 == MPFROM2SHORT(SOUND_FILE, LN_SELECT)) {
		    ifile = (int)WinSendDlgItemMsg(hwnd, SOUND_FILE, LM_QUERYSELECTION,
			MPFROMSHORT(LIT_FIRST), MPFROMLONG(0));
		    WinSendDlgItemMsg(hwnd, SOUND_FILE, LM_QUERYITEMTEXT, 
			MPFROM2SHORT(ifile,sizeof(buf)), MPFROMP(buf));
		    ievent = (int)WinSendDlgItemMsg(hwnd, SOUND_EVENT, LM_QUERYSELECTION,
			MPFROMSHORT(LIT_FIRST), MPFROMLONG(0));
		    if (ifile >= file_start) {
			if (buf[0] == '[') { /* selected a directory */
			    WinEnableWindow(WinWindowFromID(hwnd, SOUND_TEST), FALSE);
			}
			else { /* selected a WAVE file */
			    int i = WinQueryDlgItemText(hwnd, SOUND_PATH, MAXSTR, dsound[ievent].file);
			    if (dsound[ievent].file[i-1] != '\\') {
				dsound[ievent].file[i++] = '\\';
				dsound[ievent].file[i] = '\0';
			    }
			    listbox_getpath(hwnd, dsound[ievent].file + i, SOUND_FILE);
			    WinEnableWindow(WinWindowFromID(hwnd, SOUND_TEST), TRUE);
			}
		    }
		    else {
			WinEnableWindow(WinWindowFromID(hwnd, SOUND_TEST), ifile!=0);
			strcpy(dsound[ievent].file,get_sound_entry(ifile));
		    }
		}
		if (mp1 == MPFROM2SHORT(SOUND_FILE, LN_ENTER)) {
		    ifile = (int)WinSendDlgItemMsg(hwnd, SOUND_FILE, LM_QUERYSELECTION,
			MPFROMSHORT(LIT_FIRST), MPFROMLONG(0));
		    WinSendDlgItemMsg(hwnd, SOUND_FILE, LM_QUERYITEMTEXT, 
			MPFROM2SHORT(ifile,sizeof(buf)), MPFROMP(buf));
		    if (buf[0] == '[') {
		        WinQueryDlgItemText(hwnd, SOUND_PATH, sizeof(szPath), szPath);
			listbox_getpath(hwnd, szPath, SOUND_FILE);
			strcat(szPath, szWaveFilter);
			listbox_directory(hwnd, szPath, SOUND_FILE, SOUND_PATH);
			add_sounds(hwnd);
		    }
		    else {
		        WinSendMsg(hwnd, WM_COMMAND, MPFROMSHORT(SOUND_TEST), MPFROMLONG(0));
		    }
		}
		break;
	    case WM_COMMAND:
		switch (SHORT1FROMMP(mp1)) {
		    case ID_HELP:
			get_help();
		        return (MRESULT)FALSE;
		    case SOUND_EVENT:
			return (MRESULT)FALSE;
		    case SOUND_FILE:
			return (MRESULT)FALSE;
		    case SOUND_TEST:
		        ievent = (int)WinSendDlgItemMsg(hwnd, SOUND_EVENT, LM_QUERYSELECTION,
			    MPFROMSHORT(LIT_FIRST), MPFROMLONG(0));
			if (strlen(dsound[ievent].file)==0)
			    return FALSE;
			if (!(pfnMciPlayFile) || strcmp(dsound[ievent].file,BEEP)==0) {
			    DosBeep(200,200);
			    return FALSE;
			}
			if (isdigit(*dsound[ievent].file))
			    play_system_sound(dsound[ievent].file);
			else {
			    if ((*pfnMciPlayFile)(hwnd_frame, dsound[ievent].file, 0, 0, 0))
			        DosBeep(200,200);
			}
			return (MRESULT)FALSE;
		    case IDOK:
			for (ievent=0; ievent<NUMSOUND; ievent++)
				strcpy(sound[ievent].file, dsound[ievent].file);
			free_sounds();
			WinDismissDlg(hwnd, DID_OK);
			return (MRESULT)TRUE;
		    case DID_CANCEL:
			free_sounds();
			WinDismissDlg(hwnd, DID_CANCEL);
			return (MRESULT)TRUE;
		}
		break;
	}
    	return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/* update path with current selection */
/* returned path has a trailing \ if it is a directory */
void
listbox_getpath(HWND hwnd, char *path, int listbox)
{
int i;
char buf[MAXSTR];
char *p;
ULONG len;
	i = (int)WinSendDlgItemMsg(hwnd, listbox, LM_QUERYSELECTION,
	    MPFROMSHORT(LIT_FIRST), MPFROMLONG(0));
	WinSendDlgItemMsg(hwnd, listbox, LM_QUERYITEMTEXT, 
	    MPFROM2SHORT(i,sizeof(buf)), MPFROMP(buf));
	if (buf[0] == '[') {
	    /* directory or drive */
	    if (buf[1] == '-') { /* drive */
		len = MAXSTR;
		path[0] = toupper(buf[2]);
		path[1] = ':';
		path[2] = '\\';
		DosQueryCurrentDir(buf[2]+1-'a', path+3, &len);
	    }
	    else {
		if (strcmp(buf, "[..]")==0) {
		    if ((p = strrchr(path,'\\')) != (char *)NULL)
		        *(++p)='\0';
		}
		else {
	            if (path[strlen(path)-1] != '\\')
		        strcat(path,"\\");
		    if ((p = strchr(buf, ']')) != (char *)NULL)
		        *p = '\0';
		    strcat(path, buf+1);
		    strcat(path, "\\");
		}
	    }
	}
	else {
	    if (path[strlen(path)-1] != '\\')
		strcat(path,"\\");
	    strcat(path, buf);
	}
}

/* empty list box, then fill it with files and subdirectories given */
/* by path and stext */
/* if path doesn't contain directory, use directory in stext */
/* update stext to point to directory */
void
listbox_directory(HWND hwnd, char *path, int listbox, int stext)
{
APIRET rc;
FILEFINDBUF3 findbuf;
HDIR hdir;
ULONG cFilenames = 1;
char *p;
char dpath[MAXSTR];
char spath[MAXSTR];
int i;
ULONG drivenum, drivemap;
	if (strrchr(path, '\\') == (char *)NULL) {
	    WinQueryWindowText(WinWindowFromID(hwnd, stext), MAXSTR, dpath);
	    if (strlen(dpath)==0)
		strcpy(dpath, szExePath);
	    strcpy(spath, dpath);
	    if (spath[strlen(spath)-1] != '\\')
		strcat(spath,"\\");
	    strcat(spath, path);
	}
	else {
	    strcpy(spath, path);
	    strcpy(dpath, path);
	    p = strrchr(dpath,'\\');
	    *(++p)='\0';
	    if (strlen(dpath) > 3)
	        *(--p)='\0';
	}
	if ( (strlen(dpath) != 3) &&(dpath[strlen(dpath)-1] == '\\') )
	        dpath[strlen(dpath)-1] = '\0';
	WinSetWindowText(WinWindowFromID(hwnd, stext), dpath);
	strcat(dpath, (strlen(dpath) > 3) ? "\\*" : "*");

	/* stuff in filenames */
	WinSendDlgItemMsg(hwnd, listbox, LM_DELETEALL, MPFROMLONG(0), MPFROMLONG(0));
	hdir = HDIR_CREATE;
	cFilenames = 1;
	rc = DosFindFirst(spath, &hdir, FILE_NORMAL,
		&findbuf, sizeof(findbuf), &cFilenames, FIL_STANDARD);
	while (!rc) {
	    WinSendDlgItemMsg(hwnd, listbox, LM_INSERTITEM, 
		MPFROMLONG(LIT_SORTASCENDING), MPFROMP(findbuf.achName));
	    cFilenames = 1;
	    rc = DosFindNext(hdir, &findbuf, sizeof(findbuf), &cFilenames);
	}
	DosFindClose(hdir);

	/* stuff in subdirectory names */
	hdir = HDIR_CREATE;
	cFilenames = 1;
	rc = DosFindFirst(dpath, &hdir, MUST_HAVE_DIRECTORY,
		&findbuf, sizeof(findbuf), &cFilenames, FIL_STANDARD);
	while (!rc) {
	    strcpy(spath,"[");
	    strcat(spath,findbuf.achName);
	    strcat(spath,"]");
	    if (strcmp(findbuf.achName, ".") != 0)
	      WinSendDlgItemMsg(hwnd, listbox, LM_INSERTITEM, 
		MPFROMLONG(LIT_SORTASCENDING), MPFROMP(spath));
	    cFilenames = 1;
	    rc = DosFindNext(hdir, &findbuf, sizeof(findbuf), &cFilenames);
	}
	DosFindClose(hdir);

	DosQueryCurrentDisk(&drivenum, &drivemap); 
	/* stuff in drive list */
	for (i=1; i<=26; i++) {
	  if (drivemap & 1) {
	    sprintf(spath,"[-%c-]",i-1+'a');
	    WinSendDlgItemMsg(hwnd, listbox, LM_INSERTITEM, 
		MPFROMLONG(LIT_END), MPFROMP(spath));
	  }
	  drivemap >>= 1;
	}
}

