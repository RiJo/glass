#include "glass.h"

#define BOX_WIDTH       30
#define BOX_HEIGHT      20
#define BOX_PADDING     3
#define COLOR_ACTIVE    "#ff0000"
#define COLOR_INACTIVE  "#555555"
#define COLOR_TEXT      "#333333"

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

    XColor text_color;
    text_gc = XCreateGC(dpy, root, 0, 0);
    XParseColor(dpy, colormap, COLOR_TEXT, &text_color);
    XAllocColor(dpy, colormap, &text_color);
    XSetForeground(dpy, text_gc, text_color.pixel);
}

void WorkspaceBar::redraw()
{
    if (workspace_count && current_workspace) {

        XTextItem *text = new XTextItem();
        text->chars = (char *)malloc(1);
        text->nchars = 1;

        int x, y;
        for (char ws = 0; ws <= *workspace_count; ws++) {
            x = 1+((BOX_WIDTH+BOX_PADDING)*ws);
            y = 1;
            if (ws == *current_workspace)
                XDrawRectangle(dpy, root, active_gc, x, y, BOX_WIDTH, BOX_HEIGHT);
            else
                XDrawRectangle(dpy, root, inactive_gc, x, y, BOX_WIDTH, BOX_HEIGHT);
            
            text->chars[0] = (char)(48 + ws);
            XDrawText(dpy, root, text_gc, x + (BOX_WIDTH/2), y + (BOX_WIDTH/2), text, 1);
        }
        
        delete [] text->chars;
        delete [] text;
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