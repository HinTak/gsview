/* Copyright (C) 1993, 1994, 1995, Russell Lang.  All rights reserved.
  
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
#endif

PSBBOX bbox;
void tiff_long(DWORD val, int little_endian, FILE *f);
void tiff_short(WORD val, int little_endian, FILE *f);
void tiff_word(WORD val, int little_endian, FILE *f);

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
	    info_wait(IDS_WAITWRITE);
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
	    info_wait(IDS_NOWAIT);
	}
	else {
	    /* document already has DSC comments */
	    info_wait(IDS_WAITWRITE);
	    fseek(psfile.file, doc->beginheader, SEEK_SET);
	    fgets(text, PSLINELENGTH, psfile.file);
	    if (doc->epsf)
	        fputs(text,f);
	    else
	        fputs("%!PS-Adobe-3.0 EPSF-3.0\r\n",f);
	    if (!( (doc->boundingbox[LLX]==0) &&  (doc->boundingbox[LLY]==0) 
             &&  (doc->boundingbox[URX]==0) &&  (doc->boundingbox[URY]==0) )) {
	        if ( (comment = pscopyuntil(psfile.file, f, -1,
			   doc->endheader, "%%BoundingBox:")) != (char *)NULL ) {
		    free(comment);
	        }
	    }
	    fprintf(f, "%%%%BoundingBox: %d %d %d %d\r\n",
		bbox.llx, bbox.lly, bbox.urx, bbox.ury);
	    here = ftell(psfile.file);
	    pscopyuntil(psfile.file, f, here, doc->endtrailer, NULL);
	    fclose(f);
	    info_wait(IDS_NOWAIT);
	}
}
#endif

typedef struct tagWINRECT {
	WORD	left PACKED;
	WORD	top PACKED;
	WORD	right PACKED;
	WORD	bottom PACKED;
} WINRECT;

typedef struct {
    DWORD	key PACKED;
    WORD 	hmf PACKED;
    WINRECT 	bbox PACKED;
    WORD	inch PACKED;
    DWORD	reserved PACKED;
    WORD	checksum PACKED;
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
DWORD key;
int filter;
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
	    /* check if metafile already contains header */
	    fread(&key, 4, 1, epsfile);
	    fseek(epsfile, pos, SEEK_SET);	/* seek to section to extract */
	    if ( key != reorder_dword(0x9ac6cdd7L) ) {
	        /* write placeable Windows Metafile header */
	        METAFILEHEADER mfh;
	        int i, temp;
	        unsigned short *pw;
		int bo = ( reorder_dword(1) == 1 );	/* true for little endian */
	        mfh.key = 0x9ac6cdd7L;
	        mfh.hmf = 0;
		/* guess the location - this might be wrong */
	        mfh.bbox.left = 0;
	        mfh.bbox.top = 0;
		temp = (doc->boundingbox[URX] - doc->boundingbox[LLX]);
		mfh.bbox.right = (WORD)temp;	/* double transfer to avoid GCC Solaris bug */
	        mfh.bbox.bottom = (WORD)(doc->boundingbox[URY] - doc->boundingbox[LLY]);
	        temp = (doc->boundingbox[URY] - doc->boundingbox[LLY]);
		mfh.bbox.bottom = (WORD)temp;
	        mfh.inch = 72;	/* PostScript points */
	        mfh.reserved = 0L;
	        mfh.checksum =  0;
	        pw = (WORD *)&mfh;
		temp = 0;
	        for (i=0; i<10; i++) {
	    	    temp ^= *pw++;
	        }
		mfh.checksum = (WORD)temp;
	        /* fwrite(&mfh, sizeof(mfh), 1, outfile); */
	        tiff_long(mfh.key, bo, outfile);
	        tiff_word(mfh.hmf, bo, outfile);
	        tiff_word(mfh.bbox.left,   bo, outfile);
	        tiff_word(mfh.bbox.top,    bo, outfile);
	        tiff_word(mfh.bbox.right,  bo, outfile);
	        tiff_word(mfh.bbox.bottom, bo, outfile);
	        tiff_word(mfh.inch, bo, outfile);
	        tiff_long(mfh.reserved, bo, outfile);
	        tiff_word(mfh.checksum, bo, outfile);
	    }
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
	DWORD numerator PACKED;
	DWORD denominator PACKED;
};
#define TIFF_RATIONAL_SIZE 8

struct ifd_entry_s {
	WORD tag PACKED;
	WORD type PACKED;
	DWORD length PACKED;
	DWORD value PACKED;
};
#define TIFF_IFD_SIZE 12

struct tiff_head_s {
	WORD order PACKED;
	WORD version PACKED;
	DWORD ifd_offset PACKED;
};
#define TIFF_HEAD_SIZE 8

/* write DWORD as DWORD */
void
tiff_long(DWORD val, int little_endian, FILE *f)
{
unsigned char *p = (unsigned char *)&val;
    if (little_endian)
	fwrite(p, 1, 4, f);
    else {
	fwrite(p+3, 1, 1, f);
	fwrite(p+2, 1, 1, f);
	fwrite(p+1, 1, 1, f);
	fwrite(p, 1, 1, f);
    }
}

/* write WORD as DWORD */
void
tiff_short(WORD val, int little_endian, FILE *f)
{
unsigned char *p = (unsigned char *)&val;
    if (little_endian)
	fwrite(p, 1, 2, f);
    else {
	fwrite(p+1, 1, 1, f);
	fwrite(p, 1, 1, f);
    }
    fputc('\0', f);
    fputc('\0', f);
}

/* write WORD as WORD */
void
tiff_word(WORD val, int little_endian, FILE *f)
{
unsigned char *p = (unsigned char *)&val;
    if (little_endian)
	fwrite(p, 1, 2, f);
    else {
	fwrite(p+1, 1, 1, f);
	fwrite(p, 1, 1, f);
    }
}

/* Write tiff file from DIB bitmap */
/* Since this will be used by a DOS EPS file, we write an Intel TIFF file */
void
write_tiff(FILE *f, LPBITMAP2 pbm, BOOL tiff4, BOOL calc_bbox)
{
char *now;
#define IFD_MAX_ENTRY 12
WORD ifd_length;
DWORD ifd_next;
DWORD tiff_end, end;
time_t t;
int i;
unsigned char *preview;
BYTE GVHUGE *line;
int preview_width, bwidth;
BOOL soft_extra = FALSE;
BOOL date_extra = FALSE;
PREBMAP prebmap;
PSBBOX devbbox;	/* in pixel units */
int bo;	/* byte order */
int width, height;	/* size of preview */
	
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
	    else {
		devbbox.urx = prebmap.width;
		devbbox.ury = prebmap.height;
		devbbox.llx = devbbox.lly = 0;
	    }
	}
	else {
	    devbbox.urx = prebmap.width;
	    devbbox.ury = prebmap.height;
	    devbbox.llx = devbbox.lly = 0;
	}

	width  = devbbox.urx - devbbox.llx;	/* width  of dest bitmap */
	height = devbbox.ury - devbbox.lly;	/* height of dest bitmap */
	/* byte width with 1 bit/pixel, rounded up even word */
	bwidth = ((width + 15) & ~15) >> 3;

	/* byte width of source bitmap */
	preview_width = ((prebmap.width + 7) & ~7) >> 3; /* byte width with 1 bit/pixel */

	/* determine if we are big or little endian */
	/* save in global variable */
	bo = ( reorder_dword(1) == 1 );	/* true for little endian */

	/* write header */
	tiff_end = TIFF_HEAD_SIZE;
	tiff_word(0x4949, bo, f);	/* Intel = little endian */
	tiff_word(42, bo, f);
	tiff_long(tiff_end, bo, f);

	/* write count of ifd entries */
	tiff_end += sizeof(ifd_length);
	if (tiff4)
	    ifd_length = 10;
	else
	    ifd_length = 12;
	tiff_word(ifd_length, bo, f);

	tiff_end += ifd_length * sizeof(struct ifd_entry_s) + sizeof(ifd_next);
	ifd_next = 0;

	/* write each of the ifd entries */
	if (tiff4) {
	    tiff_word(0xff, bo, f);	    /* SubfileType */
	    tiff_word(TIFF_SHORT, bo, f);  /* value type */
	    tiff_long(1, bo, f);		    /* length */
	    tiff_short(0, bo, f);		    /* value */
	}
	else {
	    tiff_word(0xfe, bo, f);	/* NewSubfileType */
	    tiff_word(TIFF_LONG, bo, f);
	    tiff_long(1, bo, f);		    /* length */
	    tiff_long(0, bo, f);		    /* value */
	}

	tiff_word(0x100, bo, f);	/* ImageWidth */
	if (tiff4) {
	    tiff_word(TIFF_SHORT, bo, f);
	    tiff_long(1, bo, f);
	    tiff_short((WORD)width, bo, f);
	}
	else {
	    tiff_word(TIFF_LONG, bo, f);
	    tiff_long(1, bo, f);
	    tiff_long(width, bo, f);
	}

	tiff_word(0x101, bo, f);	/* ImageHeight */
	if (tiff4) {
	    tiff_word(TIFF_SHORT, bo, f);
	    tiff_long(1, bo, f);
	    tiff_short((WORD)height, bo, f);
	}
	else {
	    tiff_word(TIFF_LONG, bo, f);
	    tiff_long(1, bo, f);
	    tiff_long(height, bo, f);
	}

	tiff_word(0x103, bo, f);	/* Compression */
	tiff_word(TIFF_SHORT, bo, f);
	tiff_long(1, bo, f);
	tiff_short(1, bo, f);		/* no compression */

	tiff_word(0x106, bo, f);	/* PhotometricInterpretation */
	tiff_word(TIFF_SHORT, bo, f);
	tiff_long(1, bo, f);
	tiff_short(1, bo, f);		/* black is zero */

	tiff_word(0x111, bo, f);	/* StripOffsets */
	tiff_word(TIFF_LONG, bo, f);
	tiff_long(height, bo, f);
	tiff_long(tiff_end, bo, f);
	tiff_end += (height * sizeof(DWORD));

	tiff_word(0x116, bo, f);	/* RowsPerStrip */
	tiff_word(TIFF_LONG, bo, f);
	tiff_long(1, bo, f);
	tiff_long(1, bo, f);

	tiff_word(0x117, bo, f);	/* StripByteCounts */
	tiff_word(TIFF_LONG, bo, f);
	tiff_long(height, bo, f);
	tiff_long(tiff_end, bo, f);
	tiff_end += (height * sizeof(DWORD));

	tiff_word(0x11a, bo, f);	/* XResolution */
	tiff_word(TIFF_RATIONAL, bo, f);
	tiff_long(1, bo, f);
	tiff_long(tiff_end, bo, f);
	tiff_end += TIFF_RATIONAL_SIZE;

	tiff_word(0x11b, bo, f);	/* YResolution */
	tiff_word(TIFF_RATIONAL, bo, f);
	tiff_long(1, bo, f);
	tiff_long(tiff_end, bo, f);
	tiff_end += TIFF_RATIONAL_SIZE;

	if (!tiff4) {
	    tiff_word(0x131, bo, f);	/* Software */
	    tiff_word(TIFF_ASCII, bo, f);
	    i = strlen(szAppName) + 1;
	    tiff_long(i, bo, f);
	    tiff_long(tiff_end, bo, f);
	    tiff_end += i;
	    if (tiff_end & 1) { /* pad to word boundary */
	        soft_extra = TRUE;
	        tiff_end++;
	    }

	    tiff_word(0x132, bo, f);	/* DateTime */
	    tiff_word(TIFF_ASCII, bo, f);
	    t = time(NULL);
	    now = ctime(&t);
	    now[strlen(now)-1] = '\0';	/* remove trailing \n */
	    i = strlen(now) + 1;
	    tiff_long(i, bo, f);
	    tiff_long(tiff_end, bo, f);
	    tiff_end += i;
	    if (tiff_end & 1) { /* pad to word boundary */
	        date_extra = TRUE;
	        tiff_end++;
	    }
	}

	/* write end of ifd tag */
	tiff_long(ifd_next, bo, f);

	/* strip offsets */
	end = tiff_end;
	for (i=0; i<height; i++) {
		tiff_long(end, bo, f);
		end += bwidth;
	}

	/* strip byte counts */
	for (i=0; i<height; i++)
		tiff_long(bwidth, bo, f);

	/* XResolution rational */
	tiff_long((int)option.xdpi, bo, f);
	tiff_long(1, bo, f);
	/* YResolution rational */
	tiff_long((int)option.ydpi, bo, f);
	tiff_long(1, bo, f);

	/* software and time strings */
	if (!tiff4) {
	    fwrite(szAppName, 1, strlen(szAppName)+1, f);
	    if (soft_extra)
	        fputc('\0',f);
	    fwrite(now, 1, strlen(now)+1, f);
	    if (date_extra)
	        fputc('\0',f);
	}

	preview = (unsigned char *) malloc(preview_width);
	memset(preview,0xff,preview_width);

	if (prebmap.topleft)
	    line = (BYTE GVHUGE *)prebmap.bits + ((long)prebmap.bytewidth * (prebmap.height - devbbox.ury));
	else
	    line = (BYTE GVHUGE *)prebmap.bits + ((long)prebmap.bytewidth * (devbbox.ury-1));
        /* process each line of bitmap */
	for (i = 0; i < height; i++) {
	    get_dib_line(line, preview, prebmap.width, prebmap.depth);
	    if (devbbox.llx)
		shift_preview(preview, preview_width, devbbox.llx);
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
int
make_eps_tiff(int type, BOOL calc_bbox)
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
FILE *tpsfile;
char tpsname[MAXSTR];
	
	if ( (pbm = get_bitmap()) == (LPBITMAP2)NULL) {
	    play_sound(SOUND_ERROR);
	    return 1;
	}

	if ( (tiff_file = gp_open_scratch_file(szScratch, tiffname, "wb")) == (FILE *)NULL) {
	    play_sound(SOUND_ERROR);
	    release_bitmap();
	    return 1;
	}
	write_tiff(tiff_file, pbm, (type == IDM_MAKEEPST4), calc_bbox);
	fclose(tiff_file);
	release_bitmap();

	if (calc_bbox) {
	    /* we need to copy the psfile to a temporary file */
	    /* because we will be changing the %%BoundingBox line */
	    if ( (tpsfile = gp_open_scratch_file(szScratch, tpsname, "wb")) == (FILE *)NULL) {
		play_sound(SOUND_ERROR);
		return 1;
	    }
	    copy_bbox_header(tpsfile); /* adjust %%BoundingBox: comment */
	    pscopyuntil(psfile.file, tpsfile, doc->endheader, doc->endtrailer, NULL);
	    fclose(tpsfile);
	    if ( (tpsfile = fopen(tpsname, "rb")) == (FILE *)NULL) {
		play_sound(SOUND_ERROR);
		return 1;
	    }
	}
	else {
	    tpsfile = psfile.file;
	}

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
	    return 1;
	}
	epsfile = fopen(epsname,"wb");
#endif
	if (epsfile == (FILE *)NULL) {
	    play_sound(SOUND_ERROR);
	    release_bitmap();
	    return 1;
	}

	/* write DOS EPS binary header */
	eps_header.id[0] = 0xc5;
	eps_header.id[1] = 0xd0;
	eps_header.id[2] = 0xd3;
	eps_header.id[3] = 0xc6;
	eps_header.ps_begin = sizeof(eps_header);
	fseek(tpsfile, 0, SEEK_END);
	eps_header.ps_length = ftell(tpsfile);
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

	buffer = malloc(COPY_BUF_SIZE);
	if (buffer == (char *)NULL) {
	    play_sound(SOUND_ERROR);
	    fclose(epsfile);
	    unlink(epsname);
	    fclose(tiff_file);
	    unlink(tiffname);
	    if (calc_bbox) {
		fclose(tpsfile);
		unlink(tpsname);
	    }
	    return 1;
	}

	/* copy EPS file */
	rewind(tpsfile);
	if (calc_bbox) {
	    while ( (count = fread(buffer, 1, COPY_BUF_SIZE, tpsfile)) != 0 )
		fwrite(buffer, 1, count, epsfile);
	}
	else {
	    pscopyuntil(psfile.file, epsfile, doc->beginheader, doc->endtrailer, NULL);
	}
	
	/* copy tiff file */
	rewind(tiff_file);
        while ( (count = fread(buffer, 1, COPY_BUF_SIZE, tiff_file)) != 0 )
	    fwrite(buffer, 1, count, epsfile);

	free(buffer);
	fclose(tiff_file);
	unlink(tiffname);
	if (calc_bbox) {
	    fclose(tpsfile);
	    unlink(tpsname);
	}
	if (*epsname!='\0')
	   fclose(epsfile);
	return 0;
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
	    pscopyuntil(psfile.file, f, doc->beginheader, doc->endheader, NULL);
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
	pscopyuntil(psfile.file, f, 
	    doc->endpreview ? doc->endpreview : doc->endheader, 
	    doc->endtrailer, NULL);
}

/* make an EPSI file with an Interchange Preview */
/* from a PS file and a bitmap */
int
make_eps_interchange(BOOL calc_bbox)
{
char epiname[MAXSTR];
FILE *epifile;
LPBITMAP2 pbm;

	if ( (pbm = get_bitmap()) == (LPBITMAP2)NULL) {
	    play_sound(SOUND_ERROR);
	    return 1;
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
	    return 1;
	}
	epifile = fopen(epiname,"wb");
#endif

	if (epifile == (FILE *)NULL) {
	    play_sound(SOUND_ERROR);
	    release_bitmap();
	    return 1;
	}

	rewind(psfile.file);
	write_interchange(epifile, pbm, calc_bbox);
	if (*epiname!='\0')
	    fclose(epifile);
	release_bitmap();
	return 0;
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
	        ch |= (BYTE)(~(*chline++));	/* check for black pixels */
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
	   preview[i] = (unsigned char)(shifter>>bitoffset);
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

    fseek(psfile.file, doc->beginheader, SEEK_SET);
      if (!( (doc->boundingbox[LLX]==0) &&  (doc->boundingbox[LLY]==0) 
          && (doc->boundingbox[URX]==0) &&  (doc->boundingbox[URY]==0) )) {
      position = ftell(psfile.file);
      while ( (comment = pscopyuntil(psfile.file, f, position,
			   doc->endheader, "%%BoundingBox:")) != (char *)NULL ) {
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
      comment = pscopyuntil(psfile.file, f, position, doc->endheader, NULL);
    }
}

/* make a PC EPS file */
/* from a PS file and a user supplied preview */
/* preview may be WMF or TIFF */
/* returns 0 on success */
int
make_eps_user(void)
{
char epsname[MAXSTR];
char *buffer;
unsigned int count;
FILE *epsfile;
FILE *preview_file;
char preview_name[MAXSTR];
unsigned long preview_length;
#ifdef __EMX__
#pragma pack(1)
#endif
struct eps_header_s eps_header;
#ifdef __EMX__
#pragma pack()
#endif
int type = 0;
#define TIFF 1
#define WMF 2
char id[4];

	/* get user supplied preview */
#ifdef EPSTOOL
	strcpy(preview_name, upname);
#else
	if (!get_filename(preview_name, FALSE, FILTER_ALL, IDS_EPSUSERTITLE, IDS_TOPICEDIT))
	    return 1; /* failure */
#endif

	/* open preview, determine length and type */
	preview_file = fopen(preview_name, "rb");
	if (preview_file == (FILE *)NULL) {
	 
	    play_sound(SOUND_ERROR);
	    return 1;
	}

	id[0] = (char)fgetc(preview_file);
	id[1] = (char)fgetc(preview_file);
	id[2] = (char)fgetc(preview_file);
	id[3] = (char)fgetc(preview_file);
	fseek(preview_file, 0, SEEK_END);
	preview_length = ftell(preview_file);
	fseek(preview_file, 0, SEEK_SET);

	if ((id[0] == 'I') && (id[1] == 'I'))
	    type = TIFF;
	if ((id[0] == 'M') && (id[1] == 'M'))
	    type = TIFF;
	if ((id[0] == (char)0x01) && (id[1] == (char)0x00) && (id[2] == (char)0x09) && (id[3] == (char)0x00))
	    type = WMF;
	if ((id[0] == (char)0xd7) && (id[1] == (char)0xcd) && (id[2] == (char)0xc6) && (id[3] == (char)0x9a)) {
	    type = WMF;
	    preview_length -= 22;	/* skip over placeable metafile header */
	    fseek(preview_file, 22, SEEK_SET);
	}

	if (type == 0) {
	    gserror(IDS_EPSUSERINVALID, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    fclose(preview_file);
	    return 1;
	}

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
	    fclose(preview_file);
	    return 1;
	}
	epsfile = fopen(epsname,"wb");
#endif
	if (epsfile == (FILE *)NULL) {
	    play_sound(SOUND_ERROR);
	    fclose(preview_file);
	    return 1;
	}

	/* write DOS EPS binary header */
	eps_header.id[0] = 0xc5;
	eps_header.id[1] = 0xd0;
	eps_header.id[2] = 0xd3;
	eps_header.id[3] = 0xc6;
	eps_header.ps_begin = sizeof(eps_header);
	eps_header.ps_length = doc->endtrailer - doc->beginheader;
	if (type == WMF) {
	    eps_header.mf_begin = eps_header.ps_begin + eps_header.ps_length;
	    eps_header.mf_length = preview_length;
	    eps_header.tiff_begin = 0;
	    eps_header.tiff_length = 0;
	}
	else {
	    eps_header.mf_begin = 0;
	    eps_header.mf_length = 0;
	    eps_header.tiff_begin = eps_header.ps_begin + eps_header.ps_length;
	    eps_header.tiff_length = preview_length;
	}
	eps_header.checksum = -1;

	/* reverse byte order if big endian machine */
	eps_header.ps_begin = reorder_dword(eps_header.ps_begin);
	eps_header.ps_length = reorder_dword(eps_header.ps_length);
	eps_header.mf_begin = reorder_dword(eps_header.mf_begin);
	eps_header.mf_length = reorder_dword(eps_header.mf_length);
	eps_header.tiff_begin = reorder_dword(eps_header.tiff_begin);
	eps_header.tiff_length = reorder_dword(eps_header.tiff_length);
	fwrite(&eps_header, sizeof(eps_header), 1, epsfile);
	rewind(psfile.file);
	pscopyuntil(psfile.file, epsfile, doc->beginheader, doc->endtrailer, NULL);
	
	/* copy preview file */
	buffer = malloc(COPY_BUF_SIZE);
	if (buffer == (char *)NULL) {
	    play_sound(SOUND_ERROR);
	    fclose(epsfile);
	    unlink(epsname);
	    fclose(preview_file);
	    return 1;
	}
        while ( (count = fread(buffer, 1, COPY_BUF_SIZE, preview_file)) != 0 )
	    fwrite(buffer, 1, count, epsfile);
	free(buffer);
	fclose(preview_file);
	if (*epsname!='\0')
	   fclose(epsfile);
	return 0; /* success */
#undef TIFF
#undef WMF
}

