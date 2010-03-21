#ifndef _FOOBAR_H_
#define _FOOBAR_H_

#include "glass.h"

class FooBar
{
private: /* Member Variables */
    Display *dpy;
    Window root;

    Colormap colormap;
    GC active_gc;
    GC inactive_gc;
    GC text_gc;

    // Workspaces
    char &workspace_count;
    char &current_workspace;
    // Run field
    bool runfield_active;

private:
    void redrawWorkspaces();
    void redrawRunField();

    inline bool insideWorkspaces(int, int);
    inline bool insideRunfield(int, int);

public: /* Member Functions */
    FooBar(Display *, Window, char &, char &);
    void redraw();

    void setRunfield(bool active) { runfield_active = active; }
    void handleButtonEvent(XButtonEvent *);
};


#endif // _FOOBAR_H_
