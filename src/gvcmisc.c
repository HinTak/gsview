/* Copyright (C) 1993, 1994, 1995, Russell Lang.  All rights reserved.
  
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

/* gvcmisc.c */
/* Miscellaneous GSview routines common to Windows and PM */

#ifdef _Windows
#include "gvwin.h"
#else
#include "gvpm.h"
#endif

/* display error message and post quit message */
void 
error_message(char *str)
{
	message_box(str, MB_ICONHAND);
	post_close();
}

void
info_init(HWND hwnd)
{
    char buf[MAXSTR];
    int n;
    if (psfile.name[0] != '\0') {
	SetDlgItemText(hwnd, INFO_FILE, psfile.name);
	if (doc) {
	    if (psfile.ctrld)
		load_string(IDS_NOTDSC, buf, sizeof(buf));
	    else  {
		if (psfile.ispdf) {
		    load_string(IDS_PDF, buf, sizeof(buf));
		}
		else if (doc->epsf) {
		    switch (psfile.preview) {
			case IDS_EPSI:
			  load_string(IDS_EPSI, buf, sizeof(buf));
			  break;
			case IDS_EPST:
			  load_string(IDS_EPST, buf, sizeof(buf));
			  break;
			case IDS_EPSW:
			  load_string(IDS_EPSW, buf, sizeof(buf));
			  break;
			default:
			  load_string(IDS_EPSF, buf, sizeof(buf));
		    }
		}
		else
		    load_string(IDS_DSC, buf, sizeof(buf));
	    }
	    SetDlgItemText(hwnd, INFO_TYPE, buf);
	    SetDlgItemText(hwnd, INFO_TITLE, doc->title ? doc->title : "");
	    SetDlgItemText(hwnd, INFO_DATE, doc->date ? doc->date : "");
	    sprintf(buf, "%d %d %d %d", doc->boundingbox[LLX], doc->boundingbox[LLY], 
		doc->boundingbox[URX], doc->boundingbox[URY]);
	    SetDlgItemText(hwnd, INFO_BBOX, buf);
	    switch(doc->orientation) {
		case LANDSCAPE:
			load_string(IDS_LANDSCAPE, buf, sizeof(buf));
			break;
		case PORTRAIT:
			load_string(IDS_PORTRAIT, buf, sizeof(buf));
			break;
		default:
			buf[0] = '\0';
	    }
	    SetDlgItemText(hwnd, INFO_ORIENT, buf);
	    switch(doc->pageorder) {
		case ASCEND: 
			load_string(IDS_ASCEND, buf, sizeof(buf));
			break;
		case DESCEND: 
			load_string(IDS_DESCEND, buf, sizeof(buf));
			break;
		case SPECIAL:
			load_string(IDS_SPECIAL, buf, sizeof(buf));
			break;
		default:
			buf[0] = '\0';
	    }
	    SetDlgItemText(hwnd, INFO_ORDER, buf);
	    if (doc->default_page_media && doc->default_page_media->name) {
		sprintf(buf,"%s %d %d",doc->default_page_media->name,
		    doc->default_page_media->width,
		    doc->default_page_media->height);
	    }
	    else {
		buf[0] = '\0';
	    }
	    SetDlgItemText(hwnd, INFO_DEFMEDIA, buf);
	    sprintf(buf, "%d", doc->numpages);
	    SetDlgItemText(hwnd, INFO_NUMPAGES, buf);
	    n = map_page(psfile.pagenum - 1);
	    if (doc->pages)
		    sprintf(buf, "\"%s\"   %d", doc->pages[n].label ? doc->pages[n].label : "", psfile.pagenum);
	    else
		    buf[0] = '\0';
	    SetDlgItemText(hwnd, INFO_PAGE, buf);
	
	}
	else {
	    if (option.ignore_dsc)
	        load_string(IDS_IGNOREDSC, buf, sizeof(buf));
	    else
	        load_string(IDS_NOTDSC, buf, sizeof(buf));
	    SetDlgItemText(hwnd, INFO_TYPE, buf);
	}
	sprintf(buf, "%d x %d", display.width, display.height);
	SetDlgItemText(hwnd, INFO_BITMAP, buf);
    }
    else {
	load_string(IDS_NOFILE, buf, sizeof(buf));
	SetDlgItemText(hwnd, INFO_FILE, buf);
    }
}

/* read settings fron INI file */
void
read_profile()
{
int i;
char profile[128];
char *section = INISECTION;
char *device_ptr;
PROFILE *prf;
	prf = profile_open(szIniFile);
	profile_read_string(prf, section, "Version", "", profile, sizeof(profile));
	if (strcmp(profile, GSVIEW_VERSION)!=0)
	    changed_version = TRUE;
	profile_read_string(prf, section, "Origin", "", profile, sizeof(profile));
	if (sscanf(profile,"%d %d", &option.img_origin.x, &option.img_origin.y) != 2) {
		option.img_origin.x = option.img_origin.y = CW_USEDEFAULT;
	}
	profile_read_string(prf, section, "Size", "", profile, sizeof(profile));
	if (sscanf(profile,"%d %d", &option.img_size.x, &option.img_size.y) != 2) {
		option.img_size.x = option.img_size.y = CW_USEDEFAULT;
	}
	if ((option.img_size.x<32) || (option.img_size.y<32))
		option.img_size.x = option.img_size.y = CW_USEDEFAULT;
	profile_read_string(prf, section, "Maximized", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &option.img_max) != 1)
		option.img_max = 0;
	profile_read_string(prf, section, "SaveSettings", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.settings = i;
	profile_read_string(prf, section, "ButtonBar", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.button_show = i;
	profile_read_string(prf, section, "FitWindowToPage", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.fit_page = i;
	profile_read_string(prf, section, "Resolution", "", profile, sizeof(profile));
	if (sscanf(profile,"%f %f", &option.xdpi, &option.ydpi) != 2) {
		option.xdpi = option.ydpi = DEFAULT_RESOLUTION;
	}
	profile_read_string(prf, section, "ZoomResolution", "", profile, sizeof(profile));
	if (sscanf(profile,"%f %f", &option.zoom_xdpi, &option.zoom_ydpi) != 2) {
		option.zoom_xdpi = option.zoom_ydpi = DEFAULT_ZOOMRES;
	}
	profile_read_string(prf, section, "Depth", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.depth = i;
	profile_read_string(prf, section, "Media", "", profile, sizeof(profile));
	if (strlen(profile)!=0) {
		char thismedia[20];
		for (i=IDM_LETTER; i<IDM_USERSIZE; i++) {
		    if (!get_menu_string(IDM_MEDIAMENU, i, thismedia, sizeof(thismedia))) {
			/* couldn't get menu string so flag as unknown */
			option.media = IDM_USERSIZE+1;
		        strncpy(option.medianame,profile,sizeof(option.medianame));
			break;
		    }
		    if (!stricmp(thismedia, profile)) {
		        break;
		    }
		}
		if (option.media <= IDM_USERSIZE) {
		    option.media = i;
		    strncpy(option.medianame,thismedia,sizeof(option.medianame));
		}
        }
	profile_read_string(prf, section, "UserSize", "", profile, sizeof(profile));
	if (sscanf(profile,"%d %d", &option.user_width, &option.user_height) != 2) {
		/* this gives 640x480 pixels at 96dpi */
		option.user_width = 480;
		option.user_height = 360;
	}
	gsview_check_usersize();
	profile_read_string(prf, section, "EpsfClip", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.epsf_clip = i;
	profile_read_string(prf, section, "EpsfWarn", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.epsf_warn = i;
	profile_read_string(prf, section, "IgnoreDSC", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.ignore_dsc = i;
	profile_read_string(prf, section, "ShowBBox", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.show_bbox = i;
	profile_read_string(prf, section, "Orientation", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.orientation = i+IDM_PORTRAIT;
	profile_read_string(prf, section, "SwapLandscape", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.swap_landscape = i;
	profile_read_string(prf, section, "Unit", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.unit = i+IDM_UNITPT;
	profile_read_string(prf, section, "QuickOpen", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.quick = i;
	profile_read_string(prf, section, "Safer", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.safer = i;
	profile_read_string(prf, section, "AutoRedisplay", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.redisplay = i;
	profile_read_string(prf, section, "SaveLastDir", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.save_dir = i;
	if (option.save_dir) {
	    char workdir[MAXSTR];
	    gs_getcwd(workdir, sizeof(workdir));	/* save current in case chdir fails */
	    profile_read_string(prf, section, "LastDir", "", profile, sizeof(profile));
	    if (gs_chdir(profile))
	        gs_chdir(workdir);
	}
#ifdef OLD
	profile_read_string(prf, section, "Ghostscript", "", profile, sizeof(profile));
	if (profile[0] != '\0')	/* don't copy a default - assume already set */
		strcpy(option.gscommand, profile);
#endif
	profile_read_string(prf, section, "GhostscriptEXE", "", profile, sizeof(profile));
	if (profile[0] != '\0')	/* don't copy a default - assume already set */
		strcpy(option.gsexe, profile);
	profile_read_string(prf, section, "GhostscriptInclude", "", profile, sizeof(profile));
	if (profile[0] != '\0')	/* don't copy a default - assume already set */
		strcpy(option.gsinclude, profile);
	profile_read_string(prf, section, "GhostscriptOther", "", profile, sizeof(profile));
	if (profile[0] != '\0')	/* don't copy a default - assume already set */
		strcpy(option.gsother, profile);
	profile_read_string(prf, section, "GhostscriptVersion", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1) {
	    if (i <= 261) 
		option.gsversion = IDM_GS261;
	    else if (i <= 333)
		option.gsversion = IDM_GS333;
	    else
		option.gsversion = IDM_GS351;
	}
	profile_read_string(prf, section, "DrawMethod", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1) {
	    switch (i) {
		case (IDM_DRAWGPI-IDM_DRAWMENU):
		    option.drawmethod = IDM_DRAWGPI;
		    break;
		case (IDM_DRAWWIN-IDM_DRAWMENU):
		    option.drawmethod = IDM_DRAWWIN;
		    break;
		default:
		case (IDM_DRAWDEF-IDM_DRAWMENU):
		    option.drawmethod = IDM_DRAWDEF;
		    break;
	    }
	}
	profile_read_string(prf, section, "Printer", ",", profile, sizeof(profile));
	device_ptr = strtok(profile, ",");
	if (device_ptr != (char *)NULL) {
		strcpy(option.device_name, device_ptr);
		device_ptr = strtok(NULL, ",");
		if (device_ptr != (char *)NULL)
			strcpy(option.device_resolution, device_ptr);
	}
	profile_read_string(prf, section, "PrinterPort", "", profile, sizeof(option.printer_port)-1);
	if (profile[0] != '\0')
	    strcpy(option.printer_port, profile);
	for (i=0; i<NUMSOUND; i++) {
		profile_read_string(prf, section, sound[i].entry, sound[i].file, profile, sizeof(profile));
		strcpy(sound[i].file, profile);
	}
	profile_close(prf);
}

/* write settings to INI file */
void
write_profile()
{
char profile[MAXSTR];
char *section = INISECTION;
int i;
PROFILE *prf;
	prf = profile_open(szIniFile);
	profile_write_string(prf, section, "Version", GSVIEW_VERSION);
	sprintf(profile, "%d %d", option.img_origin.x, option.img_origin.y);
	profile_write_string(prf, section, "Origin", profile);
	sprintf(profile, "%d %d", option.img_size.x, option.img_size.y);
	profile_write_string(prf, section, "Size", profile);
	sprintf(profile, "%d", option.img_max);
	profile_write_string(prf, section, "Maximized", profile);
	sprintf(profile, "%d", option.settings);
	profile_write_string(prf, section, "SaveSettings", profile);
	sprintf(profile, "%d", option.button_show);
	profile_write_string(prf, section, "ButtonBar", profile);
	sprintf(profile, "%d", option.fit_page);
	profile_write_string(prf, section, "FitWindowToPage", profile);
	sprintf(profile, "%g %g", option.xdpi, option.ydpi);
	profile_write_string(prf, section, "Resolution", profile);
	sprintf(profile, "%g %g", option.zoom_xdpi, option.zoom_ydpi);
	profile_write_string(prf, section, "ZoomResolution", profile);
	sprintf(profile, "%d", option.depth);
	profile_write_string(prf, section, "Depth", profile);
	if (option.media == IDM_USERSIZE)
	    strcpy(profile, "User Defined");
	else
	    strcpy(profile, option.medianame);
	profile_write_string(prf, section, "Media", profile);
	sprintf(profile, "%u %u", option.user_width, option.user_height);
	profile_write_string(prf, section, "UserSize", profile);
	sprintf(profile, "%d", option.epsf_clip);
	profile_write_string(prf, section, "EpsfClip", profile);
	sprintf(profile, "%d", option.epsf_warn);
	profile_write_string(prf, section, "EpsfWarn", profile);
	sprintf(profile, "%d", option.ignore_dsc);
	profile_write_string(prf, section, "IgnoreDSC", profile);
	sprintf(profile, "%d", option.show_bbox);
	profile_write_string(prf, section, "ShowBBox", profile);
	sprintf(profile, "%d", option.orientation - IDM_PORTRAIT);
	profile_write_string(prf, section, "Orientation", profile);
	sprintf(profile, "%d", option.swap_landscape);
	profile_write_string(prf, section, "SwapLandscape", profile);
	sprintf(profile, "%d", option.unit - IDM_UNITPT);
	profile_write_string(prf, section, "Unit", profile);
	sprintf(profile, "%d", option.quick);
	profile_write_string(prf, section, "QuickOpen", profile);
	sprintf(profile, "%d", option.safer);
	profile_write_string(prf, section, "Safer", profile);
	sprintf(profile, "%d", option.redisplay);
	profile_write_string(prf, section, "AutoRedisplay", profile);
	sprintf(profile, "%d", option.save_dir);
	profile_write_string(prf, section, "SaveLastDir", profile);
	if (option.save_dir) {
	    gs_getcwd(profile, sizeof(profile));
	    profile_write_string(prf, section, "LastDir", profile);
	}
#ifdef OLD
	profile_write_string(prf, section, "Ghostscript", option.gscommand);
#endif
	profile_write_string(prf, section, "GhostscriptEXE", option.gsexe);
	profile_write_string(prf, section, "GhostscriptInclude", option.gsinclude);
	profile_write_string(prf, section, "GhostscriptOther", option.gsother);
	switch (option.gsversion) {
	    case IDM_GS261:
		i = 261;
		break;
	    case IDM_GS333:
		i = 333;
		break;
	    case IDM_GS351:
	    default:
		i = 351;
	}
	sprintf(profile, "%d", i);
	profile_write_string(prf, section, "GhostscriptVersion", profile);
	sprintf(profile, "%d", (option.drawmethod - IDM_DRAWMENU));
	profile_write_string(prf, section, "DrawMethod", profile);
	if (option.device_name[0] != '\0') {
	    sprintf(profile,"%s,%s",option.device_name,option.device_resolution);
	    profile_write_string(prf, section, "Printer", profile);
	}
	profile_write_string(prf, section, "PrinterPort", option.printer_port);
	for (i=0; i<NUMSOUND; i++)
	    profile_write_string(prf, section, sound[i].entry, sound[i].file);
	profile_close(prf);
}

