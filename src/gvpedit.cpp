/* Copyright (C) 1998, Ghostgum Software Pty Ltd.  All rights reserved.
  
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

/* gvpedit.c */
/* Convert PostScript to editable form using pstoedit by Wolfgang Glunz */

#include "gvpm.h"
extern "C" {
#include "pstoedll.h"
}

#define PSTOEDIT_SECTION "PStoEdit"	/* INI file */

HMODULE pstoeditModule;
setPstoeditOutputFunction_func * setPstoeditOutputFunction;
getPstoeditDriverInfo_plainC_func * getPstoeditDriverInfo_plainC;
pstoedit_plainC_func * pstoedit_plainC;
pstoedit_checkversion_func * PstoeditCheckVersionFunction;
clearPstoeditDriverInfo_plainC_func * clearPstoeditDriverInfo;
struct DriverDescription_S * pstoedit_driver_info;

typedef struct tagPSTOEDIT {
    struct DriverDescription_S *driver_info;
    struct DriverDescription_S *format;
    char default_font[MAXSTR];
    BOOL draw_text_as_polygon;
    BOOL map_to_latin1;
    float flatness;
    char driver_option[MAXSTR];
    char output_filename[MAXSTR];	/* output from pstoedit */
    char temp_filename[MAXSTR];		/* input to pstoedit */
} PSTOEDIT;

PSTOEDIT p2e;

int pstoedit_callback (void * cb_data, const char* text, unsigned long length)
/* Returns number of characters successfully written  */
{
/*
	fwrite(text, 1, length, stdout);
*/
	gs_addmess_count((char *)text, length);
	return length;
}

void
load_pstoedit_options(void)
{
PROFILE *prf;
char profile[MAXSTR];
char *section = PSTOEDIT_SECTION;
struct DriverDescription_S * dd;
int i;
float u;

    /* should load pstoedit options from INI file */
    prf = profile_open(szIniFile);
    profile_read_string(prf, section, "Format", "", profile, sizeof(profile));
    dd = p2e.driver_info;
    p2e.format = dd;
    while(dd && (dd->symbolicname) ) {
	if (strcmp(dd->symbolicname, profile) == 0) {
	    p2e.format = dd;
	    break;
	}
	dd++;
    }

    profile_read_string(prf, section, "DefaultFont", "", 
	p2e.default_font, sizeof(p2e.default_font));
    profile_read_string(prf, section, "DriverOption", "", 
	p2e.driver_option, sizeof(p2e.driver_option));
    profile_read_string(prf, section, "OutputFile", "", 
	p2e.output_filename, sizeof(p2e.output_filename));

    profile_read_string(prf, section, "Flatness", "", 
	profile, sizeof(profile));
    if (sscanf(profile,"%f", &u) == 1)
	p2e.flatness = u;
    else
	p2e.flatness = 0.0;

    profile_read_string(prf, section, "DrawTextAsPolygon", "", 
	profile, sizeof(profile));
    if (sscanf(profile,"%d", &i) == 1)
	p2e.draw_text_as_polygon = i;
    else
	p2e.draw_text_as_polygon = 0;

    profile_read_string(prf, section, "MapToISOLatin1", "", 
	profile, sizeof(profile));
    if (sscanf(profile,"%d", &i) == 1)
	p2e.map_to_latin1 = i;
    else
	p2e.map_to_latin1 = 1;

    profile_close(prf);
}

void
save_pstoedit_options(void)
{
char profile[MAXSTR];
char *section = PSTOEDIT_SECTION;
PROFILE *prf;
    prf = profile_open(szIniFile);
    if (p2e.format && p2e.format->symbolicname)
        profile_write_string(prf, section, "Format", p2e.format->symbolicname);
    profile_write_string(prf, section, "DefaultFont", p2e.default_font);
    profile_write_string(prf, section, "DriverOption", p2e.driver_option);
    profile_write_string(prf, section, "OutputFile", p2e.output_filename);
    sprintf(profile, "%g", p2e.flatness);
    profile_write_string(prf, section, "Flatness", profile);
    sprintf(profile, "%d", (int)p2e.draw_text_as_polygon);
    profile_write_string(prf, section, "DrawTextAsPolygon", profile);
    sprintf(profile, "%d", (int)p2e.map_to_latin1);
    //BUG!!    profile_write_string(prf, section, "DontMapToISOLatin1", profile);
    profile_write_string(prf, section, "MapToISOLatin1", profile);
    profile_close(prf);
}

int
unload_pstoedit(void)
{
    gs_addmess("Unloading pstoedit\n");
    /* 
     * If using pstoedit dll 301, we use clearPstoeditDriverInfo.
     */
    clearPstoeditDriverInfo(pstoedit_driver_info);
    pstoedit_driver_info = NULL;
    clearPstoeditDriverInfo = NULL;

    setPstoeditOutputFunction = NULL;
    getPstoeditDriverInfo_plainC = NULL;
    pstoedit_plainC = NULL;
    DosFreeModule(pstoeditModule);
    pstoeditModule = NULL;
    memset(&p2e, 0, sizeof(p2e));
    return 0;
}


BOOL
load_pstoedit(void)
{
  APIRET rc;
  char buf[MAXSTR+40];
  char *p;
  char dllname[MAXSTR];

  if (pstoeditModule)
    return TRUE;

  gs_addmess("Loading pstoedit\n");
  /* assume that pstoedit is in the adjacent directory */
  strcpy(dllname, szExePath);
  p = dllname + strlen(dllname) - 1;	/* remove trailing slash */
  if (*p == '\\')
    *p = '\0';
  p = strrchr(dllname, '\\');		/* remove trailing gsview */
  if (p)
    *(++p) = '\0';
  strcat(dllname, "pstoedt2\\pstoedit.dll");

  /* load pstoedit DLL */
  memset(buf, 0, sizeof(buf));
  gs_addmess("Trying to load ");
  gs_addmess(dllname);
  gs_addmess("\n");
  if ((rc=DosLoadModule((PBYTE)buf, sizeof(buf), 
			(PCSZ)dllname, &pstoeditModule)) != NO_ERROR) {
    gs_addmess("Failed\n");
    strcpy(dllname, "pstoedit.dll");
    memset(buf, 0, sizeof(buf));
    gs_addmess("Trying to load ");
    gs_addmess(dllname);
    gs_addmess("\n");
    if ((rc=DosLoadModule((PBYTE)buf, sizeof(buf), 
			  (PCSZ)dllname, &pstoeditModule)) != NO_ERROR) {
      gs_addmess("Failed\n");
      gs_addmess("pstoedit is not available\n");
      gs_addmess("See help topic 'PStoEdit'\n");
      return FALSE;
    }
  }
  gs_addmess("Succeeded\n");
  
  /* check version match */
  if (NO_ERROR!=DosQueryProcAddr(pstoeditModule, 0L, (PCSZ)"pstoedit_checkversion", 
			(PFN *)(&PstoeditCheckVersionFunction))) {
    gs_addmess("Can't find pstoedit_checkversion()\n");
    unload_pstoedit();
    return FALSE;
    }

  if (PstoeditCheckVersionFunction(pstoeditdllversion)) {
    /* load the clearPstoeditDriverInfo function, which is only
     * available in version 301 and later.
     */
    if (NO_ERROR!=DosQueryProcAddr(pstoeditModule, 0L, 
				   (PCSZ)"clearPstoeditDriverInfo_plainC", 
				   (PFN *)(&clearPstoeditDriverInfo))) {
      gs_addmess("Couldn't find clearPstoeditDriverInfo\n");
      unload_pstoedit();
      return FALSE;
    }
  }
  else {
    /* We didn't have the latest version.
     * DO NOT Check if the older version 300 is available, 
     * which has memory leak problem but is otherwise OK.
     * BUT IS NOT AVAILABLE FOR OS/2
	 */
    gs_addmess("wrong version of pstoedit.dll\n");
    unload_pstoedit();
    return FALSE;
  }

  /* install callback function */
  if (NO_ERROR!=DosQueryProcAddr(pstoeditModule, 0L, 
				 (PCSZ)"setPstoeditOutputFunction", 
				 (PFN *)(&setPstoeditOutputFunction)))
    {
      gs_addmess("Can't find setPstoeditOutputFunction()\n");
      unload_pstoedit();
      return FALSE;
    }
  setPstoeditOutputFunction(0, /* no Call Back Data ptr */
			    pstoedit_callback);

  /* find out which formats (backends) are supported */
  if (NO_ERROR!=DosQueryProcAddr(pstoeditModule, 0L, 
				 (PCSZ)"getPstoeditDriverInfo_plainC", 
				 (PFN *)(&getPstoeditDriverInfo_plainC))) {
    gs_addmess("Can't find getPstoeditDriverInfo_plainC");
    unload_pstoedit();
    return FALSE;
  }
  pstoedit_driver_info = getPstoeditDriverInfo_plainC();

  if (NO_ERROR!=DosQueryProcAddr(pstoeditModule, 0L, (PCSZ)"pstoedit_plainC", 
			(PFN *)(&pstoedit_plainC)))
    gs_addmess("Can't find pstoedit_plainC" );
    
  memset(&p2e, 0, sizeof(p2e));
  p2e.driver_info = pstoedit_driver_info;

    return TRUE;
}


/* dialog box for pstoedit options */
MRESULT EXPENTRY
PStoEditDlgProc(HWND hDlg, ULONG mess, MPARAM mp1, MPARAM mp2)
{
  struct DriverDescription_S * dd;
  char buf[MAXSTR];
  float u;
  int defct=LIT_NONE;
  int i;

  switch (mess) 
    {
    case WM_INITDLG:
      WinSendDlgItemMsg(hDlg, EDIT_FORMAT, LM_DELETEALL, 0, 0);
      dd = p2e.driver_info;
      i=0;
      while(dd && (dd->symbolicname) ) {
	sprintf(buf, "%s:  %s %s", dd->symbolicname, dd->explanation,
		dd->additionalInfo);

	//debug:
	gs_addmess(buf); gs_addmess("\n");

	if (strcmp(dd->symbolicname, p2e.format->symbolicname) == 0)
	  defct=i;
	WinInsertLboxItem(WinWindowFromID(hDlg, EDIT_FORMAT),LIT_END,buf);
	dd++;
	i++;
      }
      WinSendDlgItemMsg(hDlg, EDIT_FORMAT, LM_SELECTITEM, (MPARAM)defct,(MPARAM)TRUE);
      
      WinCheckButton(hDlg, EDIT_DT, p2e.draw_text_as_polygon);
      WinCheckButton(hDlg, EDIT_LATIN1, p2e.map_to_latin1);
      if (p2e.flatness > 0) {
	sprintf(buf, "%g", p2e.flatness);
	WinSetDlgItemText(hDlg, EDIT_FLAT, (PBYTE)buf);
      }
      WinSetDlgItemText(hDlg, EDIT_FONT, (const UCHAR *)p2e.default_font);
      WinSetDlgItemText(hDlg, EDIT_OPTION,(const UCHAR *)p2e.driver_option);
      return (MRESULT)FALSE;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case ID_HELP:
      get_help();
      break;
    case DID_OK:
      /* get format */
      WinQueryLboxItemText(WinWindowFromID(hDlg, EDIT_FORMAT),
			   WinQueryLboxSelectedItem(WinWindowFromID(hDlg, EDIT_FORMAT)),
			   buf,sizeof(buf));
      strtok(buf, ":");
      dd = p2e.driver_info;
      p2e.format = dd;
      while(dd && (dd->symbolicname) ) {
	if (strcmp(dd->symbolicname, buf) == 0) {
	  p2e.format = dd;
	  break;
	}
	dd++;
      }

      WinQueryDlgItemText(hDlg, EDIT_OPTION, sizeof(p2e.driver_option),
			  (UCHAR *)p2e.driver_option);
      WinQueryDlgItemText(hDlg, EDIT_FONT, sizeof(p2e.default_font),
			  (UCHAR *)p2e.default_font);
      
      p2e.draw_text_as_polygon = (BOOL)WinQueryButtonCheckstate(hDlg, EDIT_DT);
      p2e.map_to_latin1 = (BOOL)WinQueryButtonCheckstate(hDlg, EDIT_LATIN1);
      
      WinQueryDlgItemText(hDlg, EDIT_FLAT, sizeof(buf), (UCHAR *)buf);
      if ( (sscanf(buf, "%f", &u) == 1) && (u > 0.0))
	p2e.flatness = u;
      else
	p2e.flatness = 0.0;

      WinDismissDlg(hDlg, DID_OK);
      break;
    case DID_CANCEL:
      WinDismissDlg(hDlg, DID_CANCEL);
      break;
    }
    break;
  default:
    return WinDefDlgProc(hDlg, mess, mp1, mp2);
  }
  return (MRESULT)FALSE;
}

/* extract a temporary file for pstoedit */
BOOL
extract_temp(char *filename)
{
  FILE *pcfile;

  filename[0] = '\0';
  if ( (pcfile = gp_open_scratch_file(szScratch, filename, "wb")) == 
       (FILE *)NULL) {
    gserror(IDS_NOTEMP, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
    play_sound(SOUND_ERROR);
    return FALSE;
  }

  if (!copy_for_printer(pcfile, TRUE)) {
    unlink(filename);
    return FALSE;
  }
  
  fclose(pcfile);
  return TRUE;
}

#define PSTOEDIT_ARGC_MAX 20
void
process_pstoedit(void *arg)
{
  int argc = 0;
  char *argv[20];
  char flatarg[64];
  char format[MAXSTR];

  pending.pstoedit = FALSE;
  
  /* build command line */
  argv[argc++] = "pstoedit"; 
  if (debug)
    argv[argc++] = "-v"; 

  if (p2e.draw_text_as_polygon)
    argv[argc++] = "-dt"; 
  if (!p2e.map_to_latin1)
    argv[argc++] = "-nomaptoisolatin1"; 
  if (strlen(p2e.default_font) != 0) {
    argv[argc++] = "-df";
    argv[argc++] = p2e.default_font;
  }
  if (p2e.flatness > 0.0) {
    argv[argc++] = "-flat";
    sprintf(flatarg, "%g", p2e.flatness);
    argv[argc++] = flatarg;
  }
    
  argv[argc++] = "-f";
  strcpy(format, p2e.format->symbolicname);
  if (strlen(p2e.driver_option) != 0) {
    strcat(format, ":");
    strcat(format, p2e.driver_option);
  }
  argv[argc++] = format;

  argv[argc++] = p2e.temp_filename;
  argv[argc++] = p2e.output_filename;

  if (debug) {
    int i;
    gs_addmess("pstoedit arguments:\r\n");
    for (i=0; i<argc; i++) {
      gs_addmess("  ");
      gs_addmess(argv[i]);
      gs_addmess("\r\n");
    }
  }

  /* invoke pstoedit */
  if (pstoedit_plainC(argc, (const char **)argv, 0 ) != 0)
    post_img_message(WM_GSSHOWMESS, 0);
  
  if (!debug)
    unlink(p2e.temp_filename);
  p2e.temp_filename[0] = '\0';
  
  unload_pstoedit();
  return;
}


int 
gsview_pstoedit(void)
{
  BOOL flag;
  int pagenum;

  flag = load_pstoedit();
  if (!flag) {
    post_img_message(WM_GSSHOWMESS, 0);
    return flag;
  }

  load_pstoedit_options();

  if (flag) {
    nHelpTopic = IDS_TOPICPSTOEDIT;
    flag = (WinDlgBox(HWND_DESKTOP, hwnd_frame, PStoEditDlgProc,
			   hlanguage, IDD_PSTOEDIT, NULL) == DID_OK);
    if (!flag) {
      unload_pstoedit();
      return FALSE;
    }
  }

  if (flag)
    save_pstoedit_options();

  if (flag) {
    pagenum = psfile.pagenum;
    if ( (psfile.dsc != (CDSC *)NULL) && (psfile.dsc->page_count != 0) )
      flag = get_page(&pagenum, 
		      p2e.format->backendSupportsMultiplePages, FALSE);
    if (!flag) {
      unload_pstoedit();
      return FALSE;
    }
  }

  if (flag) {
    /* get filename */
    /* Specify extension would be nice, but it is hard to do
     * with the current get_filename interface.
     */
    flag = get_filename(p2e.output_filename, TRUE, FILTER_ALL, 
			0, IDS_TOPICPSTOEDIT);
    if (!flag) {
      unload_pstoedit();
      return FALSE;
    }
  }

  if (debug && flag) {
    char buf[MAXSTR];
    sprintf(buf, "format=%s dt=%ld latin1=%ld flat=%g\nfont=%s option=%s\n",
	    p2e.format->symbolicname, p2e.draw_text_as_polygon,
	    p2e.map_to_latin1, p2e.flatness,
	    p2e.default_font, p2e.driver_option);
    gs_addmess(buf);
    sprintf(buf, "filename=%s\n", p2e.output_filename);
    gs_addmess(buf);
  }

  if (flag)
    save_pstoedit_options();

  /* extract pages to temporary file */
  if (flag)
    flag = extract_temp(p2e.temp_filename);
  
  if (!flag) {
    unload_pstoedit();
    post_img_message(WM_GSSHOWMESS, 0);
    return FALSE;
  }

  /* shut down Ghostscript */
  pending.unload = TRUE;	
  pending.now = TRUE;	
  
  pending.pstoedit = TRUE;
  
  return flag;
}

