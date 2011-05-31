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

/* gvccmd.c */
/* Menu command module of PM and Windows GSview */

#ifdef _Windows
#include "gvwin.h"
#else
#include "gvpm.h"
#endif

void gsview_depth(int new_depth);
BOOL gsview_usersize(void);
void gsview_unzoom(void);

/* gsview menu commands */
int
gsview_command(int command)
{
char prompt[MAXSTR];		/* input dialog box prompt and message box string */
char answer[MAXSTR];		/* input dialog box answer string */
    switch (command) {
	case IDM_OPEN:
		dfreopen();
		gsview_display();
		dfclose();
		return 0;
	case IDM_CLOSE:
		dfreopen();
	    	if (gsprog.valid)
		    gsview_endfile();
		gsview_unzoom();
		psfile.name[0] = '\0';
		dfclose();
		if (page_list.select)
			free(page_list.select);
		page_list.select = NULL;
		if (doc)
			dsc_scan_clean(doc);
		doc = (PSDOC *)NULL;
		return 0;
	case IDM_NEXT:
		if (not_open())
		    return 0;
		if (doc==(PSDOC *)NULL) {
		    if (!gs_open())
		        return 0;
		    if (is_pipe_done()) {
			play_sound(SOUND_NOPAGE);
		    }
		    else {
			psfile.pagenum++;
			next_page();
		    }
		    return 0;
		}
		dfreopen();
		gsview_unzoom();
		dsc_skip(1+page_extra);
		page_extra = 0;
		dfclose();
		return 0;
	case IDM_NEXTSKIP:
		if (not_dsc())
		    return 0;
		dfreopen();
		gsview_unzoom();
		dsc_skip(page_skip+page_extra);
		page_extra = 0;
		dfclose();
		return 0;
	case IDM_REDISPLAY:
		if (not_open())
		    return 0;
		if (doc==(PSDOC *)NULL) {
		    /* don't know where we are so close and reopen */
		    if (!is_pipe_done())
			gs_close();
		}
		if (!gs_open())
		    return 0;
		dfreopen();
		gsview_unzoom();
		if (display.page)
		    next_page(); 
		if ((doc==(PSDOC *)NULL) || (doc->pages==0)) {
			gsview_displayfile(psfile.name);
			dfclose();
			return 0;
		}
		dsc_dopage();
		dfclose();
		return 0;
	case IDM_PREV:
		if (not_dsc())
			return 0;
		dfreopen();
		gsview_unzoom();
		dsc_skip(-1+page_extra);
		page_extra = 0;
		dfclose();
		return 0;
	case IDM_PREVSKIP:
		if (not_dsc())
			return 0;
		dfreopen();
		gsview_unzoom();
		dsc_skip(-page_skip+page_extra);
		page_extra = 0;
		dfclose();
		return 0;
	case IDM_GOTO:
		if (not_dsc())
			return 0;
		dfreopen();
		gsview_unzoom();
		load_string(IDS_TOPICGOTO, szHelpTopic, sizeof(szHelpTopic));
		if (doc->numpages == 0) {
		    gserror(IDS_NOPAGE, NULL, MB_ICONEXCLAMATION, SOUND_NONUMBER);
		}
		else if (get_page(&psfile.pagenum, FALSE)) {
		    if (psfile.pagenum > doc->numpages) {
			psfile.pagenum = doc->numpages;
			play_sound(SOUND_NOPAGE);
		    }
		    else if (psfile.pagenum < 1) {
			psfile.pagenum = 1;
			play_sound(SOUND_NOPAGE);
		    }
		    else {
			if (gs_open()) {
			    if (display.page)
			        next_page();
			    dsc_dopage();
			}
		    }
		}
		dfclose();
		return 0;
	case IDM_INFO:
		show_info();
		return 0;
	case IDM_SELECT:
		gsview_select();
		dfclose();
		return 0;
	case IDM_PRINT:
		if (psfile.name[0] == '\0')
		    gsview_select();
		dfreopen();
		if (psfile.name[0] != '\0')
		    gsview_print(FALSE);
		dfclose();
		return 0;
	case IDM_PRINTTOFILE:
		if (psfile.name[0] == '\0')
			gsview_select();
		dfreopen();
		if (psfile.name[0] != '\0')
		    gsview_print(TRUE);
		dfclose();
		return 0;
	case IDM_SPOOL:
		gsview_spool((char *)NULL, (char *)NULL);
		return 0;
	case IDM_EXTRACT:
		if (psfile.name[0] == '\0')
		    gsview_select();
		dfreopen();
		if (psfile.name[0] != '\0')
		    gsview_extract();
		dfclose();
		return 0;
	case IDM_TEXTEXTRACT:
		if (psfile.name[0] == '\0')
		    gsview_select();
		dfreopen();
		if (psfile.name[0] != '\0')
		    gsview_text_extract();
		dfclose();
		return 0;
	case IDM_TEXTFIND:
		gsview_text_find();
		return 0;
	case IDM_TEXTFINDNEXT:
		gsview_text_findnext();
		return 0;
	case IDM_EXIT:
		PostQuitMessage(0);
		return 0;
	case IDM_COPYCLIP:
		copy_clipboard();
		return 0;
	case IDM_PASTETO:
		paste_to_file();
		return 0;
	case IDM_CONVERT:
		clip_convert();
		return 0;
	case IDM_GSCOMMAND:
		load_string(IDS_GSCOMMAND, prompt, sizeof(prompt));
		strcpy(answer, option.gscommand);
		load_string(IDS_TOPICGSCMD, szHelpTopic, sizeof(szHelpTopic));
		if (get_string(prompt,answer))
		    strcpy(option.gscommand, answer);
		if (option.gscommand[0]=='\0')
		    strcpy(option.gscommand, DEFAULT_GSCOMMAND);
		return 0;
	case IDM_UNITPT:
	case IDM_UNITMM:
	case IDM_UNITINCH:
		gsview_unit(command);
		return 0;
	case IDM_SAFER:
		option.safer = !option.safer;
		check_menu_item(IDM_OPTIONMENU, IDM_SAFER, option.safer);
		return 0;
	case IDM_SAVEDIR:
		option.save_dir = !option.save_dir;
		check_menu_item(IDM_OPTIONMENU, IDM_SAVEDIR, option.save_dir);
		return 0;
	case IDM_BUTTONSHOW:
		option.button_show = !option.button_show;
		check_menu_item(IDM_OPTIONMENU, IDM_BUTTONSHOW, option.button_show);
		show_buttons();
		return 0;
	case IDM_FITPAGE:
		option.fit_page = !option.fit_page;
		check_menu_item(IDM_OPTIONMENU, IDM_FITPAGE, option.fit_page);
		/* should cause WM_SIZE message to be sent */
		return 0;
	case IDM_QUICK:
		option.quick = !option.quick;
		check_menu_item(IDM_OPTIONMENU, IDM_QUICK, option.quick);
		return 0;
	case IDM_AUTOREDISPLAY:
		option.redisplay = !option.redisplay;
		check_menu_item(IDM_OPTIONMENU, IDM_AUTOREDISPLAY, option.redisplay);
		return 0;
	case IDM_EPSFCLIP:
		option.epsf_clip = !option.epsf_clip;
		check_menu_item(IDM_OPTIONMENU, IDM_EPSFCLIP, option.epsf_clip);
		gs_resize();
		return 0;
	case IDM_EPSFWARN:
		option.epsf_warn = !option.epsf_warn;
		check_menu_item(IDM_OPTIONMENU, IDM_EPSFWARN, option.epsf_warn);
		return 0;
	case IDM_IGNOREDSC:
		option.ignore_dsc = !option.ignore_dsc;
		check_menu_item(IDM_OPTIONMENU, IDM_IGNOREDSC, option.ignore_dsc);
		if (psfile.name[0]) {
		    if (option.redisplay)
		        gsview_displayfile(psfile.name);
		    else
		        gsview_selectfile(psfile.name);
		}
		return 0;
	case IDM_PSTOEPS:
		if (psfile.name[0] == '\0')
		    gsview_display();
		if (psfile.name[0] != '\0') {
		    dfreopen();
		    ps_to_eps();
		    dfclose();
		}
		return 0;
	case IDM_MAKEEPSI:
		dfreopen();
		make_eps_interchange(FALSE);
		dfclose();
		return 0;
	case IDM_MAKEEPST4:
	case IDM_MAKEEPST:
		dfreopen();
		make_eps_tiff(command);
		dfclose();
		return 0;
	case IDM_MAKEEPSW:
		dfreopen();
		make_eps_metafile();
		dfclose();
		return 0;
	case IDM_EXTRACTPS:
	case IDM_EXTRACTPRE:
		dfreopen();
		extract_doseps(command);
		dfclose();
		return 0;
	case IDM_SETTINGS:
		write_profile();
		return 0;
	case IDM_SAVESETTINGS:
		option.settings = !option.settings;
		check_menu_item(IDM_OPTIONMENU, IDM_SAVESETTINGS, option.settings);
		sprintf(prompt, "%d", option.settings);
		{ PROFILE *prf = profile_open(szIniFile);
		  profile_write_string(prf, INISECTION, "SaveSettings", prompt);
		  profile_close(prf);
		}
		return 0;
	case IDM_SOUNDS:
		change_sounds();
		return 0;
	case IDM_PORTRAIT:
	case IDM_LANDSCAPE:
	case IDM_UPSIDEDOWN:
	case IDM_SEASCAPE:
	case IDM_SWAPLANDSCAPE:
		dfreopen();
		gsview_orientation(command);
		dfclose();
		return 0;
	case IDM_RESOLUTION:
		load_string(IDS_RES, prompt, sizeof(prompt));
		if (option.xdpi == option.ydpi)
		    sprintf(answer,"%g", option.xdpi);
		else 
		    sprintf(answer,"%g %g", option.xdpi, option.ydpi);
		load_string(IDS_TOPICMEDIA, szHelpTopic, sizeof(szHelpTopic));
		if (get_string(prompt,answer)) {
		    switch (sscanf(answer,"%f %f", &option.xdpi, &option.ydpi)) {
		      case EOF:
		      case 0:
			return 0;
		      case 1:
			option.ydpi = option.xdpi;
		      case 2:
			if (option.xdpi==0.0)
			    option.xdpi = DEFAULT_RESOLUTION;
			if (option.ydpi==0.0)
			    option.ydpi = DEFAULT_RESOLUTION;
			dfreopen();
			gs_resize();
			gsview_unzoom();
			dfclose();
		    }
		}
		return 0;
	case IDM_ZOOMRES:
		load_string(IDS_ZOOMRES, prompt, sizeof(prompt));
		if (option.zoom_xdpi == option.zoom_ydpi)
		    sprintf(answer,"%g", option.zoom_xdpi);
		else 
		    sprintf(answer,"%g %g", option.zoom_xdpi, option.zoom_ydpi);
		load_string(IDS_TOPICMEDIA, szHelpTopic, sizeof(szHelpTopic));
		if (get_string(prompt,answer)) {
		    switch (sscanf(answer,"%f %f", &option.zoom_xdpi, &option.zoom_ydpi)) {
		      case EOF:
		      case 0:
			return 0;
		      case 1:
			option.zoom_ydpi = option.zoom_xdpi;
		      case 2:
			if (option.zoom_xdpi==0.0)
			    option.zoom_xdpi = DEFAULT_RESOLUTION;
			if (option.zoom_ydpi==0.0)
			    option.zoom_ydpi = DEFAULT_RESOLUTION;
			dfreopen();
			gsview_unzoom();
			dfclose();
		    }
		}
		return 0;
	case IDM_ZOOM:		/* called indirectly from Right Mouse Button */
		if (not_dsc()) {
		    zoom = FALSE;
		    return 0;
		}
		if (!(display.page || display.sync)) {
		    zoom = FALSE;
	    	    gserror(IDS_NOZOOM, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    	    return 0;
		}
		dfreopen();
		if (display.page)
		    next_page(); 
		gs_resize();
		if ((doc==(PSDOC *)NULL) || (doc->pages==0)) {
			gsview_displayfile(psfile.name);
			dfclose();
			return 0;
		}
		dsc_dopage();
		dfclose();
		return 0;
	case IDM_MAGPLUS:
		gs_magnify(1.2);
		return 0;
	case IDM_MAGMINUS:
		gs_magnify(0.8333);
		return 0;
	case IDM_DEPTHDEF:
	case IDM_DEPTH1:
	case IDM_DEPTH4:
	case IDM_DEPTH8:
	case IDM_DEPTH16:
	case IDM_DEPTH24:
		gsview_depth(command);
		return 0;
	case IDM_LETTER:
	case IDM_LETTERSMALL:
	case IDM_TABLOID:
	case IDM_LEDGER:
	case IDM_LEGAL:
	case IDM_STATEMENT:
	case IDM_EXECUTIVE:
	case IDM_A3:
	case IDM_A4:
	case IDM_A4SMALL:
	case IDM_A5:
	case IDM_B4:
	case IDM_B5:
	case IDM_FOLIO:
	case IDM_QUARTO:
	case IDM_10X14:
	case IDM_USERSIZE:
		if (command == IDM_USERSIZE)
		    if (!gsview_usersize())
			return 0;
		dfreopen();
		gsview_media(command);
		dfclose();
		return 0;
	case IDM_HELPCONTENT:
#ifdef _Windows
		WinHelp(hwndimg,szHelpName,HELP_CONTENTS,(DWORD)NULL);
#else
		WinSendMsg(hwnd_help, HM_HELP_CONTENTS, 0L, 0L);
#endif
		return 0;
	case IDM_HELPSEARCH:
#ifdef _Windows
		WinHelp(hwndimg,szHelpName,HELP_PARTIALKEY,(DWORD)"");
#else
		WinSendMsg(hwnd_help, HM_HELP_INDEX, 0L, 0L);
#endif
		return 0;
	case IDM_HELPKEYS:
		load_string(IDS_TOPICKEYS, szHelpTopic, sizeof(szHelpTopic));
		get_help();
		return 0;
	case IDM_ABOUT:
		show_about();
		return 0;
	}
	return 0;
}

/* if no document open, display error message and return true */
BOOL
not_open()
{
	if (psfile.name[0] != '\0')
	    return FALSE;
	gserror(IDS_NOTOPEN, NULL, MB_ICONEXCLAMATION, SOUND_NOTOPEN);
	return TRUE;
}

/* if not DSC document or not open, display error message and return true */
BOOL
not_dsc()
{
	if (not_open())
	    return TRUE;
	if (doc!=(PSDOC *)NULL)
	    return FALSE;
	gserror(IDS_NOPAGE, NULL, MB_ICONEXCLAMATION, SOUND_NONUMBER);
	return TRUE;
}

void
gserror(UINT id, char *str, UINT icon, int sound)
{
int i;
char mess[300];
	info_wait(FALSE);
	if (sound >= 0)
	    play_sound(sound);
	i = 0;
	if (id)
	    i = load_string(id, mess, sizeof(mess)-1);
	mess[i] = '\0';
	if (str)
	    strncpy(mess+i, str, sizeof(mess)-i-1);
	message_box(mess, icon);
}

/* for gvcdsc.c errors instead of fprintf(stderr,...)! */
void
pserror(char *str)
{
	message_box(str, MB_ICONHAND);
}


int
not_implemented()
{
	message_box("Not implemented",0);
	return 0;
}

/* get user defined size */
BOOL
gsview_usersize()
{
char prompt[MAXSTR];
char answer[MAXSTR];
	load_string(IDS_TOPICMEDIA, szHelpTopic, sizeof(szHelpTopic));
	load_string(IDS_USERWIDTH, prompt, sizeof(prompt));
	sprintf(answer,"%d", option.user_width);
	if (!get_string(prompt,answer) || atoi(answer)==0)
		return FALSE;
	option.user_width = atoi(answer);
	load_string(IDS_USERHEIGHT, prompt, sizeof(prompt));
	sprintf(answer,"%d", option.user_height);
	if (!get_string(prompt,answer) || atoi(answer)==0)
		return FALSE;
	option.user_height = atoi(answer);
	return TRUE;
}


int
gsview_depth_to_menu(int depth)
{
	switch(depth) {
	    case 1:
		return IDM_DEPTH1;
	    case 4:
		return IDM_DEPTH4;
	    case 8:
		return IDM_DEPTH8;
	    case 16:
		return IDM_DEPTH16;
	    case 24:
		return IDM_DEPTH24;
	}
	return IDM_DEPTHDEF;
}

int imenu[] = {0, 1, 4, 8, 16, 24, 0};
void
gsview_depth(int new_depth)
{
int old_depth;
BOOL redisplay = FALSE;
	switch(option.depth) {
	    case 1:
		old_depth = IDM_DEPTH1;
		break;
	    case 4:
		old_depth = IDM_DEPTH4;
		break;
	    case 8:
		old_depth = IDM_DEPTH8;
		break;
	    case 16:
		old_depth = IDM_DEPTH16;
		break;
	    case 24:
		old_depth = IDM_DEPTH24;
		break;
	    default:
		old_depth = IDM_DEPTHDEF;
	}
	if (new_depth == old_depth)
		return;
	check_menu_item(IDM_DEPTHMENU, old_depth, FALSE);
	option.depth = imenu[new_depth - IDM_DEPTHDEF];
	check_menu_item(IDM_DEPTHMENU, new_depth, TRUE);
	if (gsprog.valid) {
	    if (option.redisplay && display.page && (doc != (PSDOC *)NULL))
	       redisplay = TRUE;
	    /* can't change depth with a postscript command so must close gs */
	    gs_close();
	    if (redisplay)
		display.do_display = TRUE;
	}
	return;
}

/* unzoom when redisplaying or changing page */ 
void
gsview_unzoom(void)
{
	if (zoom) {
		gs_resize();
		zoom = FALSE;
	}
}

