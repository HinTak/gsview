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

/* gvctext.c */
/* Text Extract and Search module of PM and Windows GSview */

#ifdef _Windows
#include "gvwin.h"
#else
#include "gvpm.h"
#endif

/* extract text from next line of ps file */
/* return count of characters written to dest */
int
text_extract_line(char *dest, FILE *inf)
{
char linebuf[PSLINELENGTH];
int count = 0;
char *p;
char ch;
int instring;
	*dest = '\0';
	if ( (p=fgets(linebuf, sizeof(linebuf), inf)) == (char *)NULL )
	    return 0;
	/* skip over binary sections */
	if (strncmp(linebuf, "%%BeginBinary:",14)==0) {
	    long count = 0;
	    int read;
	    char buf[1024];
	    if (sscanf(linebuf+14, "%ld", &count) != 1)
		count = 0;
	    while (count) {
		read = fread(buf, 1, sizeof(buf), inf);
		count -= read;
		if (read == 0)
		    count = 0;
	    }
	    if ( (p=fgets(linebuf, sizeof(linebuf), inf)) == (char *)NULL )
	        return 0;
	}
	if (strncmp(linebuf, "%%BeginData:",12)==0) {
	    long count;
	    int read;
	    char buf[PSLINELENGTH];
	    if (sscanf(linebuf+12, "%ld %*s %s", &count, buf) != 2)
		count = 0;
	    if (strncmp(buf, "Lines", 5) == 0) {
		while (count) {
		    count--;
		    if (fgets(buf, sizeof(buf), inf) == (char *)NULL)
			count = 0;
		}
	    }
	    else {
		while (count) {
		    read = fread(buf, 1, sizeof(buf), inf);
		    count -= read;
		    if (read == 0)
			count = 0;
		}
	    }
	    if ( (p=fgets(linebuf, sizeof(linebuf), inf)) == (char *)NULL )
	        return 0;
	}
	instring = FALSE;
	while ((ch = *p)!='\0') {
	    	if (!instring && (ch=='%'))
	    	    break;	/* comment until EOL */
	    	if (ch == '(') {
	    	    if (instring) {
			*dest++ = ch;
			count++;
		    }
	    	    instring++;
	    	}
	        else if (ch == ')') {
	            instring--;
	    	    if (instring) {
			*dest++ = ch;
			count++;
		    }
	    	    else {
			*dest++ = ' ';  /* may need to be changed */
			count++;
		    }
	        }
	        else if (instring && (ch == '\\')) {
	            ch = *++p;
	            if (ch == '\0') {
	                p--;
	            }
	            else {
			if ((ch != '(') && (ch != ')') && (ch !='\\')) {
			    *dest++ = '\\';
			    count++;
			}
			*dest++ = ch;
			count++;
		    }
	        }
	        else if (instring) {
		    *dest++ = ch;
		    count++;
		}
	    	p++;
	}
	*dest = '\0';
	return count;
}

void 
text_extract(FILE *outf, FILE *inf, unsigned long end)
{
char outline[PSLINELENGTH];
	while (ftell(inf) < end) {
	    if (text_extract_line(outline, inf)) {
		fputs(outline, outf);
		fputc('\n', outf);
	    }
	}
}



/* extract text from a range of pages */
void
gsview_text_extract()
{
	FILE *f;
	static char output[MAXSTR];
	int thispage = psfile.pagenum;

	if (psfile.name[0] == '\0') {
		gserror(IDS_NOTOPEN, NULL, MB_ICONEXCLAMATION, SOUND_NOTOPEN);
		return;
	}

	load_string(IDS_TOPICTEXT, szHelpTopic, sizeof(szHelpTopic));
	if ((doc != (PSDOC *)NULL) && (doc->numpages != 0))
	    if (!get_page(&thispage, TRUE))
	        return;

	if (!get_filename(output, TRUE, FILTER_TXT, 0, IDS_TOPICTEXT))
		return;

	if ((f = fopen(output, "w")) == (FILE *)NULL) {
		return;
	}

	load_string(IDS_WAITWRITE, szWait, sizeof(szWait));
	info_wait(TRUE);
	if (doc == (PSDOC *)NULL) {
	    /* scan whole document */
	    unsigned long end;
	    if ( (psfile.file = fopen(psfile.name, "rb")) == (FILE *)NULL ) {
		fclose(f);
		return;
	    }
	    fseek(psfile.file, 0L, SEEK_END);
	    end = ftell(psfile.file);
	    fseek(psfile.file, 0L, SEEK_SET);
	    text_extract(f, psfile.file, end);
	    dfclose();
	}
	else {
	  if (doc->numpages != 0) {
	    int i;
	    for (i = 0; i < doc->numpages; i++) {
		if (page_list.select[map_page(i)])  {
	            fseek(psfile.file, doc->pages[map_page(i)].begin, SEEK_SET);
	            text_extract(f, psfile.file, doc->pages[map_page(i)].end);
		    fputc('\f', f);
		    fputc('\n', f);
		}
	    }
	  }
	  else {
	    fseek(psfile.file, doc->begincomments, SEEK_SET);
	    text_extract(f, psfile.file, doc->endtrailer);
	  }
	}

	fclose(f);

	info_wait(FALSE);
	return;
}



char *
text_find_string(char *str, char *find, int flen)
{
char *p, *last;
int mcount = 0;
	last = p = str;
	while (*p) {
	    if (*p == ' ') {
		p++;
		continue;	/* ignore white space */
	    }
	    if (mcount) {
	        if (toupper(*p) == find[mcount])
		    mcount++;	/* matched one more character */
		else {
		    mcount = 0;
		    p = last+1;	/* retrace to just past last partial match */
		}
	    }
	    else {
		if (toupper(*p) == *find) {
		    last = p;
		    mcount++;	/* start of partial match */
		}
	    }
	    if (mcount == flen)
		return last;
	    p++;
	}
	return (char *)NULL;
}

/* if str found return malloc'd string containing match */
char *
text_find_section(FILE *inf, unsigned long end, char *str)
{
char dbuf[PSLINELENGTH+PSLINELENGTH];
char sbuf[PSLINELENGTH/4];
int dlength;
int slength;
int count;

	/* copy str to uppercase, removing spaces */
	slength = 0;
	for (count=0; str[count]; count++) {
	    if (slength > PSLINELENGTH/4)
		return NULL;
	    if (str[count] != ' ')			/* ignore spaces */
	        sbuf[slength++] = toupper(str[count]);	/* searches are case insensitive */
	}
	sbuf[slength] = '\0';
	if (slength==0)
	    return NULL;
	dlength = 0;
	while (ftell(inf) < end) {
	    while ((ftell(inf) < end) && (dlength < PSLINELENGTH)) {
		count = text_extract_line(dbuf+dlength, inf);
	    	dlength += count;
		if (count) { /* separate lines by spaces */
		    dbuf[dlength++] = ' ';
		    dbuf[dlength] = '\0';
		}
	    }
	    if (text_find_string(dbuf, sbuf, slength)) {
		str = malloc(dlength+1);
		if (str)
		    strcpy(str, dbuf);
		return str;
	    }
	    if (dlength > slength) {
	       memmove(dbuf, dbuf+dlength-slength, slength+1);
	       dlength = slength;
	    }
	    else 
	    	dlength = 0;
        }
        return NULL;
}


void
gsview_text_find()
{
char prompt[MAXSTR];		/* input dialog box prompt and message box string */
char answer[MAXSTR];		/* input dialog box answer string */
int thispage = psfile.pagenum;
	if (not_dsc())
	    return;
	if (doc->numpages == 0) {
	    gserror(IDS_NOPAGE, NULL, MB_ICONEXCLAMATION, SOUND_NONUMBER);
	    return;
	}
	load_string(IDS_TEXTFIND, prompt, sizeof(prompt));
	strcpy(answer, szFindText);
	load_string(IDS_TOPICTEXT, szHelpTopic, sizeof(szHelpTopic));
	if (!get_string(prompt,answer))
	    return;
	strcpy(szFindText, answer);
	if (!get_page(&thispage, TRUE))
	        return;
	gsview_text_findnext();
}

void
gsview_text_findnext()
{
int i;
char *p;
	if (not_dsc())
	    return;
	if (strlen(szFindText)==0) {
	    gserror(IDS_TEXTNOTFIND, NULL, MB_ICONEXCLAMATION, 0);
	    return;
	}
	dfreopen();
	load_string(IDS_WAITSEARCH, szWait, sizeof(szWait));
	info_wait(TRUE);
	for (i = 0; i < doc->numpages; i++) {
	    if (page_list.select[map_page(i)])  {
		page_list.select[map_page(i)] = FALSE;
	        fseek(psfile.file, doc->pages[map_page(i)].begin, SEEK_SET);
		p = text_find_section(psfile.file, doc->pages[map_page(i)].end, szFindText);
		if (p) {	/* found it */
		    info_wait(FALSE);
		    free(p);
		    psfile.pagenum = i+1;
		    if (gs_open())
	                dsc_dopage();
		    dfclose();
		    return;
		}
	    }
	}
	dfclose();
        info_wait(FALSE);
	gserror(IDS_TEXTNOTFIND, NULL, MB_ICONEXCLAMATION, 0);
}
