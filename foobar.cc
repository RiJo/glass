#include "glass.h"

FooBar::FooBar(Display *d, Window w, char &count, char &current) :
workspace_count(count), current_workspace(current)
{
    dpy = d;
    root = w;

    setupColors();

    // Run field
    runfield_active = false;
    runfield = (char *)malloc(RUNFIELD_BUFFER);
    resetRunField();
}

FooBar::~FooBar()
{
    delete [] runfield;
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
        
        XFillRectangle(dpy, root, background_gc, x, y, WORKSPACE_WIDTH, BAR_HEIGHT);
        if (ws == current_workspace) {
            XDrawRectangle(dpy, root, active_gc, x, y, WORKSPACE_WIDTH, BAR_HEIGHT);
        }
        else {
            XDrawRectangle(dpy, root, inactive_gc, x, y, WORKSPACE_WIDTH, BAR_HEIGHT);
        }

        if (ws > 0) {
            text->chars[0] = (char)(48 + ws);
            XDrawText(dpy, root, text_gc, x + MAGIC_NUMBER - 1, y + MAGIC_NUMBER, text, 1);
        }
    }

    delete [] text->chars;
    delete [] text;
}

void FooBar::setupColors() {
    colormap = DefaultColormap(dpy, 0);

    XColor background_color;
    background_gc = XCreateGC(dpy, root, 0, 0);
    XParseColor(dpy, colormap, COLOR_BACKGROUND, &background_color);
    XAllocColor(dpy, colormap, &background_color);
    XSetForeground(dpy, background_gc, background_color.pixel);

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

void FooBar::redrawRunField()
{
    GC gc;
    if (runfield_active)
        gc = active_gc;
    else
        gc = inactive_gc;

    int x = WORKSPACES_WIDTH;
    int y = 1;
    
    XFillRectangle(dpy, root, background_gc, x, y, RUNFIELD_WIDTH, BAR_HEIGHT);
    XDrawRectangle(dpy, root, gc, x, y, RUNFIELD_WIDTH, BAR_HEIGHT);

    XTextItem *text = new XTextItem();
    text->chars = runfield;
    text->nchars = strlen(runfield);

    XDrawText(dpy, root, text_gc, x + MAGIC_NUMBER, y + MAGIC_NUMBER, text, 1);

    delete [] text;
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

void FooBar::resetRunField()
{
    current_character = 0;
    for (int i = 0; i < RUNFIELD_BUFFER; i++) {
        runfield[i] = '\0';
    }
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

void FooBar::handleKeyEvent(XKeyEvent *e)
{
    KeySym ks=XKeycodeToKeysym(dpy, e->keycode, 0);
    if (ks == NoSymbol)
        return;
    //printf("handle key: %d\t keysym: %ld\tstring: %s\n", e->keycode, (long)ks, XKeysymToString(ks));
    switch (ks) {
        case XK_Return:
            forkExec(runfield);
            resetRunField();
        break;
        default:
            runfield[current_character++] = *XKeysymToString(ks);
        break;
    }
}