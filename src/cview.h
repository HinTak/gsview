/* cview.h */

typedef struct tagVIEW {
    IMAGE *img;
    GSDLL *gsdll;
    GSDLL_INPUT input;
/*
    PENDING pending;
    DOC *doc;
*/
} VIEW;


void view_init(VIEW *view);
IMAGE * view_get_image(VIEW *view);
void view_wait_message(VIEW *view);
void view_wait_event(VIEW *view);
int view_poll(VIEW *view);
void view_closefile(VIEW *view);
int view_reopenfile(VIEW *view);
int view_post_message(VIEW *view, int message, int param);;
int view_page_callback(VIEW *view);
