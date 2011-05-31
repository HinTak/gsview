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

int
intro(void)
{
    if (batch)
	return 0;

    /* Introduction */
    if (dialog(IDD_INTRO, GeneralDlgProc) != DID_OK) {
	strcpy(error_message, no_error);
	return 1;
    }

    /* Copyright */
    if (dialog(IDD_COPYRIGHT, GeneralDlgProc) != DID_OK) {
	strcpy(error_message, no_error);
	return 1;
    }

    return 0; /* success */
}

/* need to modify this to support UNC */
int
getdest(void)
{
int valid;
int i;
int first_try = TRUE;
    valid = 0;
    while (!valid) {
        /* Destination directory */
	if (!batch || !first_try) {
	    strcpy(get_string_answer, bootdrive);
	    strcat(get_string_answer, INSTALL_DIR);
	    if (dialog(IDD_DIR, InputDlgProc) != DID_OK) {
		strcpy(error_message, no_error);
		return 1;
	    }
	    strcpy(destdir, get_string_answer);
	}
	first_try = FALSE;
	/* remove trailing \ if not root */
	i = strlen(destdir) - 1;
	if ((i >= 2) && (destdir[i] == '\\') )
	   destdir[i] = '\0';
	if (gs_chdir(destdir)) {
	    char buf[MAXSTR];
	    char mess[MAXSTR];
	    load_string(IDS_DIRNOTEXIST, mess, sizeof(mess)-1);
	    sprintf(buf, mess, destdir);
	    if (batch || (message_box(buf, MB_MOVEABLE | MB_YESNO | MB_ICONEXCLAMATION) == IDYES)) {
		if (mkdirall(destdir)) {
	    	    load_string(IDS_MKDIRFAIL, buf, sizeof(buf)-1);
		    message_box(buf, MB_OK);
		}
		else
		    valid = 1;
	    }
	}
	else
	    valid = 1;
	gs_chdir(workdir);
    }
    /* remove trailing \ from destination directory */
    i = strlen(destdir) - 1;
    if ( (i >= 0) && (destdir[i] == '\\') )
	destdir[i] = '\0';
    return 0;
}


int
already_installed(void)
{
char gsdir[MAXSTR];
char buf[MAXSTR];
FILE *f;
    strcpy(gsdir, destdir);
    if (strlen(gsdir) == 2)
	strcat(gsdir, "\\");	/* is root directory */
    if (strlen(gsdir) && (gsdir[strlen(gsdir)-1] != '\\'))
	    strcat(gsdir, "\\");
    strcat(gsdir, GS_BASEDIR);
    strcat(gsdir, "\\");


    /* check if Ghostscript has already been installed */
    /* first look for the DLL */
    strcpy(buf, gsdir);
    strcat(buf, GS_DLLNAME);
    if ( (f = fopen(buf, "rb")) == (FILE *)NULL ) {
	return 0;
    }
    fclose(f);

    /* next look for gs_init.ps */
    strcpy(buf, gsdir);
    strcat(buf, "gs_init.ps");
    if ( (f = fopen(buf, "rb")) == (FILE *)NULL ) {
	return 0;
    }
    fclose(f);

    /* at this stage we don't look for fonts, but maybe we should */
    
    /* yes, it is already installed */
    return 1;
}
