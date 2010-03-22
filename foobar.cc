#include "glass.h"

FooBar::FooBar(Display *d, Window w, char &count, char &current) :
workspace_count(count), current_workspace(current)
{
    dpy = d;
    root = w;

    setupColors();

    runfield_active = false;
    runfield = (char *)malloc(RUNFIELD_BUFFER);
    resetRunField();
}

FooBar::~FooBar()
{
    delete [] runfield;
}

void FooBar::setupColors() {
    XColor background_color, active_color, inactive_color, text_color, dummyc;

    background_gc = XCreateGC(dpy, root, 0, 0);
    XAllocNamedColor(dpy, DefaultColormap(dpy, 0), COLOR_BACKGROUND, &background_color, &dummyc);
    XSetForeground(dpy, background_gc, background_color.pixel);

    active_gc = XCreateGC(dpy, root, 0, 0);
    XAllocNamedColor(dpy, DefaultColormap(dpy, 0), COLOR_ACTIVE, &active_color, &dummyc);
    XSetForeground(dpy, active_gc, active_color.pixel);

    inactive_gc = XCreateGC(dpy, root, 0, 0);
    XAllocNamedColor(dpy, DefaultColormap(dpy, 0), COLOR_INACTIVE, &inactive_color, &dummyc);
    XSetForeground(dpy, inactive_gc, inactive_color.pixel);

    text_gc = XCreateGC(dpy, root, 0, 0);
    XAllocNamedColor(dpy, DefaultColormap(dpy, 0), COLOR_TEXT, &text_color, &dummyc);
    XSetForeground(dpy, text_gc, text_color.pixel);
}

void FooBar::redrawWorkspaces()
{
    XTextItem *text = new XTextItem();
    text->chars = (char *)malloc(1);
    text->nchars = 1;

    int x, y;
    for (char ws = 0; ws <= workspace_count; ws++) {
        x = 1 + WORKSPACE_OFFSET(ws);
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
    if (strlen(runfield) <= RUNFIELD_MAX_LENGTH) {
        text->chars = runfield;
        text->nchars = strlen(runfield);
    }
    else {
        text->chars = runfield + (strlen(runfield) - RUNFIELD_MAX_LENGTH);
        text->nchars = RUNFIELD_MAX_LENGTH;
    }

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
    memset(runfield, '\0', RUNFIELD_BUFFER);
}

void FooBar::setRunfield(bool active)
{
    runfield_active = active;
    if (active) {
        XGrabKeyboard(dpy, root, True, GrabModeAsync, GrabModeAsync, CurrentTime);
    }
    else {
        XUngrabKeyboard(dpy, CurrentTime);
        resetRunField();
    }
}

void FooBar::handleButtonEvent(XButtonEvent *e)
{
    switch (e->button) {
        case Button1:
            setRunfield(insideRunfield(e->x, e->y));

            if (insideWorkspaces(e->x, e->y))
                wm->goToWorkspace(e->x / (WORKSPACE_WIDTH+ITEM_PADDING));
        break;
    }
}

void FooBar::handleKeyEvent(XKeyEvent *e)
{
    KeySym ks = XKeycodeToKeysym(dpy, e->keycode, 0);
    if (ks == NoSymbol)
        return;
    //printf("handle key: %d\t keysym: %ld\tstring: %s\n", e->keycode, (long)ks, XKeysymToString(ks));
    switch (ks) {
        case XK_Return:
            if (strlen(runfield) > 0)
                forkExec(runfield);
        case XK_Escape: // fallthrough
            setRunfield(false);
        break;
        case XK_BackSpace:
            if (current_character > 0)
                runfield[--current_character] = '\0';
        break;
        default:
            if (current_character < RUNFIELD_BUFFER)
            runfield[current_character++] = *XKeysymToString(ks);
        break;
    }
}