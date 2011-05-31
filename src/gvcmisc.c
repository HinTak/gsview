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

/* gvcmisc.c */
/* Miscellaneous GSview routines common to Windows and PM */

#ifdef _Windows
#include "gvwin.h"
#else
#include "gvpm.h"
#endif

/* display error message and exit Ghostscript message */
void 
error_message(char *str)
{
	message_box(str, MB_ICONHAND);
	post_img_message(WM_CLOSE, 0);
}

void
info_init(HWND hwnd)
{
    PSDOC *doc = psfile.doc;
    char buf[MAXSTR];
    char *p;
    int n;
    if (psfile.name[0] != '\0') {
	SetDlgItemText(hwnd, INFO_FILE, psfile.name);
	if (doc) {
	    p = buf;
	    *p = '\0';
	    if (psfile.gzip) {
		strcpy(p, "gzip ");
	        p += strlen(p);
	    }
	    if (psfile.ctrld)
		load_string(IDS_CTRLD, p, sizeof(buf));
	    if (psfile.pjl)
		load_string(IDS_PJL, p, sizeof(buf));
	    p += strlen(p);
	    if (psfile.ispdf) {
		load_string(IDS_PDF, p, sizeof(buf)-strlen(buf));
	    }
	    else if (doc->epsf) {
		switch (psfile.preview) {
		    case IDS_EPSI:
		      load_string(IDS_EPSI, p, sizeof(buf)-strlen(buf));
		      break;
		    case IDS_EPST:
		      load_string(IDS_EPST, p, sizeof(buf)-strlen(buf));
		      break;
		    case IDS_EPSW:
		      load_string(IDS_EPSW, p, sizeof(buf)-strlen(buf));
		      break;
		    default:
		      load_string(IDS_EPSF, p, sizeof(buf)-strlen(buf));
		}
	    }
	    else
		load_string(IDS_DSC, p, sizeof(buf)-strlen(buf));
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
	sprintf(buf, "%d x %d", bitmap.width, bitmap.height);
	SetDlgItemText(hwnd, INFO_BITMAP, buf);
    }
    else {
	load_string(IDS_NOFILE, buf, sizeof(buf));
	SetDlgItemText(hwnd, INFO_FILE, buf);
    }
}

/* read settings fron INI file */
void
read_profile(char *ininame)
{
int i, j;
char profile[MAXSTR];
char *section = INISECTION;
char *device_ptr;
PROFILE *prf;
	prf = profile_open(ininame);
	profile_read_string(prf, section, "Configured", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
	    option.configured = i;
	else
	    option.configured = FALSE;
	profile_read_string(prf, section, "GSversion", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
	    option.gsversion = i;
	else
	    option.gsversion = GS_REVISION;
	if ( (option.gsversion < GS_REVISION_MIN) ||
	     (option.gsversion > GS_REVISION_MAX) )
	    option.gsversion = GS_REVISION;
	profile_read_string(prf, section, "Version", "", profile, sizeof(profile));
	if (strcmp(profile, GSVIEW_VERSION)!=0)
	    option.configured = FALSE;
	profile_read_string(prf, section, "Language", "", profile, sizeof(profile));
	if (strcmp(profile, "de") == 0)
	    option.language = IDM_LANGDE;
	else
	    option.language = IDM_LANGEN;
	profile_read_string(prf, section, "GhostscriptDLL", "", profile, sizeof(profile));
	if (profile[0] != '\0')	/* don't copy a default - assume already set */
		strcpy(option.gsdll, profile);
	profile_read_string(prf, section, "GhostscriptInclude", "", profile, sizeof(profile));
	if (profile[0] != '\0')	/* don't copy a default - assume already set */
		strcpy(option.gsinclude, profile);
	profile_read_string(prf, section, "GhostscriptOther", "", profile, sizeof(profile));
	if (profile[0] != '\0')	/* don't copy a default - assume already set */
		strcpy(option.gsother, profile);
	profile_read_string(prf, section, "Origin", "", profile, sizeof(profile));
	option.img_origin.x = option.img_origin.y = CW_USEDEFAULT;
	if (sscanf(profile,"%d %d", &i, &j) == 2) {
		option.img_origin.x = i;
		option.img_origin.y = j;
	}
	profile_read_string(prf, section, "Size", "", profile, sizeof(profile));
	option.img_size.x = option.img_size.y = CW_USEDEFAULT;
	if (sscanf(profile,"%d %d", &i, &j) == 2) {
		option.img_size.x = i;
		option.img_size.y = j;
	}
	if ((option.img_size.x<32) || (option.img_size.y<32))
		option.img_size.x = option.img_size.y = CW_USEDEFAULT;
	profile_read_string(prf, section, "Maximized", "", profile, sizeof(profile));
	option.img_max = 0;
	if (sscanf(profile,"%d", &i) == 1)
		option.img_max = i;
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
	profile_read_string(prf, section, "TextAlphaBits", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.alpha_text = i;
        if (option.alpha_text <= 0)
		option.alpha_text = 1;
	profile_read_string(prf, section, "GraphicsAlphaBits", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.alpha_graphics = i;
        if (option.alpha_graphics <= 0)
		option.alpha_graphics = 1;
	profile_read_string(prf, section, "Media", "", profile, sizeof(profile));
	if (strlen(profile)!=0)
	    strncpy(option.medianame, profile, sizeof(option.medianame));
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
	profile_read_string(prf, section, "AutoOrientation", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.auto_orientation = i;
	profile_read_string(prf, section, "Orientation", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.orientation = i+IDM_PORTRAIT;
	profile_read_string(prf, section, "SwapLandscape", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.swap_landscape = i;
	profile_read_string(prf, section, "Unit", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.unit = i+IDM_UNITPT;
	profile_read_string(prf, section, "PStoText", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.pstotext = i;
	profile_read_string(prf, section, "QuickOpen", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.quick_open = i;
	profile_read_string(prf, section, "Safer", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.safer = i;
	profile_read_string(prf, section, "AutoRedisplay", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.redisplay = i;
	profile_read_string(prf, section, "PDF2PS", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.pdf2ps = i;
	profile_read_string(prf, section, "AutoBoundingBox", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.auto_bbox = i;
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
	profile_read_string(prf, section, "PrinterQueue", "", profile, sizeof(option.printer_queue)-1);
	if (profile[0] != '\0')
	    strcpy(option.printer_queue, profile);
	profile_read_string(prf, section, "PrintToFile", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.print_to_file = i;
	profile_read_string(prf, section, "PostScriptPrinter", "", profile, sizeof(profile));
	if (sscanf(profile,"%d", &i) == 1)
		option.psprinter = i;
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
	sprintf(profile, "%d", (int)option.gsversion);
	profile_write_string(prf, section, "GSversion", profile);
	switch (option.language) {
	    case IDM_LANGDE:
		strcpy(profile, "de");
		break;
	    default:
		strcpy(profile, "en");
	}
	profile_write_string(prf, section, "Language", profile);
	sprintf(profile, "%d", (int)option.configured);
	profile_write_string(prf, section, "Configured", profile);
	profile_write_string(prf, section, "GhostscriptDLL", option.gsdll);
	profile_write_string(prf, section, "GhostscriptInclude", option.gsinclude);
	profile_write_string(prf, section, "GhostscriptOther", option.gsother);
	sprintf(profile, "%d %d", (int)option.img_origin.x, (int)option.img_origin.y);
	profile_write_string(prf, section, "Origin", profile);
	sprintf(profile, "%d %d", (int)option.img_size.x, (int)option.img_size.y);
	profile_write_string(prf, section, "Size", profile);
	sprintf(profile, "%d", (int)option.img_max);
	profile_write_string(prf, section, "Maximized", profile);
	sprintf(profile, "%d", (int)option.settings);
	profile_write_string(prf, section, "SaveSettings", profile);
	sprintf(profile, "%d", (int)option.button_show);
	profile_write_string(prf, section, "ButtonBar", profile);
	sprintf(profile, "%d", (int)option.fit_page);
	profile_write_string(prf, section, "FitWindowToPage", profile);
	sprintf(profile, "%g %g", option.xdpi, option.ydpi);
	profile_write_string(prf, section, "Resolution", profile);
	sprintf(profile, "%g %g", option.zoom_xdpi, option.zoom_ydpi);
	profile_write_string(prf, section, "ZoomResolution", profile);
	sprintf(profile, "%d", option.depth);
	profile_write_string(prf, section, "Depth", profile);
	sprintf(profile, "%d", option.alpha_text);
	profile_write_string(prf, section, "TextAlphaBits", profile);
	sprintf(profile, "%d", option.alpha_graphics);
	profile_write_string(prf, section, "GraphicsAlphaBits", profile);
	if (option.media == IDM_USERSIZE)
	    strcpy(profile, MEDIA_USERDEFINED);
	else
	    strcpy(profile, option.medianame);
	profile_write_string(prf, section, "Media", profile);
	sprintf(profile, "%u %u", option.user_width, option.user_height);
	profile_write_string(prf, section, "UserSize", profile);
	sprintf(profile, "%d", (int)option.epsf_clip);
	profile_write_string(prf, section, "EpsfClip", profile);
	sprintf(profile, "%d", (int)option.epsf_warn);
	profile_write_string(prf, section, "EpsfWarn", profile);
	sprintf(profile, "%d", (int)option.ignore_dsc);
	profile_write_string(prf, section, "IgnoreDSC", profile);
	sprintf(profile, "%d", (int)option.show_bbox);
	profile_write_string(prf, section, "ShowBBox", profile);
	sprintf(profile, "%d", (int)option.auto_orientation);
	profile_write_string(prf, section, "AutoOrientation", profile);
	sprintf(profile, "%d", option.orientation - IDM_PORTRAIT);
	profile_write_string(prf, section, "Orientation", profile);
	sprintf(profile, "%d", (int)option.swap_landscape);
	profile_write_string(prf, section, "SwapLandscape", profile);
	sprintf(profile, "%d", option.unit - IDM_UNITPT);
	profile_write_string(prf, section, "Unit", profile);
	sprintf(profile, "%d", (int)option.quick_open);
	profile_write_string(prf, section, "QuickOpen", profile);
	sprintf(profile, "%d", (int)option.pstotext);
	profile_write_string(prf, section, "PStoText", profile);
	sprintf(profile, "%d", (int)option.safer);
	profile_write_string(prf, section, "Safer", profile);
	sprintf(profile, "%d", (int)option.redisplay);
	profile_write_string(prf, section, "AutoRedisplay", profile);
	sprintf(profile, "%d", (int)option.pdf2ps);
	profile_write_string(prf, section, "PDF2PS", profile);
	sprintf(profile, "%d", (int)option.auto_bbox);
	profile_write_string(prf, section, "AutoBoundingBox", profile);
	sprintf(profile, "%d", (int)option.save_dir);
	profile_write_string(prf, section, "SaveLastDir", profile);
	if (option.save_dir) {
	    gs_getcwd(profile, sizeof(profile));
	    profile_write_string(prf, section, "LastDir", profile);
	}
	sprintf(profile, "%d", (option.drawmethod - IDM_DRAWMENU));
	profile_write_string(prf, section, "DrawMethod", profile);
	if (option.device_name[0] != '\0') {
	    sprintf(profile,"%s,%s",option.device_name,option.device_resolution);
	    profile_write_string(prf, section, "Printer", profile);
	}
	profile_write_string(prf, section, "PrinterPort", option.printer_port);
	profile_write_string(prf, section, "PrinterQueue", option.printer_queue);
	sprintf(profile, "%d", (int)option.print_to_file);
	profile_write_string(prf, section, "PrintToFile", profile);
	sprintf(profile, "%d", (int)option.psprinter);
	profile_write_string(prf, section, "PostScriptPrinter", profile);
	for (i=0; i<NUMSOUND; i++)
	    profile_write_string(prf, section, sound[i].entry, sound[i].file);
	profile_close(prf);
}

