/* cimg.c */

#include "gvc.h"

/***********************************************************/

/* common image functions */

/* Return a palette entry for given format and index */
void
image_color(unsigned int format, int index, 
    unsigned char *r, unsigned char *g, unsigned char *b)
{
    switch (format & DISPLAY_COLORS_MASK) {
	case DISPLAY_COLORS_NATIVE:
	    switch (format & DISPLAY_DEPTH_MASK) {
		case DISPLAY_DEPTH_1:
		    *r = *g = *b = (index ? 0 : 255);
		    break;
		case DISPLAY_DEPTH_4:
		    {
		    int one = index & 8 ? 255 : 128;
		    *r = (index & 4 ? one : 0);
		    *g = (index & 2 ? one : 0);
		    *b = (index & 1 ? one : 0);
		    }
		    break;
		case DISPLAY_DEPTH_8:
		    /* palette of 96 colors */
		    /* 0->63 = 00RRGGBB, 64->95 = 010YYYYY */
		    if (index < 64) {
			int one = 255 / 3;
			*r = ((index & 0x30) >> 4) * one;
			*g = ((index & 0x0c) >> 2) * one;
			*b =  (index & 0x03) * one;
		    }
		    else {
			int val = index & 0x1f;
			*r = *g = *b = (val << 3) + (val >> 2);
		    }
		    break;
	    }
	    break;
	case DISPLAY_COLORS_GRAY:
	    switch (format & DISPLAY_DEPTH_MASK) {
		case DISPLAY_DEPTH_1:
		    *r = *g = *b = (index ? 255 : 0);
		    break;
		case DISPLAY_DEPTH_4:
		    *r = *g = *b = (unsigned char)((index<<4) + index);
		    break;
		case DISPLAY_DEPTH_8:
		    *r = *g = *b = (unsigned char)index;
		    break;
	    }
	    break;
    }
}

/* convert one line of 16BGR555 to 24BGR */
/* byte0=GGGBBBBB byte1=0RRRRRGG */
void
image_16BGR555_to_24BGR(int width, unsigned char *dest, unsigned char *source)
{
    int i;
    WORD w;
    unsigned char value;
    for (i=0; i<width; i++) {
	w = source[0] + (source[1] << 8);
	value = w & 0x1f;		/* blue */
	*dest++ = (value << 3) + (value >> 2);
	value = (w >> 5) & 0x1f;	/* green */
	*dest++ = (value << 3) + (value >> 2);
	value = (w >> 10) & 0x1f;	/* red */
	*dest++ = (value << 3) + (value >> 2);
	source += 2;
    }
}

/* convert one line of 16BGR565 to 24BGR */
/* byte0=GGGBBBBB byte1=RRRRRGGG */
void
image_16BGR565_to_24BGR(int width, unsigned char *dest, unsigned char *source)
{
    int i;
    WORD w;
    unsigned char value;
    for (i=0; i<width; i++) {
	w = source[0] + (source[1] << 8);
	value = w & 0x1f;		/* blue */
	*dest++ = (value << 3) + (value >> 2);
	value = (w >> 5) & 0x3f;	/* green */
	*dest++ = (value << 2) + (value >> 4);
	value = (w >> 11) & 0x1f;	/* red */
	*dest++ = (value << 3) + (value >> 2);
	source += 2;
    }
}

/* convert one line of 16RGB555 to 24BGR */
/* byte0=0RRRRRGG byte1=GGGBBBBB */
void
image_16RGB555_to_24BGR(int width, unsigned char *dest, unsigned char *source)
{
    int i;
    WORD w;
    unsigned char value;
    for (i=0; i<width; i++) {
	w = (source[0] << 8) + source[1];
	value = w & 0x1f;		/* blue */
	*dest++ = (value << 3) + (value >> 2);
	value = (w >> 5) & 0x1f;	/* green */
	*dest++ = (value << 3) + (value >> 2);
	value = (w >> 10) & 0x1f;	/* red */
	*dest++ = (value << 3) + (value >> 2);
	source += 2;
    }
}

/* convert one line of 16RGB565 to 24BGR */
/* byte0=RRRRRGGG byte1=GGGBBBBB */
void
image_16RGB565_to_24BGR(int width, unsigned char *dest, unsigned char *source)
{
    int i;
    WORD w;
    unsigned char value;
    for (i=0; i<width; i++) {
	w = (source[0] << 8) + source[1];
	value = w & 0x1f;		/* blue */
	*dest++ = (value << 3) + (value >> 2);
	value = (w >> 5) & 0x3f;	/* green */
	*dest++ = (value << 2) + (value >> 4);
	value = (w >> 11) & 0x1f;	/* red */
	*dest++ = (value << 3) + (value >> 2);
	source += 2;
    }
}


/* convert one line of 32CMYK to 24BGR */
void
image_32CMYK_to_24BGR(int width, unsigned char *dest, unsigned char *source,
    int sep)
{
    int i;
    int cyan, magenta, yellow, black;
    for (i=0; i<width; i++) {
	cyan = source[0];
	magenta = source[1];
	yellow = source[2];
	black = source[3];
	if (!(sep & SEP_CYAN))
	    cyan = 0;
	if (!(sep & SEP_MAGENTA))
	    magenta = 0;
	if (!(sep & SEP_YELLOW))
	    yellow = 0;
	if (!(sep & SEP_BLACK))
	    black = 0;
	*dest++ = (255 - yellow)  * (255 - black)/255; /* blue */
	*dest++ = (255 - magenta) * (255 - black)/255; /* green */
	*dest++ = (255 - cyan)    * (255 - black)/255; /* red */
	source += 4;
    }
}

void
image_to_24BGR(IMAGE *img, unsigned char *dest, unsigned char *source)
{
    unsigned char *d = dest;
    unsigned char *s = source;
    int width = img->width;
    unsigned int alpha = img->format & DISPLAY_ALPHA_MASK;
    BOOL bigendian = (img->format & DISPLAY_ENDIAN_MASK) == DISPLAY_BIGENDIAN;
    int i;

    switch (img->format & DISPLAY_COLORS_MASK) {
	case DISPLAY_COLORS_NATIVE:
	    if ((img->format & DISPLAY_DEPTH_MASK) == DISPLAY_DEPTH_16) {
		if (bigendian) {
		    if ((img->format & DISPLAY_555_MASK)
			== DISPLAY_NATIVE_555)
			image_16RGB555_to_24BGR(img->width, dest, source);
		    else
			image_16RGB565_to_24BGR(img->width, dest, source);
		}
		else {
		    if ((img->format & DISPLAY_555_MASK)
			== DISPLAY_NATIVE_555) {
			image_16BGR555_to_24BGR(img->width, dest, source);
		    }
		    else
			image_16BGR565_to_24BGR(img->width, dest, source);
		}
	    }
	    break;
	case DISPLAY_COLORS_RGB:
	    if ((img->format & DISPLAY_DEPTH_MASK) != DISPLAY_DEPTH_8)
		return;
	    for (i=0; i<width; i++) {
		if ((alpha == DISPLAY_ALPHA_FIRST) || 
		    (alpha == DISPLAY_UNUSED_FIRST))
		    s++;
		if (bigendian) {
		    *d++ = s[2];
		    *d++ = s[1];
		    *d++ = s[0];
		    s+=3;
		}
		else {
		    *d++ = *s++;
		    *d++ = *s++;
		    *d++ = *s++;
		}
		if ((alpha == DISPLAY_ALPHA_LAST) || 
		    (alpha == DISPLAY_UNUSED_LAST))
		    s++;
	    }
	    break;
	case DISPLAY_COLORS_CMYK:
	    if ((img->format & DISPLAY_DEPTH_MASK) != DISPLAY_DEPTH_8)
		return;
	    image_32CMYK_to_24BGR(width, dest, source, img->separation);
	    break;
    }
}


/***********************************************************/
/* display device callback functions */

/* handle is a pointer to a view */

/* 
#define DISPLAY_DEBUG
 */


/* New device has been opened */
/* This is the first event from this device. */
static int display_open(void *handle, void *device)
{
    VIEW *view = (VIEW *)handle;
    IMAGE *img = view_get_image(view);
#ifdef DISPLAY_DEBUG
    gs_addmessf("display_open(0x%x, 0x%x)\n", handle, device);
#endif
    
    if (img == NULL)
	return_error(DISPLAY_ERROR);

    if (img->open || img->opening)
	return_error(DISPLAY_ERROR);

    image_lock(img);
    img->opening = TRUE;
    img->handle = handle;
    img->device = device;

    img->width = img->height = img->raster = 0;
    img->format = 0;
    img->image = NULL;

    return 0;
}

/* Device is about to be closed. */
/* Device will not be closed until this function returns. */
static int display_preclose(void *handle, void *device)
{
    VIEW *view = (VIEW *)handle;
    IMAGE *img = view_get_image(view);
#ifdef DISPLAY_DEBUG
    gs_addmessf("display_preclose(0x%x, 0x%x)\n", handle, device);
#endif
    image_lock(img);
    img->open = FALSE;
    img->opening = FALSE;
    image_preclose(img);
    return 0;
}

/* Device has been closed. */
/* This is the last event from this device. */
static int display_close(void *handle, void *device)
{
    VIEW *view = (VIEW *)handle;
    IMAGE *img = view_get_image(view);
#ifdef DISPLAY_DEBUG
    gs_addmessf("display_close(0x%x, 0x%x)\n", handle, device);
#endif
    img->open = FALSE;
    img->opening = FALSE;

    img->handle = handle;
    img->device = device;

    img->width = img->height = img->raster = 0;
    img->format = 0;
    img->image = NULL;

    image_unlock(img);

    view_post_message(view, WM_GSDEVICE, 0);
    return 0;
}

/* Device is about to be resized. */
/* Resize will only occur if this function returns 0. */
static int display_presize(void *handle, void *device, int width, int height, 
	int raster, unsigned int format)
{
    VIEW *view = (VIEW *)handle;
    IMAGE *img = view_get_image(view);
#ifdef DISPLAY_DEBUG
    gs_addmessf("display_presize(0x%x, 0x%x, width=%d height=%d raster=%d\
 format=%d)\n", 
       handle, device, width, height, raster, format);
#endif
    if (!img->opening)
	image_lock(img);

    /* check if we understand format */
    if (image_presize(img, width, height, raster, format))
	return_error(DISPLAY_ERROR);

    return 0;
}
   
/* Device has been resized. */
/* New pointer to raster returned in pimage */
static int display_size(void *handle, void *device, int width, int height, 
	int raster, unsigned int format, unsigned char *pimage)
{
    VIEW *view = (VIEW *)handle;
    IMAGE *img = view_get_image(view);
#ifdef DISPLAY_DEBUG
    gs_addmessf("display_size(0x%x, 0x%x, width=%d height=%d raster=%d\
 format=%d image=0x%x)\n", 
       handle, device, width, height, raster, format, pimage);
#endif
    img->width = width;
    img->height = height;
    img->raster = raster;
    img->format = format;
    img->image = pimage;
    if (image_size(img))
	img->open = FALSE;
    else {
	if (img->opening)
	    img->open = TRUE;
	img->opening = FALSE;
    }

    image_unlock(img);
    view_post_message(view, WM_GSSIZE, 0);
    return 0;
}
   
/* flushpage */
static int display_sync(void *handle, void *device)
{
    VIEW *view = (VIEW *)handle;
    IMAGE *img = view_get_image(view);
#ifdef DISPLAY_DEBUG
    gs_addmessf("display_sync(0x%x, 0x%x)\n", handle, device);
#endif
    if (img->ignore_sync) {
	/* ignore this sync, but not the next */
	img->ignore_sync = FALSE;
	return 0;
    }
    image_sync(img);
    view_post_message(view, WM_GSSYNC, 0);
    return 0;
}

/* showpage */
/* If you want to pause on showpage, then don't return immediately */
static int display_page(void *handle, void *device, int copies, int flush)
{
    VIEW *view = (VIEW *)handle;
    IMAGE *img = view_get_image(view);

#ifdef DISPLAY_DEBUG
    gs_addmessf("display_page(0x%x, 0x%x, copies=%d flush=%d)\n", 
	handle, device, copies, flush);
#endif
    img->ignore_sync = FALSE;
    image_sync(img);
    view_page_callback(view);

    return 0;
}

/* Poll the caller for cooperative multitasking. */
/* If this function is NULL, polling is not needed */
static int display_update(void *handle, void *device, 
    int x, int y, int w, int h)
{
    return view_poll((VIEW *)handle);
}

display_callback gsdisplay = { 
    sizeof(display_callback),
    DISPLAY_VERSION_MAJOR,
    DISPLAY_VERSION_MINOR,
    display_open,
    display_preclose,
    display_close,
    display_presize,
    display_size,
    display_sync,
    display_page,
    display_update,
#ifdef OS2
    display_memalloc,
    display_memfree
#else
    NULL,	/* memalloc */
    NULL	/* memfree */
#endif
};


