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

/* gvcdsc.c */
/* DSC scanning module */

/* tasks */
/* %%BeginPaperSize: */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#ifdef EPSTOOL
#include "epstool.h"
#else
#define DWORD unsigned long	/* correct for OS/2 and MS-Windows */
#define WORD unsigned short 
#include "gvcdsc.h"
#endif

/* known media types */
PSMEDIA paper_size[34] = {
	/* taken from gs_statd.ps */
	/* which says that the following are defined in Adobe Documentation */
	{"letter", 612, 792},
	{"lettersmall", 612, 792},
	{"note", 540, 720},
	{"legal", 612, 1008},
	{"11x17", 792, 1224},
	{"ledger", 1224, 792},
	{"a3", 842, 1190},
	{"a4", 595, 842},
	{"a4small", 595, 842},
	{"b5", 501, 709},
	/* extra media types from gs_statd.ps */
	{"a0", 2380, 3368},	/* ISO standard */
	{"a1", 1684, 2380},
	{"a2", 1190, 1684},
	{"a5", 421, 595},
	{"a6", 297, 421},
	{"a7", 210, 421},
	{"a8", 148, 210},
	{"a9", 105, 148},
	{"a10", 74, 105},
	{"b0", 2836, 4008},
	{"b1", 2004, 2836},
	{"b2", 1418, 2004},
	{"b3", 1002, 1418},
	{"b4", 501, 709},
	{"archE", 2592, 3456},	/* US CAD */
	{"archD", 1728, 2592},
	{"archC", 1296, 1728},
	{"archB", 864, 1296},
	{"archA", 648, 864},
	{"flsa", 612, 936}, 	/* US foolscap */
	{"flse", 612, 936}, 	/* European foolscap */
	{"halfletter", 396, 612},
	{"tabloid", 792, 1224},	/* 11x17 */
	{NULL, 0, 0}
};

#define TRUE 1
#define FALSE 0

/* globals */
static int debug;

/* print an error message */
/* we do it this way so we can write output into a message box under */
/* windowing environment such as MS-Windows or OS/2 PM */
static int
eprintf(char *fmt, ...)
{
int count;
va_list args;
	va_start(args, fmt);
#ifdef COMMANDLINE
	count = vfprintf(stderr, fmt, args);
	count += fprintf(stderr, "\n");
#else
	{
	    char buf[256];
	    count = vsprintf(buf, fmt, args);
	    pserror(buf);	/* pserror must be provided by application linking with gvcdsc.c */
	}
#endif
	va_end(args);
	return count;
}

#ifdef DEBUG
void *mymalloc(int length)
{
void *p;
	if (debug)
	    eprintf("mymalloc: length=%d, ",length);
	p = malloc(length);
	if (debug)
	    eprintf("pointer=%p\n",p);
	return p;
}
#define malloc(len) mymalloc(len)
#endif

static int
nomemory(char *fname, int sline)
{
	eprintf("gvcdsc.c: out of memory at %s:%d", fname, sline);
	return FALSE;
}

/* Get next line, skipping over data and included documents */
static int
getline(PSDOC *psdoc)
{
	psdoc->position = ftell(psdoc->f);
	if (psdoc->enddoseps && (psdoc->position >= psdoc->enddoseps)) {
	    psdoc->line[0] = '\0';
	    return FALSE;	/* have reached end of DOSEPS ps section */
	}
	if (fgets(psdoc->line, PSLINELENGTH, psdoc->f)) {
	    psdoc->linecount++;
	    /* skip over binary sections */
	    if (strncmp(psdoc->line, "%%BeginBinary:",14)==0) {
		long count = 0;
		int read;
		char buf[1024];
		if (sscanf(psdoc->line+14, "%ld", &count) != 1)
		    count = 0;
		while (count) {
		    read = fread(buf, 1, sizeof(buf), psdoc->f);
		    count -= read;
		    if (read == 0)
		    	count = 0;
		}
	    }
	    if (strncmp(psdoc->line, "%%BeginData:",12)==0) {
	    	long count;
	    	int read;
	    	char buf[PSLINELENGTH];
	    	if (sscanf(psdoc->line+12, "%ld %*s %s", &count, buf) != 2)
	    	    count = 0;
	        if (strncmp(buf, "Lines", 5) == 0) {
	            while (count) {
	            	count--;
			do {	/* handle antisocial PostScript with excessively long lines */
			    if (fgets(buf, sizeof(buf), psdoc->f) == (char *)NULL) {
			        count = 0;
				buf[0] = '\0';
			    }
			} while ( (strlen(buf) == sizeof(buf)-1) &&
			          (buf[sizeof(buf)-2] != '\n') );
			psdoc->linecount++;
	            }
	        }
	        else {
		    while (count) {
			read = fread(buf, 1, sizeof(buf), psdoc->f);
			count -= read;
			if (read == 0)
			    count = 0;
		    }
	        }
	    }
	    /* skip over included documents */
	    if (strncmp(psdoc->line, "%%BeginDocument:",16)==0) {
		while (strncmp(psdoc->line,"%%EndDocument",13)!=0) {
		    if (!getline(psdoc))
			return FALSE;
		}
	    }
	    return TRUE;
	}
	return FALSE;
}

/* remove trailing \r and \n */
static void
stripline(char *line)
{
char *p;
	if ( (p=strchr(line, '\r')) != (char *)NULL )
	    *p = '\0';
	if ( (p=strchr(line, '\n')) != (char *)NULL )
	    *p = '\0';
}

/* copy backslashed character to dest */
/* return pointer to last character of backslash sequence */
char *
dsc_escape(char *source, char *dest)
{
	source++;	/* skip over backslash */
	if (isdigit(*source)) {
	    *dest = (char)(*source-'0');
	    source++;
	    if (isdigit(*source)) {
	        *dest = (char)((*dest<<3) + (*source-'0'));
	        source++;
		if (isdigit(*source)) {
		    *dest = (char)((*dest<<3) + (*source-'0'));
		}
	    }
	}
	else if (*source == 'n') {
	    *dest = '\n';
	}
	else if (*source == 'r') {
	    *dest = '\r';
	}
	else if (*source == 't') {
	    *dest = '\t';
	}
	else {
	    *dest = *source;
	}
	return source;
}

/* copy text from s to d, dealing with (), "", \octal */
/* return pointer to next text item in n */
/* return number of characters copied */
int
dsc_text(char *s, char *d, char **n)
{
int paren = 0;
int count = 0;
	*d = '\0';
	/* skip over leading white space */
	while (*s && ((*s==' ') || (*s=='\t')))
	   s++;
	if (*s == '(') {
	    /* delimited by () */
	    s++;
	    while (*s && ((*s!=')') || paren!=0)) {
		if (*s == '\\') {
		    s = dsc_escape(s, d);
		}
		else {
		    if (*s == '(')
		        paren++;
		    if (*s == ')')
		        paren--;
		    *d = *s;
		}
		s++;
		d++;
		count++;
	    }
	}
	else if (*s == '"') {
	    /* delimited by "" */
	    s++;
	    while (*s && *s!='"') {
		if (*s == '\\') {
		    s = dsc_escape(s, d);
		}
		else {
		    *d = *s;
		}
		s++;
		d++;
		count++;
	    }
	}
	else {
	    while (*s && *s!=' ' && *s!='\t') {
		if (*s == '\\') {
		    s = dsc_escape(s, d);
		}
		else {
		    *d = *s;
		}
		s++;
		d++;
		count++;
	    }
	}
	*d = '\0';
	if (*s != '\0') {
	    *s++ = '\0';
	    while (*s && ((*s==' ') || (*s=='\t')))
	       s++;
	}
	*n = s;
	return count;
}

static PSDOC *
cleanup(PSDOC *psdoc)
{
	if (psdoc) {
	    if (psdoc->firstline)
		free(psdoc->firstline);
	    if (psdoc->title)
		free(psdoc->title);
	    if (psdoc->date)
		free(psdoc->date);
	    if (psdoc->page_list) {
		PSPAGE  *pspage, *nextpage;
		for (pspage = psdoc->page_list; pspage != (PSPAGE *)NULL; 
			pspage = nextpage) {
		    nextpage = pspage->next;
		    free(pspage);
		}
	    }
	    if (psdoc->pages)
		free(psdoc->pages);
	    if (psdoc->doseps)
		free(psdoc->doseps);
	    if (psdoc->document_media) {
		PSMEDIA  *media, *nextmedia;
		for (media = psdoc->document_media; media != (PSMEDIA *)NULL; 
			media = nextmedia) {
	            if (media->name)
		        free(media->name);
		    nextmedia = media->next;
		    free(media);
		}
	    }
	    if (psdoc->default_page_media) {
	        if (psdoc->default_page_media->name)
		    free(psdoc->default_page_media->name);
		free(psdoc->default_page_media);
	    }
	    free(psdoc);
	}
	return (PSDOC *)NULL;
}

static int
dsc_nextsection(PSDOC *psdoc)
{
	if (strncmp(psdoc->line, "%%BeginPreview:", 15) == 0) {
	    return TRUE;
	}
	else if (strncmp(psdoc->line, "%%BeginDefaults", 15) == 0) {
	    return TRUE;
	}
	else if (strncmp(psdoc->line, "%%BeginProlog", 13) == 0) {
	    return TRUE;
	}
	else if (strncmp(psdoc->line, "%%BeginSetup", 12) == 0) {
	    return TRUE;
	}
	else if (strncmp(psdoc->line, "%%Page:", 7) == 0) {
	    return TRUE;
	}
	else if (strncmp(psdoc->line, "%%Trailer", 9) == 0) {
	    return TRUE;
	}
	return FALSE;
}

static int
dsc_document_media(PSDOC *psdoc)
{
	float width, height;
	PSMEDIA *media;
	char name[PSLINELENGTH];
	char *n;
	dsc_text(psdoc->line+16, name, &n);
	media = malloc(sizeof(PSMEDIA));
	if (media == (PSMEDIA *)NULL)
	    return nomemory(__FILE__, __LINE__);
	media->width = 0;
	media->height = 0;
	media->next = NULL;
	media->name = malloc(strlen(name)+1);
	if (media->name == (char *)NULL)
	    return nomemory(__FILE__, __LINE__);
	strcpy(media->name, name);
	if (sscanf(n, "%f %f", &width, &height) == 2) {
	    media->width  = (int)(width+0.5);
	    media->height = (int)(height+0.5);
	}
	else
	    eprintf("Incorrect %%%%DocumentMedia at line %d", psdoc->linecount);
	psdoc->document_media = media;
	while (getline(psdoc)) {
	    if (strncmp(psdoc->line, "%%+", 3) != 0)
		break; /* not a continuation line */
	    dsc_text(psdoc->line+3, name, &n);
	    media->next = malloc(sizeof(PSMEDIA));
	    if (media->next == (PSMEDIA *)NULL)
		return nomemory(__FILE__, __LINE__);
	    media = media->next;
	    media->width = 0;
	    media->height = 0;
	    media->next = NULL;
	    media->name = malloc(strlen(name)+1);
	    if (media->name == (char *)NULL)
		return nomemory(__FILE__, __LINE__);
	    strcpy(media->name, name);
	    if (sscanf(n, "%f %f", &width, &height) == 2) {
	        media->width  = (int)(width+0.5);
	        media->height = (int)(height+0.5);
	    }
	    else
		eprintf("Incorrect %%%%DocumentMedia at line %d", psdoc->linecount);
	}
	return TRUE;
}

static int
dsc_comments(PSDOC *psdoc)
{
char *p;
int readahead = 0;
	stripline(psdoc->line);
	psdoc->begincomments = psdoc->position;
	if (strncmp(psdoc->line, "\004%!PS-Adobe-",11)==0) {
	    psdoc->ctrld = TRUE;	/* for nasty MS-Windows output */
	    psdoc->begincomments++;
	}
	if ( (strncmp(psdoc->line, "%!PS-Adobe-",11)!=0) && !psdoc->ctrld )
	    return FALSE;	/* not DSC */
	psdoc->firstline = malloc(strlen(psdoc->line)+1);
	if (!psdoc->firstline)
	    return nomemory(__FILE__, __LINE__);
	strcpy(psdoc->firstline, psdoc->line);
	if (strstr(psdoc->line, "EPSF"))
	    psdoc->epsf = TRUE;
	psdoc->endcomments = ftell(psdoc->f);
	while (readahead || getline(psdoc)) {
	    readahead = FALSE;
	    if (strncmp(psdoc->line, "%%EndComments", 13) == 0) {
		getline(psdoc);
		psdoc->endcomments = psdoc->position;
		break;
	    }
	    else if (strncmp(psdoc->line, "%%Begin", 7) == 0) {
		break;	/* start of next section */
	    }
	    else if (strncmp(psdoc->line, "%%", 2) != 0) {
		if (strlen(psdoc->line) < 2)
		   break;
		if (psdoc->line[0] != '%')
		   break;
		if ((psdoc->line[1] == ' ') || (psdoc->line[1] == '\t'))
		   break;
	    }
	    else if (strncmp(psdoc->line, "%%BoundingBox:", 14) == 0) {
		if (strstr(psdoc->line + 14, "(atend)") != (char *)NULL) {
			psdoc->bbox.valid = ATEND;
		}
		else {
		    if (sscanf(psdoc->line + 14, "%d %d %d %d", &psdoc->bbox.llx, &psdoc->bbox.lly,
			&psdoc->bbox.urx, &psdoc->bbox.ury) == 4) {
			psdoc->bbox.valid = TRUE;
		    }
		    else {
			float llx, lly, urx, ury;
		        if (sscanf(psdoc->line + 14, "%f %f %f %f", &llx, &lly, &urx, &ury) == 4) {
			    psdoc->bbox.llx = (int)llx;
			    psdoc->bbox.lly = (int)lly;
			    psdoc->bbox.urx = (int)urx;
			    psdoc->bbox.ury = (int)ury;
			    psdoc->bbox.valid = TRUE;
		            eprintf("Invalid %%%%BoundingBox on line %d.  Coodinates must be integers.", psdoc->linecount);
		        }
			else {
		            eprintf("Invalid %%%%BoundingBox on line %d", psdoc->linecount);
		            return FALSE;
		    	}
		    }
		}
	    }
	    else if (strncmp(psdoc->line, "%%Pages:", 8) == 0) {
		if (strstr(psdoc->line + 8, "(atend)") != (char *)NULL)
		    psdoc->dscpages = ATEND;
		else {
		    if (sscanf(psdoc->line + 8, "%d", &psdoc->dscpages) != 1) {
		        eprintf("Invalid %%%%Pages on line %d", psdoc->linecount);
		        return FALSE;
		    }
		}
	    }
	    else if (strncmp(psdoc->line, "%%Title:", 8) == 0) {
		stripline(psdoc->line);
		p = psdoc->line+8;
		while (*p && (*p==' '))
		   p++;
		psdoc->title = malloc(strlen(p)+1);
		if (!psdoc->title)
		    return nomemory(__FILE__, __LINE__);
		strcpy(psdoc->title, p);
	    }
	    else if (strncmp(psdoc->line, "%%CreationDate:", 15) == 0) {
		stripline(psdoc->line);
		p = psdoc->line+15;
		while (*p && (*p==' '))
		   p++;
		psdoc->date = malloc(strlen(p)+1);
		if (!psdoc->date)
		    return nomemory(__FILE__, __LINE__);
		strcpy(psdoc->date, p);
	    }
	    else if (strncmp(psdoc->line, "%%PageOrder:", 12) == 0) {
		stripline(psdoc->line);
		if ( (p = strtok(psdoc->line+12, " \t")) != (char *)NULL ) {
		    if (strcmp(p, "Ascend") == 0)
			psdoc->pageorder = ASCEND;
		    else if (strcmp(p, "Descend") == 0)
			psdoc->pageorder = DESCEND;
		    else if (strcmp(p, "Special") == 0)
			psdoc->pageorder = SPECIAL;
		    else if (strcmp(p, "(atend)") == 0)
			psdoc->pageorder = ATEND;
		    else
		        eprintf("Unknown %%%%PageOrder on line %d", psdoc->linecount);
		}
		else
		        eprintf("Unknown %%%%PageOrder on line %d", psdoc->linecount);
	    }
	    else if (strncmp(psdoc->line, "%%Orientation:", 14) == 0) {
		stripline(psdoc->line);
		if ( (p = strtok(psdoc->line+14, " \t")) != (char *)NULL ) {
		    if (strcmp(p, "Portrait") == 0)
			psdoc->orientation = PORTRAIT;
		    else if (strcmp(p, "Landscape") == 0)
			psdoc->orientation = LANDSCAPE;
		    else if (strcmp(p, "(atend)") == 0)
			psdoc->orientation = ATEND;
		    else
		        eprintf("Unknown %%%%Orientation on line %d", psdoc->linecount);
		}
		else
		        eprintf("Unknown %%%%Orientation on line %d", psdoc->linecount);
	    }
	    else if (strncmp(psdoc->line, "%%PaperSize:", 12) == 0) {
		/* DSC 2.0 comment */
		stripline(psdoc->line);
		if ( (p = strtok(psdoc->line+12, " \t")) != (char *)NULL ) {
		    if (strcmp(p, "(atend)") != 0) {
		      PSMEDIA *pm;
		      for (pm = paper_size; pm->name; pm++)
			if (stricmp(pm->name, p) == 0) {
			    psdoc->default_page_media = malloc(sizeof(PSMEDIA));
			    if (!psdoc->default_page_media)
				return nomemory(__FILE__, __LINE__);
			    psdoc->default_page_media->name = malloc(strlen(pm->name)+1);
			    if (!psdoc->default_page_media->name)
				return nomemory(__FILE__, __LINE__);
			    strcpy(psdoc->default_page_media->name, pm->name);
			    psdoc->default_page_media->width = pm->width;
			    psdoc->default_page_media->height = pm->height;
			    break;	/* exit after match */
			}
		        if (psdoc->default_page_media->name == (char *)NULL)
		            eprintf("Unknown %%%%PaperSize on line %d", psdoc->linecount);
		    }
		}
		else
		        eprintf("Unknown %%%%PaperSize on line %d", psdoc->linecount);
	    }
	    else if (strncmp(psdoc->line, "%%DocumentMedia:", 16) == 0) {
		stripline(psdoc->line);
		if (strncmp(psdoc->line+17, "(atend)", 7) != 0) {
		    if (!dsc_document_media(psdoc))
	                return nomemory(__FILE__, __LINE__);
		    readahead = TRUE;
		}
	    }
	    psdoc->endcomments = ftell(psdoc->f);
	}
	return TRUE;
}

static int
dsc_preview(PSDOC *psdoc)
{
        psdoc->beginpreview = psdoc->endpreview = psdoc->position;
	while ((psdoc->line[0]=='\r') || (psdoc->line[0]=='\n'))
	    getline(psdoc);	/* skip blank lines */
	if (strncmp(psdoc->line, "%%BeginPreview:", 15) != 0) {
            /* psdoc->beginpreview = psdoc->endpreview = 0; */
	    return TRUE;
	}
	while (getline(psdoc)) {
	    if (strncmp(psdoc->line, "%%EndPreview", 12) == 0) {
		getline(psdoc);
		psdoc->endpreview = psdoc->position;
		return TRUE;
	    }
	}
	psdoc->endpreview = ftell(psdoc->f);
	return TRUE;
}

static int
dsc_defaults(PSDOC *psdoc)
{
        psdoc->begindefaults = psdoc->enddefaults = psdoc->position;
	while ((psdoc->line[0]=='\r') || (psdoc->line[0]=='\n'))
	    getline(psdoc);	/* skip blank lines */
	if (strncmp(psdoc->line, "%%BeginDefaults", 15) != 0) {
            psdoc->begindefaults = psdoc->enddefaults = 0;
	    return TRUE;
	}
	while (getline(psdoc)) {
	    if (strncmp(psdoc->line, "%%EndDefaults", 13) == 0) {
		psdoc->enddefaults = ftell(psdoc->f);
		getline(psdoc);
		return TRUE;
	    }
	    else if (strncmp(psdoc->line, "%%PageMedia:", 12) == 0) {
		PSMEDIA *media;
		char name[PSLINELENGTH];
		char *n;
		stripline(psdoc->line);
		dsc_text(psdoc->line+12, name, &n);
		media = malloc(sizeof(PSMEDIA));
		if (media == (PSMEDIA *)NULL)
		    return nomemory(__FILE__, __LINE__);
		media->width = 0;
		media->height = 0;
		media->next = NULL;
		media->name = malloc(strlen(name)+1);
		if (media->name == (char *)NULL)
		    return nomemory(__FILE__, __LINE__);
		strcpy(media->name, name);
		psdoc->default_page_media = media;
		/* set width and height from %%DocumentMedia: */
		for (media=psdoc->document_media; media; media=media->next) {
		    if (strcmp(psdoc->default_page_media->name, media->name) == 0) {
			psdoc->default_page_media->width  = media->width;
			psdoc->default_page_media->height = media->height;
			break;
		    }
		}
	    }
	}
	psdoc->enddefaults = ftell(psdoc->f);
	return TRUE;
}

static int
dsc_prolog(PSDOC *psdoc)
{
        psdoc->beginprolog = psdoc->endprolog = ftell(psdoc->f);
	if (strncmp(psdoc->line, "%%BeginProlog", 13) != 0) {
	    if (dsc_nextsection(psdoc))
		return TRUE;
	}
        psdoc->beginprolog = psdoc->endprolog = psdoc->position;
	/* %%BeginProlog may not be present */
	if (strncmp(psdoc->line, "%%EndProlog", 11) == 0) {
	    getline(psdoc);
	    psdoc->endprolog = psdoc->position;
	    return TRUE;
	}
	while (getline(psdoc)) {
	    if (strncmp(psdoc->line, "%%BeginProlog", 13) == 0) {
		psdoc->beginprolog = psdoc->position;
	    }
	    else if (strncmp(psdoc->line, "%%EndProlog", 11) == 0) {
		getline(psdoc);
		psdoc->endprolog = psdoc->position;
		return TRUE;
	    }
	    else if (dsc_nextsection(psdoc)) {
		psdoc->endprolog = psdoc->position;
		return TRUE;
	    }
	}
	psdoc->endprolog = ftell(psdoc->f);
	return TRUE;
}

static int
dsc_setup(PSDOC *psdoc)
{
        psdoc->beginsetup = psdoc->endsetup = psdoc->position;
	while ((psdoc->line[0]=='\r') || (psdoc->line[0]=='\n'))
	    getline(psdoc);	/* skip blank lines */
	if (strncmp(psdoc->line, "%%BeginSetup", 12) != 0)
		return TRUE;
	while (getline(psdoc)) {
	    if (strncmp(psdoc->line, "%%BeginSetup", 12) == 0) {
	        psdoc->beginsetup = psdoc->position;
	    }
	    else if (strncmp(psdoc->line, "%%EndSetup", 10) == 0) {
		getline(psdoc);
		psdoc->endsetup = psdoc->position;
		return TRUE;
	    }
	    else if (dsc_nextsection(psdoc)) {
		psdoc->endsetup = psdoc->position;
		return TRUE;
	    }
/* should look for %%BeginPaperSize: */
	}
	psdoc->endsetup = ftell(psdoc->f);
	return TRUE;
}

static PSPAGE *
dsc_page(PSDOC *psdoc, PSPAGE *pspage, long position)
{
char *n;
char buf[PSLINELENGTH];
	if (pspage != (PSPAGE *)NULL) {
	    pspage->end = position;
	    /* add a new page */
	    pspage->next = malloc(sizeof(PSPAGE));
	    if (pspage->next == (PSPAGE *)NULL)
	        return (PSPAGE *)nomemory(__FILE__, __LINE__);
	    pspage = pspage->next;
	}
	else {
	    /* add initial page */
	    psdoc->page_list = malloc(sizeof(PSPAGE));
	    if (psdoc->page_list == (PSPAGE *)NULL)
		return (PSPAGE *)nomemory(__FILE__, __LINE__);
	    pspage = psdoc->page_list;
	}
	pspage->begin = position;
	pspage->end = position;
	pspage->label = (char *)NULL;
	pspage->ordinal = 0;
	pspage->next = (PSPAGE *)NULL;
	psdoc->numpages++;
	dsc_text(psdoc->line+7, buf, &n);
	pspage->label = malloc(strlen(buf)+1);
	if (pspage->label == (char *)NULL)
	    return (PSPAGE *)nomemory(__FILE__, __LINE__);
	strcpy(pspage->label, buf);
	if (*n == 0) {
	    eprintf("Unknown %%%%Page on line %d", psdoc->linecount);
	}
	pspage->ordinal = atoi(n);
	return pspage;
}

static int
dsc_pages(PSDOC *psdoc)
{
long position;
PSPAGE *pspage;
	pspage = NULL;
	psdoc->numpages = 0;
	position = ftell(psdoc->f);
	if (strncmp(psdoc->line, "%%Page:", 7) == 0) {
	    pspage = dsc_page(psdoc, pspage, psdoc->endsetup);
	    if (pspage == (PSPAGE *)NULL)
	        return FALSE;
	}

	while (getline(psdoc)) {
	    if (strncmp(psdoc->line, "%%Page:", 7) == 0) {
	        pspage = dsc_page(psdoc, pspage, position);
		if (pspage == (PSPAGE *)NULL)
		    return FALSE;
	    }
	    else if (strncmp(psdoc->line, "%%Trailer", 9) == 0) {
		if (pspage)
		    pspage->end = position;
		psdoc->begintrailer = position;
		return TRUE;
	    }
	    position = ftell(psdoc->f);
	}

	position = ftell(psdoc->f);
	if (pspage)
	    pspage->end = position;
	psdoc->begintrailer = position;
	return TRUE;
}

/* fix up things not known until after trailer processed */
static dsc_fixup(PSDOC *psdoc)
{
PSPAGE *pspage;
int i;
	if (psdoc->numpages) {
	    psdoc->pages = malloc(sizeof(PSPAGE) * psdoc->numpages);
	    if (psdoc->pages == (PSPAGE *)NULL)
	        return nomemory(__FILE__, __LINE__);
	    for (pspage = psdoc->page_list, i=0; pspage != (PSPAGE *)NULL; 
		    pspage = pspage->next, i++) {
	        psdoc->pages[i].begin = pspage->begin;
	        psdoc->pages[i].end = pspage->end;
	        psdoc->pages[i].label = pspage->label;
	        psdoc->pages[i].ordinal = pspage->ordinal;
	    }
	}
	if (psdoc->dscpages == ATEND) {
	    eprintf("Missing %%%%Pages comment");
	    psdoc->dscpages = psdoc->numpages;
	}
	if (psdoc->dscpages && (psdoc->numpages != psdoc->dscpages)) {
	    eprintf("%%%%Pages comment does not match number of pages");
	    psdoc->dscpages = psdoc->numpages;
	}
	if (psdoc->pageorder == ATEND) {
	    eprintf("Missing %%%%PageOrder comment");
	    psdoc->pageorder = NONE;
	}
	if (psdoc->orientation == ATEND) {
	    eprintf("Missing %%%%Orientation comment");
	    psdoc->orientation = NONE;
	}
	if ( (psdoc->default_page_media == NULL) && 
	     (psdoc->document_media != NULL) ) {
	    /* make default_page_media = first media of %%DocumentMedia */
	    psdoc->default_page_media = malloc(sizeof(PSMEDIA));
	    if (!psdoc->default_page_media)
		return nomemory(__FILE__, __LINE__);
	    psdoc->default_page_media->name = malloc(strlen(psdoc->document_media->name)+1);
	    if (!psdoc->default_page_media->name)
		return nomemory(__FILE__, __LINE__);
	    strcpy(psdoc->default_page_media->name, psdoc->document_media->name);
	    psdoc->default_page_media->width  = psdoc->document_media->width;
	    psdoc->default_page_media->height = psdoc->document_media->height;
	}
	if ( (psdoc->default_page_media != NULL) &&
	         ((psdoc->default_page_media->width==0) ||
	          (psdoc->default_page_media->height==0)) ) {
	    /* %%DocumentMedia occurred after %%PageMedia */
	    PSMEDIA *media;
	    for (media=psdoc->document_media; media; media=media->next) {
		if (strcmp(psdoc->default_page_media->name, media->name) == 0) {
		    psdoc->default_page_media->width  = media->width;
		    psdoc->default_page_media->height = media->height;
		    break;
		}
	    }
	}
	return TRUE;
}

static int
dsc_trailer(PSDOC *psdoc)
{
int readahead = FALSE;
char *p;
	/* psdoc->begintrailer has already been set */
	while (readahead || getline(psdoc)) {
	    readahead = FALSE;
	    if (strncmp(psdoc->line, "%%EOF", 5) == 0) {
		break;
	    }
	    else if (strncmp(psdoc->line, "%%BoundingBox:", 14) == 0) {
		if ((psdoc->bbox.valid == ATEND) || (psdoc->bbox.valid == FALSE)) {
		    if (sscanf(psdoc->line + 14, "%d %d %d %d", &psdoc->bbox.llx, &psdoc->bbox.lly,
		        &psdoc->bbox.urx, &psdoc->bbox.ury) == 4)
			psdoc->bbox.valid = TRUE;
		    else {
		        eprintf("Invalid %%%%BoundingBox on line %d", psdoc->linecount);
		        return FALSE;
		    }
		}
		else
		    eprintf("Duplicate %%%%BoundingBox on line %d", psdoc->linecount);
	    }
	    else if (strncmp(psdoc->line, "%%Pages:", 8) == 0) {
		if ( (psdoc->dscpages == ATEND) || (psdoc->dscpages == NONE) ) {
		    if (sscanf(psdoc->line + 8, "%d", &psdoc->dscpages) != 1) {
		        eprintf("Invalid %%%%Pages on line %d", psdoc->linecount);
		        return FALSE;
		    }
		}
	    }
	    else if (strncmp(psdoc->line, "%%PageOrder:", 12) == 0) {
		stripline(psdoc->line);
		if (psdoc->pageorder == ATEND) {
		    if ( (p = strtok(psdoc->line+12, " \t")) != (char *)NULL ) {
			if (strcmp(p, "Ascend") == 0)
			    psdoc->pageorder = ASCEND;
			else if (strcmp(p, "Descend") == 0)
			    psdoc->pageorder = DESCEND;
			else if (strcmp(p, "Special") == 0)
			    psdoc->pageorder = SPECIAL;
			else
			    eprintf("Unknown %%%%PageOrder on line %d", psdoc->linecount);
		    }
		    else
			    eprintf("Unknown %%%%PageOrder on line %d", psdoc->linecount);
		}
		else
			    eprintf("Duplicate %%%%PageOrder on line %d", psdoc->linecount);
	    }
	    else if (strncmp(psdoc->line, "%%Orientation:", 14) == 0) {
		stripline(psdoc->line);
		if (psdoc->orientation == ATEND) {
		    if ( (p = strtok(psdoc->line+14, " \t")) != (char *)NULL ) {
			if (strcmp(p, "Portrait") == 0)
			    psdoc->orientation = PORTRAIT;
			else if (strcmp(p, "Landscape") == 0)
			    psdoc->orientation = LANDSCAPE;
			else
			    eprintf("Unknown %%%%Orientation on line %d", psdoc->linecount);
		    }
		    else
			    eprintf("Unknown %%%%Orientation on line %d", psdoc->linecount);
		}
		else
		    eprintf("Duplicate %%%%Orientation on line %d", psdoc->linecount);
	    }
	    else if (strncmp(psdoc->line, "%%PaperSize:", 12) == 0) {
		/* DSC 2.0 comment */
		stripline(psdoc->line);
		if ( (p = strtok(psdoc->line+12, " \t")) != (char *)NULL ) {
		    PSMEDIA *pm;
		    for (pm = paper_size; pm->name; pm++)
			if (stricmp(pm->name, p) == 0) {
			    psdoc->default_page_media = malloc(sizeof(PSMEDIA));
			    if (!psdoc->default_page_media)
				return nomemory(__FILE__, __LINE__);
			    psdoc->default_page_media->name = malloc(strlen(pm->name)+1);
			    if (!psdoc->default_page_media->name)
				return nomemory(__FILE__, __LINE__);
			    strcpy(psdoc->default_page_media->name, pm->name);
			    psdoc->default_page_media->width = pm->width;
			    psdoc->default_page_media->height = pm->height;
			}
		    if (psdoc->default_page_media->name == (char *)NULL)
		        eprintf("Unknown %%%%PaperSize on line %d", psdoc->linecount);
		}
		else
		        eprintf("Unknown %%%%PaperSize on line %d", psdoc->linecount);
	    }
	    else if (strncmp(psdoc->line, "%%DocumentMedia:", 16) == 0) {
		if (psdoc->document_media == (PSMEDIA *)NULL) {
		    if (!dsc_document_media(psdoc))
	                return nomemory(__FILE__, __LINE__);
		    readahead = TRUE;
		}
	    }
	}
	psdoc->endtrailer = ftell(psdoc->f);
	return TRUE;
}

static unsigned long dsc_arch = 0x00000001;

/* change byte order if architecture is big-endian */
DWORD
reorder_dword(DWORD val)
{
    if (*((char *)(&dsc_arch)))
        return val;	/* little endian machine */
    else
	return ((val&0xff) << 24) | ((val&0xff00) << 8)
             | ((val&0xff0000L) >> 8) | ((val>>24)&0xff);
}

/* change byte order if architecture is big-endian */
WORD
reorder_word(WORD val)
{
    if (*((char *)(&dsc_arch)))
        return val;	/* little endian machine */
    else
	return (WORD) ((val&0xff) << 8) | ((val&0xff00) >> 8);
}

/* DOS EPS header reading */
static int
dsc_read_doseps(PSDOC *psdoc)
{
DOSEPS doseps;
	fread(doseps.id, 1, 4, psdoc->f);
	if (! ((doseps.id[0]==0xc5) && (doseps.id[1]==0xd0) 
		&& (doseps.id[2]==0xd3) && (doseps.id[3]==0xc6)) ) {
		/* id is "EPSF" with bit 7 set */
	    rewind(psdoc->f);
	    return TRUE; 	/* OK */
	}
	fread(&doseps.ps_begin,    4, 1, psdoc->f);	/* PS offset */
	doseps.ps_begin = (unsigned long)reorder_dword(doseps.ps_begin);
	fread(&doseps.ps_length,   4, 1, psdoc->f);	/* PS length */
	doseps.ps_length = (unsigned long)reorder_dword(doseps.ps_length);
	fread(&doseps.mf_begin,    4, 1, psdoc->f);	/* Metafile offset */
	doseps.mf_begin = (unsigned long)reorder_dword(doseps.mf_begin);
	fread(&doseps.mf_length,   4, 1, psdoc->f);	/* Metafile length */
	doseps.mf_length = (unsigned long)reorder_dword(doseps.mf_length);
	fread(&doseps.tiff_begin,  4, 1, psdoc->f);	/* TIFF offset */
	doseps.tiff_begin = (unsigned long)reorder_dword(doseps.tiff_begin);
	fread(&doseps.tiff_length, 4, 1, psdoc->f);	/* TIFF length */
	doseps.tiff_length = (unsigned long)reorder_dword(doseps.tiff_length);
	fread(&doseps.checksum,    2, 1, psdoc->f);
        doseps.checksum = (unsigned short)reorder_word(doseps.checksum);
	fseek(psdoc->f, doseps.ps_begin, SEEK_SET);	/* seek to PS section */
	psdoc->doseps = malloc(sizeof(DOSEPS));
	if (psdoc->doseps == (DOSEPS *)NULL)
	    return nomemory(__FILE__, __LINE__);
	*psdoc->doseps = doseps;
	psdoc->enddoseps = doseps.ps_begin + doseps.ps_length;
	return TRUE;
}

/*********************************************************/
/* global functions */

/* file must have been opened in binary mode */
PSDOC *
dsc_scan_file(FILE *f)
{
PSDOC *psdoc;
char line[PSLINELENGTH];
	if ((psdoc = malloc(sizeof(PSDOC))) == (PSDOC *)NULL) {
	    nomemory(__FILE__, __LINE__);
	    return (PSDOC *)NULL;
	}
	memset(psdoc, 0, sizeof(PSDOC));
	/* initialise internal scratch items */
	psdoc->f = f;
	psdoc->position = 0;
	psdoc->line = line;
	psdoc->linecount = 0;
	psdoc->enddoseps = 0;
	rewind(psdoc->f);
	if (!dsc_read_doseps(psdoc))
	   return cleanup(psdoc);
	if (!getline(psdoc))
	   return cleanup(psdoc);
	/* since some of these scanning routines may read one line past */
	/* the end of the section, all scanning routines must do so */
	if (!dsc_comments(psdoc))
	   return cleanup(psdoc);
	if (!dsc_preview(psdoc))
	   return cleanup(psdoc);
	if (!dsc_defaults(psdoc))
	   return cleanup(psdoc);
	if (!dsc_prolog(psdoc))
	   return cleanup(psdoc);
	if (!dsc_setup(psdoc))
	   return cleanup(psdoc);
	if (!dsc_pages(psdoc))
	   return cleanup(psdoc);
	if (!dsc_trailer(psdoc))
	   return cleanup(psdoc);
	if (!dsc_fixup(psdoc))
	   return cleanup(psdoc);
	/* clear internal items */
	psdoc->f = NULL;
	psdoc->position = 0;
	psdoc->line = NULL;
	psdoc->linecount = 0;
	psdoc->enddoseps = 0;
	return psdoc;
}

void
dsc_scan_clean(PSDOC *psdoc)
{
	cleanup(psdoc);
}

void
dsc_scan_debug(int flag)
{
	debug = flag;
}

/* copy from file f to file tofile, between begin and end */
/* if comment given, stop when comment found */
char *
dsc_copy(FILE *f, FILE *tofile, long begin, long end, char *comment)
{
char linebuf[PSLINELENGTH];
	if (begin >= 0)
	    fseek(f, begin, SEEK_SET);
	while (ftell(f) < end) {
	    fgets(linebuf, sizeof(linebuf), f);
	    if (comment && (strncmp(linebuf, comment, sizeof(comment))==0)) {
		char *p;
		if ((p = malloc(strlen(linebuf)+1)) == (char *)NULL) {
		    nomemory(__FILE__, __LINE__);
		    return (char *)NULL;
		}
		strcpy(p, linebuf);
		return p;
	    }
	    fputs(linebuf, tofile);
	    /* copy binary sections */
	    if (strncmp(linebuf, "%%BeginBinary:",14)==0) {
		long count = 0;
		int read;
		char buf[1024];
		if (sscanf(linebuf+14, "%ld", &count) != 1)
		    count = 0;
		while (count) {
		    read = fread(buf, 1, sizeof(buf), f);
		    count -= read;
		    if (read == 0)
		    	count = 0;
		    else
			fwrite(buf, 1, read, tofile);
		}
	    }
	    if (strncmp(linebuf, "%%BeginData:",12)==0) {
	    	long count;
	    	int read;
	    	char buf[PSLINELENGTH];
	    	if (sscanf(linebuf+12, "%ld %*s %s", &count, buf) != 2)
	    	    count = 0;
	        if (strncmp(buf, "Lines", 5) == 0) {
	            while (count) {
	            	count--;
			do {	/* handle antisocial PostScript with excessively long lines */
			    if (fgets(buf, sizeof(buf), f) == (char *)NULL) {
			        count = 0;
				buf[0] = '\0';
			    }
			    else
			        fputs(buf, tofile);
			} while ( (strlen(buf) == sizeof(buf)-1) &&
			          (buf[sizeof(buf)-2] != '\n') );
	            }
	        }
	        else {
		    while (count) {
			read = fread(buf, 1, sizeof(buf), f);
			count -= read;
			if (read == 0)
			    count = 0;
		        else
			    fwrite(buf, 1, read, tofile);
		    }
	        }
	    }
	}
	return (char *)NULL;
}
