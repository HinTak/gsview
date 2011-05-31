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
	if (strlen(entries) == 0) {
	    profile_close(prf);
	    free(entries);
	    return NULL;
	}
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


/* extract a range of pages for later printing */
void
gsview_extract()
{
	FILE *f;
	static char output[MAXSTR];
	int thispage = psfile.pagenum;

	if (psfile.name[0] == '\0') {
		gserror(IDS_NOTOPEN, NULL, MB_ICONEXCLAMATION, SOUND_NOTOPEN);
		return;
	}

	if (doc == (PSDOC *)NULL) {
		gserror(IDS_NOPAGE, NULL, MB_ICONEXCLAMATION, SOUND_NONUMBER);
		return;
	}
	
	load_string(IDS_TOPICPRINT, szHelpTopic, sizeof(szHelpTopic));
	if (doc->numpages != 0)
	    if (!get_page(&thispage, TRUE))
	        return;

	if (!get_filename(output, TRUE, FILTER_PS, 0, IDS_TOPICPRINT))
		return;

	if ((f = fopen(output, "wb")) == (FILE *)NULL) {
		return;
	}

	load_string(IDS_WAITWRITE, szWait, sizeof(szWait));
	info_wait(TRUE);
	if (doc->numpages != 0)
	    psfile_extract(f);
	else {
	    dfreopen();
	    dsc_copy(psfile.file, f, doc->begincomments, doc->endtrailer, NULL);
	    dfclose();
	}

	fclose(f);

	info_wait(FALSE);
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
    int page = 1;
    int i;
    long position;

    for (i=0; i< doc->numpages; i++) {
	    if (page_list.select[i]) pages++;
    }

    position = doc->begincomments;
    while ( (comment = dsc_copy(psfile.file, f, position,
			   doc->endcomments, "%%Pages:")) != (char *)NULL ) {
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
    dsc_copy(psfile.file, f, doc->beginpreview, doc->endpreview, NULL);
    dsc_copy(psfile.file, f, doc->begindefaults, doc->enddefaults, NULL);
    dsc_copy(psfile.file, f, doc->beginprolog, doc->endprolog, NULL);
    dsc_copy(psfile.file, f, doc->beginsetup, doc->endsetup, NULL);

    page = 1;
    for (i = 0; i < doc->numpages; i++) {
	if (page_list.select[map_page(i)])  {
	    comment = dsc_copy(psfile.file, f, doc->pages[i].begin,
				  doc->pages[i].end, "%%Page:");
	    fprintf(f, "%%%%Page: %s %d\r\n",
		    doc->pages[i].label, page++);
	    free(comment);
	    dsc_copy(psfile.file, f, -1, doc->pages[i].end, NULL);
	}
    }

    position = doc->begintrailer;
    while ( (comment = dsc_copy(psfile.file, f, position,
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
gsview_cprint(BOOL to_file, char *cfname, char *optfname)
{
char buf[MAXSTR];
int i;
float print_xdpi, print_ydpi;
int width, height;
struct prop_item_s *proplist;
int pages;
int thispage = psfile.pagenum;
FILE *optfile;
FILE *pcfile;
char *fname;
char *p;
static char output[MAXSTR]; /* output filename for printing */

	fname = (char *)NULL;
	if (doc == (PSDOC *)NULL) {
		play_sound(SOUND_NONUMBER);
	    	load_string(IDS_PRINTINGALL, buf, sizeof(buf));
		if (message_box(buf, MB_ICONASTERISK) == IDCANCEL)
			return FALSE;
		fname = psfile.name;
		pages = 1;
	}
	else {
	    pages = 1;
	    if (doc->numpages != 0) {
		if (!get_page(&thispage, TRUE))
		    return FALSE;
	        pages = 0;
	        for (i=0; i< doc->numpages; i++) {
	            if (page_list.select[i]) pages++;
	        }
	    }

	    if ((cfname[0] != '\0') && !debug)
		unlink(cfname);
	    cfname[0] = '\0';
	    if ( (pcfile = gp_open_scratch_file(szScratch, cfname, "wb")) == (FILE *)NULL) {
		play_sound(SOUND_ERROR);
		return FALSE;
	    }
	    if (doc->numpages != 0)
	        psfile_extract(pcfile);
	    else {
	        dfreopen();
	        dsc_copy(psfile.file, pcfile, doc->begincomments, doc->endtrailer, NULL);
	        dfclose();
	    }
	    fclose(pcfile);
	    fname = cfname;
	}
	
	if (to_file) {
	    if (!get_filename(output, TRUE, FILTER_ALL, IDS_OUTPUTFILE, IDS_TOPICPRINT))
		return FALSE;
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
	    width = option.user_height;
	}
	else {
	    width = paper_size[i].width;
	    height = paper_size[i].height;
	}
	width  = (unsigned int)(width  / 72.0 * print_xdpi);
	height = (unsigned int)(height / 72.0 * print_ydpi);

	if ((optfname[0] != '\0') && !debug)
		unlink(optfname);
	optfname[0] = '\0';
	if ( (optfile = gp_open_scratch_file(szScratch, optfname, "w")) == (FILE *)NULL) {
		play_sound(SOUND_ERROR);
		return FALSE;
	}
	fprintf(optfile, "-dNOPAUSE\n");
	if (option.safer)
	    fprintf(optfile, "-dSAFER\n");
	fprintf(optfile, "-sDEVICE=%s\n",option.device_name);
	fprintf(optfile, "-r%gx%g\n", (double)print_xdpi, (double)print_ydpi);
	fprintf(optfile, "-g%ux%u\n",width,height);
	if (to_file) {
	    fprintf(optfile, "-sOutputFile=");
	    for (p=output; *p != '\0'; p++)
	        if (*p == '\\')
	            fputc('/',optfile);
	        else
	            fputc(*p,optfile);
	    fputc('\n',optfile);
	}
	if ((proplist = get_properties(option.device_name)) != (struct prop_item_s *)NULL) {
	    /* output current property selections */
	    for (i=0; proplist[i].name[0]; i++) {
		if (strcmp(proplist[i].value, not_defined) != 0)
		    fprintf(optfile,"-%s=%s\n", proplist[i].name, proplist[i].value);
	    }
	    free((char *)proplist);
	}
	for (p=fname; *p != '\0'; p++)
	    if (*p == '\\')
	        fputc('/',optfile);
	    else
	        fputc(*p,optfile);
        fputs("\nquit.ps\n", optfile);
	fclose(optfile);
	return TRUE;
}

