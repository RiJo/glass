#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "glass.h"

class Client
{
private: /* Member Variables */

    Display         *dpy;
    Window          root;
    XSizeHints      *size;
    Colormap        cmap;
    int             screen;

    char            *name;    // Name used to display in titlebar

    Window          window;   // actual client window
    Window          frame;    // parent window which we reparent the client to
    Window          title;    // window which holds title
    Window          trans;    // window id for which this client is transient for

//    WindowMenu *window_menu;

    set<char> tags;

    int  x, y, width, height;

    int border_width;

    int  old_x, old_y, old_width, old_height;

    bool has_focus, has_title, has_border;

    bool is_being_dragged;
    bool is_being_resized;
    bool do_drawoutline_once; // used for wire move
    bool wire_move;

    bool is_shaded;
    bool is_iconified;
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
    int text_width;
    int text_justify;
    int justify_style;

    // Used in client move
    int pointer_x, pointer_y;
    int old_cx, old_cy;

    bool button_pressed;

private: /* Member Functions */

    void initialize(Display *d);

    void redraw();
    void drawOutline();
    int  getIncsize(int *, int *, int);
    void initPosition();
    void reparent();
    int  theight();
    void sendConfig();
    void gravitate(int);

    void setShape();

public: /* Member Functions */

    Client(Display *d, Window new_client);
    ~Client();

    void getXClientName();

    void makeNewClient(Window);
    void removeClient();

    char* getClientName() const { return name; }
    char* getClientIconName() const { return name; } // for now just return application name

    Window getFrameWindow() const    { return frame; }
    Window getAppWindow()     const { return window; }
    Window getTitleWindow()    const { return title;    }
    Window getTransientWindow() const { return trans; }

    bool isTransient() { return (trans != 0); }

    bool hasWindowDecorations()     const { return has_title; }
    bool hasFocus()            const { return has_focus; }

    bool isTagged(char workspace) const;
    bool isIconified()         const { return is_iconified;     }
    bool isVisible()        const { return is_visible;    }

    void setFocus(bool focus); // (decieving name) Only paints the titlebar in the focus color

    void hide();
    void unhide();
    void iconify();
    void shade();
    void maximize();

    void setTag(char);
    void addTag(char);
    void removeTag(char);

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

#endif
