#include "foobar.h"
#include "windowmanager.h"

/*
    Use resource object in setupColors()?
    Need for redrawing filled box every time?
*/

FooBar::FooBar(Display *d, Window w, char &count, char &current) :
workspace_count(count), current_workspace(current)
{
    dpy = d;
    root = w;

    //~ setupColors();

    runfield_active = false;
    resetRunField();
}

void FooBar::redrawWorkspaces()
{
    XTextItem *text = new XTextItem();
    text->chars = (char *)malloc(1);
    text->nchars = 1;

    Point offset;
    for (char ws = 0; ws <= workspace_count; ws++) {
        offset.x = 1 + WORKSPACE_OFFSET(ws);
        offset.y = 1;

        COLOR background_color;
        COLOR border_color;
        COLOR text_color;
        
        if (ws == current_workspace) {
            background_color = COLOR_BACKGROUND_FOCUSED;
            border_color = COLOR_BORDER_FOCUSED;
            text_color = COLOR_FOREGROUND_FOCUSED;
        }
        else {
            background_color = COLOR_BACKGROUND_UNFOCUSED;
            border_color = COLOR_BORDER_UNFOCUSED;
            text_color = COLOR_FOREGROUND_UNFOCUSED;
        }

        XFillRectangle(dpy, root, resources->getGC(background_color), offset.x, offset.y, WORKSPACE_WIDTH, BAR_HEIGHT);
        XDrawRectangle(dpy, root, resources->getGC(border_color), offset.x, offset.y, WORKSPACE_WIDTH, BAR_HEIGHT);

        if (ws > 0) {
            text->chars[0] = (char)(48 + ws);
            XDrawText(dpy, root, resources->getGC(text_color), offset.x + MAGIC_NUMBER - 1, offset.y + MAGIC_NUMBER, text, 1);
        }
    }

    free(text->chars);
    delete text;
}

void FooBar::redrawRunField()
{
    COLOR background_color;
    COLOR border_color;
    COLOR text_color;
    
    if (runfield_active) {
        background_color = COLOR_BACKGROUND_FOCUSED;
        border_color = COLOR_BORDER_FOCUSED;
        text_color = COLOR_FOREGROUND_FOCUSED;
    }
    else {
        background_color = COLOR_BACKGROUND_UNFOCUSED;
        border_color = COLOR_BORDER_UNFOCUSED;
        text_color = COLOR_FOREGROUND_UNFOCUSED;
    }

    Point offset(WORKSPACES_WIDTH, 1);

    XFillRectangle(dpy, root, resources->getGC(background_color), offset.x, offset.y, RUNFIELD_WIDTH, BAR_HEIGHT);
    XDrawRectangle(dpy, root, resources->getGC(border_color), offset.x, offset.y, RUNFIELD_WIDTH, BAR_HEIGHT);

    XTextItem *text = new XTextItem();
    if (strlen(runfield) <= RUNFIELD_MAX_WIDTH) {
        text->chars = runfield;
        text->nchars = strlen(runfield);
    }
    else {
        text->chars = runfield + (strlen(runfield) - RUNFIELD_MAX_WIDTH);
        text->nchars = RUNFIELD_MAX_WIDTH;
    }

    XDrawText(dpy, root, resources->getGC(text_color), offset.x + MAGIC_NUMBER, offset.y + MAGIC_NUMBER, text, 1);

    delete text;
}

void FooBar::redraw()
{
    redrawWorkspaces();
    redrawRunField();
}

inline bool FooBar::insideWorkspaces(Point pixel)
{
    return pixel.intersects(
        Point(0, 0),
        Point(WORKSPACES_WIDTH, BAR_HEIGHT)
    );
}

inline bool FooBar::insideRunfield(Point pixel)
{
    return pixel.intersects(
        Point(WORKSPACES_WIDTH, 0),
        Point(WORKSPACES_WIDTH + RUNFIELD_WIDTH, BAR_HEIGHT)
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
    Point pixel(e->x, e->y);
    switch (e->button) {
        case Button1:
            setRunfield(insideRunfield(pixel));
            if (insideWorkspaces(pixel)) {
                wm->goToWorkspace(pixel.x / (WORKSPACE_WIDTH+ITEM_PADDING));
            }
            break;
    }
}

void FooBar::handleKeyEvent(XKeyEvent *e)
{
    KeySym ks = XKeycodeToKeysym(dpy, e->keycode, e->state & ShiftMask);
    switch (ks) {
        case NoSymbol:
        // Modifiers
        case XK_Shift_L:
        case XK_Shift_R:
        case XK_Control_L:
        case XK_Control_R:
        case XK_Caps_Lock:
        case XK_Shift_Lock:
        case XK_Meta_L:
        case XK_Meta_R:
        case XK_Alt_L:
        case XK_Alt_R:
        case XK_Super_L:
        case XK_Super_R:
        case XK_Hyper_L:
        case XK_Hyper_R:
            return;
        case XK_Return:
            if (strlen(runfield) > 0)
                wm->execute(runfield);
            //break;
        case XK_Escape:
            setRunfield(false);
            break;
        case XK_BackSpace:
            if (current_character > 0) {
                runfield[--current_character] = '\0';
            }
            break;
        default:
            if (current_character < RUNFIELD_BUFFER - 1) {
                runfield[current_character++] = ks;
            }
            break;
    }
}
