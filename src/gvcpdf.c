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

/* gvcpdf.c */
/* Code to display PDF files */

#ifdef _Windows
#include "gvwin.h"
#else
#include "gvpm.h"
#endif

/* This code will work in single thread mode, but won't check message */
/* queue */

char pdf_page_tag[] = "%GSVIEW_PDF_PAGE: ";
char pdf_media_tag[] = "%GSVIEW_PDF_MEDIA: ";
char pdf_rotate_tag[] = "%GSVIEW_PDF_ROTATE: ";
char pdf_mark_tag[] = "%GSVIEW_PDF_MARK: ";
int pdf_rotate = IDM_PORTRAIT;
void pdf_add_link(PDFLINK link);

int
pdf_head(void)
{
int code;
char filename[MAXSTR];
char *p;
    pdf_rotate = IDM_PORTRAIT;
    strcpy(filename, psfile.name);
    for (p=filename; *p; p++)
	if (*p == '\\')
	    *p = '/';

/* This can replace the "\pop\n\" line below.  It will be needed 
   along if pdfshowpage is ever modified to use setpagedevice.
GSview exch /ImagingBBox exch put\n\
*/
    /* define our routine for showing a page */
    code = gs_printf("/GSview_PDFpage {\n\
pdfgetpage /Page exch store\n\
Page /MediaBox pget\n\
 { (%s) print dup == flush\n\
   boxrect\n\
   [ 2 index 5 index sub 2 index 5 index sub ]\n\
   GSview exch /PageSize exch put\n\
   pop pop neg exch neg exch\n\
   [ 3 1 roll ]\n\
   GSview exch /PageOffset exch put\n\
 }\n\
if\n\
Page /Rotate pget not { 0 } if\n\
   (%s) print dup == flush\n\
   90 idiv dup 3 eq\n\
    { pop 1 }\n\
    { dup 1 eq { pop 3 } if }\n\
   ifelse\n\
   GSview exch /Orientation exch put\n\
<< >> setpagedevice\n\
Page pdfshowpage_init pdfshowpage_finish\n\
} def\n", pdf_media_tag, pdf_rotate_tag);

    if (!code)
        code = gs_printf("userdict /pdfmark {(%%GSVIEW_PDF_MARK: ) print ==only counttomark 2 idiv { ( ) print exch ==only ( ) print ==only} repeat pop (\\n) print flush} bind put\n");

    /* put these in userdict so we can write to them later */
    if (!code)
	code = gs_printf("/Page null def\n/Page# 0 def\n/PDFSave null def\n/DSCPageCount 0 def\n");
    /* open PDF support dictionaries */
    if (!code)
        code = gs_printf("GS_PDF_ProcSet begin\npdfdict begin\n");
    /* open PDF file */
    if (!code)
	code = gs_printf("(%s) (r) file pdfopen begin\n", filename);
    if (!code)
	code = gs_printf("/FirstPage where { pop FirstPage } { 1 } ifelse\n ");
    if (!code)
	code = gs_printf("/LastPage where { pop LastPage } { pdfpagecount } ifelse\n");
    /* flush stdout and then send PDF page marker to stdout */
    /* we capture the page numbers in the DLL callback */
    if (!code)
	code = gs_printf("flush (%s) print exch =only ( ) print =only (\n) print flush\n", pdf_page_tag);
    /* page numbers should now be captured and PSDOC structure created */
    return code;
}


/* Create a PSDOC structure for PDF file */
int 
pdf_makedoc(int first, int last)
{
int numpages = last-first+1;
int i;
char buf[16];
PSDOC *doc;
char filename[MAXSTR];
char textname[MAXSTR];
    strcpy(filename, psfile.name);	/* remember filename */
    strcpy(textname, psfile.text_name);	/* remember filename */
    psfile.text_name[0] = '\0';		/* hide filename so it doesn't get deleted */
    psfile_free(&psfile);
    strcpy(psfile.name, filename);
    strcpy(psfile.text_name, textname);
    psfile.doc = (PSDOC *) malloc(sizeof(PSDOC));
    if (psfile.doc == (PSDOC *)NULL)
	return FALSE;
    doc = psfile.doc;
    memset(doc, 0, sizeof(PSDOC));
    if (last != 0) {
        doc->pages = (struct page *) calloc(numpages, sizeof(struct page));
	for (i=0; i<numpages; i++) {
	    sprintf(buf, "%d", i+first);
	    doc->pages[i].label = malloc(strlen(buf)+1);
	    if (doc->pages[i].label == NULL) {
		psfree(doc);
		psfile.doc = (PSDOC *)NULL;
		return FALSE;
	    }
	    strcpy(doc->pages[i].label, buf);
	}
	doc->numpages = numpages;
	psfile.page_list.select = (BOOL *)malloc( doc->numpages * sizeof(BOOL) );
    }
    psfile.ispdf = TRUE;
    return TRUE;
}

int
pdf_trailer(void)
{
    pdf_free_link();
    return gs_printf("currentdict pdfclose\nend\nend\nend\n");
}

int
pdf_page(int pagenum)
{
    pdf_rotate = IDM_PORTRAIT;
    pdf_free_link();
    ignore_sync = TRUE;		/* ignore next GSDLL_SYNC */
    return gs_printf("%d GSview_PDFpage\n", pagenum);
}

/* Get next key/value pair */ 
/* Return pointer to just beyond value */
char*
pdf_parse_mark(char *str, char **key, char **value)
{
char *p;
int level;
    p = str;
    while (*p) {
	if (*p == '/')
	    break;
	p++;
    }
    if (*p == '\0')
	return NULL;
    if (*p == ']')
	return NULL;
    *key = p;
    while (*p) {	/* search for a space */
	if (*p == ' ') {
	    *p = '\0';
	    p++;
	    while (*p) {  /* skip over remaining spaces */
		if (*p != ' ')
		    break;
		p++;
	    }
	    break;
	}
	p++;
    }
    *value = p;
    /* the following isn't robust */
    /* strings within arrays containing ] will confuse it */
    if (*p == '[') {  /* skip over arrays */
	p++;
	level = 1;
	while (*p) {
	    if (*p == '[')
		level++;
	    if (*p == ']')
		level--;
	    p++;
	    if (level == 0)
		break;
	}
	if (*p) {
	    *p = '\0';
	    p++;
	}
    }
    else if (*p == '(') {  /* skip over strings */
	p++;
	level = 1;
	while (*p) {
	    if (*p == '\\') {
		p++;
		if (*p == '\0')
		    p--;	/* backup, \ at EOL */
	    }
	    else {
	        if (*p == '(')
		    level++;
		if (*p == ')')
		    level--;
	    }
	    p++;
	    if (level == 0)
		break;
	}
	if (*p) {
	    *p = '\0';
	    p++;
	}
    }
    else {
	while (*p) {
	    if (*p == ' ') {
		*p = '\0';
		p++;
		while (*p) {  /* skip over remaining spaces */
		    if (*p != ' ')
			break;
		    p++;
		}
		break;
	    }
	    p++;
	}
    }
    return p;
}

/* Check stdout for tag giving page range */
int
pdf_checktag(LPSTR str, int len)
{
int i, first, last;
char buf[MAXSTR];
float x0, x1, y0, y1;
int rotate;
#if defined(__WIN32__) || defined(OS2)
char *line = str;
#else
    char line[MAXSTR];
    if (len > MAXSTR)
        lstrcpyn(line, str, MAXSTR-1);
    else
        lstrcpy(line, str);
#endif
    if ( (len < 1) || (*line != '%') )
	return FALSE;
    if (psfile.ispdf && (len > sizeof(pdf_page_tag)) &&
	(strncmp(line, pdf_page_tag, strlen(pdf_page_tag)) == 0) ) {
	strncpy(buf, line, len);
	buf[len] = '\0';
	i = sscanf(buf+strlen(pdf_page_tag), "%d %d", &first, &last);
	if (i==2) {
	    if (debug)
		gs_addmess("Found GSVIEW_PDF_PAGE tag\n");
	    pdf_makedoc(first, last);
	    return TRUE;
	}
    }
    if (psfile.ispdf && (len > sizeof(pdf_media_tag)) &&
	(strncmp(line, pdf_media_tag, strlen(pdf_media_tag)) == 0) ) {
	strncpy(buf, line, len);
	buf[len] = '\0';
	i = sscanf(buf+strlen(pdf_media_tag), "[%f %f %f %f]", &x0, &y0, &x1, &y1);
	if (i==4) {
	    if (debug)
		gs_addmess("Found GSVIEW_PDF_MEDIA tag\n");
	    display.width  = (unsigned int)(x1  * display.xdpi / 72.0 + 0.5);
	    display.height = (unsigned int)(y1  * display.ydpi / 72.0 + 0.5);
	    return TRUE;
	}
    }
    if (psfile.ispdf && (len > sizeof(pdf_rotate_tag)) &&
	(strncmp(line, pdf_rotate_tag, strlen(pdf_rotate_tag)) == 0) ) {
	strncpy(buf, line, len);
	buf[len] = '\0';
	i = sscanf(buf+strlen(pdf_rotate_tag), "%d", &rotate);
	if (i==1) {
	    if (debug)
		gs_addmess("Found GSVIEW_PDF_ROTATE tag\n");
	    switch (rotate) {
		case 90:
		    pdf_rotate = IDM_LANDSCAPE;
		    break;
		case 180:
		    pdf_rotate = IDM_UPSIDEDOWN;
		    break;
		case 270:
		    pdf_rotate = IDM_SEASCAPE;
		    break;
		default:
		    pdf_rotate = IDM_PORTRAIT;
		    break;
	    }
	    check_menu_item(IDM_ORIENTMENU, option.orientation, FALSE);
	    option.orientation = pdf_rotate;
	    check_menu_item(IDM_ORIENTMENU, option.orientation, TRUE);
/* should disable orientation menu here */
	    return TRUE;
	}
    }
    if ((len > sizeof(pdf_mark_tag)) &&
	(strncmp(line, pdf_mark_tag, strlen(pdf_mark_tag)) == 0) ) {
	char *p, *key, *value;
	PDFLINK link;
	BOOL possibly_a_link = FALSE;
	BOOL is_a_link = FALSE;
	int code = FALSE;
	memset(&link, 0, sizeof(link));
	link.border_width = 1;
	strncpy(buf, line, len);
	buf[len] = '\0';
	p = buf+strlen(pdf_mark_tag);
        if (strncmp(p, "/LNK ", 5) == 0) {
	    p += 5;
	    possibly_a_link = TRUE;
	    is_a_link = TRUE;
	}
        if (strncmp(p, "/ANN ", 5) == 0) {
	    p += 5;
	    possibly_a_link = TRUE;
	}
	if (possibly_a_link) {
	    p = pdf_parse_mark(p, &key, &value);
	    while (p) {
		if (strcmp(key, "/Rect") == 0) {
		    if (sscanf(value+1, "%d %d %d %d", 
			&link.bbox.llx, &link.bbox.lly, &link.bbox.urx, &link.bbox.ury) == 4)
			code = TRUE;
		}
		if (strcmp(key, "/Page") == 0) {
		    if (strcmp(value, "/Next") == 0) {
			link.page = psfile.pagenum+1;
		    }
		    else if (strcmp(value, "/Prev") == 0) {
			link.page = psfile.pagenum-1;
		    }
		    else if (sscanf(value, "%d", &link.page) == 1) {
			code = TRUE;
		    }
		}
		if (strcmp(key, "/Border") == 0) {
		    if (sscanf(value+1, "%f %f %f", &link.border_xr, &link.border_yr, &link.border_width) == 3)
			code = TRUE;
		}
		if (strcmp(key, "/Color") == 0) {
		    if (sscanf(value+1, "%f %f %f", &link.colour_red, &link.colour_green, &link.colour_blue) == 3) {
			code = TRUE;
			link.colour_valid = TRUE;
		    }
		}
		if (strcmp(key, "/Subtype") == 0) {
		    if (strcmp(value, "/Link") == 0) {
			is_a_link = TRUE;
			code = TRUE;
		    }
		}
		if (strcmp(key, "/Action") == 0) {
		    if (strcmp(value, "/GoTo") == 0) {
			is_a_link = TRUE;
			code = TRUE;
		    }
		}
		p = pdf_parse_mark(p, &key, &value);
	    }
	    if (code && is_a_link)
		pdf_add_link(link);
	    return code;
	}
    }
    return FALSE;
}

int
pdf_orientation(void)
{
    return pdf_rotate;
}

/* Create DSC file to print selected pages of PDF file */
int
pdf_extract(FILE *f)
{
char filename[MAXSTR];
char *p;
int i, page, pages;
PSDOC *doc = psfile.doc;
    /* convert \ to / in filename */
    strcpy(filename, psfile.name);
    for (p=filename; *p; p++)
	if (*p == '\\')
	    *p = '/';

    if (doc->numpages == 0) {
	/* unknown number of pages */
	char buf[MAXSTR];
	load_string(IDS_PDFEXTRACTALL, buf, sizeof(buf));
	if (message_box(buf, MB_ICONASTERISK | MB_OKCANCEL) == IDCANCEL)
	    return FALSE;
	fputs("%!\n(", f);
	fputs(filename, f);
	fputs(") run\n", f);
	return TRUE;
    }

    /* count pages */
    pages = 0;
    for (i=0; i< doc->numpages; i++) {
	if (psfile.page_list.select[i]) pages++;
    }

    /* Send header */
    fputs("%!PS-Adobe 3.0\r\n%%Creator: GSview\r\n", f);
    fputs("%%Title: Ghostscript wrapper for ", f);
    fputs(filename, f);
    fputs("\r\n%%Pages: ", f);
    fprintf(f, "%d\r\n", pages);
    fputs("%%EndComments\r\n", f);
    fputs("%%BeginProlog\r\n", f);
    fputs("\
/Page null def\r\n\
/Page# 0 def\r\n\
/PDFSave null def\r\n\
/DSCPageCount 0 def\r\n\
/DoPDFPage {dup /Page# exch store pdfgetpage pdfshowpage} def\r\n\
GS_PDF_ProcSet begin\r\n\
pdfdict begin\r\n\
", f);
    fputs("%%EndProlog\r\n", f);
    fputs("%%BeginSetup\r\n", f);
    fprintf(f, "(%s) (r) file pdfopen begin\r\n", filename);
    fputs("%%EndSetup\r\n", f);

    /* Send each page */
    page = 1;
    for (i = 0; i < doc->numpages; i++) {
	if (psfile.page_list.select[i])  {
	    fprintf(f, "%%%%Page: %s %d\r\n", doc->pages[i].label, page);
	    fprintf(f, "%d DoPDFPage\r\n", i+1);
	    page++;
	}
    }

    /* Send trailer */
    fputs("%%Trailer\r\n", f);
    fputs("currentdict pdfclose\r\nend\r\nend\r\nend\r\n%%EOF\r\n", f);
    return TRUE;
}


/* Alternative convert PDF to PS */
BOOL
gsview_pdf2ps_common(char *psname, char *optname, char *output)
{
char buf[MAXSTR];
FILE *optfile;
FILE *pcfile;
char *p;
    /*  ASSUMES psfile.file is valid */

    /* create temporary file containing pages to print */
    psname[0] = '\0';
    if ( (pcfile = gp_open_scratch_file(szScratch, psname, "wb")) == (FILE *)NULL) {
	gserror(IDS_NOTEMP, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	play_sound(SOUND_ERROR);
	return FALSE;
    }

    fprintf(pcfile, "PSFile (w) file /PSout exch def\n");

    if (!pdf_extract(pcfile)) {
	fclose(pcfile);
	return FALSE;
    }

#ifdef UNUSED
    fprintf(pcfile, "(");
    for (p=psfile.name; *p != '\0'; p++)
	if (*p == '\\')
	    fputc('/',pcfile);
	    /* fputc('\\',pcfile); */
	else
	    fputc(*p,pcfile);
    fprintf(pcfile, ") run\n");
#endif

    fclose(pcfile);

    /* create options file */
    optname[0] = '\0';
    if ( (optfile = gp_open_scratch_file(szScratch, optname, "w")) == (FILE *)NULL) {
	    play_sound(SOUND_ERROR);
	    return FALSE;
    }
    fprintf(optfile, "-I\042%s\042\n", option.gsinclude);
/*
    fprintf(optfile, "-q\n");
*/
    fprintf(optfile, "-dNOPAUSE\n");
    fprintf(optfile, "-dNODISPLAY\n");
/* Can't use SAFER because we need to open PSFile
   Rely on PDF files not usually being able to open files
    if (option.safer)
	fprintf(optfile, "-dSAFER\n");
*/
    if (option.pdf2ps & OPTION_PDF2PS_BINARYOK)
        fprintf(optfile, "-dPSBinaryOK\n");
    if (option.pdf2ps & OPTION_PDF2PS_LEVEL1)
	fprintf(optfile, "-dPSLevel1\n");
    if (option.pdf2ps & OPTION_PDF2PS_NOPROCSET)
	fprintf(optfile, "-dPSNoProcSet\n");

    fprintf(optfile, "-sPSFile=\042");
    for (p=output; *p != '\0'; p++)
	if (*p == '\\')
	    /* fputc('/',optfile); */
	    fputc('\\',optfile);
	else
	    fputc(*p,optfile);
    fputc('\042',optfile);
    fputc('\n',optfile);

    p = option.gsother;
    while ((p = gs_argnext(p, buf)) != NULL)
        fprintf(optfile, "%s\n", buf);

    fclose(optfile);
    return TRUE;
}

PDFLINK *pdf_link_head;

void
pdf_free_link(void)
{
PDFLINK *next, *thislink;
    request_mutex();
    thislink = pdf_link_head;
    while (thislink != (PDFLINK *)NULL) {
        next = thislink->next;
	free(thislink);
	thislink = next;
    }
    pdf_link_head = (PDFLINK *)NULL;
    release_mutex();
}

void
pdf_add_link(PDFLINK newlink)
{
PDFLINK *thislink;
    request_mutex();
    thislink = (PDFLINK *)malloc(sizeof(PDFLINK));
    if (thislink) {
	*thislink = newlink;
	thislink->next = pdf_link_head;
	pdf_link_head = thislink;
    }
    release_mutex();
}

BOOL
pdf_get_link(int index, PDFLINK *link)
{
int i;
PDFLINK *thislink = pdf_link_head;
int code = FALSE;
    request_mutex();
    for (i=0; (thislink!=NULL) && i < index; i++)
	thislink = thislink->next;
    if (thislink) {
	*link = *thislink;
	code = TRUE;
    }
    release_mutex();
    return code;
}

BOOL
is_link(float x, float y, PDFLINK *link)
{
int i = 0;
    while ( pdf_get_link(i, link) ) {
	i++;
	if (   (link->bbox.llx < x) && (link->bbox.urx > x)
	    && (link->bbox.lly < y) && (link->bbox.ury > y) ) {
	    /* found link */
	    return TRUE;
	}
    }
    return FALSE;
}

