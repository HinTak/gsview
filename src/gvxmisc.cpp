/* Copyright (C) 2000, Ghostgum Software Pty Ltd.  All rights reserved.
  
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

/* gvxmisc.c */
/* Miscellaneous Windows GSview routines */

#include "gvx.h"

/* SetDlgItemText is a Windows API */

void
get_help(void)
{
    /* Help will be in an HTML file */
    /* Use either gnome-help-browser or netscape */
    char prog[] = "netscape";
    char remote[] = "-remote";
    char openbuf[MAXSTR];
    char buf[MAXSTR];
    char topic[MAXSTR];
    int status;
    int pid;
    char *p;
    gs_addmess("get_help: not implemented fully\n");
    strcpy(buf, szHelpName);
    strcat(buf, "#");
    load_string(nHelpTopic, topic, sizeof(topic));
    for (p = topic; *p; p++)
	if (*p == ' ')
	    *p = '_';
    strcat(buf, topic);

    if (debug & DEBUG_GENERAL)
	gs_addmessf("Display help topic %s\n", buf);

    pid = fork();
    if (pid == -1)
	return;		/* error */
    if (pid == 0) {
	/* replace child process with prog */
        char * nargv[5];
	nargv[0] = prog;
	nargv[1] = remote;
	strcpy(openbuf, "openFile(");
	strcat(openbuf, buf);
	strcat(openbuf, ")");
	nargv[2] = openbuf;
	nargv[3] = NULL;
	nargv[4] = NULL;
	if (execvp(nargv[0], nargv) == -1) {
	    fprintf(stdout, "child: failed to start \042%s\042 \042%s\042 \042%s\042 , errno=%d\n", nargv[0], nargv[1], nargv[2], errno);
	    /* exit without calling atexit functions */
	    _exit(1);
	}
    }
    else {
	/* parent */
	/* wait until netscape tells us if it succeeded */
	if (waitpid(pid, &status, 0) !=  -1) {
	    if (WIFEXITED(status)) {
		if (WEXITSTATUS(status) == 0) {
		    return;	/* existing netscape will display new topic */
		}
	    }
	}
    }

    /* netscape not running - start it now */
    exec_program(prog, buf);
}

void
delayed_message_box(int id, int icon)
{
    if (debug & DEBUG_GENERAL)
	gs_addmessf("delayed_message_box: %s\n", get_string(id));
    message_box(get_string(id), icon);
}

int
load_resource(int resource, char *str, int len)
{  
    gs_addmess("load_resource: not implemented, use load_string instead\n");
    return load_string(resource, str, len);
}


void
play_sound(int num)
{
    gs_addmess("play_sound: not implemented\n");
}


/* display or remove 'wait' message */
void
info_wait(int id)
{
    if (id)
	load_string(id, szWait, sizeof(szWait));
    else 
	szWait[0] = '\0';

    statusbar_update();
}

/* change directory and drive */
int
gs_chdir(char *dirname)
{
    return chdir(dirname);
}

char * 
gs_getcwd(char *dirname, int size)
{
    return getcwd(dirname, size);
}

const char *devices_resource =
"pdfwrite,72, 300, 600\0"
"pswrite,72, 300, 600\0"
"\0"
;

const char * epsfwarn_resource =
"/eps_warn {(Warning: EPS file must not use ) print dup == flush systemdict exch get exec} def\015\012"
"/banddevice {/banddevice eps_warn} def\015\012"
"/clear {/clear eps_warn} def\015\012"
"/cleardictstack {/cleardictstack eps_warn} def\015\012"
"/copypage {/copypage eps_warn} def\015\012"
"/erasepage {/erasepage eps_warn} def\015\012"
"/exitserver {/exitserver eps_warn} def\015\012"
"/serverdict {/serverdict eps_warn} def\015\012"
"/statusdict {/statusdict eps_warn} def\015\012"
"/framedevice {/framedevice eps_warn} def\015\012"
"/grestoreall {/grestoreall eps_warn} def\015\012"
"/initclip {/initclip eps_warn} def\015\012"
"/initgraphics {/initgraphics eps_warn} def\015\012"
"/initmatrix {/initmatrix eps_warn} def\015\012"
"/quit {/quit eps_warn} def\015\012"
"/renderbands {/renderbands eps_warn} def\015\012"
"/setglobal {/setglobal eps_warn} def\015\012"
"/setpagedevice {/setpagedevice eps_warn} def\015\012"
"/setpageparams {/setpageparams eps_warn} def\015\012"
"/setshared {/setshared eps_warn} def\015\012"
"/startjob {/startjob eps_warn} def\015\012"
"/eps_pagesize_warn {(Warning: EPS file must not set page size: ) print dup == flush pop} def\015\012"
"/11x17 {/11x17 eps_pagesize_warn} def\015\012"
"/a3 {/a3 eps_pagesize_warn} def\015\012"
"/a4 {/a4 eps_pagesize_warn} def\015\012"
"/a4small {/a4small eps_pagesize_warn} def\015\012"
"/a5 {/a5 eps_pagesize_warn} def\015\012"
"/ledger {/ledger eps_pagesize_warn} def\015\012"
"/legal {/legal eps_pagesize_warn} def\015\012"
"/letter {/letter eps_pagesize_warn} def\015\012"
"/lettersmall {/lettersmall eps_pagesize_warn} def\015\012"
"/note {/note eps_pagesize_warn} def\015\012"
"/eps_warntwo {(Warning: EPS file should be careful using ) print dup == flush systemdict exch get exec} def\015\012"
"/nulldevice {/nulldevice eps_warntwo} def\015\012"
"/setgstate {/setgstate eps_warntwo} def\015\012"
"/sethalftone {/sethalftone eps_warntwo} def\015\012"
"/setmatrix {/setmatrix eps_warntwo} def\015\012"
"/setscreen {/setscreen eps_warntwo} def\015\012"
"/settransfer {/settransfer eps_warntwo} def\015\012"
"/setcolortransfer {/setcolortransfer eps_warntwo} def\015\012"
"count /eps_count exch def\015\012"
"countdictstack /eps_countdictstack exch def\015\012"
"/gsview_eps_countcheck {count eps_count\015\012"
"  ne {(Warning: EPS file altered operand stack count\\n) print pstack flush} if\015\012"
"  countdictstack eps_countdictstack\015\012"
"  ne {(Warning: EPS file altered dictionary stack count\\n) print flush} if\015\012"
"} def\015\012"
"\0"
;

const char * viewer_resource =
"/ViewerPreProcess { systemdict begin\015\012"
"  dup length dict copy  \015\012"
"  dup /HWResolution undef\015\012"
"  dup /HWMargins undef\015\012"
"  dup /Margins undef\015\012"
"  dup /Orientation undef\015\012"
"  dup /InputAttributes undef\015\012"
"  dup /TextAlphaBits undef\015\012"
"  dup /GraphicsAlphaBits undef\015\012"
"  dup /PageSize undef\015\012"
#ifdef NOTUSED
"  GSview /ImagingBBox get null eq \015\012"
"  { GSview dup /PageSize get\015\012"
"    /Size exch put\015\012"
"    GSview /PageOffset [0 0] put\015\012"
"  }\015\012"
"  { GSview dup /ImagingBBox get\015\012"
"    [ exch\015\012"
"    dup 0 get exch dup 2 get 3 -1 roll sub exch\015\012"
"    dup 1 get exch 3 get exch sub \015\012"
"    ]\015\012"
"    /Size exch put\015\012"
"    GSview /Orientation get\015\012"
"    dup 0 eq\015\012"
"    { % portrait\015\012"
"      GSview dup /ImagingBBox get\015\012"
"      [ exch dup 0 get neg exch 1 get neg ]\015\012"
"      /PageOffset exch put\015\012"
"    }\015\012"
"    if\015\012"
"    dup 1 eq\015\012"
"    { % landscape\015\012"
"      GSview dup /ImagingBBox get\015\012"
"      [\015\012"
"        exch dup 1 get neg exch 2 get\015\012"
"	GSview /Size get\015\012"
"        0 get\015\012"
"	sub\015\012"
"        exch neg exch neg\015\012"
"      ]\015\012"
"      /PageOffset exch put\015\012"
"    }\015\012"
"    if\015\012"
"    dup 2 eq\015\012"
"    { % upside-down\015\012"
"      GSview dup /ImagingBBox get\015\012"
"      [ exch dup 2 get exch 3 get\015\012"
"	GSview /Size get\015\012"
"        dup 0 get exch 1 get\015\012"
"        exch 3 1 roll\015\012"
"	 sub 3 1 roll\015\012"
"        sub exch\015\012"
"      ]\015\012"
"      /PageOffset exch put\015\012"
"    }\015\012"
"    if\015\012"
"    dup 3 eq\015\012"
"    { % seascape\015\012"
"      GSview dup /ImagingBBox get\015\012"
"      [ exch dup 0 get exch 3 get\015\012"
"	GSview /Size get\015\012"
"        1 get sub\015\012"
"	exch neg\015\012"
"        exch neg exch neg\015\012"
"      ]\015\012"
"      /PageOffset exch put\015\012"
"    }\015\012"
"    if\015\012"
"    pop\015\012"
"  }\015\012"
"  ifelse\015\012"
#endif
"  dup /TextAlphaBits GSview /TextAlphaBits get put\015\012"
"  dup /GraphicsAlphaBits GSview /GraphicsAlphaBits get put\015\012"
#ifdef NOTUSED
"  dup /PageSize GSview /Size get put\015\012"
"  dup /HWResolution GSview /HWResolution get put\015\012"
"  dup /PageOffset GSview /PageOffset get put\015\012"
"  dup /Orientation GSview /Orientation get put\015\012"
"  GSview /ImagingBBox get null eq \015\012"
"  { % create an ImagingBBox from PageSize\015\012"
"    dup /ImagingBBox [ 0 0\015\012"
"    GSview /PageSize get\015\012"
"    dup 0 get exch 1 get ]\015\012"
"    put\015\012"
"  }\015\012"
"  { % use supplied ImagingBBox\015\012"
"    dup /ImagingBBox GSview /ImagingBBox get put\015\012"
"  }\015\012"
"  ifelse\015\012"
"  dup /Policies << /PageSize 5 >> put\015\012"
"  dup /InputAttributes <<\015\012"
"    0 << /PageSize [\015\012"
"      GSview /Orientation get\015\012"
"      dup 0 eq\015\012"
"      exch 2 eq \015\012"
"      or\015\012"
"      { % portrait or upside down\015\012"
"        GSview /Size get\015\012"
"        dup 0 get exch 1 get\015\012"
"      }\015\012"
"      { % landscape or seascape\015\012"
"        GSview /Size get\015\012"
"        dup 1 get exch 0 get\015\012"
"      }\015\012"
"      ifelse\015\012"
"    ]\015\012"
"    >>\015\012"
"  >> put\015\012"
#endif
"  dup /Policies << /PageSize 1 >> put\015\012"
"  end % pop systemdict \015\012"
/*
"(viewerpreprocess output: ) print dup { exch ==only ( ) print == } forall\015\012"
*/
"}\015\012"
"\0"
;


int
send_prolog(int resource)
{  
    if (resource == IDR_VIEWER) {
	if (debug & DEBUG_GENERAL)
	    gs_addmess("send_prolog: IDR_VIEWER\n");
	gs_execute(viewer_resource, strlen(viewer_resource));
        return 0;	/* all OK */
    }
    else if (resource == IDR_EPSFWARN) {
	if (debug & DEBUG_GENERAL)
	    gs_addmess("send_prolog: IDR_EPSFWARN\n");
	gs_execute(epsfwarn_resource, strlen(epsfwarn_resource));
        return 0;	/* all OK */
    }
 
    gs_addmess("send_prolog: unknown prolog\n");
    return 1;	/* all OK */
}

void
profile_create_section(PROFILE *prf, const char *section, int id)
{
const char *rcdata, *entry, *value;
char name[MAXSTR];
    if (id == IDR_DEVICES) {
	rcdata = devices_resource;
    }
    else {
	gs_addmessf("profile_create_section: unknown resource %d\n", id);
	return;
    }

    if (rcdata == (char *)NULL) {
	profile_close(prf);
	return;
    }
    entry = rcdata;
    while (strlen(entry)!=0) {
	for ( value = entry; 
	      (*value!='\0') && (*value!=',') && (*value!='='); 
	      value++)
	    /* nothing */;
	strncpy(name, entry, value-entry);
	name[value-entry] = '\0';
	value++;
	profile_write_string(prf, section, name, value);
	entry = value + strlen(value) + 1;
    }
}


void
SetDlgItemText(HWND hwnd, int id, const char *str)
{
    gs_addmessf("SetDlgItemText: not implemented, id=%d str=%s\n", id, str);
}

/* fork a process and run another program */
/* Used for Netscape for help and online registration */
void
exec_program(const char *prog, const char *arg)
{
    char progs[256];
    char args[256];
    char * nargv[10];
    int pid = fork();
    memset(progs, 0, sizeof(progs));
    memset(args, 0, sizeof(args));
    strncpy(progs, prog, sizeof(progs)-1);
    strncpy(args, arg, sizeof(args)-1);
    if (pid == 0) {
	/* replace child process with prog */
	nargv[0] = progs;
	nargv[1] = args;
	nargv[2] = NULL;
	nargv[3] = NULL;
	nargv[4] = NULL;
	if (execvp(nargv[0], nargv) == -1) {
	    fprintf(stdout, "child: failed to start \042%s\042 \042%s\042, errno=%d\n", prog, arg, errno);
	    /* exit without calling atexit functions */
	    _exit(1);
	}
    }
    else {
	/* parent */
	/* do nothing */
    } 
}

