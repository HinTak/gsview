/* Copyright (C) 2000-2002, Ghostgum Software Pty Ltd.  All rights reserved.
  
  This file is part of GSview.
   
  This program is distributed with NO WARRANTY OF ANY KIND.  No author
  or distributor accepts any responsibility for the consequences of using it,
  or for whether it serves any particular purpose or works at all, unless he
  or she says so in writing.  Refer to the GSview Licence (the "Licence") 
  for full details.
   
  Every copy of GSview must include a copy of the Licence, normally in a 
  plain ASCII text file named LICENCE.  The Licence grants you the right 
  to copy, modify and redistribute GSview, but only under certain conditions 
  described in the Licence.  Among other things, the Licence requires that 
  the copyright notice and this notice be preserved on all copies.
*/

/* docutil.c */

/* 
 * Utility for generating GSview documentation.
 * Documentation is now in XML, and xt is used to
 * translate this into the required format.
 * Unfortunately we can't get xt to do all that we
 * need, so this program does a little pre and post
 * processing.
 *
 * Replace ":" by "::" 
 *   docutil --pre-ipf input.xml output.xml
 * 
 * Replace "::" by "&colon." and to replace link target name 
 * by the target number.  Write index of targets names and numbers.
 *   docutil --post-ipf input.ipf output.ipf output.h
 * 
 * Split a file created by xml2htmh.xsl into multiple HTML files
 * for Windows HTML help
 *   docutil --post-htmh input.htm
 *
 * Remove new-line after <li> to make a help contents file
 * acceptable to the broken Microsoft Windows HTML Help compiler.
 *   docutil --post-hhc input.hhc output.hhc
 *
 * Remove new-line after <li> to make a help index file
 * acceptable to the broken Microsoft Windows HTML Help compiler.
 *   docutil --post-hhk input.hhk output.hhk
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXSTR 256

char line[MAXSTR];
char filename[MAXSTR];

/* for --post-htmh*/
const char anchor[] = "<a name=\042";
int anchor_len;
const char subdir[] = "html/";
const char htmlsuffix[] = ".htm";

int posthtmh(FILE *infile)
{
    FILE *f = NULL;
    anchor_len = strlen(anchor);
    while (fgets(line, sizeof(line)-1, infile)) {
	if (strncmp(line, anchor, anchor_len) == 0) {
	    /* new topic */
	    char *p;
	    if (f) {
		fputs("</body>\n</html>\n", f);
		fclose(f);
	    }
	    f = NULL;
	    strcpy(filename, subdir);
	    strcat(filename, line+anchor_len);
	    p = filename;
	    while (*p && (*p != '\042'))
		p++;
	    *p = '\0';
	    strcat(filename, htmlsuffix);
	    f = fopen(filename, "w");
	    if (f) {
		fputs("<html>\n<body>\n", f);
	    }
	}
	if (f) {
	    fputs(line, f);
	}
    } 
    if (f) {
	fputs("</body>\n</html>\n", f);
	fclose(f);
    }
    f = NULL;
    return 0;
}

int posthhc(FILE *infile, FILE *outfile)
{
    while (fgets(line, sizeof(line)-1, infile)) {
	if (strcmp(line, "<li>\n") == 0)
	    fputs("<li> ", outfile);
	else
	    fputs(line, outfile);
    } 
    return 0;
}

int posthhk(FILE *infile, FILE *outfile)
{
    while (fgets(line, sizeof(line)-1, infile)) {
	if (strcmp(line, "<li>\n") == 0)
	    fputs("<li> ", outfile);
	else
	    fputs(line, outfile);
    } 
    return 0;
}

int preipf(FILE *infile, FILE *outfile)
{
    char *p;
    while (fgets(line, sizeof(line)-1, infile)) {
	for (p=line; *p; p++) {
	    if (*p == ':')
		fputc(':', outfile);	/* duplicate colons */
	    fputc(*p, outfile);
	}
    }
    return 0;
}

const char ipflink[] = ":link reftype=hd res=";
int ipflink_len;
const char ipfhead[] = ":h1 res=";
int ipfhead_len;
struct LIST
{
	int id;
	char *string;
	struct LIST *next;
	};
struct LIST *list = NULL;
struct LIST *head = NULL;

/* look up a link name and return id */
int
lookup(char *s)
{
    char *c;
    char tokstr[MAXSTR];
    char *match; 
    int l;

    strcpy(tokstr, s);
    strtok(tokstr, ".");	/* truncate at . */
    list = head;
    while (list != NULL) {
        c = list->string;
        while (*c && (*c == ' ')) 
	    c++;
        if (strcmp(tokstr, c) == 0) 
	    return(list->id);
        list = list->next;
    }
    return(0);
}


int postipf(FILE *infile, FILE *outfile, FILE *extrafile)
{
    char *p;
    char *s;
    int i;
    ipflink_len = strlen(ipflink);
    ipfhead_len = strlen(ipfhead);
    
    /* first pass, build up index */
    while (fgets(line, sizeof(line)-1, infile)) {
	if (strncmp(line, ipfhead, ipfhead_len) == 0) {
	    if (list == NULL)    
		head = (list = (struct LIST *) malloc(sizeof(struct LIST)));
	    else
		list = (list->next = (struct LIST *) malloc(sizeof(struct LIST)));
	    s = line + ipfhead_len;
	    list->id = atoi(s);
	    while (*s && (*s != ' '))	/* skip browse number */
	        s++;
	    while (*s && (*s != '\047'))		/* find opening ' */
	        s++;
	    s++;					/* skip opening ' */
	    for (i=0; s[i] && s[i]!='\047'; i++)	/* find trailing ' */
		/* do nothing */;
	    list->string = (char *) malloc (i+1);
	    strncpy(list->string, s, i);
	    list->string[i+1] = '\0';
	}
    }

    /* second pass, do translation */
    rewind(infile);
    while (fgets(line, sizeof(line)-1, infile)) {
	for (p=line; *p; p++) {
	    if ((*p == ':') && (*(p+1) == ':')) {
		fputs("&colon.", outfile);
		p++;
	    }
	    else if (strncmp(p, ipflink, ipflink_len) == 0) {
		s = p + ipflink_len;
		i = lookup(s);
		while (*s && (*s != '.'))
		    s++;
		if (i) {
		    fwrite(p, 1, ipflink_len, outfile);
		    fprintf(outfile, "%06d.", i);
		    p = s;
		}
		else
	            fputc(*p, outfile);
	    }
	    else
	        fputc(*p, outfile);
	}
    }

    /* write out index */
    fputs("RCDATA IDR_HELP\nBEGIN\n", extrafile);
    list = head;
    while (list != NULL) {
	fprintf(extrafile, " %06d, \042%s\042,\n", list->id, list->string);
        list = list->next;
    }
    fputs(" 0, \042\042\nEND\n", extrafile);
    return 0;
}

int main(int argc, char *argv[])
{
    FILE *infile = NULL;
    FILE *outfile = NULL;
    FILE *extrafile = NULL;
    int code = 0;
    if (argc < 2) {
	fprintf(stderr,"Usage: docutil action files\n");
	return 1;
    }
    if (strcmp(argv[1], "--pre-ipf")==0) {
	if (argc == 4) {
	    infile = fopen(argv[2], "r");
	    outfile = fopen(argv[3], "w");
	}
	if ((infile == (FILE *)NULL) || (outfile == (FILE *)NULL)) {
	    fprintf(stderr,"Usage: docutil --pre-ipf inputfile outputfile\n");
	    return 1;
	}
	code = preipf(infile, outfile);
    }
    else if (strcmp(argv[1], "--post-ipf")==0) {
	if (argc == 5) {
	    infile = fopen(argv[2], "r");
	    outfile = fopen(argv[3], "w");
	    extrafile = fopen(argv[4], "w");
	}
	if ((infile == (FILE *)NULL) || (outfile == (FILE *)NULL) || 
	    (extrafile == (FILE *)NULL)) {
	    fprintf(stderr,"Usage: docutil --post-ipf inputfile outputfile indexheaderfile\n");
	    return 1;
	}
	code = postipf(infile, outfile, extrafile);
    }
    else if (strcmp(argv[1], "--post-htmh")==0) {
	if (argc == 3) {
	    infile = fopen(argv[2], "r");
	}
	if (infile == (FILE *)NULL) {
	    fprintf(stderr,"Usage: docutil --post-htmh inputfile\n");
	    return 1;
	}
	code = posthtmh(infile);
    }
    else if (strcmp(argv[1], "--post-hhc")==0) {
	if (argc == 4) {
	    infile = fopen(argv[2], "r");
	    outfile = fopen(argv[3], "w");
	}
	if ((infile == (FILE *)NULL) || (outfile == (FILE *)NULL)) {
	    fprintf(stderr,"Usage: docutil --post-hhc inputfile outputfile\n");
	    return 1;
	}
	code = posthhc(infile, outfile);
    }
    else if (strcmp(argv[1], "--post-hhk")==0) {
	if (argc == 4) {
	    infile = fopen(argv[2], "r");
	    outfile = fopen(argv[3], "w");
	}
	if ((infile == (FILE *)NULL) || (outfile == (FILE *)NULL)) {
	    fprintf(stderr,"Usage: docutil --post-hhk inputfile outputfile\n");
	    return 1;
	}
	code = posthhk(infile, outfile);
    }
    else
	code = 1;

    if (infile)
	fclose(infile);
    if (outfile)
	fclose(outfile);
    if (extrafile)
	fclose(extrafile);
    return code;
}

