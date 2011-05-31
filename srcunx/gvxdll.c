/* Copyright (C) 2000-2001, Ghostgum Software Pty Ltd.  All rights reserved.
  
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

/* partially implemented */
#include "gvx.h"
#include <dlfcn.h>

gint check_pstotext(gpointer data);
int gs_process_pstotext(void);

BOOL get_gs_string(int gs_revision, const char *name, char *ptr, int len)
{
    /* do nothing since we don't use a DLL */
    g_print("get_gs_string: not implemented\n");
    return FALSE;
}

void request_mutex(void)
{
#ifdef MULTITHREAD
    if (multithread)
	pthread_mutex_lock(&hmutex_ps);
#endif
}

void release_mutex(void)
{
#ifdef MULTITHREAD
    if (multithread)
	pthread_mutex_unlock(&hmutex_ps);
#endif
}

void
wait_event(void)
{
#ifdef MULTITHREAD
    if (multithread) {
	int val = 0;
	/* reset semaphore */
	sem_getvalue(&display.event, &val);
	while (val) {
	    sem_wait(&display.event);
	    sem_getvalue(&display.event, &val);
	}
	/* then wait for it to be set */
	sem_wait(&display.event);
    }
#endif
}

void
view_wait_message(VIEW *view)
{
    while (!pending.next && !pending.now && !pending.unload && !quitnow)
	gtk_main_iteration();
}

/* process one message */
int
get_message(void)
{
#ifdef MULTITHREAD
    if (multithread)
	g_print("get_message shouldn't be called when multithreading\n");
#endif
    gtk_main_iteration();
    return 0;
}

int
peek_message(void)
{
#ifdef MULTITHREAD
    if (multithread)
	g_print("peek_message shouldn't be called when multithreading\n");
#endif
    gtk_main_iteration_do(FALSE);
    return 0;
}


void
image_lock(IMAGE *img)
{
    if (debug) {
	if (img->lock_count)
	    gs_addmess("Attempted to lock image twice\n");
	img->lock_count++;
    }
#ifdef MULTITHREAD
    if (multithread) {
	pthread_mutex_lock(&img->hmutex);
    }
#endif
}

void
image_unlock(IMAGE *img)
{
    if (debug) {
	if (img->lock_count == 0)
	    gs_addmess("Attempted to unlock unlocked image\n");
	else 
	    img->lock_count--;
    }
#ifdef MULTITHREAD
    if (multithread)
	pthread_mutex_unlock(&img->hmutex);
#endif
}

/* Poll the caller for cooperative multitasking. */
/* If this function is NULL, polling is not needed */
int GSDLLCALL gsdll_poll(void *handle)
{
    if (!multithread)
	peek_message();
    if (pending.abort)
	return -100;	/* signal an error if we want to abort */
    return 0;
}


/*********************************************************************/

int 
image_preclose(IMAGE *img)
{
    if (img->cmap)
	gdk_rgb_cmap_free(img->cmap);
    img->cmap = NULL;
    if (img->rgbbuf)
	free(img->rgbbuf);
    img->rgbbuf = NULL;
    return 0;
}


/* Device is about to be resized. */
/* Resize will only occur if this function returns 0. */
int
image_presize(IMAGE *img, int width, int height, 
	int raster, unsigned int format)
{
    int color = format & DISPLAY_COLORS_MASK;
    int depth = format & DISPLAY_DEPTH_MASK;
    int alpha = format & DISPLAY_ALPHA_MASK;
    img->format_known = FALSE;
    if ( ((color == DISPLAY_COLORS_NATIVE) || 
	  (color == DISPLAY_COLORS_GRAY))
	     &&
	 ((depth == DISPLAY_DEPTH_1) ||
	  (depth == DISPLAY_DEPTH_4) ||
	  (depth == DISPLAY_DEPTH_8)) )
	img->format_known = TRUE;
    if ((color == DISPLAY_COLORS_RGB) && (depth == DISPLAY_DEPTH_8) &&
	(alpha == DISPLAY_ALPHA_NONE))
	img->format_known = TRUE;
    if (!img->format_known) {
	fprintf(stdout, "display_presize: format %d = 0x%x is unsupported\n", format, format);
	return_error(DISPLAY_ERROR);
    }
    return 0;
}

int image_update_time(IMAGE *img)
{
    struct timeval tv;
    int now_ms;
    gettimeofday(&tv, NULL);
    now_ms = (tv.tv_sec & 0xfffff) * 1000 + (tv.tv_usec / 1000);
    if (now_ms < img->tile_time)
	img->tile_time = now_ms;
    if (now_ms - img->tile_time < img->tile_interval)
	return 1;	/* too frequent */
    img->tile_time = now_ms;
    return 0;
}



int
image_size(IMAGE *img)
{
    int color, depth;

    if (img->cmap)
	gdk_rgb_cmap_free(img->cmap);
    img->cmap = NULL;
    if (img->rgbbuf)
	free(img->rgbbuf);
    img->rgbbuf = NULL;

    /* create palette and rgb buffer if needed */
    color = img->format & DISPLAY_COLORS_MASK;
    depth = img->format & DISPLAY_DEPTH_MASK;
    switch (color) {
	case DISPLAY_COLORS_NATIVE:
	    if (depth == DISPLAY_DEPTH_8) {
		/* palette of 96 colors */
		guint32 color[96];
		int i;
		int one = 255 / 3;
		for (i=0; i<96; i++) {
		    /* 0->63 = 00RRGGBB, 64->95 = 010YYYYY */
		    if (i < 64) {
			color[i] = 
			    (((i & 0x30) >> 4) * one << 16) + 	/* r */
			    (((i & 0x0c) >> 2) * one << 8) + 	/* g */
			    (i & 0x03) * one;		        /* b */
		    }
		    else {
			int val = i & 0x1f;
			val = (val << 3) + (val >> 2);
			color[i] = (val << 16) + (val << 8) + val;
		    }
		}
		img->cmap = gdk_rgb_cmap_new(color, 96);
		break;
	    }
	    else if (depth == DISPLAY_DEPTH_16) {
		/* need to convert to 24RGB */
		img->rgbbuf = (guchar *)malloc(img->width * img->height * 3);
		if (img->rgbbuf == NULL)
		    return -1;
	    }
	    else
		return_error(DISPLAY_ERROR);	/* not supported */
	case DISPLAY_COLORS_GRAY:
	    if (depth == DISPLAY_DEPTH_8)
		break;
	    else
		return_error(DISPLAY_ERROR);	/* not supported */
	case DISPLAY_COLORS_RGB:
	    if (depth == DISPLAY_DEPTH_8) {
		if (((img->format & DISPLAY_ALPHA_MASK) == DISPLAY_ALPHA_NONE)
		    && ((img->format & DISPLAY_ENDIAN_MASK) 
			== DISPLAY_BIGENDIAN))
		    break;
		else {
		    /* need to convert to 24RGB */
		    img->rgbbuf = (guchar *)malloc(img->width * img->height * 3);
		    if (img->rgbbuf == NULL)
			return_error(DISPLAY_ERROR);
		}
	    }
	    else
		return_error(DISPLAY_ERROR);	/* not supported */
	    break;
	case DISPLAY_COLORS_CMYK:
	    if (depth == DISPLAY_DEPTH_8) {
		/* need to convert to 24RGB */
		img->rgbbuf = (guchar *)malloc(img->width * img->height * 3);
		if (img->rgbbuf == NULL)
		    return_error(DISPLAY_ERROR);
	    }
	    else
		return_error(DISPLAY_ERROR);	/* not supported */
	    break;
    }

    img->separation = 0xf;	/* all layers */

    /* allow window to be resized without user control */
    fit_page_enabled = option.fit_page;
 
    return 0;
}
   
int 
image_sync(IMAGE *img)
{
    int color;
    int depth;
    int endian;
    int alpha;

    color = img->format & DISPLAY_COLORS_MASK;
    depth = img->format & DISPLAY_DEPTH_MASK;
    endian = img->format & DISPLAY_ENDIAN_MASK;
    alpha = img->format & DISPLAY_ALPHA_MASK;
		
    /* some formats need to be converted for use by GdkRgb */
    switch (color) {
	case DISPLAY_COLORS_NATIVE:
	    break;
	case DISPLAY_COLORS_RGB:
	    if ( (depth == DISPLAY_DEPTH_8) &&
		      (endian == DISPLAY_LITTLEENDIAN) ) {
		/* Windows BGR24 */
		int x, y;
		unsigned char *s, *d;
		for (y = 0; y<img->height; y++) {
		    s = img->image + y * img->raster;
		    d = img->rgbbuf + y * img->width * 3;
		    for (x=0; x<img->width; x++) {
			*d++ = s[2];	/* r */
			*d++ = s[1];	/* g */
			*d++ = s[0];	/* b */
			s += 3;
		    }
		}
	    }
	    break;
	case DISPLAY_COLORS_CMYK:
	    if (depth == DISPLAY_DEPTH_8) {
	    	/* Separations */
		int x, y;
		int cyan, magenta, yellow, black;
		unsigned char *s, *d;
		for (y = 0; y<img->height; y++) {
		    s = img->image + y * img->raster;
		    d = img->rgbbuf + y * img->width * 3;
		    for (x=0; x<img->width; x++) {
			cyan = *s++;
			magenta = *s++;
			yellow = *s++;
			black = *s++;
			if (!(img->separation & SEP_CYAN))
			    cyan = 0;
			if (!(img->separation & SEP_MAGENTA))
			    magenta = 0;
			if (!(img->separation & SEP_YELLOW))
			    yellow = 0;
			if (!(img->separation & SEP_BLACK))
			    black = 0;
			*d++ = (255-cyan)    * (255-black) / 255; /* r */
			*d++ = (255-magenta) * (255-black) / 255; /* g */
			*d++ = (255-yellow)  * (255-black) / 255; /* b */
		    }
		}
	    }
	    break;
    }
    return 0;
}
   

/******************************************************************/

/* platform dependent */
/* Load Ghostscript DLL */
int
gsdll_open(GSDLL *dll, const char *name)
{
const char *shortname;

    if (debug)
	gs_addmessf( "Trying to load %s\n", name);

    /* Try to load DLL first with given path */
    dll->hmodule = dlopen(name, RTLD_NOW);
    if (dll->hmodule == NULL) {
	/* failed */
	if (debug)
	    gs_addmessf( "Failed, %s\n", dlerror());
	/* try once more, this time on system search path */
	if ((shortname = strrchr(name, '/')) 
		== (const char *)NULL)
	    shortname = name;
	else
	    shortname++;
	if (debug)
	    gs_addmessf( "Trying to load %s\n", shortname);
	dll->hmodule = dlopen(shortname, RTLD_NOW);
	if (dll->hmodule == NULL) {
	    /* failed again */
	    if (debug)
		gs_addmessf( "Failed, %s\n", dlerror());
	}
    }
    if (dll->hmodule == NULL) {
	gs_addmessf("Failed to load %s: %s\n", name, dlerror());
	return_error(-1);
    }
    return 0;
}

/* Unload Ghostscript DLL */
int
gsdll_close(GSDLL *dll)
{
    dlclose(dll->hmodule);
    return 0;
}

void *
gsdll_sym(GSDLL *dll, const char *name)
{
    return (void *)dlsym(dll->hmodule, name);
}


/******************************************************************/

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
    char gs_prog[] = "gs";
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

    nargv[k++] = gs_string;
    nargv[k++] = gs_prog;

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
	gtk_timeout_add(1000, check_pstotext, (gpointer)((size_t)pstotext_pid));
    } 
    return 0;	/* all is well */
}

/******************************************************************/
