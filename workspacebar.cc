#include "glass.h"

#define BOX_SIZE        25
#define COLOR_ACTIVE    "#ff0000"
#define COLOR_INACTIVE  "#555555"

WorkspaceBar::WorkspaceBar(Display *d, Window w)
{
    dpy = d;
    root = w;

    colormap = DefaultColormap(dpy, 0);

    XColor active_color;
    active_gc = XCreateGC(dpy, root, 0, 0);
    XParseColor(dpy, colormap, COLOR_ACTIVE, &active_color);
    XAllocColor(dpy, colormap, &active_color);
    XSetForeground(dpy, active_gc, active_color.pixel);

    XColor inactive_color;
    inactive_gc = XCreateGC(dpy, root, 0, 0);
    XParseColor(dpy, colormap, COLOR_INACTIVE, &inactive_color);
    XAllocColor(dpy, colormap, &inactive_color);
    XSetForeground(dpy, inactive_gc, inactive_color.pixel);
}

void WorkspaceBar::redraw()
{
    if (workspace_count && current_workspace) {
        for (char ws = 0; ws <= *workspace_count; ws++) {
            if (ws == *current_workspace)
                XDrawRectangle(dpy, root, active_gc,
                        1+((BOX_SIZE+1)*ws), 1, BOX_SIZE, BOX_SIZE);
            else
                XDrawRectangle(dpy, root, inactive_gc,
                        1+((BOX_SIZE+1)*ws), 1, BOX_SIZE, BOX_SIZE);
        }
    }
}

void WorkspaceBar::setWorkspaceCount(char *count)
{
    workspace_count = count;
}

void WorkspaceBar::setCurrentWorkspace(char *workspace)
{
    current_workspace = workspace;
}