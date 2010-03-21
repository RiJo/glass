#include "glass.h"

#define WORKSPACE_WIDTH     30
#define WORKSPACES_WIDTH    ((WORKSPACE_WIDTH+ITEM_PADDING)*(workspace_count + 1))
#define RUNFIELD_WIDTH      (5 * WORKSPACE_WIDTH)
#define BAR_HEIGHT          20
#define BAR_WIDTH           (((WORKSPACE_WIDTH+ITEM_PADDING)*(workspace_count + 1)) + RUNFIELD_WIDTH)
#define ITEM_PADDING        3
#define COLOR_ACTIVE        "#ff0000"
#define COLOR_INACTIVE      "#555555"
#define COLOR_TEXT          "#333333"

FooBar::FooBar(Display *d, Window w, char &count, char &current) :
workspace_count(count), current_workspace(current)
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

    // Run field
    runfield_active = false;
}

void FooBar::redrawWorkspaces()
{
    XTextItem *text = new XTextItem();
    text->chars = (char *)malloc(1);
    text->nchars = 1;

    int x, y;
    for (char ws = 0; ws <= workspace_count; ws++) {
        x = 1+((WORKSPACE_WIDTH+ITEM_PADDING)*ws);
        y = 1;
        if (ws == current_workspace)
            XDrawRectangle(dpy, root, active_gc, x, y, WORKSPACE_WIDTH, BAR_HEIGHT);
        else
            XDrawRectangle(dpy, root, inactive_gc, x, y, WORKSPACE_WIDTH, BAR_HEIGHT);

        if (ws > 0) {
            text->chars[0] = (char)(48 + ws);
            XDrawText(dpy, root, text_gc, x + (WORKSPACE_WIDTH/2) - 1, y + (WORKSPACE_WIDTH/2), text, 1);
        }
    }

    delete [] text->chars;
    delete [] text;
}

void FooBar::redrawRunField()
{
    GC gc;
    if (runfield_active)
        gc = active_gc;
    else
        gc = inactive_gc;

    // temp
    int x = ((WORKSPACE_WIDTH+ITEM_PADDING)*(workspace_count + 1));
    // temp
    int y = 1;
    XDrawRectangle(dpy, root, gc, x, y, RUNFIELD_WIDTH, BAR_HEIGHT);
}

void FooBar::redraw()
{
    redrawWorkspaces();
    redrawRunField();
}

inline bool FooBar::insideWorkspaces(int x, int y)
{
    return (
        x >= 0 &&
        y >= 0 &&
        x <= WORKSPACES_WIDTH &&
        y <= BAR_HEIGHT
    );
}

inline bool FooBar::insideRunfield(int x, int y)
{
    return (
        x >= WORKSPACES_WIDTH &&
        y >= 0 &&
        x <= WORKSPACES_WIDTH + RUNFIELD_WIDTH &&
        y <= BAR_HEIGHT
    );
}

void FooBar::handleButtonEvent(XButtonEvent *e)
{
    //printf("Handle button:\n\tinsideRunfield() == %d\n\tinsideWorkspace() == %d\n", insideRunfield(e->x, e->y), insideWorkspaces(e->x, e->y));
    switch (e->button) {
        case Button1:
            runfield_active = insideRunfield(e->x, e->y);
            if (insideWorkspaces(e->x, e->y)) {
                wm->goToWorkspace(e->x / (WORKSPACE_WIDTH+ITEM_PADDING));
            }
        break;
    }
}