/*
   pstoedll.h : This file describes the interface to query information about
   the drivers available via pstoedit and to call pstoedit via the dll interface
  
   Copyright (C) 1998 Wolfgang Glunz, wglunz@geocities.com
*/

/* 
   the struct version of DriverDescription 
   this is needed in order to allow plain old C programs to use the .dll
*/

struct DriverDescription_S {
	char *	symbolicname;
	char *	explanation;
	char *	suffix;
	int		backendSupportsSubPathes;
	int		backendSupportsCurveto;
	int 	backendSupportsMerging; 
	int 	backendSupportsText;
	int 	backendSupportsImages;
	int		backendSupportsMultiplePages;
};

typedef int  (pstoeditwithghostscript_plainC_func) (int argc,const char * const argv[]);
typedef struct DriverDescription_S * (getPstoeditDriverInfo_plainC_func)(void);
/* returned result must be free()'ed !! */
/* the end of the array is indicated by p->symbolicname == 0 */

typedef int  (write_callback_func) (void * cb_data, const char* text, int length);
typedef void (setPstoeditOutputFunction_func)(void * cbData,write_callback_func* cbFunction);
