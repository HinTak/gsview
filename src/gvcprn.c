/* Copyright (C) 1993-1998, Russell Lang.  All rights reserved.
  
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

/* gvcprn.c */
/* Printer module of PM and Windows GSview */

#ifdef _Windows
#include "gvwin.h"
#else
#include "gvpm.h"
#endif

#ifndef _MSC_VER  /* Brain damaged MSVC++ 5.0 doesn't support POSIX dirent.h */
#include <dirent.h>
#endif
#include <sys/stat.h>

struct prop_item_s *
get_properties(char *device)
{
char *entries, *p, *q;
int i, numentry;
struct prop_item_s *proplist;
PROFILE *prf;
	entries = malloc(PROFILE_SIZE);
	if (entries == (char *)NULL)
	   return NULL;
	if ( (prf = profile_open(szIniFile)) == (PROFILE *)NULL) {
	    free(entries);
	    return NULL;
	}
	profile_read_string(prf, device, NULL, "", entries, PROFILE_SIZE);
/*
	if (strlen(entries) == 0) {
	    profile_close(prf);
	    free(entries);
	    return NULL;
	}
*/
	p = entries;
	for (numentry=0; p!=(char *)NULL && strlen(p)!=0; numentry++)
	    p += strlen(p) + 1;
	proplist = (struct prop_item_s *)malloc((numentry+1) * sizeof(struct prop_item_s));
	if (proplist == (struct prop_item_s *)NULL) {
	    profile_close(prf);
	    free(entries);
	    return NULL;
	}
	p = entries;
	for (i=0; i<numentry; i++) {
	    strcpy(proplist[i].name, p);
	    profile_read_string(prf, device, p, "", proplist[i].value, sizeof(proplist->value));
	    q = proplist[i].value;
	    while ((*q) && (q[strlen(q)-1]==' '))
		q[strlen(q)-1] = '\0';    /* remove trailing spaces */
	    p += strlen(p) + 1;
	}
	proplist[numentry].name[0] = '\0';
	proplist[numentry].value[0] = '\0';
	profile_close(prf);
	free(entries);
	return proplist;
}

char *
get_devices()
{
char *p;
PROFILE *prf;
	if ( (prf = profile_open(szIniFile)) == (PROFILE *)NULL)
	    return (char *)NULL;

	if ( (p = malloc(PROFILE_SIZE)) == (char *)NULL) {
	    profile_close(prf);
	    return (char *)NULL;
	}

	profile_read_string(prf, DEVSECTION, NULL, "", p, PROFILE_SIZE);
	if (strlen(p) == 0) {
	    /* [Devices] section doesn't exist.  Initialise from resources */
	    profile_create_section(prf, DEVSECTION, IDR_DEVICES);
	}
	profile_read_string(prf, DEVSECTION, NULL, "", p, PROFILE_SIZE);
	profile_close(prf);
	return p;
}


/* get a filename and spool it for printing */
void
gsview_spool(char *fname, char *port)
{
	static char filename[MAXSTR];

	if (fname == (char *)NULL) {
	    if (!get_filename(filename, FALSE, FILTER_ALL, IDS_PRINTFILE, IDS_TOPICPRINT))
		return;
	}
	else {
	    while (*fname && *fname==' ')
	        fname++;
	    strncpy(filename, fname, MAXSTR);
	}

	if (!gp_printfile(filename, port)) {
		play_sound(SOUND_ERROR);
		return;
	}
}


/* save entire file */
/* added to save files when GSview used as a WWW viewer */
void
gsview_saveas()
{
FILE *f;
char output[MAXSTR];
FILE *infile;
UINT count;
char *buffer;
int filter;

    output[0] = '\0';
    if (psfile.name[0] == '\0') {
	    gserror(IDS_NOTOPEN, NULL, MB_ICONEXCLAMATION, SOUND_NOTOPEN);
	    return;
    }

    filter = psfile.ispdf ? FILTER_PDF :
	( psfile.doc && psfile.doc->epsf ? FILTER_EPS : FILTER_PS );
    load_string(IDS_TOPICOPEN, szHelpTopic, sizeof(szHelpTopic));
    if (!get_filename(output, TRUE, filter, 0, IDS_TOPICOPEN))
	    return;

    if ((f = fopen(output, "wb")) == (FILE *)NULL) {
	    return;
    }

    /* create buffer for PS file copy */
    buffer = malloc(COPY_BUF_SIZE);
    if (buffer == (char *)NULL) {
	play_sound(SOUND_ERROR);
	fclose(f);
	unlink(output);
	return;
    }

    /* don't use dfreopen, since that wouldn't work for PDF files */
    infile = fopen(psfile_name(&psfile), "rb");
    if (infile == (FILE *)NULL) {
	play_sound(SOUND_ERROR);
	free(buffer);
	fclose(f);
	unlink(output);
	return;
    }

    info_wait(IDS_WAITWRITE);

    while ( (count = fread(buffer, 1, COPY_BUF_SIZE, infile)) != 0 ) {
	fwrite(buffer, 1, count, f);
    }
    free(buffer);
    fclose(infile);
    fclose(f);

    info_wait(IDS_NOWAIT);
    return;
}

/* extract a range of pages for later printing */
void
gsview_extract()
{
    FILE *f;
    static char output[MAXSTR];
    int thispage = psfile.pagenum;
    PSDOC *doc = psfile.doc;

    if (psfile.name[0] == '\0') {
	    gserror(IDS_NOTOPEN, NULL, MB_ICONEXCLAMATION, SOUND_NOTOPEN);
	    return;
    }

    if (doc == (PSDOC *)NULL) {
	    gserror(IDS_NOPAGE, NULL, MB_ICONEXCLAMATION, SOUND_NONUMBER);
	    return;
    }
    
    if (psfile.ispdf && (doc->numpages == 0)) {
	char buf[MAXSTR];
	load_string(IDS_PDFNOPAGE, buf, sizeof(buf));
	if (message_box(buf, MB_ICONASTERISK | MB_OKCANCEL) == IDCANCEL)
		return;
    }

    load_string(IDS_TOPICOPEN, szHelpTopic, sizeof(szHelpTopic));
    if (doc->numpages != 0)
	if (!get_page(&thispage, TRUE, FALSE))
	    return;

    if (psfile.ispdf) {
	if (!get_pdf2ps_options())
	    return;
    }

    if (!get_filename(output, TRUE, FILTER_PS, 0, IDS_TOPICOPEN))
	    return;

    if ((f = fopen(output, "wb")) == (FILE *)NULL) {
	    return;
    }

    load_string(IDS_WAITWRITE, szWait, sizeof(szWait));
    info_wait(IDS_WAITWRITE);
    if (psfile.ispdf) {
	fclose(f);
	gsview_pdf2ps(output);
	info_wait(IDS_NOWAIT);
	return;
/*
	pdf_extract(f);
*/
    }
    else  {
	if (!dfreopen()) {
	    fclose(f);
	    unlink(output);
	    gserror(0, "Couldn't reopen document", MB_ICONEXCLAMATION, SOUND_NOTOPEN);
	    return;
	}
	if (doc->numpages != 0)
	    psfile_extract(f);
	else {
	    pscopyuntil(psfile.file, f, doc->beginheader, doc->endtrailer, NULL);
        }
	dfclose();
    }

    fclose(f);

    info_wait(IDS_NOWAIT);
    return;
}


/* Copy the headers, marked pages, and trailer to f */
/* Reverse the page order if needed and possible */
void
psfile_extract(FILE *f)
{
    char line[PSLINELENGTH];
    char *comment;
    BOOL pages_written = FALSE;
    BOOL pageorder_written = FALSE;
    int pages = 0;
    int page;
    int i;
    long position;
    PSDOC *doc = psfile.doc;
    int neworder = doc->pageorder;
    BOOL reverse = psfile.page_list.reverse;
    BOOL end_header;
    BOOL line_written;

    if (neworder == NONE)	/* No page order so assume ASCEND */
	neworder = ASCEND;
    /* Don't touch SPECIAL pageorder */

    /* reverse means new page order to be DESCEND */
    if (reverse) {
	if (neworder == ASCEND)
	    neworder = DESCEND;
	else if (neworder == DESCEND) {
	    /* neworder = DESCEND;*/	/* unchanged */
	    reverse = FALSE;	/* already reversed, don't do it again */
	}
    }
    else {
	if (neworder == DESCEND) {
	    neworder = ASCEND;
	    reverse = TRUE;	/* reverse it to become ascending */
	}
    }
    /* neworder = page order of the extracted document */
    /* reverse = reverse the current page order */

    for (i=0; i< doc->numpages; i++) {
	    if (psfile.page_list.select[i]) pages++;
    }

    /* copy header, fixing up %%Pages: and %%PageOrder:
     * Write a DSC 3.0 %%Pages: or %%PageOrder: in header,
     * even if document was DSC 2.x.
     * Remove %%Pages: and %%PageOrder from trailer.
     */
    fseek(psfile.file, doc->beginheader, SEEK_SET);
    position = ftell(psfile.file);
    while ( position < doc->endheader ) {
	psfgets(line, sizeof(line), psfile.file);
	position = ftell(psfile.file);
	end_header = (strncmp(line, "%%EndComments", 13) == 0);
	if ((line[0] != '%') && (line[0] != ' ') && (line[0] != '+')
	 && (line[0] != '\t') && (line[0] != '\r') && (line[0] != '\n'))
	    end_header = TRUE;
	line_written = FALSE;
	if (end_header || strncmp(line, "%%Pages:", 8) == 0) {
	    if (!pages_written) {
		fprintf(f, "%%%%Pages: %d\r\n", pages);
		pages_written = TRUE;
	    }
	    line_written = !end_header;
	}
	if (end_header || strncmp(line, "%%PageOrder:", 12) == 0) {
	    if (!pageorder_written) {
		if (neworder == ASCEND)
		    fputs("%%PageOrder: Ascend\r\n", f);
		else if (neworder == DESCEND)
		    fputs("%%PageOrder: Descend\r\n", f);
		else 
		    fputs("%%PageOrder: Special\r\n", f);
		pageorder_written = TRUE;
	    }
	    line_written = !end_header;
	}
	if (!line_written) {
	    fputs(line, f);	
	}
    }
    if (doc->beginheader != doc->endheader) {
	if (!pages_written) {
	    fprintf(f, "%%%%Pages: %d\r\n", pages);
	    /* pages_written = TRUE; */
	}
	if (!pageorder_written) {
	    if (neworder == ASCEND)
		fputs("%%PageOrder: Ascend\r\n", f);
	    else if (neworder == DESCEND)
		fputs("%%PageOrder: Descend\r\n", f);
	    else 
		fputs("%%PageOrder: Special\r\n", f);
	    /* pageorder_written = TRUE; */
	}
    }

    pscopyuntil(psfile.file, f, doc->beginpreview, doc->endpreview, NULL);
    pscopyuntil(psfile.file, f, doc->begindefaults, doc->enddefaults, NULL);
    pscopyuntil(psfile.file, f, doc->beginprolog, doc->endprolog, NULL);
    pscopyuntil(psfile.file, f, doc->beginsetup, doc->endsetup, NULL);

    page = 1;
    i = reverse ? doc->numpages - 1 : 0;
    while ( reverse ? (i >= 0)  : (i < doc->numpages) ) {
	if (psfile.page_list.select[map_page(i)])  {
	    comment = pscopyuntil(psfile.file, f, doc->pages[i].begin,
				  doc->pages[i].end, "%%Page:");
	    if (doc->pages[i].label)
	        fprintf(f, "%%%%Page: %s %d\r\n", doc->pages[i].label, page);
	    else
	        fprintf(f, "%%%%Page: %d %d\r\n", page, page);
	    page++;
	    free(comment);
	    pscopyuntil(psfile.file, f, -1, doc->pages[i].end, NULL);
	}
        i += reverse ? -1 : 1;
    }

    /* copy trailer, removing %%Pages: and %%PageOrder: */
    fseek(psfile.file, doc->begintrailer, SEEK_SET);
    position = ftell(psfile.file);
    while ( position < doc->endtrailer ) {
	psfgets(line, sizeof(line), psfile.file);
	position = ftell(psfile.file);
	if (strncmp(line, "%%Pages:", 8) == 0) {
	    continue;	/* has already been written in header */
	}
	else if (strncmp(line, "%%PageOrder:", 12) == 0) {
	    continue;	/* has already been written in header */
	}
	else {
	    fputs(line, f);	
	}
    }
}

#ifdef _MSC_VER
/* Brain damaged Microsoft C doesn't support POSIX directory operations.
 * Implement a subset of these ourselves to keep the uniprint
 * enumeration code happy.
 */
struct dirent
{
    char        d_name[260];
};

typedef struct
{
    BOOL finished;
    HANDLE hff;
    WIN32_FIND_DATA wfd;
    char pattern[1024];
    struct dirent de;
} DIR;

DIR * opendir(const char *dirname);
struct dirent *readdir(DIR *dir);
int closedir (DIR *dir);

DIR * opendir(const char *dirname)
{
DIR *dp = malloc(sizeof(DIR));
int i;
char *p;
    if (dp == NULL)
	return NULL;
    memset((char *)dp, 0, sizeof(DIR));
    strcpy(dp->pattern, dirname);
    i = strlen(dp->pattern);
    for (p = dp->pattern; *p; p++)
	if (*p == '/')
	    *p = '\\';
    if (i && dp->pattern[i-1]!='\\')
	strcat(dp->pattern, "\\");
    strcat(dp->pattern, "*");
    dp->finished = FALSE;
    dp->hff = NULL;
    return dp;
}

int closedir (DIR *dir)
{
    if ((dir->hff) && (dir->hff != INVALID_HANDLE_VALUE))
	FindClose(dir->hff);
    free(dir);
}

struct dirent *readdir(DIR *dir)
{
    if (dir->finished)
	return NULL;

    if (dir->hff == NULL) {
	dir->hff = FindFirstFile(dir->pattern, &dir->wfd);
        if (dir->hff == INVALID_HANDLE_VALUE) {
	    dir->finished = TRUE;
    	    return NULL;
	}
    }
    else {
	if (!FindNextFile(dir->hff, &dir->wfd)) {
	    dir->finished = TRUE;
	    return NULL;
	}
    }
    strcpy(dir->de.d_name, dir->wfd.cFileName); 
    return &(dir->de);
}
#endif

/* Add the file name and description to the list
 * Return new offset.
 * If not enough space, don't copy name/description but
 * still return offset as if data was copied.
 * This allows caller to work out what buffer size is required.
 */
int
upp_add_list(char *name, char *buffer, int len, int offset)
{
char *pd;
FILE *f;
char desc[MAXSTR];
int needed, remaining;
    if ( (f = fopen(name, "r")) != (FILE *)NULL ) {
	if (fgets(desc, sizeof(desc)-1, f)) {
	    strtok(desc, "\042\n");
	    pd = strtok(NULL, "\042\n");
	    if (pd && strlen(pd)) {
	        needed = strlen(name) + strlen(pd) + 3;
	        remaining = len - offset;
		if (needed < remaining)  {
		    strcpy(buffer+offset, name);
		    offset += strlen(name) + 1;
		    strcpy(buffer+offset, pd);
		    offset += strlen(pd) + 1;
		    buffer[offset] = '\0';  /* double trailing null */
		}
		else {
		    /* don't copy data, but tell caller how much */
		    /* space it needs  */
		    offset += strlen(name) + 1;
		    offset += strlen(pd) + 1;
		}
	    }
	}
	fclose(f);
    }
    return offset;
}

/* search path for any uniprint configuration files (*.upp),
 * appending the filename and description of any found to
 * buffer at offset.  Return updated offset.
 * If offset > len, then not all data was placed in buffer.
 */
int 
enum_upp(char *path, char *buffer, int len, int offset)
{    
DIR *dirp;
struct dirent* de;
struct stat st;
char name[MAXSTR];
char *p;
    dirp = opendir(path);
    if (dirp == NULL)
	return 2;
    while ( (de = readdir(dirp)) != NULL) {
	if (strlen(path) + strlen(de->d_name) + 1 < MAXSTR) {
	    strcpy(name, path);
	    strcat(name, "\\");
	    strcat(name, de->d_name);
	    if (stat(name, &st) != -1) {
		if  (st.st_mode & S_IFDIR) {
		    /* don't recurse into subdirectories */
		}
		else {
		    /* an ordinary file */
		    p = strrchr(name,'.');
		    if (p && (stricmp(p, ".upp") == 0)) {
			offset = upp_add_list(name, buffer, len, offset);
		    }
		}
	    }
	}
    }
    closedir(dirp);
    return offset;
}

/* return number of bytes needed */
int
enum_upp_path(char *path, char *buffer, int len)
{
char pbuf[1024];
char *p, *q, *r;
int offset = 0;
    if (buffer == (char *)NULL)
	len = 0;
    if (len >= 2) {
	buffer[0] = '\0';
	buffer[1] = '\0';
    }
    p = path;
    while (p) {
	q = strchr(p, ';');
	if (q) {
	    strncpy(pbuf, p, (int)(q-p));
	    pbuf[(int)(q-p)] = '\0';
	}
	else
	    strcpy(pbuf, p);
	if (strlen(pbuf)) {
	    r = pbuf + strlen(pbuf) - 1;
	    if ( (*r == '\\') || (*r == '/') )
		*r = '\0';	/* trailing slash will be added later */
	    offset = enum_upp(pbuf, buffer, len, offset);
	}
	if (q)
	    p = q+1;
	else
	    p = NULL;
    }
    return offset + 2;
}


char * 
uppmodel_to_name(char *buffer, char *model)
{
char *p, *desc;
    for (p=buffer; *p; p+=strlen(p)+1) {
	desc = p + strlen(p) + 1;
	if (strcmp(desc, model) == 0)
	    return p;
	p = desc;
    }
    return NULL;
}

char * 
uppname_to_model(char *buffer, char *name)
{
char *p, *desc;
    for (p=buffer; *p; p+=strlen(p)+1) {
	desc = p + strlen(p) + 1;
	if (strcmp(p, name) == 0)
	    return desc;
	p = desc;
    }
    return NULL;
}


/* common printer code */
BOOL
gsview_cprint(char *psname, char *optname)
{
char buf[MAXSTR];
int i;
float print_xdpi, print_ydpi;
int width, height;
int widthpt, heightpt;
struct prop_item_s *proplist;
FILE *optfile;
FILE *pcfile;
char *p;
static char output[MAXSTR]; /* output filename for printing */
static char queue[MAXSTR];  /* output queue if not printing to file */
BOOL printtofile=FALSE;
float xoffset = 0;
float yoffset = 0;
char section[MAXSTR];
PROFILE *prf;

    /*  ASSUMES psfile.file is valid */

    /* create temporary file containing pages to print */
/*
    if ((psname[0] != '\0') && !debug)
	unlink(psname);
*/
    psname[0] = '\0';
    if ( (pcfile = gp_open_scratch_file(szScratch, psname, "wb")) == (FILE *)NULL) {
	gserror(IDS_NOTEMP, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	play_sound(SOUND_ERROR);
	return FALSE;
    }

    if (psfile.doc == (PSDOC *)NULL) {
	/* copy non-DSC file */
	char *buffer;
	int count;
	/* create buffer for PS file copy */
	buffer = malloc(COPY_BUF_SIZE);
	if (buffer == (char *)NULL) {
	    play_sound(SOUND_ERROR);
	    fclose(pcfile);
	    unlink(psname);
	    return FALSE;
	}
	while ( (count = fread(buffer, 1, COPY_BUF_SIZE, psfile.file)) != 0 ) {
	    fwrite(buffer, 1, count, pcfile);
	}
	free(buffer);
    }
    else {
	if (psfile.ispdf) {
	    if (option.psprinter) {
	        gserror(IDS_PRINTPDFPS, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
		return FALSE;
	    }
	    if (!pdf_extract(pcfile)) {
		fclose(pcfile);
		return FALSE;
	    }
	}
	else  {
	    /* copy DSC file */
	    if (psfile.doc->numpages != 0)
		psfile_extract(pcfile);
	    else
		pscopyuntil(psfile.file, pcfile, psfile.doc->beginheader, psfile.doc->endtrailer, NULL);
	}
    }

    fclose(pcfile);

    if (option.psprinter) {
	if (!gp_printfile(psname, option.printer_queue)) {
	    play_sound(SOUND_ERROR);
	    return FALSE;
	}
	unlink(psname);
	return FALSE;	/* don't continue with Ghostscript */
    }
	
    if (option.print_to_file || (strcmp(option.printer_queue, "FILE:")==0)) {
	printtofile = TRUE;
	if (!get_filename(output, TRUE, FILTER_ALL, IDS_OUTPUTFILE, IDS_TOPICPRINT))
	    return FALSE;
    }
    else {
	strcpy(queue, szSpoolPrefix);
	strcat(queue, option.printer_queue);
    }

    /* calculate image size */
    switch (sscanf(option.device_resolution,"%fx%f", &print_xdpi, &print_ydpi)) {
	case EOF:
	case 0:
	    print_xdpi = print_ydpi = DEFAULT_RESOLUTION;
	    break;
	case 1:
	    print_ydpi = print_xdpi;
    }
    i = get_paper_size_index();
    if (i < 0) {
	widthpt = option.user_width;
	heightpt = option.user_height;
    }
    else {
	widthpt = papersizes[i].width;
	heightpt = papersizes[i].height;
    }
    width  = (unsigned int)(widthpt  / 72.0 * print_xdpi + 0.5);
    height = (unsigned int)(heightpt / 72.0 * print_ydpi + 0.5);

    /* create options file */
/*
    if ((optname[0] != '\0') && !debug)
	    unlink(optname);
*/
    optname[0] = '\0';
    if ( (optfile = gp_open_scratch_file(szScratch, optname, "w")) == (FILE *)NULL) {
	    play_sound(SOUND_ERROR);
	    return FALSE;
    }
    fprintf(optfile, "-I\042%s\042\n", option.gsinclude);
    fprintf(optfile, "-dNOPAUSE\n");
    if (option.safer)
	fprintf(optfile, "-dSAFER\n");
    if (strcmp(option.device_name, "uniprint") == 0) {
	/* uniprint sets the device name and resolution in */
	/* a configuration file */
	/* Since we don't know the resolution, set the page size in points */
	fprintf(optfile, "-dDEVICEWIDTHPOINTS=%u\n", widthpt);
	fprintf(optfile, "-dDEVICEHEIGHTPOINTS=%u\n", heightpt);
    }
    else {
	fprintf(optfile, "-sDEVICE=%s\n",option.device_name);
	fprintf(optfile, "-dDEVICEXRESOLUTION=%g\n", (double)print_xdpi);
        fprintf(optfile, "-dDEVICEYRESOLUTION=%g\n", (double)print_ydpi);
	fprintf(optfile, "-dDEVICEWIDTH=%u\n", width);
	fprintf(optfile, "-dDEVICEHEIGHT=%u\n", height);
    }

    fprintf(optfile, "-sOutputFile=\042");
    for (p=(printtofile) ? output : queue; *p != '\0'; p++)
	if (*p == '\\')
	    /* fputc('/',optfile); */
	    fputc('\\',optfile);
	else
	    fputc(*p,optfile);
    fputc('\042',optfile);
    fputc('\n',optfile);

    strcpy(section, option.device_name);
    strcat(section, " Options");
    if ( (prf = profile_open(szIniFile)) != (PROFILE *)NULL ) {
        /* PageOffset */
	profile_read_string(prf, section, "Xoffset", "0", buf, sizeof(buf)-2);
	if (sscanf(buf, "%f", &xoffset) != 1)
	    xoffset = 0;
	profile_read_string(prf, section, "Yoffset", "0", buf, sizeof(buf)-2);
	if (sscanf(buf, "%f", &yoffset) != 1)
	    yoffset = 0;
	if ((xoffset != 0) || (yoffset != 0))
	    fprintf(optfile, "-c \042<< /PageOffset [%g %g] >> setpagedevice\042\n-f\n", 
	    (double)xoffset, (double)yoffset);

	/* Options */
	profile_read_string(prf, section, "Options", "", buf, sizeof(buf)-2);
	if (strlen(buf) > 0)
	   fprintf(optfile, "%s\n", buf);
	profile_close(prf);
    }


    if ((proplist = get_properties(option.device_name)) != (struct prop_item_s *)NULL) {
	/* output current property selections */
	for (i=0; proplist[i].name[0]; i++) {
	    if (strcmp(proplist[i].value, not_defined) != 0)
		fprintf(optfile,"-%s=%s\n", proplist[i].name, proplist[i].value);
	}
	free((char *)proplist);
    }
    p = option.gsother;
    while ((p = gs_argnext(p, buf)) != NULL)
        fprintf(optfile, "%s\n", buf);

    fclose(optfile);
    return TRUE;
}

