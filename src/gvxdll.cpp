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

/* not yet implemented */
#include "gvx.h"


BOOL get_gs_string(int gs_revision, char *name, char *ptr, int len)
{
    /* do nothing since we don't use a DLL */
    g_print("get_gs_string: not implemented\n");
    return FALSE;
}

void request_mutex(void)
{
    /* do nothing since we are not multithreaded */
}

void release_mutex(void)
{
    /* do nothing since we are not multithreaded */
}

void begin_crit_section(void)
{
    /* do nothing since we are not multithreaded */
}
void end_crit_section(void)
{
    /* do nothing since we are not multithreaded */
}

int pstotext_pid = 0;

gint check_pstotext(gpointer data)
{
    int rc = 0;
    int status = 0;

    if (pstotext_pid == 0)
	return FALSE;	/* pstotext not running, remove timer */

    /* check if pstotext has exited */
    if ( (rc = waitpid(pstotext_pid, &status, WNOHANG)) > 0 ) {
	pstotext_pid = 0;
	if (debug & DEBUG_GENERAL)
	    gs_addmess("check_pstotext: pstotext has finished\n");
	if (WIFEXITED(status)) {
	    /* normal exit */
	    if (debug & DEBUG_GENERAL)
		gs_addmessf("pstotext exit code %d\n", WEXITSTATUS(status));
	    if (WEXITSTATUS(status) != 0) {
		gs_addmess("\npstotext failed\n");
		gs_showmess();	/* show error message */
		unlink(psfile.text_name);
		psfile.text_name[0] = '\0';
	    }
	    else {
		/* normal, trigger redisplay */
		gs_addmess("\npstotext successful\n");
		if (psfile.text_extract)
		    post_img_message(WM_COMMAND, IDM_TEXTEXTRACT_SLOW);
		else
		    post_img_message(WM_COMMAND, IDM_TEXTFINDNEXT);
	    }
	}
	else {
	    /* someone killed it */
	    if (debug & DEBUG_GENERAL)
		gs_addmessf("pstotext was killed rc=%d, status=%d\n", 
		    rc, status); 
	    unlink(psfile.text_name);
	    psfile.text_name[0] = '\0';
	}
	return FALSE;	/* remove timer */
    }
    return TRUE;	/* keep checking */
}

/* start pstotext and wait for it to terminate */
int gs_process_pstotext(void)
{
    int real_orientation;
#define MAXARG 12
    char *nargv[MAXARG];
    char pstotext_string[] = "pstotext";
    char bboxes_string[] = "-bboxes";
    char cork_string[] = "-cork";
    char landscape_string[] = "-landscape";
    char landscape_other_string[] = "-landscapeOther";
    char portrait_string[] = "-portrait";
    char output_string[] = "-output";
    char gs_string[] = "-gs";
    int k = 0;

    if (pstotext_pid != 0) {
	gs_addmess("gs_process_pstotext: already busy\n");
	return 1;
    }

    gs_addmess("Extracting text using pstotext...\n");

    post_img_message(WM_GSWAIT, IDS_WAITTEXT);

    nargv[k++] = pstotext_string;
    nargv[k++] = bboxes_string;

    if (option.pstotext == IDM_PSTOTEXTCORK - IDM_PSTOTEXTMENU - 1)
        nargv[k++] = cork_string;

    switch(d_orientation(psfile.pagenum)) {
	default:
	case 0:
	    real_orientation = IDM_PORTRAIT;
	    break;
	case 1:
	    real_orientation = IDM_SEASCAPE;
	    break;
	case 2:
	    real_orientation = IDM_UPSIDEDOWN;
	    break;
	case 3:
	    real_orientation = IDM_LANDSCAPE;
	    break;
    }

    if (psfile.ispdf)
	real_orientation = pdf_orientation(psfile.pagenum);

    switch (real_orientation) {
	case IDM_LANDSCAPE:
	    nargv[k++] = landscape_string;
	    break;
	case IDM_SEASCAPE:
	    nargv[k++] = landscape_other_string;
	    break;
	case IDM_PORTRAIT:
	    nargv[k++] = portrait_string;
    }

    /* open output file */
    if ( (pstotextOutfile = gp_open_scratch_file(szScratch, psfile.text_name, "w")) == (FILE *)NULL) {
	gs_addmess("Can't open temporary file for text extraction\n");
	return 1;
    }
    fclose(pstotextOutfile);
    pstotextOutfile = NULL;
    nargv[k++] = output_string;
    nargv[k++] = psfile.text_name;

    if (strlen(option.gsdll)) {
        nargv[k++] = gs_string;
        nargv[k++] = option.gsdll;
    }

    nargv[k++] = psfile_name(&psfile);
    nargv[k++] = NULL;

    if (k >= MAXARG) {
	gs_addmess("gs_process_pstotext: Too may arguments\n");
	return 1;
    }
#undef MAXARG
    if (debug & DEBUG_GENERAL) {
	char **p = nargv;
	gs_addmessf("Running %s", *p++);
	while (*p)
	    gs_addmessf(" %s", *p++);
	gs_addmess("\n");
    }

    pstotext_pid = fork();
    if (pstotext_pid == 0) {
	/* replace child process with prog */
	if (execvp(nargv[0], nargv) == -1) {
	    fprintf(stdout, "child: failed to start \042%s\042, errno=%d\n", 
		nargv[0], errno);
	    /* exit without calling atexit functions */
	    _exit(1);
	}
    }
    else {
	/* parent */
	/* Check every second if pstotext has finished */
	gtk_timeout_add(1000, check_pstotext, (gpointer)pstotext_pid);
    } 
    return 0;	/* all is well */
}


