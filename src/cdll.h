/* cdll.h */

#ifndef CDLL_INCLUDED
#define CDLL_INCLUDED


#define return_error(val) return val

/* State of GS DLL */
/* State transitions are usually between adjacent states */
typedef enum {
  /* Ghostscript is unloaded, or has been loaded and has
   * not been initialised
   */
  GS_UNINIT	= 0,
  /* Ghostscript has been initialised and no input is being
   * sent to it.
   */
  GS_IDLE	= 1,
  /* Input is being sent to Ghostscript */
  GS_BUSY	= 2,
  /* Waiting at showpage */
  GS_PAGE	= 3
} GSSTATE;


#ifdef UNIX
#define GGMODULE void *
#endif
#ifdef OS2
#define GGMODULE HMODULE
#endif
#ifdef _Windows
#define GGMODULE HINSTANCE
#endif

/* main structure with info about the GS DLL */
typedef struct tagGSDLL {
	BOOL		open;		/* true if DLL loaded */
	GGMODULE	hmodule;	/* handle to module */
	GSSTATE		state;
	long		revision_number;
	gs_main_instance *minst;

	/* pointers to DLL functions */
	PFN_gsapi_revision revision;
	PFN_gsapi_new_instance new_instance;
	PFN_gsapi_delete_instance delete_instance;
	PFN_gsapi_set_stdio set_stdio;
	PFN_gsapi_set_poll set_poll;
	PFN_gsapi_set_display_callback set_display_callback;
	PFN_gsapi_init_with_args init_with_args;
	PFN_gsapi_run_string run_string;
	PFN_gsapi_run_string_begin run_string_begin;
	PFN_gsapi_run_string_continue run_string_continue;
	PFN_gsapi_run_string_end run_string_end;
	PFN_gsapi_exit exit;

} GSDLL;


/* platform dependent */
int gsdll_open(GSDLL *dll, const char *name);
int gsdll_close(GSDLL *dll);
void * gsdll_sym(GSDLL *dll, const char *name);

/* platform independent */
int gsdll_load(GSDLL *dll, const char *name);
int gsdll_free(GSDLL *dll);

#endif
