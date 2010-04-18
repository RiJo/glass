/*******************************************************************************
 * Class name: Widget
 *
 * Description:
 *   Superclass for all widgets integrated in the windowmanager
 ******************************************************************************/

#ifndef _WIDGET_H_
#define _WIDGET_H_

#include "glass.h"

class Widget
{
private: /* Member Variables */
    Display *dpy;
    Window window;

    Point position;
    Point size;

private: /* Member Functions */


public: /* Member Functions */
    Widget(Display *, Window, Point, Point);
    Point getPosition() { return position; }
    Point getSize() { return size; }
};


#endif // _WIDGET_H_
