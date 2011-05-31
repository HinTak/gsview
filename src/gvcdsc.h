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
// gvcdsc.h: interface for the CDSC class.
//////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef CDSC_MESSAGE_DEFINED
#define CDSC_MESSAGE_DEFINED
#define CDSC_MESSAGE_BBOX 0
#define CDSC_MESSAGE_EARLY_TRAILER 1
#define CDSC_MESSAGE_EARLY_EOF 2
#define CDSC_MESSAGE_PAGE_IN_TRAILER 3
#define CDSC_MESSAGE_PAGE_ORDINAL 4
#define CDSC_MESSAGE_PAGES_WRONG 5
#define CDSC_MESSAGE_EPS_NO_BBOX 6
#define CDSC_MESSAGE_EPS_PAGES 7
#define CDSC_MESSAGE_NO_MEDIA 8
#define CDSC_MESSAGE_ATEND 9
#define CDSC_MESSAGE_DUP_COMMENT 10
#define CDSC_MESSAGE_DUP_TRAILER 11
#define CDSC_MESSAGE_BEGIN_END 12
#define CDSC_MESSAGE_BAD_SECTION 13
#define CDSC_MESSAGE_LONG_LINE 14
#define CDSC_MESSAGE_INCORRECT_USAGE 15
#endif

#define DSC_LINE_LENGTH 255

#define CDSC_STRING_CHUNK 4096
#define CDSC_PAGE_CHUNK 128

// When reading DSC files, read in large chunks for efficiency
#define CDSC_READ_LENGTH 4096
// We need a buffer which contains one full DSC line, plus the following
//   return + newline  (2)
//   %%+               (3)
//   return + newline  (2)
#define CDSC_READ_EXTRA 7
#define CDSC_READ_BUFFER_LENGTH (CDSC_READ_LENGTH + DSC_LINE_LENGTH + CDSC_READ_EXTRA)


#define CDSC_UNKNOWN 0

enum PreviewType {
    CDSC_NOPREVIEW = 0,
    CDSC_EPSI = 1,
    CDSC_TIFF = 2,
    CDSC_WMF = 3,
    CDSC_PICT = 4
};

enum PageOrder {
    // CDSC_UNKNOWN = 0,
    CDSC_ASCEND = 1,
    CDSC_DESCEND = 2,
    CDSC_SPECIAL = 3
};

enum PageOrientation {
    // CDSC_UNKNOWN = 0,
    CDSC_PORTRAIT = 1,
    CDSC_LANDSCAPE = 2,
};

enum DocumentData {
    // CDSC_UNKNOWN = 0,
    CDSC_CLEAN7BIT = 1,
    CDSC_CLEAN8BIT = 2,
    CDSC_BINARY = 3
};

typedef struct tagCDSCMEDIA {
    char *name;
    float width;	/* PostScript points */
    float height;
    float weight;	/* GSM */
    char *colour;
    char *type;
} CDSCMEDIA;

#define CDSC_KNOWN_MEDIA 11
extern CDSCMEDIA dsc_known_media[CDSC_KNOWN_MEDIA];

typedef struct tagCDSCBBOX {
    int llx;
    int lly;
    int urx;
    int ury;
} CDSCBBOX;

typedef struct tagCDSCPAGE {
    int ordinal;
    char *label;
    ULONG begin;
    ULONG end;
    UINT orientation;
    CDSCMEDIA *media;
    CDSCBBOX *bbox;
} CDSCPAGE;

typedef struct tagCDSCDOSEPS {
    DWORD ps_begin;
    DWORD ps_length;
    DWORD wmf_begin;
    DWORD wmf_length;
    DWORD tiff_begin;
    DWORD tiff_length;
    WORD checksum;
} CDSCDOSEPS;

typedef struct tagCDSCSTRING {
    unsigned int index;
    unsigned int length;
    char *data;
    struct tagCDSCSTRING *next;
} CDSCSTRING;

//////////////////////////////////////////////////////////////////////
// DSC error reporting
//////////////////////////////////////////////////////////////////////

// severity
#define CDSC_ERROR_INFORM	0	// Not an error
#define CDSC_ERROR_WARN		1	// Not a DSC error itself, 
					// but might be if misused
#define CDSC_ERROR_ERROR	2	// DSC error

// response
#define CDSC_OK		0
#define CDSC_CANCEL	1
#define CDSC_IGNORE_ALL	2

typedef int (*DSC_ERROR_FN)(unsigned int explanation, unsigned int line_count, 
	char *line, unsigned int line_len);

extern char * dsc_message[];
extern int dsc_severity[];

//////////////////////////////////////////////////////////////////////

class CDSC // : public CObject  
{
public:
	BOOL PageValid(UINT new_page);
//	void Dump(CDumpContext &dc);
	CDSC();
	virtual ~CDSC();

    BOOL dsc;		/* TRUE if DSC comments found */
    BOOL ctrld;		/* TRUE if has CTRLD at start of stream */
    BOOL pjl;		/* TRUE if has HP PJL at start of stream */
    BOOL epsf;		/* TRUE if EPSF */
    BOOL pdf;		/* TRUE if Portable Document Format */
    UINT preview;	/* enum PreviewType */
    char *dsc_version;	/* first line of file */
    UINT language_level;
    UINT document_data;	/* Clean7Bit, Clean8Bit, Binary */
    /* DSC sections */
    ULONG begincomments;
    ULONG endcomments;
    ULONG beginpreview;
    ULONG endpreview;
    ULONG begindefaults;
    ULONG enddefaults;
    ULONG beginprolog;
    ULONG endprolog;
    ULONG beginsetup;
    ULONG endsetup;
    ULONG begintrailer;
    ULONG endtrailer;
    CDSCPAGE *page;
    UINT page_chunk_length;	/* number of pages allocated */
    UINT page_count;		/* number of %%Page: pages in document */
    UINT page_pages;		/* number of pages in document from %%Pages: */
    UINT page_order;		/* enum PageOrder */
    UINT page_orientation;	/* the default page orientation */
    UINT media_count;		/* number of media items */
    CDSCMEDIA *media;		/* the array of media */
    CDSCMEDIA *page_media;	/* the default page media */
    CDSCBBOX *bbox;		/* the document bounding box */
    CDSCBBOX *page_bbox;	/* the default page bounding box */
    CDSCDOSEPS *doseps;		/* DOS binary header */
    ULONG doseps_end;		/* ps_begin+ps_length, otherwise 0 */
    char *dsc_title;
    char *dsc_creator;
    char *dsc_date;
    char *dsc_for;

    /* functions */

    /* Locate DSC comments */
    /* return value 0 = OK, 1 = fault or ignore DSC, -1 = out of memory */
    int Scan(CFile *cf);

    /* free all memory and set pointer to NULL */
    /* Must be called by the constructor and destructor */
    void Reset(void);

    /* display structure */
    void Display( void (*display)(char *str) );

    /* Set page array for PDF */
    int PDFpages(unsigned int first, unsigned int last);

    /* install print function for debug messages */
    void SetDebug( void (*debug_fn)(char *str) );

    /* install error query function */
    void SetErrorFunction(DSC_ERROR_FN fn);

private:

    DSC_ERROR_FN dsc_error_fn;
    int DSCerror(unsigned int explanation, char *line, unsigned int line_len);

    CFile *cfile;
    unsigned long file_length;

    /* Locate DSC comments */
    int Scan(void);
    int scan_section;	/* section currently being scanned */

    /* function for printing debug messages */
    void (*debug_print_fn)(char *str);

    /* print a debug message if debug print function installed */
    void DebugPrint(char *str);

    /* skip over continuation of a DSC line */
    void SkipComment();

    /* print an unknown DSC line as a debug message */
    void UnknownDSC(void);

    /* more efficient string storage than malloc */
    CDSCSTRING *string_head;	// linked list head
    CDSCSTRING *string;		// current list item
    char *dsc_alloc_string(char *str);
    char *dsc_alloc_string(char *str, int len);
    char *AddString(char *line, unsigned int len, unsigned int *offset);
    char *AddLine(char *line, unsigned int len);

    /* buffer for input */
    char *dsc_data;	// start of buffer
    unsigned int dsc_data_length;	// length of buffer
    unsigned int dsc_data_index;	// offset to next char in buffer
    ULONG dsc_data_base;		// ftell like offset to dsc_data

    char *dsc_line;		// pointer to last read DSC line
				// not null terminated
    unsigned int dsc_line_length; // number of characters in line
    BOOL dsc_eol;		// dsc_line contains EOL
    unsigned int line_count;	// line number
    BOOL long_line;		// found a line longer than 255 characters

    /* read values from line */
    char *CopyString(char *str, unsigned int slen, char *line, unsigned int len, unsigned int *offset);
    int GetInt(char *line, unsigned int len, unsigned int *offset);
    float GetReal(char *line, unsigned int len, unsigned int *offset);

    /* scanning individual sections */
    int is_section(void);
    BOOL is_continued(void);
    int ScanType(void);
    DWORD GetDWORD(unsigned char *buf);
    WORD GetWORD(unsigned char *buf);
    int ReadDOSEPSheader(void);
    int ParsePages(void);
    int ParseBoundingBox(CDSCBBOX **pbbox, int offset);
    int ParseOrientation(UINT *porientation, int offset);
    int ParseOrder(void);
    int ExtendMedia(unsigned int len);
    int ParseDocumentMedia(void);
    int ParseMedia(CDSCMEDIA **page_media);
    int BeginEndMatch(const char *str, int count);
    int MatchCheck(const char *str, int count);
    int ScanComments(void);
    int ScanProlog(void);
    int ScanPreview(void);
    int ScanDefaults(void);
    int ScanSetup(void);
    int ScanPages(void);
    int AddPage(void);
    int ScanTrailer(void);
    int ReadLine(void);
    int NextDSCline(void);
    int SkipBlank(void);
    int Seek(LONG iMove, DWORD dwOrigin, ULONG *piNewPosition);
};

