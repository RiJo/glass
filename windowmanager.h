#ifndef _WINDOWMANAGER_H_
#define _WINDOWMANAGER_H_

#include "glass.h"
#include "foobar.h"

class WindowManager
{
private: /* member variables */

    list<Client*> client_list;
    list<Window> client_window_list;

    FooBar *foobar;

    Client* focused_client;
    XFontStruct *font;
    GC invert_gc, string_gc, border_gc, unfocused_gc, focused_title_gc;
    XColor fg, bg, bd, fc;

    XColor focused_border, unfocused_border;

    Cursor move_curs, arrow_curs;

    Display *dpy;
    Window root;
    Window _button_proxy_win;

    int screen;
    char current_workspace;

    // The screen max resolutions (x,y)
    int xres;
    int yres;

    int shape, shape_event;

    string  command_line;
    char     workspace_count;
    int     focus_model;
    char    *opt_display,
            *opt_fc,
            *opt_fg,
            *opt_fm,
            *opt_bg,
            *opt_bd,
            *opt_tj,
            *opt_wm,
            *opt_wp,
            *opt_es,
            *opt_new1;

    int     opt_bw;
    int     opt_text_justify;
    bool    wire_move;
    bool    rand_window_placement;
    bool    edge_snap;

    static KeySym alt_keys[];

    Atom    atom_wm_state;
    Atom    atom_wm_change_state;
    Atom    atom_wm_protos;
    Atom    atom_wm_delete;
    Atom    atom_wm_cmapwins;
    Atom    atom_wm_takefocus;

private: /* Member Functions */

    void setupSignalHandlers();
    void setupDisplay();
    void cleanup();
    void doEventLoop();
    void scanWins();

    void handleKeyPressEvent(XEvent *ev);
    void handleButtonPressEvent(XEvent *ev);
    void handleButtonReleaseEvent(XEvent *ev);
    void handleConfigureRequestEvent(XEvent *ev);
    void handleMotionNotifyEvent(XEvent *ev);
    void handleMapRequestEvent(XEvent *ev);
    void handleUnmapNotifyEvent(XEvent *ev);
    void handleDestroyNotifyEvent(XEvent *ev);
    void handleEnterNotifyEvent(XEvent *ev);
    void handleLeaveNotifyEvent(XEvent *ev);
    void handleFocusInEvent(XEvent *ev);
    void handleFocusOutEvent(XEvent *ev);
    void handleClientMessageEvent(XEvent *ev);
    void handleColormapNotifyEvent(XEvent *ev);
    void handlePropertyNotifyEvent(XEvent *ev);
    void handleExposeEvent(XEvent *ev);
    void handleDefaultEvent(XEvent *ev);

    static void sigHandler(int signal);

public: /* Member Functions */

    WindowManager(int argc, char** argv);

    void parseCommandLine(int argc, char** argv);
    void quitNicely();
    void restart();

    Client* getFocusedClient() { return focused_client; }

    inline list<Client*> getClientList() const { return client_list; }

    void addClient(Client *c);
    void removeClient(Client* c);
    Client* findClient(Window w);

    void focusPreviousWindowInStackingOrder();
    void unfocusAnyStrayClients();
    void findTransientsToMapOrUnmap(Window win, bool hide);

    inline XFontStruct* getFont()   const { return font;         }
    inline GC getInvertGC()         const { return invert_gc;     }
    inline GC getStringGC()         const { return string_gc;     }
    inline GC getBorderGC()         const { return border_gc;     }
    inline GC getUnfocusedGC()      const { return unfocused_gc;     }
    inline GC getFocusedTitleGC()   const { return focused_title_gc;}
    inline Cursor getMoveCursor()   const { return move_curs;    }
    inline Cursor getArrowCursor()  const { return arrow_curs;     }
    inline XColor getFGColor()      const { return fg;         }
    inline XColor getFCColor()      const { return fc;         }
    inline XColor getBGColor()      const { return bg;         }
    inline XColor getBDColor()      const { return bd;         }

    inline XColor getFocusedBorderColor() const { return focused_border; }
    inline XColor getUnFocusedBorderColor() const { return unfocused_border; }

    inline int getShape()         const { return shape; }
    inline int getShapeEvent()     const { return shape_event; }

    long getWMState(Window window);
    void setWMState(Window window, int state);
    void sendWMDelete(Window window);
    int sendXMessage(Window w, Atom a, long mask, long x);

    void getMousePosition(int *x, int *y);
    void goToWorkspace(char);
    inline char getCurrentWorkspace() const { return current_workspace; }
    bool setCurrentWorkspace(char);

    // Returns a number corresponding to the current focus model.
    inline int getFocusModel() const { return focus_model; }

    // Accepts a number corresponding to a new focus model.
    inline void setFocusModel(int new_fm)
    {
        switch (new_fm)
        {
            case FOCUS_FOLLOW:
            case FOCUS_SLOPPY:
            case FOCUS_CLICK:
                focus_model = new_fm;
            break;

            default:
                focus_model=FOCUS_CLICK;
            break;
        }
    }

    inline int getTextJustify()    const { return opt_text_justify; }
    inline bool getWireMove()     const { return wire_move; }
    inline bool getEdgeSnap()     const { return edge_snap; }
    inline bool getRandPlacement()     const { return rand_window_placement; }
    inline char getWorkspaceCount()     const { return workspace_count; }

    inline int getXRes() const { return xres; }
    inline int getYRes() const { return yres; }

    void setWorkspaceCount(char x) { workspace_count = x; }

    inline int getOptBW()     const { return opt_bw;         }

    inline Atom getWMChangeStateAtom() const { return atom_wm_change_state; }
    inline Atom getWMProtosAtom() const { return atom_wm_protos; }
    inline Atom getWMTakeFocusAtom() const { return atom_wm_takefocus; }

    void grabKeys(Window w);
    void ungrabKeys(Window w);


    Client *focusedClient();
    void nextWorkspace();
    void previousWorkspace();
    void nextClient();
    void runDialog();
    void closeFocusedClient();
};

extern WindowManager *wm;

#endif // _WINDOWMANAGER_H_
