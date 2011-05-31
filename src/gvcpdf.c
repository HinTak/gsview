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
int pdf_rotate = IDM_PORTRAIT;

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
Page /MediaBox pget { (%s) print dup == flush\n\
Page /Rotate pget not { 0 } if (%s) print dup == flush\n\
dup 90 eq exch 270 eq or\n\
  { [ exch aload pop exch ] }\n\
  if\n\
dup [ exch dup 2 get exch 3 get ]\n\
GSview exch /PageSize exch put\n\
pop\n\
<< >> setpagedevice} { } ifelse\n\
save /PDFSave exch store Page pdfshowpage\n\
PDFSave restore } def\n", pdf_media_tag, pdf_rotate_tag);

    /* put these in userdict so we can write to them later */
    if (!code)
	code = gs_printf("/Page null def\n/PDFSave null def\n");
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
    return gs_printf("currentdict pdfclose\nend\nend\nend\n");
}

int
pdf_page(int pagenum)
{
    pdf_rotate = IDM_PORTRAIT;
    return gs_printf("%d GSview_PDFpage\n", pagenum);
}

/* Check stdout for tag giving page range */
int
pdf_checktag(char *str, int len)
{
int i, first, last;
char buf[MAXSTR];
float x0, x1, y0, y1;
int rotate;
    if ( (len < 1) || (*str != '%') )
	return FALSE;
    if ((len > sizeof(pdf_page_tag)) &&
	(strncmp(str, pdf_page_tag, strlen(pdf_page_tag)) == 0) ) {
	strncpy(buf, str, len);
	buf[len] = '\0';
	i = sscanf(buf+strlen(pdf_page_tag), "%d %d", &first, &last);
	if (i==2) {
	    if (debug)
		gs_addmess("Found GSVIEW_PDF_PAGE tag\n");
	    pdf_makedoc(first, last);
	    return TRUE;
	}
    }
    if ((len > sizeof(pdf_media_tag)) &&
	(strncmp(str, pdf_media_tag, strlen(pdf_media_tag)) == 0) ) {
	strncpy(buf, str, len);
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
    if ((len > sizeof(pdf_rotate_tag)) &&
	(strncmp(str, pdf_rotate_tag, strlen(pdf_rotate_tag)) == 0) ) {
	strncpy(buf, str, len);
	buf[len] = '\0';
	i = sscanf(buf+strlen(pdf_rotate_tag), "%d", &rotate);
	if (i==1) {
	    if (debug)
		gs_addmess("Found GSVIEW_PDF_ROTATE tag\n");
	    switch (rotate) {
		case 90:
		    pdf_rotate = IDM_SEASCAPE;
		    break;
		case 180:
		    pdf_rotate = IDM_UPSIDEDOWN;
		    break;
		case 270:
		    pdf_rotate = IDM_LANDSCAPE;
		    break;
		default:
		    pdf_rotate = IDM_PORTRAIT;
		    break;
	    }
	    return TRUE;
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
    fputs("/Page# null def\r\n/Page null def\r\n/PDFSave null def\r\n", f);
    fputs("GS_PDF_ProcSet begin\r\npdfdict begin\r\n", f);
    fprintf(f, "(%s) (r) file pdfopen begin\r\n", filename);
    fputs("%%EndProlog\r\n", f);

    /* Send each page */
    page = 1;
    for (i = 0; i < doc->numpages; i++) {
	if (psfile.page_list.select[i])  {
	    fprintf(f, "%%%%Page: %s %d\r\n", doc->pages[i].label, page);
	    fprintf(f, "(%%%%Page: %s %d\\n) print flush\r\n", doc->pages[i].label, page);
	    page++;
	    fprintf(f, "/Page# %s store\r\n", doc->pages[i].label);
	    fprintf(f, "%s pdfgetpage /Page exch store\r\n", doc->pages[i].label);
	    fprintf(f, "save /PDFSave exch store\r\n");
	    fprintf(f, "Page pdfshowpage\r\n");
	    fprintf(f, "PDFSave restore\r\n");
	}
    }

    /* Send trailer */
    fputs("%%Trailer\r\n", f);
    fputs("currentdict pdfclose\r\nend\r\nend\r\nend\r\n", f);
    return TRUE;
}
