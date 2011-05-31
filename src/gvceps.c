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

/* gvceps.c */
/* EPS file manipulation module of PM and Windows GSview */

#ifdef EPSTOOL
#include "epstool.h"
#else	/* GSview */
#ifdef _Windows
#include "gvwin.h"
#else
#include "gvpm.h"
#endif
#include "gvceps.h"
#endif

PSBBOX bbox;

#ifndef EPSTOOL
/* At present only allows bounding box to be specified */
void
ps_to_eps(void)
{
char output[MAXSTR];
FILE *f;
char *buffer;
UINT count;
FILE *infile;
time_t t;
char *now;
char text[PSLINELENGTH];
char *comment;
long here;

	load_string(IDS_EPSREAD, output, sizeof(output));
	if (message_box(output, MB_YESNO | MB_ICONQUESTION)
	    != IDYES) {
	    load_string(IDS_TOPICPSTOEPS, szHelpTopic, sizeof(szHelpTopic));
	    get_help();
	    return;
	}

	if (!(display.page || display.sync)) {
	    gserror(IDS_EPSNOBBOX, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    return;
	}

	if ((doc != (PSDOC *)NULL) && (doc->numpages > 1)) {
	    gserror(IDS_EPSONEPAGE, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    return;
	}
	if (doc == (PSDOC *)NULL) {
	    char mess[MAXSTR];
	    load_string(IDS_EPSQPAGES, mess, sizeof(mess));
	    if (message_box(mess, MB_YESNO | MB_ICONQUESTION) != IDYES)
		return;
	}

	if (!get_bbox()) {
	    play_sound(SOUND_ERROR);
	    return;
	}

	output[0] = '\0';
	if (!get_filename(output, TRUE, FILTER_PS, 0, IDS_TOPICPSTOEPS))
	    return;

	if ((f = fopen(output, "wb")) == (FILE *)NULL) {
	    play_sound(SOUND_ERROR);
	    return;
	}

	if (doc == (PSDOC *)NULL) {
	    load_string(IDS_WAITWRITE, szWait, sizeof(szWait));
	    info_wait(TRUE);
	    fputs("%!PS-Adobe-3.0 EPSF-3.0\r\n",f);
	    /* if this is not a single page document then gsview has just lied */
	    fprintf(f, "%%%%BoundingBox: %u %u %u %u\r\n",
		bbox.llx,bbox.lly,bbox.urx,bbox.ury);
	    fprintf(f,"%%%%Title: %s\r\n",psfile.name);
	    fprintf(f,"%%%%Creator: %s from %s\r\n",szAppName,psfile.name);
	    t = time(NULL);
	    now = ctime(&t);
	    now[strlen(now)-1] = '\0';	/* remove trailing \n */
	    fprintf(f,"%%%%CreationDate: %s\r\n",now);
	    fputs("%%Pages: 1\r\n",f);
	    fputs("%%EndComments\r\n",f);

	    fputs("%%Page: 1 1\r\n",f);
	    fprintf(f,"%%BeginDocument: %s\r\n",psfile.name);

	    /* create buffer for PS file copy */
	    buffer = malloc(COPY_BUF_SIZE);
	    if (buffer == (char *)NULL) {
	        play_sound(SOUND_ERROR);
	        fclose(f);
		unlink(output);
	        return;
	    }

	    infile = fopen(psfile.name, "rb");
	    if (infile == (FILE *)NULL) {
	        play_sound(SOUND_ERROR);
	        fclose(f);
		unlink(output);
	        return;
	    }

            while ( (count = fread(buffer, 1, COPY_BUF_SIZE, infile)) != 0 ) {
	        fwrite(buffer, 1, count, f);
	    }
	    free(buffer);
	    fclose(infile);

	    fputs("%%EndDocument\r\n",f);
	    fputs("%%Trailer\r\n",f);
	    fclose(f);
	    info_wait(FALSE);
	}
	else {
	    /* document already has DSC comments */
	    load_string(IDS_WAITWRITE, szWait, sizeof(szWait));
	    info_wait(TRUE);
	    dfreopen();
	    fseek(psfile.file, doc->begincomments, SEEK_SET);
	    fgets(text, PSLINELENGTH, psfile.file);
	    if (doc->epsf)
	        fputs(text,f);
	    else
	        fputs("%!PS-Adobe-3.0 EPSF-3.0\r\n",f);
	    if (doc->bbox.valid) {
	        if ( (comment = dsc_copy(psfile.file, f, -1,
			   doc->endcomments, "%%BoundingBox:")) != (char *)NULL ) {
		    free(comment);
	        }
	    }
	    fprintf(f, "%%%%BoundingBox: %d %d %d %d\r\n",
		bbox.llx, bbox.lly, bbox.urx, bbox.ury);
	    here = ftell(psfile.file);
	    dsc_copy(psfile.file, f, here, doc->endtrailer, NULL);
	    dfclose();
	    fclose(f);
	    info_wait(FALSE);
	}
}
#endif

typedef struct tagWINRECT {
	WORD	left;
	WORD	top;
	WORD	right;
	WORD	bottom;
} WINRECT;

typedef struct {
    DWORD	key;
    WORD 	hmf;
    WINRECT 	bbox;
    WORD	inch;
    DWORD	reserved;
    WORD	checksum;
} METAFILEHEADER;


/* extract EPS or TIFF or WMF file from DOS EPS file */
void 
extract_doseps(int command)
{
unsigned long pos;
unsigned long len;
unsigned int count;
char *buffer;
FILE* epsfile;
BOOL is_meta = TRUE;
char outname[MAXSTR];
FILE *outfile;
unsigned int filter;
	if ((doc == (PSDOC *)NULL) || (doc->doseps == (DOSEPS *)NULL)) {
	    gserror(IDS_NOPREVIEW, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    return;
	}
	epsfile = fopen(psfile.name,"rb");
	pos = doc->doseps->ps_begin;
	len = doc->doseps->ps_length;
	if (command == IDM_EXTRACTPRE) {
	    pos = doc->doseps->mf_begin;
	    len = doc->doseps->mf_length;
	    if (pos == 0L) {
	        pos = doc->doseps->tiff_begin;
	        len = doc->doseps->tiff_length;
	        is_meta = FALSE;
	    }
	}
	if (pos == 0L) {
	    fclose(epsfile);
	    gserror(IDS_NOPREVIEW, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    return;
	}
	fseek(epsfile, pos, SEEK_SET);	/* seek to section to extract */


#ifdef EPSTOOL
	/* assume outname already exists */
	strcpy(outname, oname);
	if (*outname!='\0')
	    outfile = fopen(outname,"wb");
	else
	    outfile = stdout;
#else
	/* create postscript or preview file */
	outname[0] = '\0';
	if (command == IDM_EXTRACTPRE) {
	    if (is_meta)
	        filter = FILTER_WMF;
	    else
	        filter = FILTER_TIFF;
	}
	else
	    filter = FILTER_PS;
	if (!get_filename(outname, TRUE, filter, 0, IDS_TOPICEDIT)) {
	    fclose(epsfile);
	    return;
	}
	outfile = fopen(outname, "wb");
#endif
	if (outfile == (FILE *)NULL) {
	    play_sound(SOUND_ERROR);
	    fclose(epsfile);
	    return;
	}
	
	/* create buffer for file copy */
	buffer = malloc(COPY_BUF_SIZE);
	if (buffer == (char *)NULL) {
	    play_sound(SOUND_ERROR);
	    fclose(epsfile);
	    if (*outname!='\0')
	        fclose(outfile);
	    return;
	}

	if ((command == IDM_EXTRACTPRE) && is_meta) {
	    /* write placeable Windows Metafile header */
	    METAFILEHEADER mfh;
	    int i;
	    unsigned short *pw;
	    mfh.key = reorder_dword(0x9ac6cdd7L);
	    mfh.hmf = 0;
	    mfh.bbox.left = 0;
	    mfh.bbox.right = reorder_word((WORD)(doc->bbox.urx - doc->bbox.llx));
	    mfh.bbox.top = 0;
	    mfh.bbox.bottom = reorder_word((WORD)(doc->bbox.ury - doc->bbox.lly));
	    mfh.inch = reorder_word(72);	/* PostScript points */
	    mfh.reserved = 0L;
	    mfh.checksum =  0;
	    pw = (WORD *)&mfh;
	    for (i=0; i<10; i++) {
	    	mfh.checksum ^= *pw++;
	    }
	    fwrite(&mfh, sizeof(mfh), 1, outfile);
	}

        while ( (count = (unsigned int)min(len,COPY_BUF_SIZE)) != 0 ) {
	    count = fread(buffer, 1, count, epsfile);
	    fwrite(buffer, 1, count, outfile);
	    len -= count;
	}
	free(buffer);
	fclose(epsfile);
	if (*outname!='\0')
	    fclose(outfile);
}


/* These routines deal with a PBM bitmap under Unix */
/* and a BMP under OS/2 or MS-DOS */

/* a structure for holding details of a bitmap used for constructing */
/* an EPS preview */
typedef struct tagPREBMAP {
    int  width;
    int  height;
    int  depth;
    int  bytewidth;	/* length of each scan line in bytes */
    BYTE GVHUGE* bits;
    BOOL topleft;
} PREBMAP;

void scan_bbox(PREBMAP *pprebmap, PSBBOX *psbbox);
void shift_preview(unsigned char *preview, int bwidth, int offset);


char isblack[256];	/* each byte is non-zero if that colour is black */

BOOL
iswhitespace(char c)
{
	return (c==' ' || c=='\t' || c=='\r' || c=='\n');
}

/* this doesn't do a good job of scanning pbm format */
/* instead it makes assumptions about the way Ghostscript writes pbm files */
void
scan_pbmplus(PREBMAP *ppbmap, LPBITMAP2 pbm)
{
char *pbitmap;
char *p;
int i;
    pbitmap = (char *)pbm;
    if (pbitmap[0] == 'P' && (pbitmap[1] == '4' || pbitmap[1] == '5')) {
	/* pbmraw */
	p = pbitmap+3;
	while (*p!='\n')
	    p++;	/* skip comment line */
	p++;
	ppbmap->width = atoi(p);
	while (isdigit(*p))
	    p++;
	while (iswhitespace(*p))
	    p++;
	ppbmap->height = atoi(p);
	while (isdigit(*p))
	    p++;
	if (pbitmap[1] == '4') {	/* pbmraw */
	    ppbmap->depth = 1;
	    isblack[0] = 0;
	    isblack[1] = 1;
	}
	else {				/* pgmraw */
	    while (iswhitespace(*p))
	        p++;
	    ppbmap->depth = atoi(p);
	    while (isdigit(*p))
	        p++;
	    for (i=0; i<ppbmap->depth; i++)
		isblack[i] = (char)(i!=0);
	}
	ppbmap->bits = ((BYTE GVHUGE *)p) +1;
        ppbmap->bytewidth = (( ppbmap->width * ppbmap->depth + 7) & ~7) >> 3;
	ppbmap->topleft = TRUE;
    }
    else {
	gserror(0, "Unknown bitmap format", MB_ICONEXCLAMATION, SOUND_ERROR);
    }
}

#ifdef UNIX
void
scan_dib(PREBMAP *ppbmap, LPBITMAP2 pbm)
{
    fprintf(stderr, "Can't handle BMP format under Unix");
}

#else
void scan_colors(PREBMAP *ppbmap, LPBITMAP2 pbm);

void
scan_dib(PREBMAP *ppbmap, LPBITMAP2 pbm)
{
    if (pbm->biSize == sizeof(BITMAP1)) {
	ppbmap->width = ((LPBITMAP1)pbm)->bcWidth;
	ppbmap->height = ((LPBITMAP1)pbm)->bcHeight;
	ppbmap->depth = ((LPBITMAP1)pbm)->bcBitCount;
        ppbmap->bytewidth = (( ppbmap->width * ppbmap->depth + 31) & ~31) >> 3;
	ppbmap->bits =  (((BYTE GVHUGE *)pbm) + pbm->biSize)
			+ dib_pal_colors(pbm) * sizeof(RGB3); 
	ppbmap->topleft = FALSE;
    }
    else {
	ppbmap->width = (int)pbm->biWidth;
	ppbmap->height = (int)pbm->biHeight;
	ppbmap->depth = pbm->biBitCount;
        ppbmap->bytewidth = (int)(((pbm->biWidth * pbm->biBitCount + 31) & ~31) >> 3);
	ppbmap->bits =  (((BYTE GVHUGE *)pbm) + pbm->biSize)
			+ dib_pal_colors(pbm) * sizeof(RGB4); 
	ppbmap->topleft = FALSE;
    }
    scan_colors(ppbmap, pbm);
}

/* return number of bytes per line, rounded up to multiple of 4 bytes */
unsigned long
dib_bytewidth(LPBITMAP2 pbm)
{
unsigned long l;
    if (pbm->biSize == sizeof(BITMAP1)) {
        l = ((( ((LPBITMAP1)pbm)->bcWidth * ((LPBITMAP1)pbm)->bcBitCount + 31) & ~31) >> 3);
    }
    else
        l = (((pbm->biWidth * pbm->biBitCount + 31) & ~31) >> 3);
    return l;
}

/* return number of colors in color table */
unsigned int
dib_pal_colors(LPBITMAP2 pbm)
{
    if (pbm->biSize == sizeof(BITMAP1)) {
	LPBITMAP1 pbm1 = (LPBITMAP1)pbm;
	if (pbm1->bcBitCount != 24)
	    return 1<<(pbm1->bcBitCount * pbm1->bcPlanes);
    }
    else {
	if (pbm->biBitCount != 24)
	    return (pbm->biClrUsed) ? (unsigned int)(pbm->biClrUsed) :
		1<<(pbm->biBitCount * pbm->biPlanes);
    }
    return 0;
}


void
scan_colors(PREBMAP *ppbmap, LPBITMAP2 pbm)
{
	LPRGB4 prgbquad;
	LPRGB3 prgbtriple;
	unsigned char rr;
	unsigned char gg;
	unsigned char bb;
	int type_one = FALSE;
	int clrtablesize;
	int i;

	prgbquad   = (LPRGB4)(((BYTE GVHUGE *)pbm) + pbm->biSize);
	prgbtriple = (LPRGB3)prgbquad;
	if (pbm->biSize == sizeof(BITMAP1))
	    type_one = TRUE;
	/* read in the color table */
	clrtablesize = dib_pal_colors(pbm);
	for (i = 0; i < clrtablesize; i++) {
		if (type_one) {
		    bb = prgbtriple[i].rgbtBlue;
		    gg = prgbtriple[i].rgbtGreen;
		    rr = prgbtriple[i].rgbtRed;
		}
		else {
		    bb = prgbquad[i].rgbBlue;
		    gg = prgbquad[i].rgbGreen;
		    rr = prgbquad[i].rgbRed;
		}
		isblack[i] = (unsigned char)((rr < 0xff) || (gg < 0xff) || (bb < 0xff));
	}
}

/* return pointer to bitmap bits */
BYTE GVHUGE *
get_dib_bits(LPBITMAP2 pbm)
{
BYTE GVHUGE *lpDibBits;
	lpDibBits = (((BYTE GVHUGE *)pbm) + pbm->biSize);
	if (pbm->biSize == sizeof(BITMAP1))
	    lpDibBits += dib_pal_colors(pbm) * sizeof(RGB3); 
	else
	    lpDibBits += dib_pal_colors(pbm) * sizeof(RGB4); 
	return lpDibBits;
}
#endif /* !UNIX */

/* get line from DIB and store as 1 bit/pixel in preview */
/* also works for PBM bitmap */
/* preview has 0=black, 1=white */
void
get_dib_line(BYTE GVHUGE *line, unsigned char *preview, int width, int bitcount)
{
int bwidth = ((width + 7) & ~7) >> 3; /* byte width with 1 bit/pixel */
unsigned char omask;
int oroll;
unsigned char c = 0;
int j;
	memset(preview,0xff,bwidth);
	omask = 0x80;
	oroll = 7;
	if (bitcount == 1) {
	    if (isblack[0])
		for (j = 0; j < bwidth ; j++)
			preview[j] = line[j];
	    else
		for (j = 0; j < bwidth ; j++)
			preview[j] = (unsigned char)~line[j];
	    preview[bwidth-1] |= (unsigned char)(width & 7 ? (1<<(8-(width&7)))-1 : 0);	/* mask for edge of bitmap */
	}
	else {
	    for (j = 0; j < width; j++) {
		switch (bitcount) {
			case 4:
				c = line[j>>1];
				if (!(j&1))
					c >>= 4;
				c = isblack[ c & 0x0f ];
				break;
			case 8:
				c = isblack[ (int)(line[j]) ];
				break;
			case 24:
				c = (unsigned char)(
				    (line[j*3] < 0xff) || 
				    (line[j*3+1] < 0xff) || 
				    (line[j*3+2] < 0xff) );
				break;
		}
		if (c) 
		    preview[j/8] &= (unsigned char)(~omask);
		else
		    preview[j/8] |= omask;
		oroll--;
		omask >>= 1;
		if (oroll < 0) {
		    omask = 0x80;
		    oroll = 7;
		}
	    }
	}
}

#define TIFF_BYTE 1
#define TIFF_ASCII 2
#define TIFF_SHORT 3
#define TIFF_LONG 4
#define TIFF_RATIONAL 5

struct rational_s {
	DWORD numerator;
	DWORD denominator;
};

struct ifd_entry_s {
	WORD tag;
	WORD type;
	DWORD length;
	DWORD value;
};

struct tiff_head_s {
	WORD order;
	WORD version;
	DWORD ifd_offset;
};

/* write tiff file from DIB bitmap */
void
write_tiff(FILE *f, LPBITMAP2 pbm, BOOL tiff4)
{
char *now;
DWORD *strip;
#define IFD_MAX_ENTRY 12
struct tiff_head_s tiff_head;
WORD ifd_length;
DWORD ifd_next;
struct ifd_entry_s ifd_entry[IFD_MAX_ENTRY];
struct ifd_entry_s *pifd_entry;
struct rational_s rational;
DWORD tiff_end, end;
time_t t;
int i;
unsigned char *preview;
BYTE GVHUGE *line;
int bwidth;
BOOL soft_extra = FALSE;
BOOL date_extra = FALSE;
PREBMAP prebmap;
	
	if (*(char *)pbm == 'P')
	    scan_pbmplus(&prebmap, pbm);
	else
	    scan_dib(&prebmap, pbm);

	/* byte width with 1 bit/pixel, rounded up even word */
	bwidth = ((prebmap.width + 15) & ~15) >> 3;

	tiff_end = sizeof(tiff_head);
	if (reorder_word(1) == 1)
	    tiff_head.order = 0x4949;	/* Intel = little endian */
	else
	    tiff_head.order = 0x4d4d;	/* Motorola = big endian */
	tiff_head.version = 42;
	tiff_head.ifd_offset = tiff_end;

	tiff_end += sizeof(ifd_length);

	if (tiff4)
	    ifd_length = 10;
	else
	    ifd_length = 12;

	tiff_end += ifd_length * sizeof(struct ifd_entry_s) + sizeof(ifd_next);
	ifd_next = 0;
	pifd_entry = &ifd_entry[0];
	if (tiff4) {
	    pifd_entry->tag = 0xff;	/* SubfileType */
	    pifd_entry->type = TIFF_SHORT;
	}
	else {
	    pifd_entry->tag = 0xfe;	/* NewSubfileType */
	    pifd_entry->type = TIFF_LONG;
	}
	pifd_entry->length = 1;
	pifd_entry->value = 0;

	pifd_entry = &ifd_entry[1];
	pifd_entry->tag = 0x100;	/* ImageWidth */
	if (tiff4)
	    pifd_entry->type = TIFF_SHORT;
	else
	    pifd_entry->type = TIFF_LONG;
	pifd_entry->length = 1;
	pifd_entry->value = prebmap.width;

	pifd_entry = &ifd_entry[2];
	pifd_entry->tag = 0x101;	/* ImageLength */
	if (tiff4)
	    pifd_entry->type = TIFF_SHORT;
	else
	    pifd_entry->type = TIFF_LONG;
	pifd_entry->length = 1;
	pifd_entry->value = prebmap.height;

	pifd_entry = &ifd_entry[3];
	pifd_entry->tag = 0x103;	/* Compression */
	pifd_entry->type = TIFF_SHORT;
	pifd_entry->length = 1;
	pifd_entry->value = 1;		/* no compression */

	pifd_entry = &ifd_entry[4];
	pifd_entry->tag = 0x106;	/* PhotometricInterpretation */
	pifd_entry->type = TIFF_SHORT;
	pifd_entry->length = 1;
	pifd_entry->value = 1;		/* black is zero */

	pifd_entry = &ifd_entry[5];
	pifd_entry->tag = 0x111;	/* StripOffsets */
	pifd_entry->type = TIFF_LONG;
	pifd_entry->length = prebmap.height;
	pifd_entry->value = tiff_end;
	tiff_end += (pifd_entry->length * sizeof(DWORD));

	pifd_entry = &ifd_entry[6];
	pifd_entry->tag = 0x116;	/* RowsPerStrip */
	pifd_entry->type = TIFF_LONG;
	pifd_entry->length = 1;
	pifd_entry->value = 1;

	pifd_entry = &ifd_entry[7];
	pifd_entry->tag = 0x117;	/* StripByteCounts */
	pifd_entry->type = TIFF_LONG;
	pifd_entry->length = prebmap.height;
	pifd_entry->value = tiff_end;
	tiff_end += (pifd_entry->length * sizeof(DWORD));

	pifd_entry = &ifd_entry[8];
	pifd_entry->tag = 0x11a;	/* XResolution */
	pifd_entry->type = TIFF_RATIONAL;
	pifd_entry->length = 1;
	pifd_entry->value = tiff_end;
	tiff_end += sizeof(struct rational_s);

	pifd_entry = &ifd_entry[9];
	pifd_entry->tag = 0x11b;	/* YResolution */
	pifd_entry->type = TIFF_RATIONAL;
	pifd_entry->length = 1;
	pifd_entry->value = tiff_end;
	tiff_end += sizeof(struct rational_s);

	if (!tiff4) {
	    pifd_entry = &ifd_entry[10];
	    pifd_entry->tag = 0x131;	/* Software */
	    pifd_entry->type = TIFF_ASCII;
	    pifd_entry->length = strlen(szAppName) + 1;
	    pifd_entry->value = tiff_end;
	    tiff_end += pifd_entry->length;
	    if (tiff_end & 1) { /* pad to word boundary */
	        soft_extra = TRUE;
	        tiff_end++;
	    }

	    pifd_entry = &ifd_entry[11];
	    pifd_entry->tag = 0x132;	/* DateTime */
	    pifd_entry->type = TIFF_ASCII;
	    t = time(NULL);
	    now = ctime(&t);
	    now[strlen(now)-1] = '\0';	/* remove trailing \n */
	    pifd_entry->length = strlen(now)+1;
	    pifd_entry->value = tiff_end;
	    tiff_end += pifd_entry->length;
	    if (tiff_end & 1) { /* pad to word boundary */
	        date_extra = TRUE;
	        tiff_end++;
	    }
	}

	fwrite(&tiff_head, sizeof(tiff_head), 1, f);
	fwrite(&ifd_length, sizeof(ifd_length), 1, f);
	fwrite(ifd_entry, ifd_length * sizeof(struct ifd_entry_s), 1, f);
	fwrite(&ifd_next, sizeof(ifd_next), 1, f);
	strip = (DWORD *)malloc(prebmap.height * sizeof(DWORD));
	end = tiff_end;
	for (i=0; i<prebmap.height; i++) {
		strip[i] = end;		/* strip offsets */
		end += bwidth;
	}
	fwrite(strip, 1, prebmap.height * sizeof(DWORD), f);
	for (i=0; i<prebmap.height; i++)
		strip[i] = bwidth;	/* strip byte counts */
	fwrite(strip, 1, prebmap.height * sizeof(DWORD), f);
	free((void *)strip);
	rational.numerator = (int)option.xdpi;
	rational.denominator = 1;
	fwrite(&rational, sizeof(rational), 1, f);
	rational.numerator = (int)option.ydpi;
	rational.denominator = 1;
	fwrite(&rational, sizeof(rational), 1, f);
	if (!tiff4) {
	    fwrite(szAppName, 1, strlen(szAppName)+1, f);
	    if (soft_extra)
	        fputc('\0',f);
	    fwrite(now, 1, strlen(now)+1, f);
	    if (date_extra)
	        fputc('\0',f);
	}

	preview = (unsigned char *) malloc(bwidth);
	memset(preview,0xff,bwidth);

	if (prebmap.topleft)
	    line = (BYTE GVHUGE *)prebmap.bits;
	else
	    line = (BYTE GVHUGE *)prebmap.bits + ((long)prebmap.bytewidth * (prebmap.height-1));
        /* process each line of bitmap */
	for (i = 0; i < prebmap.height; i++) {
	    get_dib_line(line, preview, prebmap.width, prebmap.depth);
	    fwrite(preview, 1, bwidth, f);
	    if (prebmap.topleft)
		line += prebmap.bytewidth;
	    else
		line -= prebmap.bytewidth;
	}
	free(preview);
}


/* make a PC EPS file with a TIFF Preview */
/* from a PS file and a bitmap */
void
make_eps_tiff(int type)
{
char epsname[MAXSTR];
LPBITMAP2 pbm;
char *buffer;
unsigned int count;
FILE *epsfile;
FILE *tiff_file;
char tiffname[MAXSTR];
#ifdef __EMX__
#pragma pack(1)
#endif
struct eps_header_s eps_header;
#ifdef __EMX__
#pragma pack()
#endif

	
	if ( (pbm = get_bitmap()) == (LPBITMAP2)NULL) {
	    play_sound(SOUND_ERROR);
	    return;
	}

	if ( (tiff_file = gp_open_scratch_file(szScratch, tiffname, "wb")) == (FILE *)NULL) {
	    play_sound(SOUND_ERROR);
	    release_bitmap();
	    return;
	}
	write_tiff(tiff_file, pbm, (type == IDM_MAKEEPST4));
	fclose(tiff_file);
	release_bitmap();

#ifdef EPSTOOL
	strcpy(epsname, oname);
	if (*epsname!='\0')
	    epsfile = fopen(epsname,"wb");
	else
	    epsfile = stdout;
#else
	/* create EPS file */
	epsname[0] = '\0';
	if (!get_filename(epsname, TRUE, FILTER_EPS, 0, IDS_TOPICEDIT)) {
	    unlink(tiffname);
	    return;
	}
	epsfile = fopen(epsname,"wb");
#endif
	if (epsfile == (FILE *)NULL) {
	    play_sound(SOUND_ERROR);
	    release_bitmap();
	    return;
	}

	/* write DOS EPS binary header */
	eps_header.id[0] = 0xc5;
	eps_header.id[1] = 0xd0;
	eps_header.id[2] = 0xd3;
	eps_header.id[3] = 0xc6;
	eps_header.ps_begin = sizeof(eps_header);
	fseek(psfile.file, 0, SEEK_END);
	eps_header.ps_length = ftell(psfile.file);
	eps_header.mf_begin = 0;
	eps_header.mf_length = 0;
	eps_header.tiff_begin = eps_header.ps_begin + eps_header.ps_length;
	tiff_file = fopen(tiffname,"rb");
	fseek(tiff_file, 0, SEEK_END);
	eps_header.tiff_length = ftell(tiff_file);
	eps_header.checksum = -1;
	/* reverse byte order if big endian machine */
	eps_header.ps_begin = reorder_dword(eps_header.ps_begin);
	eps_header.ps_length = reorder_dword(eps_header.ps_length);
	eps_header.tiff_begin = reorder_dword(eps_header.tiff_begin);
	eps_header.tiff_length = reorder_dword(eps_header.tiff_length);
	fwrite(&eps_header, sizeof(eps_header), 1, epsfile);
	rewind(psfile.file);
	dsc_copy(psfile.file, epsfile, doc->begincomments, doc->endtrailer, NULL);
	
	/* copy tiff file */
	rewind(tiff_file);
	buffer = malloc(COPY_BUF_SIZE);
	if (buffer == (char *)NULL) {
	    play_sound(SOUND_ERROR);
	    fclose(epsfile);
	    unlink(epsname);
	    fclose(tiff_file);
	    unlink(tiffname);
	    return;
	}
        while ( (count = fread(buffer, 1, COPY_BUF_SIZE, tiff_file)) != 0 )
	    fwrite(buffer, 1, count, epsfile);
	free(buffer);
	fclose(tiff_file);
	unlink(tiffname);
	if (*epsname!='\0')
	   fclose(epsfile);
}

static char hex[16] = "0123456789ABCDEF";

/* write interchange preview to file f */
void
write_interchange(FILE *f, LPBITMAP2 pbm, BOOL calc_bbox)
{
	int i, j;
	unsigned char *preview;
	BYTE GVHUGE *line;
	int preview_width, bwidth;
	int lines_per_scan;
	PREBMAP prebmap;
	PSBBOX devbbox;	/* in pixel units */
	
	if (*(char *)pbm == 'P')
	    scan_pbmplus(&prebmap, pbm);
	else
	    scan_dib(&prebmap, pbm);
	
	if (calc_bbox) {
	    scan_bbox(&prebmap, &devbbox);
	    if (devbbox.valid) {
	    	/* copy to global bbox as if obtained by PS to EPS */
	    	bbox.llx = devbbox.llx * 72.0 / option.xdpi;
	    	bbox.lly = devbbox.lly * 72.0 / option.ydpi;
	    	bbox.urx = devbbox.urx * 72.0 / option.xdpi;
	    	bbox.ury = devbbox.ury * 72.0 / option.ydpi;
	    	bbox.valid = TRUE;
	    }
	    copy_bbox_header(f); /* adjust %%BoundingBox: comment */
	}
	else {
	    devbbox.urx = prebmap.width;
	    devbbox.ury = prebmap.height;
	    devbbox.llx = devbbox.lly = 0;
	    dsc_copy(psfile.file, f, doc->begincomments, doc->endcomments, NULL);
	}

	bwidth = (((devbbox.urx-devbbox.llx) + 7) & ~7) >> 3; /* byte width with 1 bit/pixel */
	preview_width = ((prebmap.width + 7) & ~7) >> 3; /* byte width with 1 bit/pixel */

	preview = (unsigned char *) malloc(preview_width);

	lines_per_scan = ((bwidth-1) / 32) + 1;
	fprintf(f,"%%%%BeginPreview: %u %u 1 %u",(devbbox.urx-devbbox.llx), (devbbox.ury-devbbox.lly), 
	    (devbbox.ury-devbbox.lly)*lines_per_scan);
	fputs(EOLSTR, f);

	if (prebmap.topleft)
	    line = (BYTE GVHUGE *)prebmap.bits + ((long)prebmap.bytewidth * (prebmap.height - devbbox.ury));
	else
	    line = (BYTE GVHUGE *)prebmap.bits + ((long)prebmap.bytewidth * (devbbox.ury-1));
	/* process each line of bitmap */
	for (i = 0; i < (devbbox.ury-devbbox.lly); i++) {
		get_dib_line(line, preview, prebmap.width, prebmap.depth);
		if (devbbox.llx)
		    shift_preview(preview, preview_width, devbbox.llx);
		fputs("% ",f);
		for (j=0; j<bwidth; j++) {
		    if (j && ((j & 31) == 0)) {
			fputs(EOLSTR, f);
		        fputs("% ",f);
		    }
		    fputc(hex[15-((preview[j]>>4)&15)],f);
		    fputc(hex[15-((preview[j])&15)],f);
		}
		fputs(EOLSTR, f);
		if (prebmap.topleft)
		    line += prebmap.bytewidth;
		else 
		    line -= prebmap.bytewidth;
	}

	fputs("%%EndPreview",f);
	fputs(EOLSTR, f);
	free(preview);
	dsc_copy(psfile.file, f, doc->endpreview, doc->endtrailer, NULL);
}

/* make an EPSI file with an Interchange Preview */
/* from a PS file and a bitmap */
void
make_eps_interchange(BOOL calc_bbox)
{
char epiname[MAXSTR];
FILE *epifile;
LPBITMAP2 pbm;

	if ( (pbm = get_bitmap()) == (LPBITMAP2)NULL) {
	    play_sound(SOUND_ERROR);
	    return;
	}

#ifdef EPSTOOL
	strcpy(epiname, oname);
	if (*epiname!='\0')
	    epifile = fopen(epiname,"wb");
	else
	    epifile = stdout;
#else
	/* create EPI file */
	epiname[0] = '\0';
	if (!get_filename(epiname, TRUE, FILTER_EPI, 0, IDS_TOPICEDIT)) {
	    play_sound(SOUND_ERROR);
	    release_bitmap();
	    return;
	}
	epifile = fopen(epiname,"wb");
#endif

	if (epifile == (FILE *)NULL) {
	    play_sound(SOUND_ERROR);
	    release_bitmap();
	    return;
	}

	rewind(psfile.file);
	write_interchange(epifile, pbm, calc_bbox);
	if (*epiname!='\0')
	    fclose(epifile);
	release_bitmap();
}


/* scan bitmap and return bbox measured in pixels */
void
scan_bbox(PREBMAP *pprebmap, PSBBOX *devbbox)
{
	unsigned char *preview;
	BYTE GVHUGE *line;
	int bwidth = ((pprebmap->width + 7) & ~7) >> 3; /* byte width with 1 bit/pixel */
	int i, j, k, l;
	int x;
	BYTE ch;
	BYTE GVHUGE *chline;
	unsigned char omask;

	devbbox->llx = pprebmap->width;
	devbbox->lly = pprebmap->height;
	devbbox->urx = devbbox->ury = 0;
	devbbox->valid = FALSE;

	preview = (unsigned char *) malloc(bwidth);
	memset(preview,0xff,bwidth);

	if (pprebmap->topleft)
	    line = (BYTE GVHUGE *)pprebmap->bits + ((long)pprebmap->bytewidth * (pprebmap->height-1));
	else
	    line = (BYTE GVHUGE *)pprebmap->bits;
        /* process each line of bitmap */
	for (i = 0; i < pprebmap->height; i++) {
	    /* get 1bit/pixel line, 0=black, 1=white */
	    get_dib_line(line, preview, pprebmap->width, pprebmap->depth);
	    chline = preview;
	    ch = 0;
	    for (j=0; j<bwidth; j++)
	        ch |= ~(*chline++);	/* check for black pixels */
	    if (ch) {
		/* adjust y coordinates of bounding box */
		if (i < devbbox->lly)
		    devbbox->lly = i;
		if (i+1 > devbbox->ury)
		    devbbox->ury = i+1;
		/* scan for x coordinates of black pixels */
	        chline = preview;
		for (k=0; k<bwidth; k++) {
		    if (~(*chline)) { /* a pixel is black */
			omask = 0x80;
			for (l=0; l<8; l++) {
			    if ( ~*chline & omask ) {
			    	x = k*8 + l;
			    	if (x < devbbox->llx)
			    	    devbbox->llx = x;
			    	if (x+1 > devbbox->urx)
			    	    devbbox->urx = x+1;
			    }
		            omask >>= 1;
			}
		    }
		    chline++;
		}
	    }
	    if (pprebmap->topleft)
		line -= pprebmap->bytewidth;
	    else
		line += pprebmap->bytewidth;
	}
	free(preview);
	if ( (devbbox->lly < devbbox->ury) && (devbbox->llx < devbbox->urx) )
	    devbbox->valid = TRUE;
	
#ifdef EPSTOOL
	{   char buf[256];
	    sprintf(buf, "bbox=%d %d %d %d\n", devbbox->llx, devbbox->lly, devbbox->urx, devbbox->ury);
	    pserror(buf);
	}
#endif
}

/* shift preview by offset bits to the left */
/* width is in bytes */
/* fill exposed bits with 1's */
void
shift_preview(unsigned char *preview, int bwidth, int offset)
{
int bitoffset;
int byteoffset;
int newwidth;
int shifter;
int i;
	if (offset == 0)
	    return;
	byteoffset = offset / 8;
	newwidth = bwidth - byteoffset;
	/* first remove byte offset */
	memmove(preview, preview+byteoffset, newwidth);
	memset(preview+newwidth, 0xff, bwidth-newwidth);
	/* next remove bit offset */
	bitoffset = offset - byteoffset*8;
	if (bitoffset==0)
	    return;
	bitoffset = 8 - bitoffset;
	for (i=0; i<newwidth; i++) {
	   shifter = preview[i] << 8;
	   if (i==newwidth-1)
	       shifter += 0xff;	/* can't access preview[bwidth] */
	   else
	       shifter += preview[i+1];  
	   preview[i] = shifter>>bitoffset;
	}
}

/* Copy the header to file f */
/* change bbox line if present, or add bbox line */
void
copy_bbox_header(FILE *f)
{
    char text[PSLINELENGTH];
    char *comment;
    BOOL bbox_written = FALSE;
    long position;

    fseek(psfile.file, doc->begincomments, SEEK_SET);
    if (doc->bbox.valid) {
      position = ftell(psfile.file);
      while ( (comment = dsc_copy(psfile.file, f, position,
			   doc->endcomments, "%%BoundingBox:")) != (char *)NULL ) {
	position = ftell(psfile.file);
	if (bbox_written) {
	    free(comment);
	    continue;
	}
	fprintf(f, "%%%%BoundingBox: %d %d %d %d\r\n",
	    bbox.llx, bbox.lly, bbox.urx, bbox.ury);
	bbox_written = TRUE;
	free(comment);
      }
    }
    else {
      fgets(text, PSLINELENGTH, psfile.file);
      fputs(text,f);
      fprintf(f, "%%%%BoundingBox: %d %d %d %d\r\n",
	    bbox.llx, bbox.lly, bbox.urx, bbox.ury);
      position = ftell(psfile.file);
      comment = dsc_copy(psfile.file, f, position, doc->endcomments, NULL);
    }
}

