#ifndef _WINDOWMANAGER_H_
#define _WINDOWMANAGER_H_

#include "glass.h"
#include "resources.h"
#include "foobar.h"

class WindowManager
{
private: /* member variables */

    list<Client*> client_list;
    list<Window> client_window_list;

    Resources *resources;
    FooBar *foobar;

    Client* focused_client;
    XFontStruct *font;
    GC      focused_title_fg_gc,
            unfocused_title_fg_gc,
            focused_title_bg_gc,
            unfocused_title_bg_gc,
            focused_border_gc,
            unfocused_border_gc,
            focused_border2_gc,
            unfocused_border2_gc;

    XColor  col_fg_focus,
            col_fg_unfocus,
            col_bg_focus,
            col_bg_unfocus,
            col_title_bd_focus,
            col_title_bd_unfocus,
            col_cli_bd_focus,
            col_cli_bd_unfocus;

    Cursor move_curs, arrow_curs;

    Display *dpy;
    Window root;
    Window _button_proxy_win;

    int screen;
    char current_workspace;

    // The screen max resolutions (x,y)
    int xres, yres;

    int shape, shape_event;

    string  command_line;
    char workspace_count;
    int focus_model;
    char *display, *opt_wm, *opt_wp, *opt_es;

    bool wire_move;
    bool rand_window_placement;
    bool edge_snap;

    static KeySym alt_keys[];

    Atom atom_wm_state;
    Atom atom_wm_change_state;
    Atom atom_wm_protos;
    Atom atom_wm_delete;
    Atom atom_wm_cmapwins;
    Atom atom_wm_takefocus;

private: /* Member Functions */

    void setupSignalHandlers();
    void setupDisplay();
    void cleanup();
    void doEventLoop();
    void scanWins();

    void print_help();
    void print_usage();
    void print_version();

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

    inline XFontStruct* getFont()   const { return font; }

    inline GC getFocusedTitleFGGC()             const { return focused_title_fg_gc;     }
    inline GC getUnfocusedTitleFGGC()           const { return unfocused_title_fg_gc;   }
    inline GC getFocusedTitleBGGC()             const { return focused_title_bg_gc;     }
    inline GC getUnfocusedTitleBGGC()           const { return unfocused_title_bg_gc;   }
    inline GC getFocusedBorderGC()              const { return focused_border_gc;       }
    inline GC getUnfocusedBorderGC()            const { return unfocused_border_gc;     }
    inline GC getFocusedBorder2GC()             const { return focused_border2_gc;      }
    inline GC getUnfocusedBorder2GC()           const { return unfocused_border2_gc;    }

    inline XColor getFGColor()                  const { return col_fg_focus;            }
    inline XColor getFRColor()                  const { return col_fg_unfocus;          }
    inline XColor getBGColor()                  const { return col_bg_focus;            }
    inline XColor getFCColor()                  const { return col_bg_unfocus;          }
    inline XColor getBRColor()                  const { return col_title_bd_focus;      }
    inline XColor getBDColor()                  const { return col_title_bd_unfocus;    }
    inline XColor getFocusedBorderColor()       const { return col_cli_bd_focus;        }
    inline XColor getUnFocusedBorderColor()     const { return col_cli_bd_unfocus;      }

    inline Cursor getMoveCursor()   const { return move_curs;           }
    inline Cursor getArrowCursor()  const { return arrow_curs;          }

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

    inline bool getWireMove()     const { return wire_move; }
    inline bool getEdgeSnap()     const { return edge_snap; }
    inline bool getRandPlacement()     const { return rand_window_placement; }
    inline char getWorkspaceCount()     const { return workspace_count; }

    inline int getXRes() const { return xres; }
    inline int getYRes() const { return yres; }

    void setWorkspaceCount(char x) { workspace_count = x; }

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
