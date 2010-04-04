#ifndef _FOOBAR_H_
#define _FOOBAR_H_

#include "glass.h"

#define WORKSPACE_WIDTH     30
#define WORKSPACE_OFFSET(x) ((WORKSPACE_WIDTH + ITEM_PADDING) * x)
#define WORKSPACES_WIDTH    WORKSPACE_OFFSET( (workspace_count + 1) )
#define RUNFIELD_WIDTH      (5 * WORKSPACE_WIDTH)
#define BAR_HEIGHT          20
#define BAR_WIDTH           (WORKSPACES_WIDTH + RUNFIELD_WIDTH)
#define ITEM_PADDING        3

#define RUNFIELD_BUFFER     255
#define RUNFIELD_MAX_LENGTH 20

#define MAGIC_NUMBER 15

class FooBar
{
private: /* Member Variables */
    Display *dpy;
    Window root;

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

private: /* Member Functions */
    void setupColors();
    void redrawWorkspaces();
    void redrawRunField();

    inline bool insideWorkspaces(int, int);
    inline bool insideRunfield(int, int);

public: /* Member Functions */
    FooBar(Display *, Window, char &, char &);
    ~FooBar();

    void redraw();
    void handleButtonEvent(XButtonEvent *);
    void handleKeyEvent(XKeyEvent *);

    void setRunfield(bool);
    bool getRunfield() { return runfield_active; }
    void resetRunField();
};


#endif // _FOOBAR_H_
