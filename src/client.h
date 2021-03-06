/*******************************************************************************
 * Class name: Client
 *
 * Description:
 *   A wrapper for X11 Window(s). Used to manage the Window(s) and to render
 *   additional window decorations.
 ******************************************************************************/

#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "glass.h"
#include "windowmanager.h"

#include <stdlib.h>     /* itoa() */
#include <sstream>      /* put pid in name */
#include <set>          /* tags */

/*##############################################################################
#   GROUPING   #################################################################
##############################################################################*/

#include <vector>     /* windows */

class WindowManager; // forward declaration
struct character; // forward declaration

/*
    What is gravity???
    What is transient??? dialogs/message boxes
*/

enum { APPLY_GRAVITY=1, REMOVE_GRAVITY=-1 };

struct win {
    pid_t pid;
    char *name;
    char *title;
    int title_width;
    Window window;
};

/*##############################################################################
#   GROUPING   #################################################################
##############################################################################*/

class Client
{
private: /* Member Variables */

    Display *dpy;
    Window root;
    XSizeHints *xsize;
    Colormap cmap;
    int screen;

    Window frame;       // parent window which we reparent the client to
    Window title;       // window which holds title
    Window transient;   // window id for which this client is transient for

    set<char> tags;
    int current_window;
    vector<win *> windows;

    int border_width;
    int title_height;

    Point size;
    Point position;
    Point old_position;
    Point old_size;

    bool has_focus;
    bool has_title;
    bool has_border;
    bool is_being_dragged;
    bool is_being_resized;
    bool do_drawoutline_once; // used for wire move
    bool is_shaded;
    bool is_maximized;
    bool is_visible;
    bool has_been_shaped;

    Time last_button1_time;
    int ignore_unmap;

    // For window title placement
    XCharStruct overall;
    int direction;
    int ascent;
    int descent;

    // Used in client move
    Point cursor;
    Point old_cursor;

private: /* Member Functions */

    void initialize(Display *d);

    void redraw();
    void drawOutline();
    int  getIncsize(int *, int *, int);
    void initPosition();
    void reparent();
    void sendConfig();
    void gravitate(int);
    void generateTitle(win *);

    void setShape();

public: /* Member Functions */

    Client(Display *, Window, character *);
    ~Client();

    void getXClientName(win *);

    void makeNewClient(Window, character *);
    void removeClient();

    unsigned int Windows() const { return windows.size(); }
    Window getFrameWindow() const { return frame; }
    Window getAppWindow() const { return windows[current_window]->window; }
    Window getTitleWindow() const { return title; }
    Window getTransientWindow() const { return transient; }

    bool isTransient() const { return (transient != 0); }

    bool hasWindowDecorations() const { return has_title; }
    bool hasFocus() const { return has_focus; }

    bool isTagged(char workspace) const;
    bool isVisible() const { return is_visible; }

    void setFocus(bool focus); // (decieving name) Only paints the titlebar in the focus color

    void hide();
    void unhide();
    void shade();
    void maximize();

    void setTag(char);
    void addTag(char);
    void removeTag(char);
    void toggleTag(char);

    void handleButtonEvent(XButtonEvent *);
    void handleConfigureRequest(XConfigureRequestEvent *);
    void handleMapRequest(XMapRequestEvent *);
    void handleUnmapEvent(XUnmapEvent *);
    void handleDestroyEvent(XDestroyWindowEvent *);
    void handleClientMessage(XClientMessageEvent *);
    void handlePropertyChange(XPropertyEvent *);
    void handleEnterEvent(XCrossingEvent *);
    void handleColormapChange(XColormapEvent *);
    void handleExposeEvent(XExposeEvent *);
    void handleFocusInEvent(XFocusChangeEvent *);
    void handleMotionNotifyEvent(XMotionEvent *);
    void handleShapeChange(XShapeEvent *);
};

#endif // _CLIENT_H_
