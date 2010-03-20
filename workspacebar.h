#ifndef _WORKSPACEBAR_H_
#define _WORKSPACEBAR_H_

#include "glass.h"

class WorkspaceBar
{
private: /* Member Variables */
    Display *dpy;
    Window root;

    Colormap colormap;
    GC active_gc;
    GC inactive_gc;
    GC text_gc;

    char *workspace_count;
    char *current_workspace;

public: /* Member Functions */
    WorkspaceBar(Display *, Window);
    void redraw();

    void setWorkspaceCount(char *);
    void setCurrentWorkspace(char *);

};


#endif // _WORKSPACEBAR_H_
