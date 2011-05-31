/* Copyright (C) 1993-1998, Ghostgum Software Pty Ltd.  All rights reserved.
  
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
char pdf_crop_tag[] = "%GSVIEW_PDF_CROP: ";
char pdf_rotate_tag[] = "%GSVIEW_PDF_ROTATE: ";
char pdf_mark_tag[] = "%GSVIEW_PDF_MARK: ";
int pdf_rotate = IDM_PORTRAIT;
void pdf_add_link(PDFLINK link);

int
pdf_head(void)
{
int code;
char filename[MAXSTR];
char *p, *s;
    pdf_rotate = IDM_PORTRAIT;
    p = filename;
    for (s = psfile_name(&psfile); *s; s++) {
	*p++ = *s;
	if (*s == '\\')
	    *p++ = '\\';
    }
    *p = '\0';

    /* Define our routine for preparing to show a page. */
    /* This writes out some tags which we capture in the */
    /* callback to obtain the page size and orientation. */
    code = gs_printf("/GSview_PDFpage {\n\
pdfgetpage /Page exch store\n\
Page /MediaBox pget\n\
 { (%s) print == flush\n\
 }\n\
if\n\
Page /CropBox pget\n\
 { (%s) print == flush\n\
 }\n\
if\n\
Page /Rotate pget not { 0 } if\n\
   (%s) print == flush\n\
} def\n", pdf_media_tag, pdf_crop_tag, pdf_rotate_tag);


    /* we will need to update this pdfmark code to handle */
    /* embedded dictionaries */
    if (!code)
        code = gs_printf("userdict /pdfmark {(%s) print ==only counttomark 2 idiv { ( ) print exch ==only ( ) print ==only} repeat pop (\\n) print flush} bind put\n", pdf_mark_tag);

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
    /* page numbers should now be captured and CDSC object created */
    return code;
}


/* Create a CDSC object for PDF file */
int 
pdf_makedoc(int first, int last)
{
CDSC *dsc;
char filename[MAXSTR];
char textname[MAXSTR];
char tname[MAXSTR];
    strcpy(filename, psfile.name);	/* remember filename */
    strcpy(textname, psfile.text_name);	/* remember filename */
    strcpy(tname, psfile.tname);	/* remember filename */
    psfile.text_name[0] = '\0';		/* hide filename so it doesn't get deleted */
    psfile.tname[0] = '\0';		/* hide filename so it doesn't get deleted */
    psfile_free(&psfile);
    strcpy(psfile.name, filename);
    strcpy(psfile.text_name, textname);
    strcpy(psfile.tname, tname);
    psfile.dsc = new CDSC;
    if (psfile.dsc == (CDSC *)NULL)
	return FALSE;
    dsc = psfile.dsc;
    dsc->SetDebug(gs_addmess);
    if (last != 0) {
	if (dsc->PDFpages(first, last))
	   return FALSE;
	psfile.page_list.select = 
		(BOOL *)malloc( dsc->page_count * sizeof(BOOL) );
    }
    /* put in some dummy values for the page size and bounding box */
    dsc->page_bbox = (CDSCBBOX *)malloc(sizeof(CDSCBBOX));
    if (dsc->page_bbox == (CDSCBBOX *)NULL) {
	delete dsc;
	psfile.dsc = (CDSC *)NULL;
	return FALSE;
    }
    dsc->bbox = (CDSCBBOX *)malloc(sizeof(CDSCBBOX));
    if (dsc->bbox == (CDSCBBOX *)NULL) {
	delete dsc;
	psfile.dsc = (CDSC *)NULL;
	return FALSE;
    }

    dsc->page_bbox->llx = dsc->page_bbox->lly = 0;
    dsc->page_bbox->urx = get_paper_width();
    dsc->page_bbox->ury = get_paper_height();

    dsc->bbox->llx = dsc->bbox->lly = 0;
    dsc->bbox->urx = dsc->page_bbox->urx;
    dsc->bbox->ury = dsc->page_bbox->ury;
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
pdf_page_init(int pagenum)
{
    /* Prepare to show a page */
    /* This obtains the page size and orientation */
    pdf_rotate = IDM_PORTRAIT;
    pdf_free_link();
    ignore_sync = TRUE;		/* ignore next GSDLL_SYNC */
    return gs_printf("%d GSview_PDFpage\n", pagenum);
}

int
pdf_page(void)
{
    /* Display a page, assuming page size and orientation already correct */
    /* We avoid using pdfshowpage becuase this would call */
    /* pdf_showpage_setpage and undo our display pagesize setup */
    return gs_printf("Page pdfshowpage_init pdfshowpage_finish\n");
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

#define MAX_TAG_LEN 4096

/* Check stdout for tag giving page range, pdfmarks etc. */
int
pdf_process_tag(char *line)
{
int i, first, last;
float x0, x1, y0, y1;
float temp;
int rotate;
unsigned int len = strlen(line);

    if ( (len < 1) || (*line != '%') )
	return FALSE;

    if (psfile.ispdf && (len > sizeof(pdf_page_tag)) &&
	(strncmp(line, pdf_page_tag, strlen(pdf_page_tag)) == 0) ) {
	i = sscanf(line+strlen(pdf_page_tag), "%d %d", &first, &last);
	if (i==2) {
	    if (debug)
		gs_addmess("Found GSVIEW_PDF_PAGE tag\n");
	    if (!pdf_makedoc(first, last)) {
		/* stop processing ASAP */
		request_mutex();
		pending.abort = TRUE;
		pending.now = FALSE;
		release_mutex();
		post_img_message(WM_COMMAND, IDM_GSMESS);
	    }
	    return TRUE;
	}
    }
    if (psfile.ispdf && (len > sizeof(pdf_media_tag)) &&
	(strncmp(line, pdf_media_tag, strlen(pdf_media_tag)) == 0) ) {
	i = sscanf(line+strlen(pdf_media_tag), "[%f %f %f %f]", &x0, &y0, &x1, &y1);
	if (i==4) {
	    if (debug)
		gs_addmess("Found GSVIEW_PDF_MEDIA tag\n");
	    /* PDF page size is stored in default page bbox */
	    if (x0 > x1) {
		temp = x0;
		x0 = x1;
		x1 = temp;
	    }
	    if (y0 > y1) {
		temp = y0;
		y0 = y1;
		y1 = temp;
	    }
	    if (psfile.dsc->page_bbox != (CDSCBBOX *)NULL) {
		psfile.dsc->page_bbox->llx = (int)x0;
		psfile.dsc->page_bbox->lly = (int)y0;
		psfile.dsc->page_bbox->urx = (int)x1;
		psfile.dsc->page_bbox->ury = (int)y1;
	    }
	    return TRUE;
	}
    }
    if (psfile.ispdf && (len > sizeof(pdf_crop_tag)) &&
	(strncmp(line, pdf_crop_tag, strlen(pdf_crop_tag)) == 0) ) {
	i = sscanf(line+strlen(pdf_crop_tag), "[%f %f %f %f]", &x0, &y0, &x1, &y1);
	if (i==4) {
	    if (debug)
		gs_addmess("Found GSVIEW_PDF_CROP tag\n");
	    /* perform clipping by using epsf clipping code */
	    /* PDF crop box is stored in document bbox */
	    if (x0 > x1) {
		temp = x0;
		x0 = x1;
		x1 = temp;
	    }
	    if (y0 > y1) {
		temp = y0;
		y0 = y1;
		y1 = temp;
	    }
	    psfile.dsc->bbox->llx = (int)x0;
	    psfile.dsc->bbox->lly = (int)y0;
	    psfile.dsc->bbox->urx = (int)x1;
	    psfile.dsc->bbox->ury = (int)y1;
	    return TRUE;
	}
    }
    if (psfile.ispdf && (len > sizeof(pdf_rotate_tag)) &&
	(strncmp(line, pdf_rotate_tag, strlen(pdf_rotate_tag)) == 0) ) {
	i = sscanf(line+strlen(pdf_rotate_tag), "%d", &rotate);
	if (i==1) {
	    if (debug)
		gs_addmess("Found GSVIEW_PDF_ROTATE tag\n");
	    while (rotate < 0)
		rotate += 360;
	    while (rotate >= 360)
		rotate -= 360;
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
	    /* pdf_rotate is used if orientation is auto */
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
	p = line+strlen(pdf_mark_tag);
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
		    float fllx, flly, furx, fury;
		    if (sscanf(value+1, "%f %f %f %f", 
			&fllx, &flly, &furx, &fury) == 4) {
			link.bbox.llx = (int)fllx;
			link.bbox.lly = (int)flly;
			link.bbox.urx = (int)(furx + 0.5);
			link.bbox.ury = (int)(fury + 0.5);
			code = TRUE;
		    }
		    else if (debug)
			gs_addmess("Invalid /Rect\n");
		}
		if (strcmp(key, "/Page") == 0) {
		    if (strcmp(value, "/Next") == 0) {
			link.page = psfile.pagenum+1;
		    }
		    else if (strcmp(value, "/Prev") == 0) {
			link.page = psfile.pagenum-1;
		    }
		    else if (strcmp(value, "/GoBack") == 0) {
			/* This is wrong - we should go to the
			 * previously displayed page.
			 * This has no effect in gs 5.50 because
			 * it attempts to translate to a page number,
			 * fails, and never gives us the /GoBack.
			 */
			link.page = psfile.pagenum-1;
		    }
		    else if (sscanf(value, "%d", &link.page) == 1) {
			code = TRUE;
		    }
		}
		if (strcmp(key, "/Border") == 0) {
		    if (sscanf(value+1, "%f %f %f", &link.border_xr, &link.border_yr, &link.border_width) == 3)
			code = TRUE;
		    else if (debug)
			gs_addmess("Invalid /Border\n");
		}
		if (strcmp(key, "/Color") == 0) {
		    if (sscanf(value+1, "%f %f %f", &link.colour_red, &link.colour_green, &link.colour_blue) == 3) {
			code = TRUE;
			link.colour_valid = TRUE;
		    }
		    else if (debug)
			gs_addmess("Invalid /Color\n");
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

char pdf_tag_line[MAX_TAG_LEN];

/* Check stdout for tag giving page range */
int
pdf_checktag(LPSTR str, int len)
{
char *p;
BOOL quote_next = FALSE;
BOOL inparen = FALSE;
BOOL found_eol = FALSE;
int code = FALSE;
unsigned int tag_len;

    /* append to local copy */
    tag_len = strlen(pdf_tag_line);

    if ( tag_len + len < sizeof(pdf_tag_line) ) {
#if defined(__WIN32__) || defined(OS2)
        memcpy(pdf_tag_line+tag_len, str, len);
#else
        _fmemcpy(pdf_tag_line+tag_len, str, len);
#endif
	pdf_tag_line[tag_len + len] = '\0';
    }
    else
	pdf_tag_line[0] = '\0';

    /* only lines starting with % are tags */
    if (pdf_tag_line[0] != '%') {
	pdf_tag_line[0] = '\0';
	return FALSE;
    }

    /* check if line is complete */
    for (p = pdf_tag_line; *p; p++) {
	if (quote_next) {
	    quote_next = FALSE;
	    continue;
	}

	if (*p == '\\')
	   quote_next = TRUE;
	else if (*p == '(') 
	   inparen = TRUE;
	else if (*p == ')') 
	   inparen = FALSE;
	else if ((*p == '\n') && !inparen)
	    found_eol = TRUE;
    }


    if (!found_eol)
	return FALSE; /* not yet complete */

    if ( (len >= 1) && (*pdf_tag_line == '%') )
	code = pdf_process_tag(pdf_tag_line);
    pdf_tag_line[0] = '\0';
    return code;
}
	

int
pdf_orientation(void)
{
    return pdf_rotate;
}

/* Create DSC file to print selected pages of PDF file */
int
pdf_extract(FILE *f, int copies)
{
char filename[MAXSTR];
char *p, *s;
int i, page, pages;
CDSC *dsc = psfile.dsc;
BOOL reverse = psfile.page_list.reverse;
    p = filename;
    for (s = psfile_name(&psfile); *s; s++) {
	*p++ = *s;
	if (*s == '\\')
	    *p++ = '\\';
    }
    *p = '\0';

    if (dsc->page_count == 0) {
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
    for (i=0; i< (int)dsc->page_count; i++) {
	if (psfile.page_list.select[i]) pages++;
    }

    /* Send header */
    fputs("%!PS-Adobe 3.0\r\n%%Creator: GSview\r\n", f);
    fputs("%%Title: Ghostscript wrapper for ", f);
    fputs(psfile_name(&psfile), f);
    fputs("\r\n%%Pages: ", f);
    fprintf(f, "%d\r\n", pages);
    fprintf(f, "%%%%PageOrder: %s\r\n", reverse ? "Descend" : "Ascend");
    fputs("%%EndComments\r\n", f);
    fputs("%%BeginProlog\r\n", f);
    fputs("\
/Page null def\r\n\
/Page# 0 def\r\n\
/PDFSave null def\r\n\
/DSCPageCount 0 def\r\n\
/DoPDFPage {\r\n\
  (Page ) print dup == flush\r\n\
  dup /Page# exch store\r\n\
  pdfgetpage pdfshowpage\r\n\
} def\r\n\
GS_PDF_ProcSet begin\r\n\
pdfdict begin\r\n\
", f);
    fputs("%%EndProlog\r\n", f);
    fputs("%%BeginSetup\r\n", f);
    if (copies > 1)
	add_copies(f, copies);
    fprintf(f, "(%s) (r) file pdfopen begin\r\n", filename);
    fputs("%%EndSetup\r\n", f);

    /* Send each page */
    page = 1;
    i = reverse ? dsc->page_count - 1 : 0;
    while ( reverse ? (i >= 0)  : (i < (int)dsc->page_count) ) {
	if (psfile.page_list.select[i])  {
	    fprintf(f, "%%%%Page: %s %d\r\n", dsc->page[i].label, page);
	    fprintf(f, "%d DoPDFPage\r\n", i+1);
	    page++;
	}
        i += reverse ? -1 : 1;
    }

    /* Send trailer */
    fputs("%%Trailer\r\n", f);
    fputs("currentdict pdfclose\r\nend\r\nend\r\nend\r\n%%EOF\r\n", f);
    return TRUE;
}


/* This doesn't work in GS 5.50 because pdf_2ps.ps has been removed */
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

    if (!pdf_extract(pcfile, 1)) {
	fclose(pcfile);
	return FALSE;
    }

#ifdef UNUSED
    fprintf(pcfile, "(");
    for (p=psfile_name(&psfile); *p != '\0'; p++)
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
int temp;
    if (newlink.bbox.llx > newlink.bbox.urx) {
	temp = newlink.bbox.llx;
	newlink.bbox.llx = newlink.bbox.urx;
	newlink.bbox.urx = temp;
    }
    if (newlink.bbox.lly > newlink.bbox.ury) {
	temp = newlink.bbox.lly;
	newlink.bbox.lly = newlink.bbox.ury;
	newlink.bbox.ury = temp;
    }
	
    if (debug) {
	char buf[MAXSTR];
	sprintf(buf, "Adding link /Page %d /Rect [%d %d %d %d] /Border [%g %g %g]",
	    newlink.page,
	    newlink.bbox.llx, newlink.bbox.lly,
	    newlink.bbox.urx, newlink.bbox.ury,
	    newlink.border_xr, newlink.border_yr, newlink.border_width);
	gs_addmess(buf);
	if (newlink.colour_valid) {
	    sprintf(buf, " /Color [%g %g %g]",
	      newlink.colour_red, newlink.colour_green, newlink.colour_blue);
	    gs_addmess(buf);
	}
        gs_addmess("\n");
    }

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

