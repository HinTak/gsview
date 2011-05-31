/* Copyright (C) 1993-1997, Russell Lang.  All rights reserved.
  
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

void gsview_drawmethod(int new_drawmethod);
BOOL gsview_usersize(void);
void gsview_unzoom(void);

/* gsview menu commands */
int
gsview_command(int command)
{
    switch (command) {
	case IDM_OPEN:
		if (pending.psfile) {
		    play_sound(SOUND_BUSY);
		    return 0;
		}
		gsview_display();
		return 0;
	case IDM_LASTFILE1:
	case IDM_LASTFILE2:
	case IDM_LASTFILE3:
	case IDM_LASTFILE4:
		if (pending.psfile) {
		    play_sound(SOUND_BUSY);
		    return 0;
		}
		gsview_displayfile(last_files[command-IDM_LASTFILE1]);
		return 0;
	case IDM_CLOSE:
		/* doesn't unload DLL */
		/* close file */
	  	if (gsdll.valid && gsdll.state) {
		    PSFILE *tpsfile;
		    if (pending.psfile) {
			play_sound(SOUND_BUSY);
			return 0;
		    }
		    tpsfile = (PSFILE *)malloc(sizeof(PSFILE));
		    if (tpsfile == NULL)
			return 0;
		    memset((char *)tpsfile, 0, sizeof(PSFILE));
		    pending.psfile = tpsfile;
		    pending.now = TRUE;
		    if (psfile.name[0] && psfile.doc==(PSDOC *)NULL)
			pending.abort = TRUE;
		}
		else {
		    /* DLL isn't loaded */
		    if (psfile.file)
			dfclose();	/* just to make sure */
		    psfile_free(&psfile);
		    post_img_message(WM_GSTITLE, 0);
		    info_wait(IDS_NOWAIT);
		}
		return 0;
	case IDM_CLOSE_DONE:
		if (selectname[0] != '\0') {
		    /* pending IDM_SELECT */
		    PSFILE *tpsfile;
		    tpsfile = gsview_openfile(selectname);
		    if (tpsfile) {
		        psfile = *tpsfile;
		        free(tpsfile);
		    }
		    selectname[0] = '\0';
		    post_img_message(WM_GSTITLE, 0);
		    info_wait(IDS_NOWAIT);
		}
		return 0;
	case IDM_NEXTHOME:
#ifdef _Windows
		PostMessage(hwndimgchild ,WM_VSCROLL,SB_TOP,0L);
#else
		WinPostMsg(hwnd_frame, WM_VSCROLL, MPFROMLONG(0), MPFROM2SHORT(0, SB_TOP));
#endif
		/* fall thru */
	case IDM_NEXT:
		if (not_open())
		    return 0;
		gs_page_skip(1);
		return 0;
	case IDM_NEXTSKIP:
		if (not_dsc())
		    return 0;
		if (order_is_special())
		    return 0;
		gs_page_skip(page_skip);
		return 0;
	case IDM_REDISPLAY:
		if (not_open())
		    return 0;
		if (psfile.doc==(PSDOC *)NULL) {
		    /* don't know where we are so close and reopen */
		    if (gsdll.state != IDLE) {
			if (!pending.psfile) {
			    pending.psfile = (PSFILE *)malloc(sizeof(PSFILE));
			    if (pending.psfile)
			        *pending.psfile = psfile;
			}
			pending.psfile->pagenum = pending.pagenum = 1;
			pending.abort = TRUE;
			pending.now = TRUE;
		    }
		}
		else {
		    pending.pagenum = -1;  /* default page number is current page */
		    if (psfile.doc->pageorder == SPECIAL)
		        pending.pagenum = 1;	/* restart */
		}
		gsview_unzoom();
		pending.now = TRUE;
		return 0;
	case IDM_PREVHOME:
#ifdef _Windows
		PostMessage(hwndimgchild ,WM_VSCROLL,SB_TOP,0L);
#else
		WinPostMsg(hwnd_frame, WM_VSCROLL, MPFROMLONG(0), MPFROM2SHORT(0, SB_TOP));
#endif
		/* fall thru */
	case IDM_PREV:
		if (not_dsc())
			return 0;
		if (order_is_special())
		    return 0;
		gs_page_skip(-1);
		return 0;
	case IDM_PREVSKIP:
		if (not_dsc())
			return 0;
		if (order_is_special())
		    return 0;
		gs_page_skip(-page_skip);
		return 0;
	case IDM_GOTO:
		if (not_dsc())
			return 0;
		if (order_is_special())
		    return 0;
		if (psfile.doc->numpages == 0) {
		    gserror(IDS_NOPAGE, NULL, MB_ICONEXCLAMATION, SOUND_NONUMBER);
		    return 0;
		}
		load_string(IDS_TOPICGOTO, szHelpTopic, sizeof(szHelpTopic));
		{ int pagenum;
		    pagenum = psfile.pagenum;
		    if (get_page(&pagenum, FALSE, FALSE)) {
			if (pagenum > psfile.doc->numpages) {
			    pagenum = psfile.doc->numpages;
			    play_sound(SOUND_NOPAGE);
			}
			else if (pagenum < 1) {
			    pagenum = 1;
			    play_sound(SOUND_NOPAGE);
			}
			else {
			    gsview_unzoom();
			    pending.pagenum = pagenum;
			    pending.now = TRUE;
			}
		    }
		}
		return 0;
	case IDM_INFO:
		show_info();
		return 0;
	case IDM_SELECT:
		if (pending.psfile) {
		    play_sound(SOUND_BUSY);
		    return 0;
		}
		gsview_select();
		return 0;
	case IDM_PRINT:
	case IDM_PRINTTOFILE:
		if (psfile.name[0] == '\0')
		    gsview_select();
		if (gsdll.state == BUSY) {
		    play_sound(SOUND_BUSY);
		    return 0;
		}
		if (!dfreopen())
		    return 0;
		if (command == IDM_PRINTTOFILE)
		    option.print_to_file = TRUE;
		if (psfile.name[0] != '\0')
		    gsview_print();
		dfclose();
		return 0;
	case IDM_SPOOL:
		gsview_spool((char *)NULL, (char *)NULL);
		return 0;
	case IDM_SAVEAS:
		if (gsdll.state == BUSY) {
		    play_sound(SOUND_BUSY);
		    return 0;
		}
		if (psfile.name[0] == '\0')
		    gsview_select();
		if (psfile.name[0] != '\0')
		    gsview_saveas();
		return 0;
	case IDM_EXTRACT:
		if (gsdll.state == BUSY) {
		    play_sound(SOUND_BUSY);
		    return 0;
		}
		if (psfile.name[0] == '\0')
		    gsview_select();
		if (order_is_special())
		    return 0;
		if (psfile.name[0] != '\0')
		    gsview_extract();
		return 0;
	case IDM_TEXTEXTRACT:
		if (psfile.name[0] == '\0')
		    gsview_select();
		if (psfile.name[0] != '\0')
		    gsview_text_extract();
		return 0;
	case IDM_TEXTEXTRACT_SLOW:
	        gsview_text_extract_slow();
		return 0;
	case IDM_TEXTFIND:
		gsview_text_find();
		return 0;
	case IDM_TEXTFINDNEXT:
		gsview_text_findnext();
		return 0;
	case IDM_GSMESS:
		gs_showmess();	/* show messages from Ghostscript */
		return 0;
	case IDM_EXIT:
		post_img_message(WM_CLOSE, 0);
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
	case IDM_CFG:
		config_wizard();
		return 0;
	case IDM_GSCOMMAND:
	        install_gsdll();
		return 0;
	case IDM_UNITPT:
	case IDM_UNITMM:
	case IDM_UNITINCH:
		gsview_unit(command);
		return 0;
	case IDM_LANGEN:
	case IDM_LANGDE:
	case IDM_LANGFR:
		gsview_language(command);
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
	case IDM_QUICK_OPEN:
		option.quick_open = !option.quick_open;
		check_menu_item(IDM_OPTIONMENU, IDM_QUICK_OPEN, option.quick_open);
		return 0;
	case IDM_PSTOTEXTDIS:
	case IDM_PSTOTEXTNORM:
	case IDM_PSTOTEXTCORK:
		check_menu_item(IDM_PSTOTEXTMENU, option.pstotext + IDM_PSTOTEXTMENU + 1, FALSE);
		option.pstotext = command - IDM_PSTOTEXTMENU - 1;
		check_menu_item(IDM_PSTOTEXTMENU, option.pstotext + IDM_PSTOTEXTMENU + 1, TRUE);
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
	case IDM_SHOWBBOX:
		option.show_bbox = !option.show_bbox;
		check_menu_item(IDM_OPTIONMENU, IDM_SHOWBBOX, option.show_bbox);
#ifdef _Windows
		PostMessage(hwndimg, WM_GSSYNC, 0, 0L);
#else
		if (!WinInvalidateRect(hwnd_bmp, (PRECTL)NULL, TRUE))
			error_message("error invalidating rect");
  		if (!WinUpdateWindow(hwnd_bmp))
			error_message("error updating window");
#endif
		return 0;
	case IDM_PSTOEPS:
		if (not_open())
		    return 0;
		if (psfile.name[0] != '\0') {
		    if (!dfreopen())
			return 0;
		    ps_to_eps();
		    dfclose();
		}
		return 0;
	case IDM_MAKEEPSI:
		if ( (option.orientation == IDM_PORTRAIT) ||
		     (option.auto_orientation == TRUE) ) {
		    if (!dfreopen())
			return 0;
		    if (gsdll.lock_device && gsdll.device)
			gsdll.lock_device(gsdll.device, 1);
		    make_eps_interchange(FALSE);
		    if (gsdll.lock_device && gsdll.device)
			gsdll.lock_device(gsdll.device, 0);
		    dfclose();
	  	}
		else
		    gserror(IDS_MUSTUSEPORTRAIT, 0, MB_ICONEXCLAMATION, 0); 
		return 0;
	case IDM_MAKEEPST4:
	case IDM_MAKEEPST6U:
	case IDM_MAKEEPST6P:
		if ( (option.orientation == IDM_PORTRAIT) ||
		     (option.auto_orientation == TRUE) ) {
		    if (!dfreopen())
			return 0;
		    if (gsdll.lock_device && gsdll.device)
			gsdll.lock_device(gsdll.device, 1);
		    make_eps_tiff(command, FALSE);
		    if (gsdll.lock_device && gsdll.device)
			gsdll.lock_device(gsdll.device, 0);
		    dfclose();
		}
		else
		    gserror(IDS_MUSTUSEPORTRAIT, 0, MB_ICONEXCLAMATION, 0); 
		return 0;
	case IDM_MAKEEPSW:
		if ( (option.orientation == IDM_PORTRAIT) ||
		     (option.auto_orientation == TRUE) ) {
		    if (!dfreopen())
			return 0;
		    if (gsdll.lock_device && gsdll.device)
			gsdll.lock_device(gsdll.device, 1);
		    make_eps_metafile(FALSE);
		    if (gsdll.lock_device && gsdll.device)
			gsdll.lock_device(gsdll.device, 0);
		    dfclose();
		}
		else
		    gserror(IDS_MUSTUSEPORTRAIT, 0, MB_ICONEXCLAMATION, 0); 
		return 0;
	case IDM_MAKEEPSU:
		if (!dfreopen())
		    return 0;
		make_eps_user();
		dfclose();
		return 0;
	case IDM_EXTRACTPS:
	case IDM_EXTRACTPRE:
		if (!dfreopen())
		    return 0;
		extract_doseps(command);
		dfclose();
		return 0;
	case IDM_SETTINGS:
		write_profile();
		return 0;
	case IDM_SAVESETTINGS:
		option.settings = !option.settings;
		check_menu_item(IDM_OPTIONMENU, IDM_SAVESETTINGS, option.settings);
		{ char buf[MAXSTR];
		  PROFILE *prf = profile_open(szIniFile);
		  sprintf(buf, "%d", (int)option.settings);
		  profile_write_string(prf, INISECTION, "SaveSettings", buf);
		  profile_close(prf);
		}
		return 0;
	case IDM_SOUNDS:
		change_sounds();
		return 0;
	case IDM_AUTOORIENT:
	case IDM_PORTRAIT:
	case IDM_LANDSCAPE:
	case IDM_UPSIDEDOWN:
	case IDM_SEASCAPE:
	case IDM_SWAPLANDSCAPE:
		gsview_orientation(command);
		return 0;
	case IDM_ZOOM:		/* called indirectly from Right Mouse Button */
		if (not_dsc()) {
		    zoom = FALSE;
		    return 0;
		}
		if (order_is_special()) {
		    zoom = !zoom;
		    return 0;
		}
		if (! ((gsdll.state == PAGE) || (gsdll.state == IDLE)) ) {
		    zoom = FALSE;
	    	    gserror(IDS_NOZOOM, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    	    return 0;
		}
		gs_resize();
	        pending.pagenum = -1;  /* default page number is current page */
		pending.now = TRUE;
		return 0;
	case IDM_MAGPLUS:
		gs_magnify(1.2);
		return 0;
	case IDM_MAGMINUS:
		gs_magnify(0.8333);
		return 0;
	case IDM_DISPLAYSETTINGS:
		display_settings();
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
		gsview_media(command);
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

/* if order is SPECIAL, display error message and return TRUE */
BOOL
order_is_special()
{
char buf[MAXSTR];
	if (psfile.doc==(PSDOC *)NULL)
	    return TRUE;
	if (psfile.doc->pageorder != SPECIAL)
	    return FALSE;
	if (psfile.doc->numpages == 1)
	    return FALSE;	/* can't reorder anyway */
	if (psfile.ignore_special)
	    return FALSE;
        load_string(IDS_PAGESPECIAL, buf, sizeof(buf)-1);
        if (message_box(buf, MB_OKCANCEL) == IDOK) {
	    /* don't show this warning again for this file */
	    psfile.ignore_special = TRUE;
	    return FALSE;	/* User override */
	}
	return TRUE;		/* order is special */
}


/* if not DSC document or not open, display error message and return true */
BOOL
not_dsc()
{
	if (not_open())
	    return TRUE;
	if (psfile.doc!=(PSDOC *)NULL)
	    return FALSE;
	gserror(IDS_NOPAGE, NULL, MB_ICONEXCLAMATION, SOUND_NONUMBER);
	return TRUE;
}

void
gserror(UINT id, LPSTR str, UINT icon, int sound)
{
int i;
char mess[MAXSTR+MAXSTR];
	if (sound >= 0)
	    play_sound(sound);
	i = 0;
	if (id)
	    i = load_string(id, mess, sizeof(mess)-1);
	mess[i] = '\0';
	if (str)
#if defined(_Windows) && !defined(__WIN32__)
	    lstrcpyn(mess+i, str, sizeof(mess)-i-1);
#else
	    strncpy(mess+i, str, sizeof(mess)-i-1);
#endif
	message_box(mess, icon);
}

/* for ps.c errors instead of fprintf(stderr,...)! */
void
pserror(char *str)
{
	message_box(str, MB_ICONHAND);
}


int
not_implemented()
{
        gserror(IDS_NOTIMPLEMENTED, NULL, 0, 0);
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
        gsview_check_usersize();
	load_string(IDS_USERHEIGHT, prompt, sizeof(prompt));
	sprintf(answer,"%d", option.user_height);
	if (!get_string(prompt,answer) || atoi(answer)==0)
		return FALSE;
	option.user_height = atoi(answer);
	if ((option.user_width==0) || (option.user_height == 0)) {
	    option.user_width = 640;
	    option.user_width = 480;
	}
        gsview_check_usersize();
	return TRUE;
}

void
gsview_check_usersize()
{
	if ( (option.user_width > 5669) || (option.user_height > 5669) ) {
	    gserror(IDS_LARGEMEDIA, NULL, 0, SOUND_ERROR);
	}
}

/* unzoom when redisplaying or changing page */ 
void
gsview_unzoom(void)
{
	if (zoom) {
		zoom = FALSE;
		gs_resize();
	}
}

void
gsview_language(int new_language)
{
	if (load_language(new_language)) {
	    check_menu_item(IDM_LANGMENU, option.language, FALSE);
	    option.language = new_language;
	    check_menu_item(IDM_LANGMENU, option.language, TRUE);
	    change_language();
	}
}

