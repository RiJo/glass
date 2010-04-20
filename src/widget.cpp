#include "widget.h"
#include "windowmanager.h"

Widget::Widget(Display *display, Window parent, Point position, Point size)
        : position(position), size(size) {
    dpy = display;

    unsigned long background = wm->getResources()->getColor(COLOR_BORDER_FOCUSED).pixel;

    window = XCreateSimpleWindow(dpy, parent, position.x, position.y, size.x, size.y, 0, 0, background);

    XSelectInput(dpy, window, FocusChangeMask);
    XMapWindow(dpy, window);
    //XFlush(dpy);

    //~ XEvent e;
    //~ char *msg = (char *)"Hello, World!";
    //~ int s = DefaultScreen(dpy);

    //~ if (fork() == 0) {
        //~ while (1) {
            //~ XNextEvent(dpy, &e);
            //~ if (e.type == Expose) {
                //~ XFillRectangle(dpy, window, DefaultGC(dpy, s), 20, 20, 10, 10);
                //~ XDrawString(dpy, window, DefaultGC(dpy, s), 50, 50, msg, strlen(msg));
            //~ }
        //~ }
    //~ }

}