/* Copyright (C) 1993-1995, Russell Lang.  All rights reserved.
  
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

    output[0] = '\0';
    if (psfile.name[0] == '\0') {
	    gserror(IDS_NOTOPEN, NULL, MB_ICONEXCLAMATION, SOUND_NOTOPEN);
	    return;
    }

    load_string(IDS_TOPICOPEN, szHelpTopic, sizeof(szHelpTopic));
    if (!get_filename(output, TRUE, FILTER_PS, 0, IDS_TOPICOPEN))
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
    infile = fopen(psfile.name, "rb");
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

    if (!get_filename(output, TRUE, FILTER_PS, 0, IDS_TOPICOPEN))
	    return;

    if ((f = fopen(output, "wb")) == (FILE *)NULL) {
	    return;
    }

    load_string(IDS_WAITWRITE, szWait, sizeof(szWait));
    info_wait(IDS_WAITWRITE);
    if (psfile.ispdf) {
	pdf_extract(f);
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
void
psfile_extract(FILE *f)
{
    char text[PSLINELENGTH];
    char *comment;
    BOOL pages_written = FALSE;
    BOOL pages_atend = FALSE;
    int pages = 0;
    int page;
    int i;
    long position;
    PSDOC *doc = psfile.doc;

    for (i=0; i< doc->numpages; i++) {
	    if (psfile.page_list.select[i]) pages++;
    }

    position = doc->beginheader;
    while ( (comment = pscopyuntil(psfile.file, f, position,
			   doc->endheader, "%%Pages:")) != (char *)NULL ) {
	position = ftell(psfile.file);
	if (pages_written || pages_atend) {
	    free(comment);
	    continue;
	}
	sscanf(comment+8, "%s", text);
	if (strcmp(text, "(atend)") == 0) {
	    fputs(comment, f);
	    pages_atend = TRUE;
	} else {
	    switch (sscanf(comment+8, "%*d %d", &i)) {
		case 1:
		    fprintf(f, "%%%%Pages: %d %d\r\n", pages, i);
		    break;
		default:
		    fprintf(f, "%%%%Pages: %d\r\n", pages);
		    break;
	    }
	    pages_written = TRUE;
	}
	free(comment);
    }
    pscopyuntil(psfile.file, f, doc->beginpreview, doc->endpreview, NULL);
    pscopyuntil(psfile.file, f, doc->begindefaults, doc->enddefaults, NULL);
    pscopyuntil(psfile.file, f, doc->beginprolog, doc->endprolog, NULL);
    pscopyuntil(psfile.file, f, doc->beginsetup, doc->endsetup, NULL);

    page = 1;
    for (i = 0; i < doc->numpages; i++) {
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
    }

    position = doc->begintrailer;
    while ( (comment = pscopyuntil(psfile.file, f, position,
			   doc->endtrailer, "%%Pages:")) != (char *)NULL ) {
	position = ftell(psfile.file);
	if (pages_written) {
	    free(comment);
	    continue;
	}
	switch (sscanf(comment+8, "%*d %d", &i)) {
	    case 1:
		fprintf(f, "%%%%Pages: %d %d\r\n", pages, i);
		break;
	    default:
		fprintf(f, "%%%%Pages: %d\r\n", pages);
		break;
	}
	pages_written = TRUE;
	free(comment);
    }
}

/* common printer code */
BOOL
gsview_cprint(BOOL to_file, char *psname, char *optname)
{
char buf[MAXSTR];
int i;
float print_xdpi, print_ydpi;
int width, height;
struct prop_item_s *proplist;
FILE *optfile;
FILE *pcfile;
char *p;
static char output[MAXSTR]; /* output filename for printing */
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
	
    if (to_file || (strcmp(option.printer_port, "FILE:")==0)) {
	if (!get_filename(output, TRUE, FILTER_ALL, IDS_OUTPUTFILE, IDS_TOPICPRINT))
	    return FALSE;
    }
    else {
	strcpy(output, szSpoolPrefix);
	strcat(output, option.printer_port);
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
	width = option.user_width;
	height = option.user_height;
    }
    else {
	width = papersizes[i].width;
	height = papersizes[i].height;
    }
    width  = (unsigned int)(width  / 72.0 * print_xdpi + 0.5);
    height = (unsigned int)(height / 72.0 * print_ydpi + 0.5);

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
    fprintf(optfile, "-sDEVICE=%s\n",option.device_name);
    fprintf(optfile, "-dDEVICEXRESOLUTION=%g\n", (double)print_xdpi);
    fprintf(optfile, "-dDEVICEYRESOLUTION=%g\n", (double)print_ydpi);
    fprintf(optfile, "-dDEVICEWIDTH=%u\n", width);
    fprintf(optfile, "-dDEVICEHEIGHT=%u\n", height);

    fprintf(optfile, "-sOutputFile=\042");
    for (p=output; *p != '\0'; p++)
	if (*p == '\\')
	    /* fputc('/',optfile); */
	    fputc('\\',optfile);
	else
	    fputc(*p,optfile);
    fputc('\042',optfile);
    fputc('\n',optfile);

    /* PageOffset */
    strcpy(section, option.device_name);
    strcat(section, " PageOffset");
    if ( (prf = profile_open(szIniFile)) != (PROFILE *)NULL ) {
	profile_read_string(prf, section, "X", "0", buf, sizeof(buf)-2);
	if (sscanf(buf, "%f", &xoffset) != 1)
	    xoffset = 0;
	profile_read_string(prf, section, "Y", "0", buf, sizeof(buf)-2);
	if (sscanf(buf, "%f", &yoffset) != 1)
	    yoffset = 0;
	profile_close(prf);
    }
    if ((xoffset != 0) || (yoffset != 0))
	fprintf(optfile, "-c \042<< /PageOffset [%g %g] >> setpagedevice\042\n-f\n", 
	(double)xoffset, (double)yoffset);


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

