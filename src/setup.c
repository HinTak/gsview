/* Copyright (C) 1993-1996, Russell Lang.  All rights reserved.
  
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

/* setup.c */
/* common code for setup */
/* included into winsetup.c or os2setup.c */

/* recursive mkdir */
/* requires a full path to be specified, so ignores root \ */
/* apart from root \, must not contain trailing \ */
/* Examples:
 *  c:\          (OK, but useless)
 *  c:\gstools   (OK)
 *  c:\gstools\  (incorrect)
 *  c:gstools    (incorrect)
 *  gstools      (incorrect)
 * The following UNC names should work,
 * but didn't under Win3.1 because gs_chdir wouldn't accept UNC names
 * Needs to be tested under Windows 95.
 *  \\server\sharename\gstools    (OK)
 *  \\server\sharename\           (OK, but useless)
 */
int
mkdirall(char *dirname)
{
char newdir[MAXSTR];
char *p;
    if (strlen(dirname) < 3)
	return -1;

    if (isalpha(dirname[0]) && dirname[1]==':' && dirname[2]=='\\') {
	/* drive mapped path */
	p = dirname+3;
    }
    else if (dirname[1]=='\\' && dirname[1]=='\\') {
	/* UNC path */
	p = strchr(dirname+2, '\\');	/* skip servername */
	if (p == NULL)
	    return -1;
 	p++;
	p = strchr(p, '\\');		/* skip sharename */
	if (p == NULL)
	    return -1;
    }
    else {
	/* not full path so error */
	return -1;
    }

    while (1) {
	strncpy(newdir, dirname, (int)(p-dirname));
	newdir[(int)(p-dirname)] = '\0';
	if (gs_chdir(newdir)) {
	    if (mkdir(newdir
#ifdef __EMX__
			   , 0
#endif
                              ))
		return -1;
	}
	p++;
        if (p >= dirname + strlen(dirname))
	    break;		/* all done */
	p = strchr(p, '\\');
 	if (p == NULL)
	    p = dirname + strlen(dirname);
    }

    return gs_chdir(dirname);
}



int copyfile(char *dname, char *sname)
{
FILE *dfile, *sfile;
char *buffer;
int count;
#define COPY_BUF_SIZE 16384
    sfile = fopen(sname, "rb");
    if (sfile == (FILE *)NULL) {
	sprintf(error_message, "Can't open %s for reading", sname);
	return 1;
    }
    dfile = fopen(dname, "wb");
    if (dfile == (FILE *)NULL) {
	sprintf(error_message, "Can't open %s for writing", dname);
	fclose(sfile);
	return 1;
    }
    if ( (buffer = malloc(COPY_BUF_SIZE)) == (char *)NULL ) {
	fclose(sfile);
	fclose(dfile);
	sprintf(error_message, "Can't allocate memory for copy buffer");
	return 1;
    }

    while ( (count = fread(buffer, 1, COPY_BUF_SIZE, sfile)) != 0 )
	fwrite(buffer, 1, count, dfile);

    free(buffer);
    fclose(dfile);
    fclose(sfile);
    return 0;
}

