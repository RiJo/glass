/*******************************************************************************
 * Class name: WindowManager
 *
 * Description:
 *   The windowmanager itself. Handles signals, X events and manages all the
 *   clients.
 ******************************************************************************/

#ifndef _WINDOWMANAGER_H_
#define _WINDOWMANAGER_H_

#include "glass.h"
#include "client.h"
#include "resources.h"
#include "foobar.h"

#include <signal.h>
#include <sys/wait.h>       /* wait() */
#include <getopt.h>
#include <list>

class Client; // forward declaration


enum wm_action {
    WM_QUIT,
    WM_RESTART,
    WM_LOCK,
    WM_NEXT_CLIENT,
    WM_CLOSE_CLIENT,
    WM_RUN_DIALOG,
    WM_WS_SHIFT_RIGHT,
    WM_WS_SHIFT_LEFT,
    WM_EXEC
};

struct action {
    wm_action type;
    string command;
};

struct alias {
    string identifier;
    action foo;
};

struct key_binding {
    KeySym key;
    unsigned int modifier;
    action foo;
};

/*##############################################################################
#   GROUPING   #################################################################
##############################################################################*/

#include <queue>
#include <set>

struct character {
    string command;
    pid_t pid;
    Point position;
    Point size;
    set<char> tags;
    unsigned int durability;
};


/*##############################################################################
#   GROUPING   #################################################################
##############################################################################*/

class WindowManager
{
private: /* member variables */
    
/*##############################################################################
#   GROUPING   #################################################################
##############################################################################*/

    //queue<char *> commands;
    character pending_window;

/*##############################################################################
#   GROUPING   #################################################################
##############################################################################*/

    set<Client *> clients;
    map<Window, Client *> windows;

    FooBar *foobar;
    Client* focused_client;

    Display *dpy;
    Window root;
    Window _button_proxy_win;

    int screen;
    char current_workspace;

    Point screen_size;

    int shape, shape_event;

    string command_line;
    char workspace_count;
    int focus_model;
    char *display;

    bool wire_move;
    bool rand_window_placement;
    unsigned int edge_snap;

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

    void parseCommandLine(int argc, char** argv);
    void quitNicely();
    void restart();

    void handleAction(action);
    pid_t forkExec(string);
    void parseCommand(string);
    void updateCharacteristics();

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

    void grabKeys(Window w);
    void ungrabKeys(Window w);

    static void signalHandler(int signal);

public: /* Member Functions */

    WindowManager(int argc, char** argv);

    void execute(string);

    Client* getFocusedClient() { return focused_client; }

    void addClient(Client *c);
    void removeClient(Client* c);
    Client* findClient(Window w);

    void focusPreviousWindowInStackingOrder();
    void unfocusAnyStrayClients();
    void findTransientsToMapOrUnmap(Window win, bool hide);

    inline int getShape()         const { return shape; }
    inline int getShapeEvent()     const { return shape_event; }

    long getWMState(Window window);
    void setWMState(Window window, int state);
    void sendWMDelete(Window window);
    int sendXMessage(Window w, Atom a, long mask, long x);

    void getMousePosition(int *x, int *y);
    bool goToWorkspace(char);
    inline char getCurrentWorkspace() const { return current_workspace; }
    bool setCurrentWorkspace(char);

    inline bool getWireMove()     const { return wire_move; }
    inline unsigned int getEdgeSnap()     const { return edge_snap; }
    inline bool getRandPlacement()     const { return rand_window_placement; }
    inline char getWorkspaceCount()     const { return workspace_count; }

    inline int getXRes() const { return screen_size.x; }
    inline int getYRes() const { return screen_size.y; }

    void setWorkspaceCount(char x) { workspace_count = x; }

    inline Atom getWMChangeStateAtom() const { return atom_wm_change_state; }
    inline Atom getWMProtosAtom() const { return atom_wm_protos; }
    inline Atom getWMTakeFocusAtom() const { return atom_wm_takefocus; }

    Client *focusedClient();
    void nextClient();
    void runDialog();
    void closeFocusedClient();
};

extern WindowManager *wm;
extern Resources *resources;

#endif // _WINDOWMANAGER_H_
