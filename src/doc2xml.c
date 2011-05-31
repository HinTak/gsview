/*
 * doc2xml.c  -- program to convert Gnuplot .DOC format to 
 *        eXtensible Markup Language
 *
 * Created by Russell Lang from doc2html
 *
 * usage:  doc2xml gsviewen.txt gsviewen.xml
 *
 */


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#define MAX_LINE_LEN	1024
#define TRUE 1
#define FALSE 0

struct LIST
{
	int level;
	int line;
	char *string;
	struct LIST *next;
	};

struct LIST *list = NULL;
struct LIST *head = NULL;

struct LIST *keylist = NULL;
struct LIST *keyhead = NULL;

int debug = FALSE;
int nolinks = FALSE;

void parse(FILE *a);
int lookup(char *s);
char *title_from_index(int id);
void refs(int l, FILE *f);
void convert(FILE *a,FILE *b);
void process_line(char *line, FILE *b);

int
main(int argc,char *argv[])
{
FILE * infile;
FILE * outfile;

    if (argv[argc-1][0]=='-' && argv[argc-1][1]=='d') {
        debug = TRUE;
	argc--;
    }

    if (argv[argc-1][0]=='-' && argv[argc-1][1]=='n') {
        nolinks = TRUE;
	argc--;
    }

    if ( (argc > 3) || (argc == 1) ) {
        fprintf(stderr,"Usage: %s infile outfile\n", argv[0]);
        return(1);
    }
    if ( (infile = fopen(argv[1],"r")) == (FILE *)NULL) {
        fprintf(stderr,"%s: Can't open %s for reading\n",
            argv[0], argv[1]);
        return(1);
    }
    if (argc == 3) {
      if ( (outfile = fopen(argv[2],"w")) == (FILE *)NULL) {
        fprintf(stderr,"%s: Can't open %s for writing\n",
            argv[0], argv[2]);
      }
    }
    else {
        outfile = stdout;
    }
    parse(infile);
    convert(infile,outfile);
    return(0);
}

/* scan the file and build a list of line numbers where particular levels are */
void parse(FILE *a)
{
    static char line[MAX_LINE_LEN];
    char *c;
    int lineno=0;
    int lastline=0;

    /* skip title line */
    fgets(line,MAX_LINE_LEN,a);

    while (fgets(line,MAX_LINE_LEN,a)) 
    {
      lineno++;
      if (isdigit(line[0]))
      {
        if (list == NULL)    
            head = (list = (struct LIST *) malloc(sizeof(struct LIST)));
        else
            list = (list->next = (struct LIST *) malloc(sizeof(struct LIST)));
        list->line = lastline = lineno;
        list->level = line[0] - '0';
        list->string = (char *) malloc (strlen(line)+1);
        c = strtok(&(line[1]),"\n");
        strcpy(list->string, c);
        list->next = NULL;
      }
      if (line[0]=='?')
      {
        if (keylist == NULL)    
            keyhead = (keylist = (struct LIST *) malloc(sizeof(struct LIST)));
        else
            keylist = (keylist->next = (struct LIST *) malloc(sizeof(struct LIST)));
        keylist->line = lastline;
        keylist->level = line[0] - '0';
        c = strtok(&(line[1]),"\n");
        if( c == NULL || *c == '\0' ) c = list->string ;
        keylist->string = (char *) malloc (strlen(c)+1);
        strcpy(keylist->string, c);
        keylist->next = NULL;
      }
    }
    rewind(a);
    }

/* look up an in text reference */
int
lookup(char *s)
{
	char *c;
	char tokstr[MAX_LINE_LEN];
	char *match; 
	int l;

	strcpy(tokstr, s);

	/* first try the ? keyword entries */
	keylist = keyhead;
	while (keylist != NULL)
	{
		c = keylist->string;
		while (isspace(*c)) c++;
		if (!strcmp(s, c)) return(keylist->line);
		keylist = keylist->next;
		}

	/* then try titles */
#ifdef GNUPLOT
	match = strtok(tokstr, " \n\t");
	l = 0; /* level */
	
	list = head;
	while (list != NULL)
	{
		c = list->string;
		while (isspace(*c)) c++;
		if (!strcmp(match, c)) 
		{
			l = list->level;
			match = strtok(NULL, "\n\t ");
			if (match == NULL)
			{
				return(list->line);
				}
			}
		if (l > list->level)
			break;
		list = list->next;
		}
#else
    /* we list keys explicitly, rather than building them from multiple levels  */
    list = head;
    while (list != NULL)
    {
        c = list->string;
        while (isspace(*c)) c++;
        if (!strcmp(s, c)) return(list->line);
        list = list->next;
        }
#endif
	return(-1);
}

char *title_from_index(int id)
{
struct LIST *l = NULL;
    if (id < 0)
	return "";
    l = head;
    while (l != NULL) {
	if (id == l->line)
	    return l->string;
        l = l->next;
    }
    return NULL;
}


#ifdef NOT_USED
/* search through the list to find any references */
void
refs(int l, FILE *f)
{
    int curlevel;
    char str[MAX_LINE_LEN];
    char *c;
    int inlist = FALSE;

    /* find current line */
    list = head;
    while (list->line != l)
        list = list->next;
    curlevel = list->level;
    list = list->next;        /* look at next element before going on */
    if (list != NULL)
    {
	inlist = TRUE;
	if (inbody)
	fprintf(f,"</body>\n");
	inbody = 0;
	}

    while (list != NULL) {
        /* we are onto the next topic so stop */
        if (list->level == curlevel)
            break;
        /* these are the next topics down the list */
        if (list->level == curlevel+1) {
            c = list->string;
	    while (isspace(*c)) c++;
	    if (nolinks)
	        fprintf(f,"<b>%s</b><br></br>\n", c);
	    else {
		char *p = c;
	        fprintf(f,"<link target=\042");
		while (*p) {
		    if (*p == ' ')
			fputc('_', f);
		    else 
			fputc(*p, f);
		    p++;
		}
	        fprintf(f,"\042>%s</link>\n", c);
	    }
	}
        list = list->next;
    }
    if (inlist)
	fprintf(f,"<P>\n");
}
#endif

void
convert(FILE *a,FILE *b)
{
    static char line[MAX_LINE_LEN];
    
    /* generate html header */
    fprintf(b,"<?xml version=\0421.0\042?>\n");
    fprintf(b,"<help>\n");
    fgets(line,MAX_LINE_LEN,a);
    strtok(line, "\n");
    fprintf(b,"<helptitle>%s</helptitle>\n", line+1);

    /* process each line of the file */
    while (fgets(line,MAX_LINE_LEN,a)) {
       process_line(line, b);
    }

    /* close final page and generate trailer */
    strcpy(line, "0\n");
    process_line(line, b);

    fprintf(b,"\n</help>");
}

#define PLATFORM_NONE 0
#define PLATFORM_WIN 1
#define PLATFORM_OS2 2
#define PLATFORM_WINOS2 3
#define PLATFORM_X11 4

void
end_platform(int platform, FILE *b)
{
    if (platform != PLATFORM_NONE) {
	switch (platform) {
	    case PLATFORM_WIN:
	        fputs("</windows>\n", b);
		break;
	    case PLATFORM_OS2:
	        fputs("</os2>\n", b);
		break;
	    case PLATFORM_WINOS2:
	        fputs("</winos2>\n", b);
		break;
	    case PLATFORM_X11:
	        fputs("</x11>\n", b);
		break;
	}
    }
}

void
start_platform(int platform, FILE *b)
{
    switch (platform) {
	case PLATFORM_WIN:
	    fputs("<windows>\n", b);
	    break;
	case PLATFORM_OS2:
	    fputs("<os2>\n", b);
	    break;
	case PLATFORM_WINOS2:
	    fputs("<winos2>\n", b);
	    break;
	case PLATFORM_X11:
	    fputs("<x11>\n", b);
	    break;
    }
}

void
process_line(char *line, FILE *b)
{
    static int line_count = 0;
    static char line2[MAX_LINE_LEN];
    static int last_line;
    char hyplink1[64] ;
    char *pt, *tablerow ;
    int i;
    int j;
    static int startpage = 1;
    char str[MAX_LINE_LEN];
    char topic[MAX_LINE_LEN];
    int k, l;
    static int tabl=0;
    static int para=0;
    static int inquote = FALSE;
    static int inref = FALSE;
    static int prev_level = 0;
    int new_level = 0;
    static int inbody = FALSE;
    static int platform = PLATFORM_NONE;
    static int inplatform = FALSE;

    line_count++;

    i = 0;
    j = 0;
    while (line[i] != '\0')
    {
        switch(line[i])
        {
            case '<':
                strcpy( &line2[j], "&lt;" ) ;
                j += strlen( "&lt." ) - 1 ;
                break ;

            case '>':
                strcpy( &line2[j], "&gt;" ) ;
                j += strlen( "&gt." ) - 1 ;
                break ;

            case '&':
                strcpy( &line2[j], "&amp;" ) ;
                j += strlen( "&amp;" ) - 1 ;
                break ;
                                
            case '\r':
            case '\n':
                break;
            case '`':    /* backquotes mean boldface or link */
                if ((!inref) && (!inquote))
                {
                    k=i+1;    /* index into current string */
                    l=0;    /* index into topic string */
                    while ((line[k] != '`') && (line[k] != '\0'))
                    {
                        topic[l] = line[k];
                        k++;
                        l++;
                        }
                    topic[l] = '\0';
                    k = lookup(topic);
		    if ((k > 0) && (k != last_line))
                    {
			if (nolinks)
                            sprintf( hyplink1, "<b>") ;
			else {
			    char *p = title_from_index(k);
			    char *t;
                            sprintf( hyplink1, "<link href=\042");
			    t = hyplink1 + strlen(hyplink1);
			
			    while (p && *p) {
				if (*p == ' ')
				    *t++ = '_';
				else 
				    *t++ = *p;
				p++;
			    }
			    *t = '\0';
			    strcat(hyplink1, "\042>") ;
			}
                        strcpy( line2+j, hyplink1 ) ;
                        j += strlen( hyplink1 )-1 ;
                        
                        inref = k;
                        }
                    else
                    {
                        if (debug && k != last_line)
                            fprintf(stderr,"Can't make link for \042%s\042 on line %d\n",topic,line_count);
                        line2[j++] = '<';
                        line2[j++] = 'b';
                        line2[j] = '>';
                        inquote = TRUE;
                        }
                    }
                else
                {
                    if (inquote && inref)
                        fprintf(stderr, "Warning: Reference Quote conflict line %d\n", line_count);
                    if (inquote)
                    {
                        line2[j++] = '<';
                        line2[j++] = '/';
                        line2[j++] = 'b';
                        line2[j] = '>';
                        inquote = FALSE;
                        }
                    if (inref)
                    {
                        /* must be inref */
                        line2[j++] = '<';
                        line2[j++] = '/';
			if (nolinks)
                            line2[j++] = 'b';
			else {
                            line2[j++] = 'l';
                            line2[j++] = 'i';
                            line2[j++] = 'n';
                            line2[j++] = 'k';
			}
                        line2[j] = '>';
                        inref = 0;
                        }
                }
                break;
            default:
		if ((unsigned int)(line[i]) > 127) {
		    /* quote non-ASCII characters */
		    unsigned int value = line[i] & 0xff;
		    unsigned int digit;
		    line2[j++] = '&';
		    line2[j++] = '#';
		    digit = value / 100;
		    value -= digit * 100;
		    line2[j++] = digit + '0';
		    digit = value / 10;
		    value -= digit * 10;
		    line2[j++] = digit + '0';
		    line2[j++] = value + '0';
		    line2[j] = ';';
		}
		else
                    line2[j] = line[i];
            }
        i++;
        j++;
        line2[j] = '\0';
        }

    i = 1;

    switch(line[0]) {        /* control character */
       case '?': {            /* interactive help entry */
#ifdef FIXLATER
               if( intable ) intablebut = TRUE ;
               fprintf(b,"\n:i1. %s", &(line[1])); /* index entry */
#endif
	       if ((line2[1] != '\0') && (line2[1] != ' '))
		    fprintf(b, "<key>%s</key>\n", &(line2[1]));
               break;
       }
       case '@': {            /* start/end table */
          break;            /* ignore */
       }
       case '#': {            /* latex table entry */
          break;            /* ignore */
       }
       case '%': {            /* troff table entry */
          break;            /* ignore */
       }
       case 'W':
	    if (tabl)
	        fprintf(b,"</fixed>\n"); /* rjl */
	    tabl = FALSE;
	    if (para)
		fprintf(b,"</p>\n");
	    end_platform(platform, b);
	    if (line[1] == 'P')
		platform = PLATFORM_WINOS2;
	    else
		platform = PLATFORM_WIN;
	    if (inbody) {
		start_platform(platform, b);
		inplatform = TRUE;
	    }
	    if (para)
		fprintf(b,"<p>\n");
	    break;
       case 'P':
	    if (tabl)
	        fprintf(b,"</fixed>\n"); /* rjl */
	    tabl = FALSE;
	    if (para)
		fprintf(b,"</p>\n");
	    end_platform(platform, b);
	    platform = PLATFORM_OS2;
	    if (inbody) {
		start_platform(platform, b);
		inplatform = TRUE;
	    }
	    if (para)
		fprintf(b,"<p>\n");
	    break;
       case 'X':
	    if (tabl)
	        fprintf(b,"</fixed>\n"); /* rjl */
	    tabl = FALSE;
	    if (para)
		fprintf(b,"</p>\n");
	    end_platform(platform, b);
	    platform = PLATFORM_X11;
	    if (inbody) {
		start_platform(platform, b);
		inplatform = TRUE;
	    }
	    if (para)
		fprintf(b,"<p>\n");
	    break;
       case 'E':
	    if (tabl)
	        fprintf(b,"</fixed>\n"); /* rjl */
	    tabl = FALSE;
	    if (para)
		fprintf(b,"</p>\n");
	    end_platform(platform, b);
	    platform = PLATFORM_NONE;
	    if (para)
		fprintf(b,"<p>\n");
	    break;
       case '\n':            /* empty text line */
	    if (tabl)
	        fprintf(b,"</fixed>\n"); /* rjl */
	    if (para)
	        fprintf(b,"</p>\n"); /* rjl */
            para = 0;
            tabl = 0;
            break;
       case ' ': {            /* normal text line */
	  if (!inbody)
	    fprintf(b, "<body>\n");
	  inbody = 1;
	  if (!inplatform)
	      start_platform(platform, b);
	  inplatform = TRUE;
          if ((line2[1] == '\0') || (line2[1] == '\n'))
          {
		if (para)
		    fprintf(b,"</p>\n"); /* rjl */
                para = 0;
		tabl = 0;
                }
          if (line2[1] == ' ') 
          {
		if (para)
		    fprintf(b,"</p>\n"); /* rjl */
                para = 0;
                if (!tabl)
                {
		    fprintf(b,"<fixed>\n");
                    }
                fprintf(b,"%s\n",&line2[1]); 
                tabl = 1;
                }
          else if (strncmp(line2+1, "{bml", 4)==0)
	  {
		/* assume bitmap is available as GIF */
		char *p;

		fprintf(b, "\n<bmp>");
		for (p=line2+1; *p && *p!=' '; p++)
		   ; /* skip over bml text */
		for (; *p && *p==' '; p++)
		   ; /* skip over spaces */
		for (; *p && *p!='.' && *p!=' '; p++)
		    fprintf(b, "%c", *p);
		fprintf(b, ".bmp</bmp>\n");

		fprintf(b, "\n<gif>");
		for (p=line2+1; *p && *p!=' '; p++)
		   ; /* skip over bml text */
		for (; *p && *p==' '; p++)
		   ; /* skip over spaces */
		for (; *p && *p!='.' && *p!=' '; p++)
		    fprintf(b, "%c", *p);
		fprintf(b, ".gif</gif>>\n");
	      }
	  else 
          {
		if (tabl) {
		    fprintf(b,"</fixed>\n"); /* rjl */
		}
                tabl = 0;
                if (!para) {
		    fprintf(b,"<p>"); /* rjl */
                    para = 1;        /* not in para so start one */
		}
		else 
		    fputc(' ', b);
                  fprintf(b,"%s\n",&line2[1]); 
                }
          break;
       }
       default: {
          if (isdigit(line[0])) { /* start of section */
	    if (tabl)
	          fprintf(b,"</fixed>\n"); /* rjl */
	    if (para)
		fprintf(b, "</p>\n");
	    para = 0;
	    if (platform)
		end_platform(platform, b);
	    inplatform = FALSE;
	    if (inbody)
		fprintf(b, "</body>\n");
	    inbody = 0;
	    new_level = line[0]-'0';
#ifdef NOT_USED
            if (!startpage)
            {
                refs(last_line,b);
                }
#endif
            para = 0;                    /* not in a paragraph */
            tabl = 0;
            last_line = line_count;
            startpage = 0;
	    if (debug)
		fprintf( stderr, "%d: %s\n", line_count, &line2[1] ) ;
            k=lookup(&line2[1]) ;
	 
	    while ((new_level <= prev_level) && prev_level) {
                fprintf(b,"</topic>\n");
		prev_level--;
	    }
	    prev_level = new_level;

	    if (new_level > 0) {
		/* output unique ID and section title */
		if (nolinks)
		    fprintf(b,"<topic level=\042%d\042>\n", new_level);
		else {
		    fprintf(b,"<topic level=\042%d\042 id=\042", new_level);
		    char *p = &(line2[1]);
		    while (*p) {
			if (*p == ' ')
			    fputc('_', b);
			else 
			    fputc(*p, b);
			p++;
		    }
		    fprintf(b,"\042>\n");
		}
		fprintf(b,"<browse>%05d0</browse>\n", line_count);
		fprintf(b, "<name>%s</name>\n", &(line2[1])); /* title */
	    }
          } else
            fprintf(stderr, "unknown control code '%c' in column 1, line %d\n",
                line[0], line_count);
          break;
       }
    }
}

