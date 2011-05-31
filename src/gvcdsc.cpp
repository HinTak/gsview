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

//////////////////////////////////////////////////////////////////////
// gvcdsc.cpp: implementation of the CDSC class.
//////////////////////////////////////////////////////////////////////


// This is a DSC parser, based on the DSC 3.0 spec, 
// with a few DSC 2.1 additions for page size.
//
// Current limitations:
// %%+ may be used after any comment, but is currently only supported by
//   %%DocumentMedia
//   ignored DSC comments
//   unknown DSC comments

// DSC 2.1 additions (discontinued in DSC 3.0):
// %%DocumentPaperColors: 
// %%DocumentPaperForms: 
// %%DocumentPaperSizes: 
// %%DocumentPaperWeights: 
// %%PaperColor:   (ignored)
// %%PaperForm:    (ignored)
// %%PaperSize: 
// %%PaperWeight:  (ignored)


/*
#define STRICT
#include <windows.h>
*/

// stdio accesses to files should be removed later...
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#ifdef UNIX
#define stricmp(s,t) strcasecmp(s,t)
#endif


#define ULONG unsigned long
#define LONG long
#define UINT unsigned int
#define INT int
#define WORD unsigned short
#define DWORD unsigned long
#define BOOL unsigned int
#define FALSE (BOOL)0
#define TRUE (BOOL)(!FALSE)
#ifndef min
#define min(a,b)  ((a) < (b) ? (a) : (b))
#endif

#include "gvcfile.h"

#define CDSC_DEFINE_MESSAGES	// include messages as an array
#include "gvcdsc.h"
#define MAXSTR 256


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef EPSTOOL
void * debug_malloc(size_t size);
void  * debug_realloc(void *block, size_t size);
void debug_free(void *block);
#define malloc(size) debug_malloc(size)
#define calloc(nitems, size) debug_calloc(nitems, size)
#define realloc(block, size) debug_realloc(block, size)
#define free(block) debug_free(block)
#endif


CDSCMEDIA dsc_known_media[CDSC_KNOWN_MEDIA] = {
    // These sizes taken from Ghostscript gs_statd.ps
    {"11x17", 792, 1224, 0, NULL, NULL},
    {"A3", 842, 1190, 0, NULL, NULL},
    {"A4", 595, 842, 0, NULL, NULL},
    {"A5", 421, 595, 0, NULL, NULL},
    {"B4", 709, 1002, 0, NULL, NULL}, // ISO, but not Adobe standard
    {"B5", 501, 709, 0, NULL, NULL},  // ISO, but not Adobe standard
    {"Ledger", 1224, 792, 0, NULL, NULL},
    {"Legal", 612, 1008, 0, NULL, NULL},
    {"Letter", 612, 792, 0, NULL, NULL},
    {"Note", 612, 792, 0, NULL, NULL},
    {NULL, 0, 0, 0, NULL, NULL}
};

enum ScanSection {
    scan_none = 0,
    scan_comments = 1,
    scan_preview = 2,
    scan_defaults = 3,
    scan_prolog = 4,
    scan_setup = 5,
    scan_pages = 6,
    scan_page_trailer = 7,
    scan_trailer = 8
};

//////////////////////////////////////////////////////////////////////
// DSC error reporting
//////////////////////////////////////////////////////////////////////


int dsc_severity[] = {
    CDSC_ERROR_WARN, 	// CDSC_MESSAGE_BBOX
    CDSC_ERROR_WARN, 	// CDSC_MESSAGE_EARLY_TRAILER
    CDSC_ERROR_WARN, 	// CDSC_MESSAGE_EARLY_EOF
    CDSC_ERROR_ERROR, 	// CDSC_MESSAGE_PAGE_IN_TRAILER
    CDSC_ERROR_ERROR, 	// CDSC_MESSAGE_PAGE_ORDINAL
    CDSC_ERROR_ERROR, 	// CDSC_MESSAGE_PAGES_WRONG
    CDSC_ERROR_ERROR, 	// CDSC_MESSAGE_EPS_NO_BBOX
    CDSC_ERROR_ERROR, 	// CDSC_MESSAGE_EPS_PAGES
    CDSC_ERROR_WARN, 	// CDSC_MESSAGE_NO_MEDIA
    CDSC_ERROR_WARN, 	// CDSC_MESSAGE_ATEND
    CDSC_ERROR_INFORM, 	// CDSC_MESSAGE_DUP_COMMENT
    CDSC_ERROR_INFORM, 	// CDSC_MESSAGE_DUP_TRAILER
    CDSC_ERROR_WARN, 	// CDSC_MESSAGE_BEGIN_END
    CDSC_ERROR_INFORM, 	// CDSC_MESSAGE_BAD_SECTION
    CDSC_ERROR_INFORM,  // CDSC_MESSAGE_LONG_LINE
    CDSC_ERROR_WARN, 	// CDSC_MESSAGE_INCORRECT_USAGE
    0
};

#define DSC_MAX_ERROR ((sizeof(dsc_severity) / sizeof(int))-2)


/* We always include the English version of this text in
 * an array for debug reporting.  Language specific versions
 * are stored in resources.
 */ 

char *dsc_message[] = {
// CDSC_MESSAGE_BBOX
 "This line is incorrect.\n\
The bounding box must be in integer coordinates,\n\
not floating point. A floating point bounding box should\n\
be specified using %%HiResBoundingBox:\n\
A %%BoundingBox: should still be provided with integer values.\n\
\n\
'OK' will convert the coordinates to integers,\n\
rounding them outwards.\n\
'Cancel' will ignore this line\n",

// CDSC_MESSAGE_EARLY_TRAILER
"The trailer is normally at the end of a document.\n\
This line was found more than 32k bytes from the end\n\
of the file.  Trailers are not usually this long.\n\
It is more likely that another PostScript file has been included,\n\
without enclosing it in %%BeginDocument / %%EndDocument.\n\
\n\
'OK' will assume this is part of an included file.\n\
'Cancel' will assume this is the trailer\n",

// CDSC_MESSAGE_EARLY_EOF
"This line normally occurs at the end of a document.\n\
This line was found more than 100 bytes from the end\n\
of the file.\n\
It is more likely that another PostScript file has been included,\n\
without enclosing it in %%BeginDocument / %%EndDocument.\n\
\n\
'OK' will assume this is part of an included file.\n\
'Cancel' will assume this is part of the trailer\n",

// CDSC_MESSAGE_PAGE_IN_TRAILER
"This %%Page: line occurred in the trailer, which is not legal.\n\
EPS files should be encapsulated in %%BeginDocument / %%EndDocument.\n\
If is possible that an EPS file was incorrectly encapsulated,\n\
and that we have been confused by the %%Trailer in an EPS file.\n\
\n\
'OK' will process more pages.\n\
'Cancel' will ignore this page comment\n",

// CDSC_MESSAGE_PAGE_ORDINAL
"This line is incorrect.\n\
The two arguments should be a label and an ordinal.\n\
The ordinal should be 1 for the first page and should\n\
increase by 1 for each following page.\n\
The page ordinal is missing or is not in sequence.\n\
\n\
It is likely that this line is part of an included file that\n\
was not enclosed in %%BeginDocument / %%EndDocument.\n\
\n\
'OK' will ignore this page, assuming it is part of an\n\
incorrectly encapsulated EPS file.\n\
'Cancel' will treat this as a valid page\n",

// CDSC_MESSAGE_PAGES_WRONG
"%%Pages: doesn't match number of %%Page:\n\
\n\
'OK' will adjust the page count to match the number of %%Page:\n\
comments found\n\
'Cancel' will set the page count from %%Pages:\n",

// CDSC_MESSAGE_EPS_NO_BBOX
"This EPS file is missing the required %%BoundingBox:\n\
See the GSview help for details of how to add/adjust the\n\
bounding box.\n\
\n\
'OK' will assume this as an EPS file\n\
'Cancel' will assume this is NOT an EPS file.\n",

// CDSC_MESSAGE_EPS_PAGES
"EPS files may have 0 or 1 pages.  This 'EPS' file has\n\
more than this and so is not an EPS file.\n\
You cannot use `Print To` `Encapsulated PostScript File` for\n\
printing multipage files.  The correct method is to connect\n\
the printer to FILE: or to select 'Print to File'.\n\
\n\
'OK' will assume this as an EPS file\n\
'Cancel' will assume this is NOT an EPS file.\n",

// CDSC_MESSAGE_NO_MEDIA
"Media was specified with %%DocumentMedia:, but no default\n\
media was specified with %%PageMedia:.\n\
\n\
'OK' will set the default media to the first listed media.\n\
'Cancel' will not set a default media.\n",

// CDSC_MESSAGE_ATEND
"This line is incorrect\n\
To defer a value, you must use '(atend)', not 'atend'\n\
\n\
'OK' will assume that (atend) was meant.\n\
'Cancel' will ignore this line.\n",

// CDSC_MESSAGE_DUP_COMMENT
"This comment is duplicated in the header, which is unnecessary.\n\
The first occurence of a header line takes precedence.\n\
\n\
'OK' or 'Cancel' will ignore the duplicate line.\n",

// CDSC_MESSAGE_DUP_TRAILER
"This comment is duplicated in the trailer, which is unnecessary.\n\
The last occurence of a trailer line takes precedence.\n\
\n\
'OK' or 'Cancel' will use the duplicate line.\n",

// CDSC_MESSAGE_BEGIN_END
"The number of Begin and End comments do not match.\n\
\n\
'OK' or 'Cancel' will ignore this mismatch.\n",

// CDSC_MESSAGE_BAD_SECTION
"This line is incorrect because it should not occur\n\
within a page.  This probably indicates that an EPS file\n\
was incorrectly encapsulated.  The line will be ignored.\n\
\n\
'OK' or 'Cancel' will ignore this line.\n",

// CDSC_MESSAGE_LONG_LINE
"\n\nLines in DSC documents must be shorter than 255 characters.\n",

// CDSC_MESSAGE_INCORRECT_USAGE
"This DSC comment line is incorrect.\n\
Please refer to the DSC specification.\n\
'OK' or 'Cancel' will ignore this mismatch.\n",

   NULL
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDSC::CDSC()
{
	page_count = 0;
	page = NULL;
	media = NULL;
	page_media = NULL;
	bbox = NULL;
	page_bbox = NULL;
	doseps = NULL;
	string_head = NULL;
	string = NULL;
	dsc_data = NULL;
	debug_print_fn = NULL;
        dsc_error_fn = NULL;

	Reset();
}

CDSC::~CDSC()
{
	Reset();
}

#define compare(p,str) (strncmp((const char *)(p), (str), sizeof(str)-1)==0)

#define dsc_start  (dsc_data_base + dsc_data_index - dsc_line_length)
#define dsc_end  (dsc_data_base + dsc_data_index)
#define is_dsc(str) (compare(dsc_line, str))
#define iswhite(ch) (((ch)==' ') || ((ch)=='\t') || ((ch)=='\r') || ((ch)=='\n'))


void 
CDSC::Reset(void)
{
    // Clear public members
    dsc = FALSE;
    ctrld = FALSE;
    pjl = FALSE;
    epsf = FALSE;
    pdf = FALSE;
    epsf = FALSE;
    preview = 0;
    language_level = 0;
    document_data = 0;
    dsc_version = NULL;
    begincomments = 0;
    endcomments = 0;
    beginpreview = 0;
    endpreview = 0;
    begindefaults = 0;
    enddefaults = 0;
    beginprolog = 0;
    endprolog = 0;
    beginsetup = 0;
    endsetup = 0;
    begintrailer = 0;
    endtrailer = 0;
	
    UINT i;
    for (i=0; i<page_count; i++) {
	// page media is pointer to an element of media
	// do not free it.

	if (page[i].bbox)
	    free(page[i].bbox);
    }
    if (page)
	free(page);
    page = NULL;
	
    page_chunk_length = 0;
    page_count = 0;
    page_pages = 0;
    page_order = 0;
    page_orientation = 0;
	
    page_media = NULL;

    if (media)
	free(media);
    media = NULL;
    media_count = 0;
    if (bbox)
	free(bbox);
    bbox = NULL;
    if (page_bbox)
	free(page_bbox);
    page_bbox = NULL;
    if (doseps)
	free(doseps);
    doseps = NULL;
	
    doseps_end = 0;
    dsc_title = NULL;
    dsc_for = NULL;
    dsc_creator = NULL;
    dsc_date = NULL;
	
    cfile = NULL;
    file_length = 0;
	
    // Clear private members
    string = string_head;
    while (string != (CDSCSTRING *)NULL) {
	if (string->data)
	    free(string->data);
	string_head = string;
	string = string->next;
	free(string_head);
    }
    string_head = NULL;
    string = NULL;
	
    if (dsc_data)
        free(dsc_data);
    dsc_data = NULL;
    dsc_data_length = 0;
    dsc_data_index = 0;
    dsc_data_base = 0;
	
    dsc_line = 0;
    dsc_line_length = 0;
    dsc_eol = 0;
    line_count = 1;
    long_line = FALSE;
}

int 
CDSC::Scan(CFile *cf)
{
    // make sure no one else is using this
    ASSERT(cfile == NULL);
    Reset();
    cfile = cf;
    scan_section = scan_none;
    file_length = cfile->Seek(0, CFile::end);
    cfile->Seek(0, CFile::begin);
    return Scan();
	
}

/* Scan entire document */
int 
CDSC::Scan()
{
    int rc;
    string_head = (CDSCSTRING *)malloc(sizeof(CDSCSTRING));
    if (string_head == NULL)
	return -1;	// no memory
    string = string_head;
    string->next = NULL;
    string->data = (char *)malloc(CDSC_STRING_CHUNK);
    if (string->data == NULL) {
	Reset();
	return -1;	// no memory
    }
    string->index = 0;
    string->length = CDSC_STRING_CHUNK;
	
    page = (CDSCPAGE *)malloc(CDSC_PAGE_CHUNK * sizeof(CDSCPAGE));
    if (page == NULL) {
	Reset();
	return -1;	// no memory
    }
    page_chunk_length = CDSC_PAGE_CHUNK;
    page_count = 0;
	
    dsc_data = (char *)malloc(CDSC_READ_BUFFER_LENGTH);
    if (dsc_data == NULL) {
	Reset();
	return -1;	// no memory
    }
    dsc_line = NULL;
    dsc_data_length = 0;
    dsc_data_index = dsc_data_length;
    memset(dsc_data, 0, CDSC_READ_BUFFER_LENGTH);
	
    if ((rc = ScanType()) != 0) {
        Reset();
        return rc;
    }
    if (!dsc)
	return 0;
    if ((rc = ScanComments()) != 0) {
        Reset();
        return rc;
    }
    if ((rc = ScanPreview()) != 0) {
        Reset();
        return rc;
    }
    if ((rc = ScanDefaults()) != 0) {
        Reset();
        return rc;
    }
    if ((rc = ScanProlog()) != 0) {
        Reset();
        return rc;
    }
    if ((rc = ScanSetup()) != 0) {
        Reset();
        return rc;
    }
    if ((rc = ScanPages()) != 0) {
        Reset();
        return rc;
    }
    if ((rc = ScanTrailer()) != 0) {
        Reset();
        return rc;
    }
	
    /* fixup DSC errors */
    /* Fix DSC error: code between %%EndSetup and %%Page */
    if (page_count && (page[0].begin != endsetup)) {
	endsetup = page[0].begin;
	DebugPrint("Warning: code included between setup and first page\n");
    }

    /* Last page contained a false trailer, */
    /* so extend last page to start of trailer */
    if (page_count && (page[page_count-1].end != begintrailer)) {
	DebugPrint("Ignoring earlier misplaced trailer\n");
	DebugPrint("and extending last page to start of trailer\n"); 
	page[page_count-1].end = begintrailer;
    }

    /* %%Page: found in wrong place and ignored, resulting in 0 pages. */
    if ((page_count==0) && (endsetup != begintrailer)) {
	DebugPrint("No pages found.  Extending setup section to trailer\n");
	endsetup = begintrailer;
    }
	
    /* Warnings and Errors that we can now identify */
    if ((page_count != page_pages)) {
	int rc = DSCerror(CDSC_MESSAGE_PAGES_WRONG, NULL, 0);
	switch (rc) {
	    case CDSC_OK:
		// adjust incorrect page count
		page_pages = page_count;
		break;
	    case CDSC_CANCEL:
		break;;
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }

    if (epsf && (bbox == (CDSCBBOX *)NULL)) {
	/* EPS files MUST include a BoundingBox */
	int rc = DSCerror(CDSC_MESSAGE_EPS_NO_BBOX, NULL, 0);
	switch (rc) {
	    case CDSC_OK:
		// Assume that it is EPS
		break;
	    case CDSC_CANCEL:
		// Is NOT an EPS file
		epsf = FALSE;
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }

    if (epsf && ((page_count > 1) || (page_pages > 1))) {
	int rc = DSCerror(CDSC_MESSAGE_EPS_PAGES, NULL, 0);
	switch (rc) {
	    case CDSC_OK:
		// Is an EPS file
		break;
	    case CDSC_CANCEL:
		// Is NOT an EPS file
		epsf = FALSE;
		break;
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }

    if ((media_count == 1) && (page_media == NULL)) {
	// if one only media was specified, and default page media 
	// was not specified, assume that default is the only media.
	page_media = &media[0];
    }

    if ((media_count != 0) && (page_media == NULL)) {
	int rc = DSCerror(CDSC_MESSAGE_NO_MEDIA, NULL, 0);
	switch (rc) {
	    case CDSC_OK:
		// default media is first listed
		page_media = &media[0];
		break;
	    case CDSC_CANCEL:
		// No default media
		break;
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }

    // make sure all pages have a label
    unsigned int i;
    char buf[32];
    for (i=0; i<page_count; i++) {
	if (strlen(page[i].label) == 0) {
	    sprintf(buf, "%d", i+1);
	    if ((page[i].label = dsc_alloc_string(buf)) == (char *)NULL)
		return -1;	// no memory
	}
    }

    return 0;
}


int
CDSC::is_section(void)
{
    if ( !((dsc_line[0]=='%') && (dsc_line[1]=='%')) )
	return 0;
    if (is_dsc("%%BeginPreview"))
	return 1;
    if (is_dsc("%%BeginDefaults"))
	return 1;
    if (is_dsc("%%BeginProlog"))
	return 1;
    if (is_dsc("%%BeginSetup"))
	return 1;
    if (is_dsc("%%Page:"))
	return 1;
    if (is_dsc("%%Trailer"))
	return 1;
    if (is_dsc("%%EOF"))
	return 1;
    return 0;
}

BOOL
CDSC::is_continued(void)
{
    if (dsc_data_index + 3 > dsc_data_length)
	return FALSE;	// no room for %%+
    if ( (dsc_data[dsc_data_index]=='%') &&
	(dsc_data[dsc_data_index+1]=='%') &&
	(dsc_data[dsc_data_index+2]=='+') )
	return TRUE;
    return FALSE;
}

int 
CDSC::ScanType(void)
{
    char *p;
    // These should be done by Reset()
    Seek(0, SEEK_SET, NULL);
    if (!ReadLine())
	return -1;	// no memory
    // Types that should be known:
    //   DSC
    //   EPSF
    //   PJL + any of above
    //   ^D + any of above
    //   DOS EPS
    //   PDF
    //   non-DSC
    if ((unsigned char)dsc_line[0] == '\004') {
	dsc_line_length--;
	dsc_line++;
	ctrld = TRUE;
    }
    if (compare(dsc_line, "\033%-12345X")) {
	pjl = TRUE;
	// skip until first DSC comment
	do {
	    ReadLine();
	}
	while ((dsc_line_length && (unsigned char)dsc_line[0]!='%'));
    }
    if ( ((unsigned char)dsc_line[0]==0xc5) && 
	 ((unsigned char)dsc_line[1]==0xd0) && 
	 ((unsigned char)dsc_line[2]==0xd3) && 
	 ((unsigned char)dsc_line[3]==0xc6) ) {
	// id is "EPSF" with bit 7 set
	// read DOS EPS header, seek to PS section, read first line,
	//  set end of PS section
	if (ReadDOSEPSheader())
	    return 1;
    }
	
    dsc_version = AddLine(dsc_line, dsc_line_length);
    if (compare(dsc_line, "%!PS-Adobe")) {
	dsc = TRUE;
	begincomments = dsc_start;
	if (dsc_version == NULL)
		return 1;	// no memory
	p = dsc_line + 14;
	while (*p == ' ')
		p++;
	if (compare(p, "EPSF-"))
		epsf = TRUE;
	ReadLine();
	return 0;
    }
    if (compare(dsc_line, "%PDF-")) {
	pdf = TRUE;
	return 0;
    }
    if (compare(dsc_line, "%!")) {
	return 0;
    }
    return 1;	// unrecognised
}

DWORD
CDSC::GetDWORD(unsigned char *buf)
{
	DWORD dw;
    dw = (DWORD)buf[0];
    dw += ((DWORD)buf[1])<<8;
    dw += ((DWORD)buf[2])<<16;
    dw += ((DWORD)buf[3])<<24;
    return dw;
}

WORD
CDSC::GetWORD(unsigned char *buf)
{
    WORD w;
    w = (WORD)buf[0];
    w |= (WORD)(buf[1]<<8);
    return w;
}

int
CDSC::ReadDOSEPSheader(void)
{
    unsigned char buf[30];
    ULONG cbRead;
	
    Seek(0, SEEK_SET, NULL);
    cbRead = cfile->Read(buf, 30);
    if (cbRead == 0)
	return 1;
    if (cbRead != 30)
	return 1;
    if ((doseps = (CDSCDOSEPS *)malloc(sizeof(CDSCDOSEPS))) == NULL)
	return -1;	// no memory
	
    doseps->ps_begin = GetDWORD(buf+4);
    doseps->ps_length = GetDWORD(buf+8);
    doseps->wmf_begin = GetDWORD(buf+12);
    doseps->wmf_length = GetDWORD(buf+16);
    doseps->tiff_begin = GetDWORD(buf+20);
    doseps->tiff_length = GetDWORD(buf+24);
    doseps->checksum = GetWORD(buf+28);
	
    doseps_end = doseps->ps_begin + doseps->ps_length;
    Seek(doseps->ps_begin, SEEK_SET, NULL);
    ReadLine();
    return 0;
}

int 
CDSC::ParsePages(void)
{
    int i, ip, io; 
    char *p;
    if ((page_pages != 0) && (scan_section == scan_comments)) {
	int rc = DSCerror(CDSC_MESSAGE_DUP_COMMENT, dsc_line, dsc_line_length);
	switch (rc) {
	    case CDSC_OK:
	    case CDSC_CANCEL:
		return 0;	// ignore duplicate comments in header
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }
    if ((page_pages != 0) && (scan_section == scan_trailer)) {
	int rc = DSCerror(CDSC_MESSAGE_DUP_TRAILER, dsc_line, dsc_line_length);
	switch (rc) {
	    case CDSC_OK:
	    case CDSC_CANCEL:
		break;		// use duplicate comments in header
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }

    p = dsc_line + 8;
    while (*p == ' ')
	p++;
    if (compare(p, "atend")) {
	int rc = DSCerror(CDSC_MESSAGE_ATEND, dsc_line, dsc_line_length);
	switch (rc) {
	    case CDSC_OK:
		// assume (atend)
		// we should mark it as deferred
		break;
	    case CDSC_CANCEL:
		// ignore it
		break;
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }
    else if (compare(p, "(atend)")) {
	// do nothing
	// we should mark it as deferred
    }
    else {
	i = sscanf(p, "%d %d", &ip, &io);
	if (i > 0)
	    page_pages = ip;
	if (i == 2) {
	    // DSC 2 uses extra integer to indicate page order
	    // DSC 3 uses %%PageOrder:
	    if (page_order == CDSC_UNKNOWN)
		switch (i) {
	    case -1:
		page_order = CDSC_DESCEND;
		break;
	    case 0:
		page_order = CDSC_SPECIAL;
		break;
	    case 1:
		page_order = CDSC_ASCEND;
		break;
	    }
	}
    }
    return 0;
}

int 
CDSC::ParseBoundingBox(CDSCBBOX** pbbox, int offset)
{
    int llx, lly, urx, ury;
    float fllx, flly, furx, fury;
    char *p;
    // Process first %%BoundingBox: in comments, and last in trailer
    if ((*pbbox != NULL) && (scan_section == scan_comments)) {
	int rc = DSCerror(CDSC_MESSAGE_DUP_COMMENT, dsc_line, dsc_line_length);
	switch (rc) {
	    case CDSC_OK:
	    case CDSC_CANCEL:
		return 0;	// ignore duplicate comments in header
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }
    if ((*pbbox != NULL) && (scan_section == scan_pages)) {
	int rc = DSCerror(CDSC_MESSAGE_DUP_COMMENT, dsc_line, dsc_line_length);
	switch (rc) {
	    case CDSC_OK:
	    case CDSC_CANCEL:
		return 0;	// ignore duplicate comments in header
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }
    if ((*pbbox != NULL) && (scan_section == scan_trailer)) {
	int rc = DSCerror(CDSC_MESSAGE_DUP_TRAILER, dsc_line, dsc_line_length);
	switch (rc) {
	    case CDSC_OK:
	    case CDSC_CANCEL:
		break;		// use duplicate comments in trailer
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }
    if (*pbbox != NULL) {
	free(*pbbox);
	*pbbox = NULL;
    }

    // only process first %%BoundingBox:
    p = dsc_line + offset;
    while (*p == ' ')
	p++;
    
    if (compare(p, "atend")) {
	int rc = DSCerror(CDSC_MESSAGE_ATEND, dsc_line, dsc_line_length);
	switch (rc) {
	    case CDSC_OK:
		// assume (atend)
		// we should mark it as deferred
		break;
	    case CDSC_CANCEL:
		// ignore it
		break;
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }
    else if (compare(p, "(atend)")) {
	// do nothing
	// we should mark it as deferred
    }
    else if (sscanf(p, "%d %d %d %d", &llx, &lly, &urx, &ury) == 4) {
	*pbbox = (CDSCBBOX *)malloc(sizeof(CDSCBBOX));
	if (*pbbox == NULL)
	    return -1;	// no memory
	(*pbbox)->llx = llx;
	(*pbbox)->lly = lly;
	(*pbbox)->urx = urx;
	(*pbbox)->ury = ury;
    }
    else {
	int rc = DSCerror(CDSC_MESSAGE_BBOX, dsc_line, dsc_line_length);
	switch (rc) {
	    case CDSC_OK:
		if (sscanf(p, "%f %f %f %f", &fllx, &flly, &furx, &fury) == 4) {
		    *pbbox = (CDSCBBOX *)malloc(sizeof(CDSCBBOX));
		    if (*pbbox == NULL)
			return -1;	// no memory
		    (*pbbox)->llx = (int)fllx;
		    (*pbbox)->lly = (int)flly;
		    (*pbbox)->urx = (int)(furx+0.999);
		    (*pbbox)->ury = (int)(fury+0.999);
		}
		return 0;
	    case CDSC_CANCEL:
		return 0;
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }
    return 0;
}

int 
CDSC::ParseOrientation(UINT *porientation, int offset)
{
    char *p;
    if ((page_orientation != CDSC_UNKNOWN) && 
	(scan_section == scan_comments)) {
	int rc = DSCerror(CDSC_MESSAGE_DUP_COMMENT, dsc_line, dsc_line_length);
	switch (rc) {
	    case CDSC_OK:
	    case CDSC_CANCEL:
		return 0;	// ignore duplicate comments in header
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }
    if ((page_orientation != CDSC_UNKNOWN) && 
	(scan_section == scan_trailer)) {
	int rc = DSCerror(CDSC_MESSAGE_DUP_TRAILER, dsc_line, dsc_line_length);
	switch (rc) {
	    case CDSC_OK:
	    case CDSC_CANCEL:
		break;		// use duplicate comments in header;
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }
    p = dsc_line + offset;
    while (*p == ' ')
	p++;
    if (compare(p, "atend")) {
	int rc = DSCerror(CDSC_MESSAGE_ATEND, dsc_line, dsc_line_length);
	switch (rc) {
	    case CDSC_OK:
		// assume (atend)
		// we should mark it as deferred
		break;
	    case CDSC_CANCEL:
		// ignore it
		break;
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }
    else if (compare(p, "(atend)")) {
	// do nothing
	// we should mark it as deferred
    }
    else if (compare(p, "Portrait")) {
	*porientation = CDSC_PORTRAIT;
    }
    else if (compare(p, "Landscape")) {
	*porientation = CDSC_LANDSCAPE;
    }
    else {
	UnknownDSC();
    }
    return 0;
}

int 
CDSC::ParseOrder(void)
{
    char *p;
    if ((page_order != CDSC_UNKNOWN) && (scan_section == scan_comments)) {
	int rc = DSCerror(CDSC_MESSAGE_DUP_COMMENT, dsc_line, dsc_line_length);
	switch (rc) {
	    case CDSC_OK:
	    case CDSC_CANCEL:
		return 0;	// ignore duplicate comments in header
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }
    if ((page_order != CDSC_UNKNOWN) && (scan_section == scan_trailer)) {
	int rc = DSCerror(CDSC_MESSAGE_DUP_TRAILER, dsc_line, dsc_line_length);
	switch (rc) {
	    case CDSC_OK:
	    case CDSC_CANCEL:
		break;		// use duplicate comments in trailer
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }

    p = dsc_line + 13;
    while (*p == ' ')
	p++;
    if (compare(p, "atend")) {
	int rc = DSCerror(CDSC_MESSAGE_ATEND, dsc_line, dsc_line_length);
	switch (rc) {
	    case CDSC_OK:
		// assume (atend)
		// we should mark it as deferred
		break;
	    case CDSC_CANCEL:
		// ignore it
		break;
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }
    else if (compare(p, "(atend)")) {
	// do nothing
	// we should mark it as deferred
    }
    else if (compare(p, "Ascend")) {
	page_order = CDSC_ASCEND;
    }
    else if (compare(p, "Descend")) {
	page_order = CDSC_DESCEND;
    }
    else if (compare(p, "Special")) {
	page_order = CDSC_SPECIAL;
    }
    else {
	UnknownDSC();
    }
    return 0;
}

int 
CDSC::ParseMedia(CDSCMEDIA **page_media)
{
    char media_name[MAXSTR];
    int n = 12; // %%PageMedia:
    unsigned int i;

    if (CopyString(media_name, sizeof(media_name)-1, 
	dsc_line+n, dsc_line_length-n, NULL)) {
	for (i=0; i<media_count; i++) {
	    if (stricmp(media_name, media[i].name) == 0) {
		*page_media = &media[i];
		return 0;
	    }
	}
    }
    UnknownDSC();
    
    return 0;
}

int CDSC::ExtendMedia(unsigned int len) 
{
    if (len < media_count)
	return 0;
    // extend media array 
    if (media == NULL) {
	media = (CDSCMEDIA *)malloc(len * sizeof(CDSCMEDIA));
	if (media == NULL)
	    return -1;	// out of memory
    }
    else {
	media = (CDSCMEDIA *)realloc(media, len * sizeof(CDSCMEDIA));
	if (media == NULL)
	    return -1;	// out of memory
    }
    for (unsigned int i=media_count; i<len; i++) {
	media[i].name = NULL;
	media[i].width = 595.0;
	media[i].height = 842.0;
	media[i].weight = 80.0;
	media[i].colour = NULL;
	media[i].type = NULL;
    }
    media_count=len;
    return 0;
}


int 
CDSC::ParseDocumentMedia(void)
{
    unsigned int i, n;
    CDSCMEDIA lmedia;
    BOOL continued;
    BOOL blank_line;

    do {
	if (compare(dsc_line, "%%DocumentMedia"))
	    n = 16;
	else if (compare(dsc_line, "%%+"))
	    n = 3;
	else
	    return 1;	// error

	// check for blank remainder of line
	blank_line = TRUE;
	for (i=n; i<dsc_line_length; i++) {
	    if (!iswhite(dsc_line[i])) {
		blank_line = FALSE;
		break;
	    }
	}

	if (!blank_line) {
	    lmedia.name = lmedia.colour = lmedia.type = NULL;
	    lmedia.width = lmedia.height = lmedia.weight = 0;
	    lmedia.name = AddString(dsc_line+n, dsc_line_length-n, &i);
	    if (lmedia.name == NULL)
		return 1;
	    n+=i;
	    if (i)
		lmedia.width = GetReal(dsc_line+n, dsc_line_length-n, &i);
	    n+=i;
	    if (i)
		lmedia.height = GetReal(dsc_line+n, dsc_line_length-n, &i);
	    n+=i;
	    if (i)
		lmedia.weight = GetReal(dsc_line+n, dsc_line_length-n, &i);
	    n+=i;
	    if (i)
		lmedia.colour = AddString(dsc_line+n, dsc_line_length-n, &i);
	    if (lmedia.colour == NULL)
		return 1;
	    n+=i;
	    if (i)
		lmedia.type = AddString(dsc_line+n, dsc_line_length-n, &i);
	    if (lmedia.type == NULL)
		return 1;

	    if (i==0)
		UnknownDSC(); // we didn't get all fields
	    else {
		if (ExtendMedia(media_count+1))
		    return 1;	// out of memory
		media[media_count-1] = lmedia;
	    }
	}
	if ((continued = is_continued()) == TRUE)
	    NextDSCline();
    } while (continued);
    return 0;
}


int 
CDSC::ScanComments(void)
{
    // Comments section ends at
    //  %%EndComments
    //  another section
    //  line that does not start with %%
    // Save a few important lines
    scan_section = scan_comments;
    DebugPrint("Scanning comments\n");
    do {
	if (is_dsc("%%EndComments")) {
		ReadLine();
		endcomments = dsc_start;
		return 0;
	}
	else if (is_section() && (!is_dsc("%%BeginComments"))) {
		endcomments = dsc_start;
		return 0;
	}
	else if (dsc_line[0] == '%' && iswhite(dsc_line[1])) {
		endcomments = dsc_start;
		return 0;
	}
	else if (dsc_line[0] != '%') {
		endcomments = dsc_start;
		return 0;
	}
	else if (is_dsc("%%Begin")) {
		endcomments = dsc_start;
		return 0;
	}
	
	if (is_dsc("%%Pages:")) {
		if (ParsePages() != 0)
		   return 1;
	}
	else if (is_dsc("%%Creator:")) {
	    dsc_creator = AddLine(dsc_line+10, dsc_line_length-10);
	    if (dsc_creator==NULL)
		return 1;
	}
	else if (is_dsc("%%CreationDate:")) {
	    dsc_date = AddLine(dsc_line+15, dsc_line_length-15);
	    if (dsc_date==NULL)
		return 1;
	}
	else if (is_dsc("%%Title:")) {
	    dsc_title = AddLine(dsc_line+8, dsc_line_length-8);
	    if (dsc_title==NULL)
		return 1;
	}
	else if (is_dsc("%%For:")) {
	    dsc_for = AddLine(dsc_line+6, dsc_line_length-6);
	    if (dsc_for==NULL)
		return 1;
	}
	else if (is_dsc("%%BoundingBox:")) {
	    if (ParseBoundingBox(&bbox, 14))
		return 1;
	}
	else if (is_dsc("%%Orientation:")) {
	    if (ParseOrientation(&page_orientation ,14))
		return 1;
	}
	else if (is_dsc("%%PageOrder:")) {
	    if (ParseOrder())
		return 1;
	}
	else if (is_dsc("%%DocumentMedia:")) {
	    if (ParseDocumentMedia())
		return 1;
	}
	else if (is_dsc("%%DocumentPaperSizes:")) {
	    // DSC 2.1
	    unsigned int n = 21;
	    unsigned int count = 0;
	    unsigned int i = 1;
	    char *p;
	    while (i && (dsc_line[n]!='\r') && (dsc_line[n]!='\n')) {
		p = AddString(dsc_line+n, dsc_line_length-n, &i);
		if (p == NULL)
		    return 1;
		if (i && p) {
		    if (count >= media_count) {
			if (ExtendMedia(count+1))
			    return 1;
		    }
		    media[count].name = p;
		    CDSCMEDIA *m = dsc_known_media;
		    while (m && m->name) {
			if (stricmp(p, m->name)==0) {
			    media[count].width = m->width;
			    media[count].height = m->height;
			    break;
			}
			m++;
		    }
		}
		n+=i;
		count++;
	    }
	}
	else if (is_dsc("%%DocumentPaperForms:")) {
	    // DSC 2.1
	    unsigned int n = 21;
	    unsigned int count = 0;
	    unsigned int i = 1;
	    char *p;
	    while (i && (dsc_line[n]!='\r') && (dsc_line[n]!='\n')) {
		p = AddString(dsc_line+n, dsc_line_length-n, &i);
		if (p == NULL)
		    return 1;
		if (i && p) {
		    if (count >= media_count) {
			if (ExtendMedia(count+1))
			    return 1;
		    }
		    media[count].type = p;
		}
		n+=i;
		count++;
	    }
	}
	else if (is_dsc("%%DocumentPaperColors:")) {
	    // DSC 2.1
	    unsigned int n = 22;
	    unsigned int count = 0;
	    unsigned int i = 1;
	    char *p;
	    while (i && (dsc_line[n]!='\r') && (dsc_line[n]!='\n')) {
		p = AddString(dsc_line+n, dsc_line_length-n, &i);
		if (p == NULL)
		    return 1;
		if (i && p) {
		    if (count >= media_count) {
			if (ExtendMedia(count+1))
			    return 1;
		    }
		    media[count].colour = p;
		}
		n+=i;
		count++;
	    }
	}
	else if (is_dsc("%%DocumentPaperWeights:")) {
	    // DSC 2.1
	    unsigned int n = 23;
	    unsigned int count = 0;
	    unsigned int i = 1;
	    float w;
	    while (i && (dsc_line[n]!='\r') && (dsc_line[n]!='\n')) {
		w = GetReal(dsc_line+n, dsc_line_length-n, &i);
		if (i) {
		    if (count >= media_count) {
			if (ExtendMedia(count+1))
			    return 1;
		    }
		    media[count].weight = w;
		}
		n+=i;
		count++;
	    }
	}
	else if (is_dsc("%%LanguageLevel:")) {
	    int i;
	    if (sscanf(dsc_line + 17, "%d", &i) == 1) {
		if ( (i==1) || (i==2) || (i==3) )
		    language_level = i;
		else {
		    UnknownDSC();
		}
	    }
	}
	else if (is_dsc("%%DocumentData:")) {
	    char *p = dsc_line + 16;
	    if (compare(p, "Clean7Bit"))
		document_data = CDSC_CLEAN7BIT;
	    else if (compare(p, "Clean8Bit"))
		document_data = CDSC_CLEAN8BIT;
	    else if (compare(p, "Binary"))
		document_data = CDSC_BINARY;
	    else
	        UnknownDSC();
	}
	else if (is_dsc("%%Requirements:")) {
	    // ignore
	    SkipComment();
	}
	else if (is_dsc("%%DocumentNeededFonts:")) {
	    // ignore
	    SkipComment();
	}
	else if (is_dsc("%%DocumentSuppliedFonts:")) {
	    // ignore
	    SkipComment();
	}
	else if (dsc_line[0] == '%' && iswhite(dsc_line[1])) {
	    // ignore
	}
	else {
	    UnknownDSC();
	}
	ReadLine();
    } while (dsc_line_length);
    endpreview = dsc_end;
    return 0;
}

int 
CDSC::ScanPreview(void)
{
    // Preview section ends at
    //  %%EndPreview
    //  another section
    // Preview section must start with %%BeginPreview
    scan_section = scan_preview;
    DebugPrint("Scanning preview\n");
    SkipBlank();	// skip blank lines
    beginpreview = endpreview = dsc_start;
    if (!is_dsc("%%BeginPreview"))
	return 0;	// some other section
    do {
	if (is_dsc("%%EndPreview")) {
	    endpreview = dsc_end;
	    ReadLine();
	    return 0;
	}
	
	if (is_dsc("%%BeginPreview")) {
	    // ignore since we have already set beginpreview
	}
	else if (is_section()) {
	    endpreview = dsc_start;
	    return 0;
	}
	else {
	    UnknownDSC();
	}
	NextDSCline();
    } while (dsc_line_length);
    endpreview = dsc_end;
    return 0;
}

int 
CDSC::ScanDefaults(void)
{
    // Defaults section ends at
    //  %%EndDefaults
    //  another section
    // Defaults section must start with %%BeginDefaults
    scan_section = scan_defaults;
    DebugPrint("Scanning defaults\n");
    SkipBlank();	// skip blank lines
    begindefaults = enddefaults = dsc_start;
    if (!is_dsc("%%BeginDefaults"))
	return 0;	// some other section
    do {
	if (is_dsc("%%EndDefaults")) {
	    enddefaults = dsc_end;
	    ReadLine();
	    return 0;
	}

	if (is_dsc("%%BeginDefaults")) {
	    // ignore since we have already set begindefaults
	}
	else if (is_section()) {
	    enddefaults = dsc_start;
	    return 0;
	}
	else if (is_dsc("%%PageMedia:")) {
	    ParseMedia(&page_media);
	}
	else if (is_dsc("%%PageOrientation:")) {
	    // This can override %%Orientation: 
	    if (ParseOrientation(&page_orientation, 18))
		return 1;
	}
	else if (is_dsc("%%PageBoundingBox:")) {
	    if (ParseBoundingBox(&page_bbox, 18))
		return 1;
	}
	else {
	    UnknownDSC();
	}
	NextDSCline();
    } while (dsc_line_length);
    enddefaults = dsc_end;
    return 0;
}

// CDSC_OK and CDSC_CANCEL mean ignore the mismatch (default)
// We should write code which looks for matching End
int CDSC::BeginEndMatch(const char *str, int count)
{
    if (count != 0) {
	char buf[MAXSTR+MAXSTR];
	if (dsc_line_length < (unsigned int)(sizeof(buf)-1))  {
	    strncpy(buf, dsc_line, dsc_line_length);
	    buf[dsc_line_length] = '\0';
	}
	sprintf(buf+strlen(buf), "\n%%%%Begin%.40s: / %%%%End%.40s\n", str, str);
	int rc = DSCerror(CDSC_MESSAGE_BEGIN_END, buf, strlen(buf));
	return rc;
    }
    return CDSC_CANCEL;
}

int CDSC::MatchCheck(const char *str, int count)
{
    if (BeginEndMatch(str, count) == CDSC_IGNORE_ALL)
	return 1;
    return 0;
}


int 
CDSC::ScanProlog(void)
{
    const char *font = "Font";
    const char *feature = "Feature";
    const char *resource = "Resource";
    const char *procset = "ProcSet";
    int ncount = 0;
    int fcount = 0;
    int rcount = 0;
    int pcount = 0;
    int rc;
    // Prolog section ends at
    //  %%EndProlog
    //  another section
    // Prolog section may start with %%BeginProlog or non-dsc line
    scan_section = scan_prolog;
    DebugPrint("Scanning prolog\n");
    SkipBlank();
    beginprolog = dsc_start;
    if ((dsc_line[0]!='%') || (dsc_line[1]!='%')) 
	NextDSCline();
    do {
	if (is_dsc("%%EndProlog")) {
	    endprolog = dsc_end;
	    ReadLine();
	    rc = 0;
	    if (!rc)
	        rc = MatchCheck(font, ncount);
	    if (!rc)
	        rc = MatchCheck(feature, fcount);
	    if (!rc)
	        rc = MatchCheck(resource, rcount);
	    if (!rc)
	        rc = MatchCheck(procset, pcount);
	    return rc;
	}
	if (is_dsc("%%BeginProlog")) {
	    // ignore since we have already set beginprolog
	}
	else if (is_section()) {
	    endprolog = dsc_start;
	    rc = 0;
	    if (!rc)
	        rc = MatchCheck(font, ncount);
	    if (!rc)
	        rc = MatchCheck(feature, fcount);
	    if (!rc)
	        rc = MatchCheck(resource, rcount);
	    if (!rc)
	        rc = MatchCheck(procset, pcount);
	    return rc;
	}
	else if (is_dsc("%%BeginFont:")) {
	    // ignore Begin/EndFont, apart form making sure
	    // that they are matched.
	    ncount++;
	}
	else if (is_dsc("%%EndFont")) {
	    ncount--;
	}
	else if (is_dsc("%%BeginFeature:")) {
	    // ignore Begin/EndFeature, apart form making sure
	    // that they are matched.
	    fcount++;
	}
	else if (is_dsc("%%EndFeature")) {
	    fcount--;
	}
	else if (is_dsc("%%BeginResource:")) {
	    // ignore Begin/EndResource, apart form making sure
	    // that they are matched.
	    rcount++;
	}
	else if (is_dsc("%%EndResource")) {
	    rcount--;
	}
	else if (is_dsc("%%BeginProcSet:")) {
	    // ignore Begin/EndProcSet, apart form making sure
	    // that they are matched.
	    pcount++;
	}
	else if (is_dsc("%%EndProcSet")) {
	    pcount--;
	}
	else if ((dsc_line[0]=='%') && (dsc_line[0]=='%'))  {
	    UnknownDSC();
	}
	NextDSCline();
    } while (dsc_line_length);
    endprolog = dsc_end;
    rc = 0;
    if (!rc)
	rc = MatchCheck(font, ncount);
    if (!rc)
	rc = MatchCheck(feature, fcount);
    if (!rc)
	rc = MatchCheck(resource, rcount);
    if (!rc)
	rc = MatchCheck(procset, pcount);
    return rc;
}

int 
CDSC::ScanSetup(void)
{
    const char *feature = "Feature";
    int fcount = 0;
    const char *resource = "Resource";
    int rcount = 0;
    int rc;
    // Setup section ends at
    //  %%EndSetup
    //  another section
    // Setup section must start with %%BeginSetup
    scan_section = scan_prolog;
    DebugPrint("Scanning setup\n");
    SkipBlank();	// skip blank lines
    beginsetup = endsetup = dsc_start;
    if (!is_dsc("%%BeginSetup"))
		return 0;	// some other section
    while (NextDSCline()) {
	if (is_dsc("%%EndSetup")) {
	    endsetup = dsc_end;
	    ReadLine();
	    return MatchCheck(feature, fcount);
	}
	if (is_dsc("%%BeginProlog")) {
	    // ignore since we have already set beginprolog
	}
	else if (is_section()) {
	    endsetup = dsc_start;
	    return MatchCheck(feature, fcount);
	}
	else if (is_dsc("%%BeginFeature:")) {
	    // ignore Begin/EndFeature, apart form making sure
	    // that they are matched.
	    fcount++;
	}
	else if (is_dsc("%%EndFeature")) {
	    fcount--;
	}
	else if (is_dsc("%%Feature:")) {
	    // ignore
	}
	else if (is_dsc("%%BeginResource:")) {
	    // ignore Begin/EndResource, apart form making sure
	    // that they are matched.
	    rcount++;
	}
	else if (is_dsc("%%EndResource")) {
	    rcount--;
	}
	else if (is_dsc("%%PaperColor:")) {
	    // ignore
	}
	else if (is_dsc("%%PaperForm:")) {
	    // ignore
	}
	else if (is_dsc("%%PaperWeight:")) {
	    // ignore
	}
	else if (is_dsc("%%PaperSize:")) {
	    // DSC 2.1
	    int n = 12;
	    char buf[MAXSTR];
	    buf[0] = '\0';
	    CopyString(buf, sizeof(buf)-1, dsc_line+n, dsc_line_length-n, NULL);
	    CDSCMEDIA *m = media;
	    while (m && m->name) {
		if (stricmp(buf, m->name)==0) {
		    page_media = m;
		    break;
		}
		m++;
	    }
	    if ((m == NULL) || (m->name == NULL)) {
		// It didn't match %%DocumentPaperSizes:
		// Try our known media
		m = dsc_known_media;
		while (m->name) {
		    if (stricmp(buf, m->name)==0) {
			page_media = m;
			break;
		    }
		    m++;
		}
		if (m->name == NULL)
		    UnknownDSC();
	    }
	}
	else if ((dsc_line[0]=='%') && (dsc_line[0]=='%')) {
	    UnknownDSC();
	}
    }
    endsetup = dsc_end;
    rc = MatchCheck(resource, rcount);
    if (!rc)
	rc = MatchCheck(feature, fcount);
    return rc;
}

int 
CDSC::ScanPages(void)
{
    int rc;
    const char *feature = "Feature";
    int fcount = 0;
    const char *resource = "Resource";
    int rcount = 0;
    const char *font = "Font";
    int ncount = 0;
    // Page section ends at
    //  %%Page
    //  %%Trailer
    //  %%EOF
    scan_section = scan_pages;
    DebugPrint("Scanning pages\n");
    SkipBlank();	// skip blank lines
    if (!is_dsc("%%Page:")) {
	// %%Page didn't follow %%EndSetup
	// Keep reading until reach %%Page or %%Trailer
	// and add it to setup section
	if (is_dsc("%%Trailer")) {
	    endsetup = dsc_start;
	    rc = MatchCheck(resource, rcount);
	    if (!rc)
	        rc = MatchCheck(feature, fcount);
	    if (!rc)
	        rc = MatchCheck(font, ncount);
	    return rc;
	}
	else if (is_dsc("%%EOF")) {
	    endsetup = dsc_start;
	    rc = MatchCheck(resource, rcount);
	    if (!rc)
	        rc = MatchCheck(feature, fcount);
	    if (!rc)
	        rc = MatchCheck(font, ncount);
	    return rc;
	}

	while (NextDSCline()) {
	    if (is_dsc("%%Page:")) {
		endsetup = dsc_start;
		break;
	    }
	    else if (is_dsc("%%Trailer")) {
		endsetup = dsc_start;
		rc = MatchCheck(resource, rcount);
		if (!rc)
		    rc = MatchCheck(feature, fcount);
		if (!rc)
		    rc = MatchCheck(font, ncount);
		return rc;
	    }
	    else if (is_dsc("%%EOF")) {
		endsetup = dsc_start;
		rc = MatchCheck(resource, rcount);
		if (!rc)
		    rc = MatchCheck(feature, fcount);
		if (!rc)
		    rc = MatchCheck(font, ncount);
		return rc;
	    }
	    else if (is_dsc("%%BeginFeature:")) {
		// ignore Begin/EndFeature, apart form making sure
		// that they are matched.
		fcount++;
	    }
	    else if (is_dsc("%%EndFeature")) {
		fcount--;
	    }
	    else if ((dsc_line[0]=='%') && (dsc_line[0]=='%')) {
		UnknownDSC();
	    }
	}
    }
    if (!is_dsc("%%Page:"))
	return 0;
    if ((rc = AddPage()) != 0)
	return rc;
    while (NextDSCline()) {
	if (is_dsc("%%Page:")) {
            scan_section = scan_pages;
	    // end curent page
	    page[page_count-1].end = dsc_start;
	    if (MatchCheck(feature, fcount) != 0)
		return 1;
	    if (MatchCheck(resource, rcount) != 0)
		return 1;
	    if (MatchCheck(font, ncount) != 0)
		return 1;
	    // start new page
	    if ((rc = AddPage()) != 0)
		return rc;
	}
	else if (is_section()) {
	    if (is_dsc("%%Trailer")) {
		if ((!doseps && (dsc_end + 32768 < file_length)) ||
		     (doseps && (dsc_end + 32768 < doseps_end)) ) {
		    int rc = DSCerror(CDSC_MESSAGE_EARLY_TRAILER, 
			dsc_line, dsc_line_length);
		    switch (rc) {
			case CDSC_OK:
			    // ignore early trailer
			    break;
			case CDSC_CANCEL:
			    // this is the trailer
			    page[page_count-1].end = dsc_start;
			    endsetup = dsc_start;
			    return 0;
			case CDSC_IGNORE_ALL:
			    return 1;
		    }
		}
		else {
		    page[page_count-1].end = dsc_start;
		    rc = MatchCheck(resource, rcount);
		    if (!rc)
			rc = MatchCheck(feature, fcount);
		    return rc;
		}
	    }
	    else if (is_dsc("%%EOF")) {
		page[page_count-1].end = dsc_start;
		if ( (dsc_end+100 < file_length) ||
		     (doseps && (dsc_end + 100 < doseps_end)) ) {
		    int rc = DSCerror(CDSC_MESSAGE_EARLY_EOF, 
			dsc_line, dsc_line_length);
		    switch (rc) {
			case CDSC_OK:
			    // %%EOF is wrong, ignore it
			    break;
			case CDSC_CANCEL:
			    // %%EOF is correct
			    return 0;
			case CDSC_IGNORE_ALL:
			    return 1;
		    }
		}
		else {
		    rc = MatchCheck(resource, rcount);
		    if (!rc)
			rc = MatchCheck(feature, fcount);
		    return rc;
		}
	    }
	    else {
		// Section comment, probably from a badly
		// encapsulated EPS file.
	        int rc = DSCerror(CDSC_MESSAGE_BAD_SECTION, 
			dsc_line, dsc_line_length);
		if (rc == CDSC_IGNORE_ALL)
		    return 1;
	    }
	}
	else if (is_dsc("%%PageTrailer")) {
	    scan_section = scan_page_trailer;
	}
	else if (is_dsc("%%BeginPageSetup")) {
	    // ignore
	}
	else if (is_dsc("%%EndPageSetup")) {
	    // ignore
	}
	else if (is_dsc("%%PageMedia:")) {
	    ParseMedia(&(page[page_count-1].media));
	}
	else if (is_dsc("%%PaperColor:")) {
	    // ignore
	}
	else if (is_dsc("%%PaperForm:")) {
	    // ignore
	}
	else if (is_dsc("%%PaperWeight:")) {
	    // ignore
	}
	else if (is_dsc("%%PaperSize:")) {
	    // DSC 2.1
	    int n = 12;
	    char buf[MAXSTR];
	    buf[0] = '\0';
	    CopyString(buf, sizeof(buf)-1, dsc_line+n, dsc_line_length-n, NULL);
	    CDSCMEDIA *m = media;
	    while (m && m->name) {
		if (stricmp(buf, m->name)==0) {
		    page[page_count-1].media = m;
		    break;
		}
		m++;
	    }
	    if ((m == NULL) || (m->name == NULL)) {
		// It didn't match %%DocumentPaperSizes:
		// Try our known media
		m = dsc_known_media;
		while (m->name) {
		    if (stricmp(buf, m->name)==0) {
			page[page_count-1].media = m;
			break;
		    }
		    m++;
		}
		if (m->name == NULL)
		    UnknownDSC();
	    }
	}
	else if (is_dsc("%%PageOrientation:")) {
	    if (ParseOrientation(&page[page_count-1].orientation ,18))
		return 1;
	}
	else if (is_dsc("%%PageBoundingBox:")) {
	    if (ParseBoundingBox(&page[page_count-1].bbox, 18))
		return 1;
	}
	else if (is_dsc("%%BeginFeature:")) {
	    // ignore Begin/EndFeature, apart form making sure
	    // that they are matched.
	    fcount++;
	}
	else if (is_dsc("%%EndFeature")) {
	    fcount--;
	}
	else if (is_dsc("%%PageResources:")) {
	    // ignore
	    SkipComment();
	}
	else if (is_dsc("%%BeginResource:")) {
	    // ignore Begin/EndResource, apart form making sure
	    // that they are matched.
	    rcount++;
	}
	else if (is_dsc("%%EndResource")) {
	    rcount--;
	}
	else if (is_dsc("%%BeginFont:")) {
	    // ignore Begin/EndFont, apart form making sure
	    // that they are matched.
	    ncount++;
	}
	else if (is_dsc("%%EndFont")) {
	    ncount--;
	}
	else if (is_dsc("%%IncludeFont:")) {
	    // ignore
	    SkipComment();
	}
	else {
	    UnknownDSC();
	}
    }
    page[page_count-1].end = dsc_start;
    return 0;
}

// Copy string on line to new allocated string newstr
// String is no longer than len
// Return pointer to string 
// Store number of used characters from line
// Don't copy outside ()
char *
CDSC::AddString(char *line, unsigned int len, unsigned int *offset)
{
    char newline[MAXSTR];
    char *newstring;

    newstring = CopyString(newline, sizeof(newline)-1, line, len, offset);
    if (newstring)
        return dsc_alloc_string(newline, strlen(newline)+1);
    return NULL;
}

// Copy string on line to new allocated string newstr
// String is always null terminated
// String is no longer than len
// Return pointer to string 
// Store number of used characters from line
// Don't copy enclosing ()
char *
CDSC::CopyString(char *str, unsigned int slen, char *line, 
	unsigned int len, unsigned int *offset)
{
    int quoted = FALSE;
    int instring=0;
    unsigned int newlength = 0;
    unsigned int i = 0;
    unsigned char ch;
    if (len > slen)
	len = slen-1;
    while ( (i<len) && line[i]==' ')
	i++;	// skip leading spaces
    if (line[i]=='(') {
	quoted = TRUE;
	instring++;
	i++; // don't copy outside ()
    }
    while (i < len) {
	str[newlength] = ch = line[i];
	i++;
	if (quoted) {
	    if (ch == '(')
		    instring++;
	    if (ch == ')')
		    instring--;
	    if (instring==0)
		    break;
	}
	else if (ch == ' ')
	    break;

	if (ch == '\r')
	    break;
	if (ch == '\n')
	    break;
	else if ( (ch == '\\') && (i+1 < len) ) {
	    ch = line[i];
	    if ((ch >= '0') && (ch <= '9')) {
		// octal coded character
		int j = 3;
		ch = 0;
		while (j && (i < len) && line[i]>='0' && line[i]<='7') {
		    ch = (unsigned char)((ch<<3) + (line[i]-'0'));
		    i++;
		    j--;
		}
		str[newlength] = ch;
	    }
	    else if (ch == '(') {
		str[newlength] = ch;
		i++;
	    }
	    else if (ch == ')') {
		str[newlength] = ch;
		i++;
	    }
	    else if (ch == 'b') {
		str[newlength] = '\b';
		i++;
	    }
	    else if (ch == 'f') {
		str[newlength] = '\b';
		i++;
	    }
	    else if (ch == 'n') {
		str[newlength] = '\n';
		i++;
	    }
	    else if (ch == 'r') {
		str[newlength] = '\r';
		i++;
	    }
	    else if (ch == 't') {
		str[newlength] = '\t';
		i++;
	    }
	    else if (ch == '\\') {
		str[newlength] = '\\';
		i++;
	    }
	}
	newlength++;
    }
    str[newlength] = '\0';
    if (offset != (unsigned int *)NULL)
        *offset = i;
    return str;
}

int 
CDSC::GetInt(char *line, unsigned int len, unsigned int *offset)
{
    char newline[MAXSTR];
    int newlength = 0;
    unsigned int i = 0;
    unsigned char ch;

    while ( (i<len) && line[i]==' ')
	i++;	// skip leading spaces
    while (i < len) {
	newline[newlength] = ch = line[i];
	i++;
	if (ch == ' ')
	    break;
	if (ch == '\r')
	    break;
	if (ch == '\n')
	    break;
	newlength++;
    }
    newline[newlength] = '\0';
    if (offset != (unsigned int *)NULL)
        *offset = i;
    return atoi(newline);
}

float 
CDSC::GetReal(char *line, unsigned int len, unsigned int *offset)
{
    char newline[MAXSTR];
    int newlength = 0;
    unsigned int i = 0;
    unsigned char ch;

    while ( (i<len) && line[i]==' ')
	i++;	// skip leading spaces
    while (i < len) {
	newline[newlength] = ch = line[i];
	i++;
	if (ch == ' ')
	    break;
	if (ch == '\r')
	    break;
	if (ch == '\n')
	    break;
	newlength++;
    }
    newline[newlength] = '\0';
    if (offset != (unsigned int *)NULL)
        *offset = i;
    return atof(newline);
}

// store line, ignoring leading spaces
char *
CDSC::AddLine(char *line, unsigned int len)
{
    char *newline;
    unsigned int i;
    while (len && (*line==' ')) {
	len--;
	line++;
    }
    newline = dsc_alloc_string(line, len);
    if (newline == NULL)
	return NULL;

    for (i=0; i<len; i++) {
	if (newline[i] == '\r') {
	    newline[i]='\0';
	    break;
	}
	if (newline[i] == '\n') {
	    newline[i]='\0';
	    break;
	}
    }
    return newline;
}

int CDSC::PDFpages(unsigned int first, unsigned int last)
{
    unsigned int i;
    char buf[32];
   
    if (page)
	free(page);
    page = NULL;

    page_count = page_pages = last - first + 1;
    page = (CDSCPAGE *)malloc(page_count * sizeof(CDSCPAGE));
    if (page == NULL)
	return -1;	// no memory
    memset((char *)page, 0, page_count * sizeof(CDSCPAGE));

    for (i=0; i<page_count; i++) {
	sprintf(buf, "%d", first+i);
	page[i].ordinal = i+1;
	if ( (page[i].label = dsc_alloc_string(buf)) == (char *)NULL )
	    return -1;	// no memory
	page[i].orientation = CDSC_PORTRAIT;
	page[i].begin = 0;
	page[i].end = 0;
	page[i].media = NULL;
	page[i].bbox = NULL;
    }
    return 0;
}

int 
CDSC::AddPage(void)
{
    char *p;
    unsigned int i;
    char page_label[MAXSTR];
    char *pl;
    int page_ordinal;

    page[page_count].begin = dsc_start;
    page[page_count].end = dsc_start;
    page[page_count].label = NULL;
    page[page_count].ordinal = 0;
    page[page_count].media = NULL;
    page[page_count].bbox = NULL;

    p = dsc_line + 7;
    pl = CopyString(page_label, sizeof(page_label), p, dsc_line_length-7, &i);
    if (pl == NULL)
	return 1;
    p += i;
    page_ordinal = atoi(p);

    if ( (page_ordinal == 0) || (strlen(page_label) == 0) ||
       (page_count && (page_ordinal != page[page_count-1].ordinal+1)) ) {
	int rc = DSCerror(CDSC_MESSAGE_PAGE_ORDINAL, dsc_line, dsc_line_length);
	switch (rc) {
	    case CDSC_OK:
		// ignore this page
		return 0;
	    case CDSC_CANCEL:
		// accept the page
		break;
	    case CDSC_IGNORE_ALL:
		return 1;
	}
    }

    page[page_count].label = dsc_alloc_string(page_label, strlen(page_label)+1);
    page[page_count].ordinal = page_ordinal;
    page[page_count].media = NULL;
    page[page_count].bbox = NULL;

    if (page[page_count].label == NULL)
	return -1;	// no memory
	
    page_count++;
    if (page_count >= page_chunk_length) {
	page = (CDSCPAGE *)realloc(page, 
	(CDSC_PAGE_CHUNK+page_count) * sizeof(CDSCPAGE));
	if (page == NULL)
	    return -1;	// out of memory
	page_chunk_length = CDSC_PAGE_CHUNK+page_count;
    }
    return 0;
}

/* Valid Trailer comments are
 * %%Trailer
 * %%EOF
 * or the following deferred with (atend)
 * %%BoundingBox:
 * %%DocumentCustomColors:
 * %%DocumentFiles:
 * %%DocumentFonts:
 * %%DocumentNeededFiles:
 * %%DocumentNeededFonts:
 * %%DocumentNeededProcSets:
 * %%DocumentNeededResources:
 * %%DocumentProcSets:
 * %%DocumentProcessColors:
 * %%DocumentSuppliedFiles:
 * %%DocumentSuppliedFonts:
 * %%DocumentSuppliedProcSets: 
 * %%DocumentSuppliedResources: 
 * %%Orientation: 
 * %%Pages: 
 * %%PageOrder: 
 *
 * Our supported subset is
 * %%Trailer
 * %%EOF
 * %%BoundingBox:
 * %%Orientation: 
 * %%Pages: 
 * %%PageOrder: 
 * In addition to these, we support
 * %%DocumentMedia:
 * 
 * A %%PageTrailer can have the following:
 * %%PageBoundingBox: 
 * %%PageCustomColors: 
 * %%PageFiles: 
 * %%PageFonts: 
 * %%PageOrientation: 
 * %%PageProcessColors: 
 * %%PageResources: 
 */

int 
CDSC::ScanTrailer(void)
{
    // Trailer section start at
    //  %%Trailer
    // and ends at
    //  %%EOF
    BOOL foundEOF = FALSE;
    scan_section = scan_trailer;
    DebugPrint("Scanning trailer\n");
    SkipBlank();	// skip blank lines
    begintrailer = endtrailer = dsc_start;
    while (NextDSCline()) {
	if (is_dsc("%%EOF")) {
	    endtrailer = dsc_end;
	    // keep scanning, in case we have a false trailer
	    // return 0;
            foundEOF = TRUE;
	}
	else if (is_dsc("%%Trailer")) {
	    // Cope with no pages with code after setup and before trailer.
	    // Last trailer is the correct one.
	    begintrailer = dsc_start;
	}
	else if (is_dsc("%%Pages:")) {
	    if (ParsePages()!=0)
		return 1;
	}
	else if (is_dsc("%%BoundingBox:")) {
	    if (ParseBoundingBox(&bbox, 14))
		return 1;
	}
	else if (is_dsc("%%Orientation:")) {
	    if (ParseOrientation(&page_orientation, 14))
		return 1;
	}
	else if (is_dsc("%%PageOrder:")) {
	    if (ParseOrder())
		return 1;
	}
	else if (is_dsc("%%DocumentMedia:")) {
	    if (ParseDocumentMedia())
		return 1;
	}
	else if (is_dsc("%%Page:")) {
	    UnknownDSC();
	    if (page_count) {
		int rc = DSCerror(CDSC_MESSAGE_PAGE_IN_TRAILER, 
			dsc_line, dsc_line_length);
		switch (rc) {
		    case CDSC_OK:
	    		// Assume that we are really in the previous 
			// page, not the trailer
			foundEOF = FALSE;
			page[page_count-1].end = dsc_start;
			ScanPages();
			// restart trailer scan
			scan_section = scan_trailer;
			DebugPrint("Scanning trailer\n");
			SkipBlank();	// skip blank lines
			begintrailer = endtrailer = dsc_start;
			break;
		    case CDSC_CANCEL:
			// ignore pages in trailer
			break;
		    case CDSC_IGNORE_ALL:
			return 1;
		}
	    }
	}
	else if (is_dsc("%%DocumentNeededFonts:")) {
	    // ignore
	    SkipComment();
	}
	else if (is_dsc("%%DocumentSuppliedFonts:")) {
	    // ignore
	    SkipComment();
	}
	else if ((dsc_line[0]=='%') && (dsc_line[0]=='%')) {
	    UnknownDSC();
	}
    }
    if (!foundEOF)
        endtrailer = dsc_end;
    return 0;
}


char *
CDSC::dsc_alloc_string(char *str, int len)
{
    char *p;
    if (string_head == NULL) {
	string_head = (CDSCSTRING *)malloc(sizeof(CDSCSTRING));
	if (string_head == NULL)
	    return NULL;	// no memory
	string = string_head;
	string->next = NULL;
	string->data = (char *)malloc(CDSC_STRING_CHUNK);
	if (string->data == NULL) {
	    Reset();
	    return NULL;	// no memory
	}
	string->index = 0;
	string->length = CDSC_STRING_CHUNK;
    }
    if ( string->index + len + 1 > string->length) {
	// allocate another string block
	CDSCSTRING *newstring = (CDSCSTRING *)malloc(sizeof(CDSCSTRING));
	if (newstring == NULL) {
	    DebugPrint("Out of memory\n");
	    return NULL;
	}
        newstring->next = NULL;
	newstring->length = 0;
	newstring->index = 0;
	newstring->data = (char *)malloc(CDSC_STRING_CHUNK);
	if (newstring->data == NULL) {
	    free(newstring);
	    DebugPrint("Out of memory\n");
	    return NULL;	// no memory
	}
	newstring->length = CDSC_STRING_CHUNK;
	string->next = newstring;
	string = newstring;
    }
    if ( string->index + len + 1 > string->length)
	return NULL;	/* failed */
    p = string->data + string->index;
    memcpy(p, str, len);
    *(p+len) = '\0';
    string->index += len + 1;
#ifdef DEBUG_MALLOC
{char buf[256];
sprintf(buf,"strings=%d/%d\n", string->index, string->length);
DebugPrint(buf);
}
#endif
    return p;
}

char *
CDSC::dsc_alloc_string(char *str)
{
    return dsc_alloc_string(str, strlen(str));
}


/* read line of up to 255 characters */
/* return count of characters read */
// doesn't actually copy characters to dsc_line, so don't modify them
int
CDSC::ReadLine(void)
{
    char *p, *last;
    dsc_line = NULL;
    // Try to have enough data for
    //   255 characters
    //   return + newline
    //   %%+
    //   return + newline
    // total=262
    if (dsc_data_index + DSC_LINE_LENGTH + CDSC_READ_EXTRA  > dsc_data_length) {
        ULONG cbRead;
        if (dsc_data_length && dsc_data_index) {
            memmove(dsc_data, dsc_data+dsc_data_index, 
		dsc_data_length-dsc_data_index);
		dsc_data_base += dsc_data_index;
            dsc_data_length = dsc_data_length - dsc_data_index;
            dsc_data_index = 0;
	}
	cbRead = cfile->Read(dsc_data + dsc_data_length, CDSC_READ_LENGTH);
        dsc_data_length += cbRead;
        if (dsc_data_length == 0) {
	    dsc_line = dsc_data;
	    dsc_line_length = 0;
	    return 0;	/* no more characters */
	}
	if ( doseps_end && (dsc_data_base + dsc_data_length > doseps_end) )
	    dsc_data_length = doseps_end - dsc_data_base;
    }
    dsc_line = dsc_data + dsc_data_index;
    // return at most 255 characters
    last = dsc_data + ((dsc_data_index + DSC_LINE_LENGTH < dsc_data_length) ? 
		dsc_data_index + DSC_LINE_LENGTH : dsc_data_length);
	
	
    if (dsc_eol)	// if previous line was complete, increment line count
	line_count++;
	
    dsc_eol = FALSE;
    for (p = dsc_line; p < last; p++) {
        if (*p == '\r') {
            if ((p<last) && (*(p+1) == '\n'))
                p++;	/* include newline also */
			p++;
			dsc_eol = TRUE;	// dsc_line is a complete line
			break;
        }
        if (*p == '\n') {
			p++;
			dsc_eol = TRUE;	// dsc_line is a complete line
            break;
		}
        if (*p == '\032') {
			dsc_eol = TRUE;
		}
    }
    dsc_data_index += dsc_line_length = (p - dsc_line);
	
    if ((dsc_line[0]=='%') && (dsc_line[1]=='%'))  {
	if (compare(dsc_line, "%%BeginData:")) {
	    //%%BeginData: <numberof>[ <type> [ <bytesorlines> ] ] 
	    //<numberof> ::= <uint> (Lines or physical bytes) 
	    //<type> ::= Hex | Binary | ASCII (Type of data) 
	    //<bytesorlines> ::= Bytes | Lines (Read in bytes or lines) 
	    char begindata[MAXSTR+1];
	    int cnt;
	    char *numberof, *bytesorlines;
	    memcpy(begindata, dsc_line, dsc_line_length);
	    begindata[dsc_line_length] = '\0';
	    numberof = strtok(begindata+12, " \r\n");
	    strtok(NULL, " \r\n");	// dump type
	    bytesorlines = strtok(NULL, " \r\n");
	    if ( (numberof == NULL) || (bytesorlines == NULL) ) {
		// invalid usage of %%BeginData
		// ignore that we ever saw it
		int rc = DSCerror(CDSC_MESSAGE_INCORRECT_USAGE, dsc_line, 
			dsc_line_length);
		switch (rc) {
		    case CDSC_OK:
		    case CDSC_CANCEL:
			break;;
		    case CDSC_IGNORE_ALL:
			return 0;
		}
	    }
	    else {
		cnt = atoi(numberof);
		if (cnt) {
		    if (bytesorlines && (stricmp(bytesorlines, "Lines")==0)) {
			// skip cnt lines
			while (cnt) {
			    ReadLine();
			    if (dsc_eol)  // handle excessively long lines
				cnt--;
			}
			ReadLine();
		    }
		    else {
			// byte count doesn't includes \n or \r\n 
			// or \r of %%BeginData:
			// skip cnt bytes
			Seek(cnt, SEEK_CUR, NULL);
			ReadLine();
		    }
		}
	    }
	}
	else if (compare(dsc_line, "%%BeginBinary:")) {
	    // byte count doesn't includes \n or \r\n or \r of %%BeginBinary:
	    ULONG skip_bytes = atoi(dsc_line + 14);
	    Seek(skip_bytes, SEEK_CUR, NULL);
	    ReadLine();
	}
    }
	
    if ((dsc_line[0]=='%') && (dsc_line[1]=='%') &&
	compare(dsc_line, "%%BeginDocument:") ) {
	// Skip over embedded document, recursively
	do {
	    dsc_line_length = NextDSCline();
	} while (dsc_line_length && !compare(dsc_line, "%%EndDocument"));
	ReadLine();  // return line after %%EndDocument
    }

    if (!dsc_eol && !long_line && (dsc_data_length - dsc_data_index)) {
	DSCerror(CDSC_MESSAGE_LONG_LINE, dsc_line, dsc_line_length);
        long_line = TRUE;
    }
	
    return dsc_line_length;
}

/* DSC line, skipping over included files and */
/* binary/data sections */
int
CDSC::NextDSCline(void)
{
    int bytes_read;
    do {
	bytes_read = ReadLine();
    } while (bytes_read && !((dsc_line[0]=='%') && (dsc_line[1])=='%'));
    return bytes_read;
}

// skip over any blank lines
int
CDSC::SkipBlank(void)
{
    int bytes_read = dsc_line_length;
    while (bytes_read && (dsc_line[0]=='\r' || dsc_line[0]=='\n'))
	bytes_read = ReadLine();
    return bytes_read;
}

int
CDSC::Seek(LONG iMove, DWORD dwOrigin, ULONG *piNewPosition)
{
    if (dwOrigin == SEEK_CUR)
	dsc_data_base = cfile->Seek(iMove+dsc_data_base+dsc_data_index, CFile::begin); 
    else
	dsc_data_base = cfile->Seek(iMove, CFile::begin);
    dsc_data_length = dsc_data_index = 0;
    dsc_line = dsc_data;
    dsc_line_length = 0;
    if (piNewPosition)
	*piNewPosition = dsc_data_base;
    return 0;	// OK
}

#ifdef AFX
void CDSC::Dump(CDumpContext & dc)
{
    Display(
    if (dsc)
        dc << "DSC ";
    if (epsf)
        dc << "EPSF ";
    if (doseps)
        dc << "DOSEPS ";
    if (pdf)
        dc << "PDF ";
    if (pjl)
        dc << "PJL ";
    if (ctrld)
        dc << "CTRLD ";
    dc << "\n";
    dc << "comments " <<  begincomments << " " 
		<< endcomments << "\n";
    dc << "preview " << beginpreview << " " 
		<< endpreview << "\n";
    dc << "defaults " << begindefaults << " " 
		<< enddefaults << "\n";
    dc << "prolog " << beginprolog << " " 
		<< endprolog << "\n";
    dc << "setup " << beginsetup << " " 
		<< endsetup << "\n";
    if (bbox) {
		dc << "boundingbox " 
			<< bbox->llx << " " 
			<< bbox->lly << " " 
			<< bbox->urx << " " 
			<< bbox->ury << "\n";
    }
    dc << "pages " << page_pages << "\n";
	char *p;
    switch (page_order) {
	case CDSC_UNKNOWN:
		p = "unknown, assume ascend";
		break;
	case CDSC_ASCEND:
		p = "ascend";
		break;
	case CDSC_DESCEND:
		p = "descend";
		break;
	case CDSC_SPECIAL:
		p = "special";
		break;
    }
    dc << "page order " << p << "\n";
    for (UINT i=0; i<page_count; i++)
        dc << "page "
		<< page[i].label << " " 
		<< page[i].ordinal << " at " 
		<< page[i].begin << " " 
		<< page[i].end << "\n";
    dc << "trailer " 
		<< begintrailer << " " 
		<< endtrailer << "\n";
}
#endif



BOOL CDSC::PageValid(UINT new_page)
{
    if ( !dsc || (new_page == 0) || (page_count == 0) || 
	(new_page > page_count) )
	return FALSE;
    return TRUE;
}


void CDSC::SetDebug( void (*debug_fn)(char *str) )
{
    debug_print_fn = debug_fn;
}

void CDSC::DebugPrint( char *str )
{
    if (debug_print_fn)
	debug_print_fn(str);
}

void CDSC::SkipComment()
{
    // skip over continuation of DSC line
    while (dsc_line_length && is_continued())
	NextDSCline();
}

void CDSC::UnknownDSC(void)
{
    if (debug_print_fn) {
	DebugPrint(" Unknown: ");
	char line[DSC_LINE_LENGTH];
	unsigned int length = min(DSC_LINE_LENGTH-1, dsc_line_length);
	strncpy(line, dsc_line, length);
	line[length] = '\0';
	DebugPrint(line);
    }

    SkipComment();
}

void CDSC::Display( void (*dfn)(char *str) )
{
    unsigned int i;
    char *p;
    char buf[MAXSTR];
    (*dfn)("== DSC dump ==\n");
    if (dsc)
        (*dfn)("DSC ");
    if (epsf)
        (*dfn)("EPSF ");
    if (doseps)
        (*dfn)("DOSEPS ");
    if (pdf)
        (*dfn)("PDF ");
    if (pjl)
        (*dfn)("PJL ");
    if (ctrld)
        (*dfn)("CTRLD ");
    (*dfn)("\n");

    if (dsc_version) {
	(*dfn)("version ");
	(*dfn)(dsc_version);
	(*dfn)("\n");
    }
    if (dsc_title) {
	(*dfn)("title ");
	(*dfn)(dsc_title);
	(*dfn)("\n");
    }
    if (dsc_creator) {
	(*dfn)("creator ");
	(*dfn)(dsc_creator);
	(*dfn)("\n");
    }
    if (dsc_date) {
	(*dfn)("date ");
	(*dfn)(dsc_date);
	(*dfn)("\n");
    }
    if (dsc_for) {
	(*dfn)("for ");
	(*dfn)(dsc_for);
	(*dfn)("\n");
    }

    if (doseps) {
	sprintf(buf, "doseps: ps=%ld %ld  wmf=%ld %ld  tiff=%ld %ld\n",
	    doseps->ps_begin, doseps->ps_length,
	    doseps->wmf_begin, doseps->wmf_length,
	    doseps->tiff_begin, doseps->tiff_length);
	(*dfn)(buf);
    }
    if (bbox) {
	sprintf(buf, "boundingbox %d %d %d %d\n",
	    bbox->llx, bbox->lly, bbox->urx, bbox->ury);
        (*dfn)(buf);
    }
    if (language_level != 0) {
        sprintf(buf, "language level %d\n", language_level);
        (*dfn)(buf);
    }
    if (document_data != CDSC_UNKNOWN) {
	switch (document_data) {
	    case CDSC_CLEAN7BIT:
		    p = "Clean7Bit";
		    break;
	    case CDSC_CLEAN8BIT:
		    p = "Clean8Bit";
		    break;
	    case CDSC_BINARY:
		    p = "Binary";
		    break;
	    default:
		    p = "Unknown";
	}
	sprintf(buf, "document data %s\n", p);
	(*dfn)(buf);
    }


    if (page_order != CDSC_UNKNOWN) {
	switch (page_order) {
	    case CDSC_ASCEND:
		    p = "Ascend";
		    break;
	    case CDSC_DESCEND:
		    p = "Descend";
		    break;
	    case CDSC_SPECIAL:
		    p = "Special";
		    break;
	    default:
		    p = "Unknown";
	}
	sprintf(buf, "page order %s\n", p);
	(*dfn)(buf);
    }

    if (page_orientation != CDSC_UNKNOWN) {
	switch (page_orientation) {
	    case CDSC_PORTRAIT:
		    p = "Portrait";
		    break;
	    case CDSC_LANDSCAPE:
		    p = "Landscape";
		    break;
	    default:
		    p = "Unknown";
	}
	sprintf(buf, "page orientation %s\n", p);
	(*dfn)(buf);
    }
    if (page_bbox) {
	sprintf(buf, "default page boundingbox %d %d %d %d\n",
	    page_bbox->llx, page_bbox->lly, page_bbox->urx, page_bbox->ury);
        (*dfn)(buf);
    }

    for (i=0; media && (i<media_count); i++) {
	sprintf(buf, "media %d   (%.50s) %g %g %g (%.50s) (%.50s)\n", i,
	    media[i].name, media[i].width, media[i].height,
	    media[i].weight, media[i].colour, media[i].type);
        (*dfn)(buf);
    }
    if (page_media) {
	sprintf(buf, "default page media %.50s\n", page_media->name);
        (*dfn)(buf);
    }

    sprintf(buf, "comments %ld %ld\n", begincomments, endcomments);
    (*dfn)(buf);
    sprintf(buf, "preview %ld %ld\n", beginpreview, endpreview);
    (*dfn)(buf);
    sprintf(buf, "defaults %ld %ld\n", begindefaults, enddefaults);
    (*dfn)(buf);
    sprintf(buf, "prolog %ld %ld\n", beginprolog, endprolog);
    (*dfn)(buf);
    sprintf(buf, "setup %ld %ld\n", beginsetup, endsetup);
    (*dfn)(buf);
    sprintf(buf, "pages %d\n", page_pages);
    (*dfn)(buf);
    for (i=0; i<page_count; i++) {
	sprintf(buf, "page %.20s %d  %ld %ld\n",
	    page[i].label, page[i].ordinal, page[i].begin, page[i].end);
        (*dfn)(buf);
	if (page[i].media) {
	    sprintf(buf, "  page media %s\n", page[i].media->name);
            (*dfn)(buf);
	}
	if (page[i].orientation != CDSC_UNKNOWN) {
	    switch (page[i].orientation) {
		case CDSC_PORTRAIT:
			p = "Portrait";
			break;
		case CDSC_LANDSCAPE:
			p = "Landscape";
			break;
		default:
			p = "Unknown";
	    }
	    sprintf(buf, "  page orientation %s\n", p);
	    (*dfn)(buf);
	}
    }
    sprintf(buf, "trailer %ld %ld\n", begintrailer, endtrailer);
    (*dfn)(buf);
    (*dfn)("== END DSC dump ==\n");
}


//////////////////////////////////////////////////////////////////////

// DSC error reporting

void CDSC::SetErrorFunction(DSC_ERROR_FN fn)
{
    dsc_error_fn = fn;
}

// return code = 
//    CDSC_CONTINUE   (usually treat line as being correct)
//    CDSC_IGNORE     (usually treat line as being incorrect)
//    CDSC_IGNORE_ALL (ignore all DSC)
// explanation = an index to two messages:
//    A one line description of the error
//    A multiline explanation
// line = pointer to the offending DSC line (if any)

// This should be replaced by a GUI supplied by the caller


int CDSC::DSCerror(unsigned int explanation, char *line, unsigned int line_len)
{
    int response;
    char buf[MAXSTR];
    if (explanation > DSC_MAX_ERROR)
	return CDSC_OK;

    int severity = dsc_severity[explanation];

    // If debug function provided, copy messages there
    if (debug_print_fn) {
	switch (severity) {
	    case CDSC_ERROR_INFORM:
		DebugPrint("\nDSC Information");
		break;
	    case CDSC_ERROR_WARN:
		DebugPrint("\nDSC Warning");
		break;
	    case CDSC_ERROR_ERROR:
		DebugPrint("\nDSC Error");
	}
	DebugPrint("\n");
	if (explanation <= DSC_MAX_ERROR) {
	    if (line && line_len) {
		sprintf(buf, "At line %d:\n", line_count);
		DebugPrint(buf);
		int length = min(line_len, sizeof(buf)-1);
		strncpy(buf, line, length);
		buf[length]='\0';
		DebugPrint("  ");
		DebugPrint(buf);
	    }
	    DebugPrint(dsc_message[explanation]);
	}
    }

    // Call user supplied error function
    if (dsc_error_fn)
	response = dsc_error_fn(explanation, line_count, line, line_len);
    else {
	// if no error function, treat DSC as being correct
	response = CDSC_CANCEL;
    }

    if (debug_print_fn) {
	switch (response) {
	    case CDSC_OK:
		DebugPrint("Response = OK\n");
		break;
	    case CDSC_CANCEL:
		DebugPrint("Response = Cancel\n");
		break;
	    case CDSC_IGNORE_ALL:
		DebugPrint("Response = Ignore All DSC\n");
		break;
	}
    }

    return response;
}
