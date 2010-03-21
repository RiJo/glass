#ifndef _FOOBAR_H_
#define _FOOBAR_H_

#include "glass.h"

#define WORKSPACE_WIDTH     30
#define WORKSPACES_WIDTH    ((WORKSPACE_WIDTH+ITEM_PADDING)*(workspace_count + 1))
#define RUNFIELD_WIDTH      (5 * WORKSPACE_WIDTH)
#define BAR_HEIGHT          20
#define BAR_WIDTH           (((WORKSPACE_WIDTH+ITEM_PADDING)*(workspace_count + 1)) + RUNFIELD_WIDTH)
#define ITEM_PADDING        3
#define COLOR_BACKGROUND    "#050505"
#define COLOR_ACTIVE        "#ff0000"
#define COLOR_INACTIVE      "#555555"
#define COLOR_TEXT          "#333333"

#define RUNFIELD_BUFFER     255

#define MAGIC_NUMBER 15

class FooBar
{
private: /* Member Variables */
    Display *dpy;
    Window root;

    Colormap colormap;
    GC background_gc;
    GC active_gc;
    GC inactive_gc;
    GC text_gc;

    // Workspaces
    char &workspace_count;
    char &current_workspace;
    // Run field
    bool runfield_active;
    char *runfield;
    unsigned int current_character;

private:
    void setupColors();
    void redrawWorkspaces();
    void redrawRunField();

    inline bool insideWorkspaces(int, int);
    inline bool insideRunfield(int, int);

public: /* Member Functions */
    FooBar(Display *, Window, char &, char &);
    ~FooBar();

    void redraw();

    void setRunfield(bool active) { runfield_active = active; }
    bool getRunfield() { return runfield_active; }
    void resetRunField();

    void handleButtonEvent(XButtonEvent *);
    void handleKeyEvent(XKeyEvent *);
};


#endif // _FOOBAR_H_
