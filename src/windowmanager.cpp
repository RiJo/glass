#include "windowmanager.h"

WindowManager* wm;
Resources *resources;

/*##############################################################################
#   TEST   #####################################################################
##############################################################################*/

static const alias aliases[] = {
    {"wm_quit",         {WM_QUIT,       EXEC_NONE}                          },
    {"wm_restart",      {WM_RESTART,    EXEC_NONE}                          },
    {"lock",            {WM_LOCK,       EXEC_NONE}                          },
    {"terminal",        {WM_EXEC,       EXEC_TERMINAL}                      },
    {"webbrowser",      {WM_EXEC,       EXEC_WEBBROWSER}                    },
    {"editor",          {WM_EXEC,       EXEC_EDITOR}                        }
};
#define ALIASES_COUNT (unsigned int)(sizeof(aliases)/sizeof(alias))

static const key_binding key_bindings[] = {
    {XK_End,        (ControlMask|Mod1Mask), aliases[0].foo                  },
    {XK_Home,       (ControlMask|Mod1Mask), aliases[1].foo                  },
    {XK_Tab,        (Mod1Mask),             {WM_NEXT_CLIENT,    EXEC_NONE}  },
    {XK_F4,         (Mod1Mask),             {WM_CLOSE_CLIENT,   EXEC_NONE}  },
    {XK_Right,      (Mod4Mask),             {WM_WS_SHIFT_RIGHT, EXEC_NONE}  },
    {XK_Left,       (Mod4Mask),             {WM_WS_SHIFT_LEFT,  EXEC_NONE}  },
    {XK_r,          (Mod4Mask),             {WM_RUN_DIALOG,     EXEC_NONE}  },
    {XK_l,          (Mod4Mask),             aliases[2].foo                  },
    {XK_Return,     (Mod4Mask),             aliases[3].foo                  },
    {XK_w,          (Mod4Mask),             aliases[4].foo                  },
    {XK_s,          (Mod4Mask),             aliases[5].foo                  }
};
#define KEY_BINDING_COUNT (unsigned int)(sizeof(key_bindings)/sizeof(key_binding))

/*##############################################################################
#   TEST   #####################################################################
##############################################################################*/

int handleXError(Display *dpy, XErrorEvent *e) {
    if (e->error_code == BadAccess && e->resourceid == RootWindow(dpy, DefaultScreen(dpy)) ) {
        fprintf(stderr, "Error: root window unavailable (maybe another wm is running?)\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

WindowManager::WindowManager(int argc, char** argv)
{
    wm = this;
    display = NULL;
    workspace_count = DEFAULT_WORKSPACE_COUNT;
    current_workspace = INITIAL_WORKSPACE;
    focused_client = NULL;
    rand_window_placement = RANDOM_WINDOW_PLACEMENT;
    edge_snap = EDGE_SNAP;
    wire_move = WIRE_MOVE;

    parseCommandLine(argc, argv);

    setupSignalHandlers();
    setupDisplay();
    scanWins();

    foobar = new FooBar(dpy, root, workspace_count, current_workspace);

    doEventLoop();
}

void WindowManager::print_help() {
    printf("Usage: %s [-hv] [-d DISPLAY] [-w WORKSPACES]\n", PROGRAM_NAME);
    printf("Small windowmanager\n\n");
    printf("  -d, --display DISPLAY          the display to start the window manager on, default is \"%s\"\n", getenv("DISPLAY"));
    printf("  -h, --help                     display this help and exit\n");
    printf("  -v, --version                  display version information and exit\n");
    printf("  -w, --workspaces WORKSPACES    specify the number of workspaces, default is %d\n", DEFAULT_WORKSPACE_COUNT);
}

void WindowManager::print_usage() {
    printf("Usage: %s [-hv] [-d DISPLAY] [-w WORKSPACES]\n", PROGRAM_NAME);
    printf("Try '%s --help' for more information.\n", PROGRAM_NAME);
}

void WindowManager::print_version() {
    printf("%s, version %s, released %s\n", PROGRAM_NAME, PROGRAM_VERSION, PROGRAM_DATE);
    printf("   compiled %s, %s\n\n", __DATE__, __TIME__);
    printf("Rikard Johansson, 2010\n");
}

void WindowManager::parseCommandLine(int argc, char** argv)
{
    struct option opt_list[] = {
        {"display",     0, NULL, 'd'},
        {"help",        0, NULL, 'h'},
        {"version",     0, NULL, 'v'},
        {"workspaces",  0, NULL, 'w'},
        {0,0,0,0}
    };
    int arg = EOF;
    while((arg = getopt_long(argc, argv, "d:hvw:", opt_list, NULL)) != EOF) {
        switch (arg) {
            case 'd':
                display = (char *)malloc(strlen(optarg) + 1);
                strcpy(display, optarg);
                break;
            case 'h':
                print_help();
                exit(EXIT_SUCCESS);
            case 'v':
                print_version();
                exit(EXIT_SUCCESS);
            case 'w':
                workspace_count = atoi(optarg);
                if (workspace_count <= 0)
                    workspace_count = DEFAULT_WORKSPACE_COUNT;
                break;
            default:
                print_usage();
                exit(EXIT_FAILURE);
        }
    }

    // Keep command line arguments for the restart action
    for (int i = 0; i < argc; i++) command_line = command_line + " " + argv[i];
}

void WindowManager::signalHandler(int signal)
{
    DEBUG("signal received: %d\n", signal);
    switch (signal) {
        case SIGINT:
        case SIGTERM:
            wm->quitNicely();
            break;
        case SIGHUP:
            wm->restart();
            break;
        case SIGCHLD:
            wait(NULL); // remove ghost processes
            break;
        case SIGALRM:
            action lock = {WM_LOCK, EXEC_NONE};
            wm->handleAction(lock);
            break;
    }
}

void WindowManager::setupSignalHandlers()
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGCHLD, signalHandler);
    signal(SIGALRM, signalHandler);
}

pid_t WindowManager::forkExec(string command) {
    if (command.length() == 0) {
        DEBUG("no command to execute\n");
        return 0;
    }
    DEBUG("executing \"%s\"\n", command.c_str());

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Error: could not fork\n");
        return 0;
    }
    else if (pid == 0) {
        execlp("/bin/sh", "sh", "-c", command.c_str(), NULL);
        fprintf(stderr, "Error: exec failed, cleaning up child\n");
        exit(EXIT_FAILURE);
    }
    return pid;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void WindowManager::handleAction(action a)
{
    DEBUG("action received: {%d, \"%s\"}\n", a.type, a.command.c_str());
    switch (a.type) {
        case WM_QUIT:
            quitNicely();
            break;

        case WM_RESTART:
            restart();
            break;

        case WM_LOCK:
            alarm(0); // turn off timer
            forkExec(LOCK_COMMAND);
            break;


        case WM_NEXT_CLIENT:
            nextClient();
            break;

        case WM_CLOSE_CLIENT:
            closeFocusedClient();
            break;

        case WM_WS_SHIFT_RIGHT:
            goToWorkspace(current_workspace + 1);
            break;

        case WM_WS_SHIFT_LEFT:
            goToWorkspace(current_workspace - 1);
            break;

        case WM_RUN_DIALOG:
            runDialog();
            break;

        case WM_EXEC: {
            /* stores the parsed data in pending_window */
            parseCommand(a.command);
            pid_t pid = forkExec(pending_window.command);
            pending_window.pid = pid;
            break;
        }
        default:
            fprintf(stderr, "Warning: not a valid action type: %d\n", a.type);
            break;
    }
}

void WindowManager::parseCommand(string command) {
    //~ DEBUG("parsing command: \"%s\"\n", command.c_str());
    char workspace = current_workspace;
    string separator(RUNFIELD_COMMAND_SEPARATOR);
    size_t offset = command.find(separator);
    if (offset != string::npos) {
        workspace = atoi(command.substr(0, offset).c_str());
        pending_window.command = command.substr(offset + 1);
    }
    else {
        pending_window.command = command;
    }
    pending_window.position.reset();
    pending_window.size.reset();
    pending_window.tags.clear();
    pending_window.tags.insert(workspace);
    pending_window.durability = 1;
    //~ DEBUG("command parsed:\tcmd: \"%s\"\tws: %d\n", pending_window.command.c_str(), workspace);
}

void WindowManager::execute(string command)
{
    size_t separator;
    for (register unsigned int i = 0; i < ALIASES_COUNT; i++) {
        separator = command.find(RUNFIELD_COMMAND_SEPARATOR);
        if (separator == string::npos) {
            separator = 0;
        }
        else {
            separator += strlen(RUNFIELD_COMMAND_SEPARATOR);
        }
        if (command.substr(separator).compare(aliases[i].identifier) == 0) {
            if (aliases[i].foo.type == WM_EXEC) {
                command = command.substr(0, separator) + aliases[i].foo.command;
                break;
            }
            else {
                handleAction(aliases[i].foo);
                return;
            }
        }
    }
    // no alias for command, execute it as normal
    action exec;
    exec.type = WM_EXEC;
    exec.command = command;
    handleAction(exec);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void WindowManager::updateCharacteristics() {
    if (pending_window.durability == 1) { // final window
        pending_window.command = "";
        pending_window.pid = 0;
        pending_window.position.reset();
        pending_window.size.reset();
        pending_window.tags.clear();
    }
    pending_window.durability--;
}

bool WindowManager::setCurrentWorkspace(char x)
{
    if (x < 0) {
        x = 0;
    }
    else if (x > workspace_count) {
        x = workspace_count;
    }

    if (x != current_workspace) {
        current_workspace = x;
        DEBUG("workspace changed to %d\n", x);
        return true;
    }
    else {
        return false;
    }
}

bool WindowManager::goToWorkspace(char x)
{
    if (setCurrentWorkspace(x)) {
        XSetInputFocus(dpy, _button_proxy_win, RevertToNone, CurrentTime);

        // Preserve stacking order
        unsigned int nwins;
        Window dummyw1, dummyw2, *wins;
        XQueryTree(dpy, root, &dummyw1, &dummyw2, &wins, &nwins);

        Client* c;
        for (unsigned int i = 0; i < nwins; i++) {
            c = findClient(wins[i]);

            if (c) {
                if (c->isTagged(current_workspace)) {
                    c->unhide();
                }
                else {
                    c->hide();
                }
            }
        }
        XFree(wins);

        foobar->redraw();

        return true;
    }
    return false;
}

void WindowManager::scanWins(void)
{
    unsigned int nwins, i;
    Window dummyw1, dummyw2, *wins;
    XWindowAttributes attr;
    Client *c = NULL;

    XQueryTree(dpy, root, &dummyw1, &dummyw2, &wins, &nwins);
    for(i = 0; i < nwins; i++) {
        XGetWindowAttributes(dpy, wins[i], &attr);
        if (!attr.override_redirect && attr.map_state == IsViewable) {
            c = new Client(dpy, wins[i], NULL);
            //~ windows[wins[i]] = c;
            //~ windows[c->getFrameWindow()] = c;
            //~ windows[c->getTitleWindow()] = c;
        }
    }
    XFree(wins);

    XMapWindow(dpy, _button_proxy_win);
    grabKeys(_button_proxy_win);
    XSetInputFocus(dpy, _button_proxy_win, RevertToNone, CurrentTime);
}

void WindowManager::setupDisplay()
{
    XSetWindowAttributes sattr;

    if (display)
        setenv("DISPLAY", display, 1);
    else
        display = getenv("DISPLAY");

    dpy = XOpenDisplay(display);
    if (!dpy) {
        cerr << "can't open display! check your DISPLAY variable." << endl;
        exit(1);
    }

    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);

    resources = new Resources(dpy, root, screen);

    screen_size.x = DisplayWidth(dpy, screen);
    screen_size.y = DisplayHeight(dpy, screen);

    XSetErrorHandler(handleXError);

    // ICCCM atoms
    atom_wm_state = XInternAtom(dpy, "WM_STATE", False);
    atom_wm_change_state = XInternAtom(dpy, "WM_CHANGE_STATE", False);
    atom_wm_protos = XInternAtom(dpy, "WM_PROTOCOLS", False);
    atom_wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    atom_wm_cmapwins = XInternAtom(dpy, "WM_COLORMAP_WINDOWS", False);
    atom_wm_takefocus = XInternAtom(dpy, "WM_TAKE_FOCUS", False);

    XSetWindowAttributes pattr;
    pattr.override_redirect = True;
    _button_proxy_win = XCreateSimpleWindow(dpy, root, -80, -80, 24, 24,0,0,0);
    XChangeWindowAttributes(dpy, _button_proxy_win, CWOverrideRedirect, &pattr);

    int dummy;
    shape = XShapeQueryExtension(dpy, &shape_event, &dummy);

    XDefineCursor(dpy, root, resources->getCursor(CURSOR_ARROW));

    sattr.event_mask = SubstructureRedirectMask |
                SubstructureNotifyMask |
                ColormapChangeMask |
                ButtonPressMask |
                ButtonReleaseMask |
                FocusChangeMask |
                EnterWindowMask |
                LeaveWindowMask |
                PropertyChangeMask |
                ButtonMotionMask;

    XChangeWindowAttributes(dpy, root, CWEventMask, &sattr);

    grabKeys(root);
}

void WindowManager::doEventLoop()
{
    XEvent ev;

    while (1) {
        XNextEvent(dpy, &ev);
        switch (ev.type) {
            case KeyPress:
                DEBUG("* KeyPress (%ld)\n", (long)ev.xkey.window);
                handleKeyPressEvent(&ev);
                break;

            case ButtonPress:
                DEBUG("* ButtonPress (%ld)\n", (long)ev.xbutton.window);
                handleButtonPressEvent(&ev);
                break;

            case ButtonRelease:
                DEBUG("* ButtonRelease (%ld)\n", (long)ev.xbutton.window);
                handleButtonReleaseEvent(&ev);
                break;

            case ConfigureRequest:
                DEBUG("* ConfigureRequest (%ld)\n", (long)ev.xconfigurerequest.window);
                handleConfigureRequestEvent(&ev);
                break;

            case MotionNotify:
                DEBUG("* MotionNotify (%ld)\n", (long)ev.xmotion.window);
                handleMotionNotifyEvent(&ev);
                break;

            case MapRequest:
                DEBUG("* MapRequest (%ld)\n", (long)ev.xmaprequest.window);
                handleMapRequestEvent(&ev);
                break;

                case UnmapNotify:
                DEBUG("* UnmapNotify (%ld)\n", (long)ev.xunmap.window);
                handleUnmapNotifyEvent(&ev);
                break;

            case DestroyNotify:
                DEBUG("* DestroyNotify (%ld)\n", (long)ev.xdestroywindow.window);
                handleDestroyNotifyEvent(&ev);
                break;

            //~ case EnterNotify:
                //~ DEBUG("* EnterNotify\n");
                //~ handleEnterNotifyEvent(&ev);
            //~     break;

            //~ case LeaveNotify:
                //~ DEBUG("* LeaveNotify\n");
                //~ handleLeaveNotifyEvent(&ev);
            //~     break;

            case FocusIn:
                DEBUG("* FocusIn (%ld)\n", (long)ev.xfocus.window);
                handleFocusInEvent(&ev);
                break;

            case FocusOut:
                DEBUG("* FocusOut (%ld)\n", (long)ev.xfocus.window);
                handleFocusOutEvent(&ev);
                break;

            case ClientMessage:
                DEBUG("* ClientMessage (%ld)\n", (long)ev.xclient.window);
                handleClientMessageEvent(&ev);
                break;

            case ColormapNotify:
                DEBUG("* ColormapNotify (%ld)\n", (long)ev.xcolormap.window);
                handleColormapNotifyEvent(&ev);
                break;

            case PropertyNotify:
                DEBUG("* PropertyNotify (%ld)\n", (long)ev.xproperty.window);
                handlePropertyNotifyEvent(&ev);
                break;

            case Expose:
                DEBUG("* Expose (%ld)\n", (long)ev.xexpose.window);
                handleExposeEvent(&ev);
                break;

            default:
                DEBUG("* Unhandled type: %d\n", ev.type);
                //handleDefaultEvent(&ev);
                break;
        }

        /* Note: it's improtant that the INACTIVITY_COMMAND not sends any xevents
            to the server, though this will cause the alarm timer to start again */
        if (LOCK_TIMEOUT > 0) {
            alarm(LOCK_TIMEOUT);
        }
    }
}

void WindowManager::grabKeys(Window window)
{
    // Workspace/tag switchers
    for (int i = 0; i <= workspace_count; i++) {
        XGrabKey(dpy,XKeysymToKeycode(dpy, XK_0 + i), MODIFIER_GOTO_WORKSPACE,
                window, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy,XKeysymToKeycode(dpy, XK_0 + i), MODIFIER_TAG_CLIENT,
                window, True, GrabModeAsync, GrabModeAsync);
    }
    // Keybindings
    for (unsigned int i = 0; i <= KEY_BINDING_COUNT; i++) {
        // grab with caps-lock both activated and inactivated
        XGrabKey(dpy, XKeysymToKeycode(dpy,key_bindings[i].key), key_bindings[i].modifier,
                window, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, XKeysymToKeycode(dpy,key_bindings[i].key), key_bindings[i].modifier|LockMask,
                window, True, GrabModeAsync, GrabModeAsync);
    }
}

void WindowManager::ungrabKeys(Window window)
{
    // Workspace/tag switchers
    for (int i = 0; i <= workspace_count; i++) {
        XUngrabKey(dpy,XKeysymToKeycode(dpy, XK_0 + i), MODIFIER_GOTO_WORKSPACE, window);
        XUngrabKey(dpy,XKeysymToKeycode(dpy, XK_0 + i), MODIFIER_TAG_CLIENT, window);
    }
    // Keybindings
    for (unsigned int i = 0; i <= KEY_BINDING_COUNT; i++) {
        // ungrab with caps-lock both activated and inactivated
        XUngrabKey(dpy, XKeysymToKeycode(dpy,key_bindings[i].key), key_bindings[i].modifier, window);
        XUngrabKey(dpy, XKeysymToKeycode(dpy,key_bindings[i].key), key_bindings[i].modifier | LockMask, window);
    }
}

void WindowManager::handleKeyPressEvent(XEvent *ev)
{
    KeySym ks = XKeycodeToKeysym(dpy, ev->xkey.keycode, 0);
    if (ks == NoSymbol)
        return;
    unsigned int state = ev->xkey.state;

    // Switch workspace
    for (int i = 0; i <= workspace_count; i++) {
        if (ks == (unsigned int)(XK_0 + i) && state == MODIFIER_GOTO_WORKSPACE) {
            goToWorkspace(i);
            return;
        }
        if (ks == (unsigned int)(XK_0 + i) && state == MODIFIER_TAG_CLIENT) {
            Client *focused = focusedClient();
            if (focused)
                focused->toggleTag(i);
            return;
        }
    }

    // Key bindings
    for (unsigned int i = 0; i < KEY_BINDING_COUNT; i++) {
        if (key_bindings[i].key == ks && (key_bindings[i].modifier & (state & ~LockMask))) {
            handleAction(key_bindings[i].foo);
            return;
        }
    }

    foobar->handleKeyEvent(&ev->xkey);
}

void WindowManager::handleButtonPressEvent(XEvent *ev)
{
    if (ev->xbutton.window == root) {
        switch (ev->xbutton.button) {
            case Button1: // left
            case Button2: // middle
            case Button3: // right
                foobar->handleButtonEvent(&ev->xbutton);
                break;

            case Button4: // scroll up
                goToWorkspace(current_workspace + 1);
                break;

            case Button5: // scroll down
                goToWorkspace(current_workspace - 1);
                break;
        }
    }
    else {
        Client* c = findClient(ev->xbutton.window);
        if (c && c->hasWindowDecorations()) {
            if ( (ev->xbutton.button == Button1) &&
                (ev->xbutton.type == ButtonPress) &&
                (ev->xbutton.state == Mod1Mask) &&
                (c->getFrameWindow() == ev->xbutton.window)
            ) {
                if (!XGrabPointer(dpy, c->getFrameWindow(), False,
                        PointerMotionMask|ButtonReleaseMask, GrabModeAsync,
                        GrabModeAsync, None, resources->getCursor(CURSOR_MOVE),
                        CurrentTime) == GrabSuccess) {
                    return;
                }
            }
        }

        // if this is the first time the client window's clicked, focus it
        if (c && c != focused_client) {
            XSetInputFocus(dpy, c->getAppWindow(), RevertToNone, CurrentTime);
            focused_client = c;
        }

        // otherwise, handle the button click as usual
        if (c && c == focused_client) {
            c->handleButtonEvent(&ev->xbutton);
        }
    }
}

void WindowManager::handleButtonReleaseEvent(XEvent *ev)
{
    Client* c = findClient(ev->xbutton.window);
    if (c) {
        XUngrabPointer(dpy, CurrentTime);

        c->handleButtonEvent(&ev->xbutton);
    }
}

void WindowManager::handleConfigureRequestEvent(XEvent *ev)
{
    Client* c = findClient(ev->xconfigurerequest.window);
    if (c) {
        c->handleConfigureRequest(&ev->xconfigurerequest);
    }
    else {
        // Since this window isn't yet a client lets delegate
        // the configure request back to the window so it can
        // use it.

        XWindowChanges wc;
        wc.x = ev->xconfigurerequest.x;
        wc.y = ev->xconfigurerequest.y;
        wc.width = ev->xconfigurerequest.width;
        wc.height = ev->xconfigurerequest.height;
        wc.sibling = ev->xconfigurerequest.above;
        wc.stack_mode = ev->xconfigurerequest.detail;
        XConfigureWindow(dpy, ev->xconfigurerequest.window, ev->xconfigurerequest.value_mask, &wc);
    }
}

void WindowManager::handleMotionNotifyEvent(XEvent *ev)
{
    Client* c = findClient(ev->xmotion.window);
    if (c) {
        c->handleMotionNotifyEvent(&ev->xmotion);
        foobar->redraw();
    }
}

void WindowManager::handleMapRequestEvent(XEvent *ev)
{
    Client* c = findClient(ev->xmaprequest.window);
    if (c) {
        c->handleMapRequest(&ev->xmaprequest);
    }
    else {
        c = new Client(dpy, ev->xmaprequest.window, &pending_window);
        //~ windows[ev->xmaprequest.window] = c;
        //~ windows[c->getFrameWindow()] = c;
        //~ windows[c->getTitleWindow()] = c;

        DEBUG("new client: window: %ld\t%ld\n", (long)ev->xmaprequest.window, (long)c);
        updateCharacteristics();
    }
}

void WindowManager::handleUnmapNotifyEvent(XEvent *ev)
{
    Client* c = findClient(ev->xunmap.window);
    if (c) {
        c->handleUnmapEvent(&ev->xunmap);
        // if unmapping it, note that it's no longer focused
        focused_client = NULL;
    }
}

void WindowManager::handleDestroyNotifyEvent(XEvent *ev)
{
    Client* c = findClient(ev->xdestroywindow.window);
    if (c) {
        c->handleDestroyEvent(&ev->xdestroywindow);
        // if destroying it, note that it's no longer focused
        focused_client = NULL;
    }

    XSendEvent(dpy, _button_proxy_win, False, SubstructureNotifyMask, ev);
}

void WindowManager::handleEnterNotifyEvent(XEvent *ev)
{
    // not in use
}

void WindowManager::handleLeaveNotifyEvent(XEvent *ev)
{
    // not in use
}

void WindowManager::handleFocusInEvent(XEvent *ev)
{
    if ((ev->xfocus.mode == NotifyGrab) || (ev->xfocus.mode == NotifyUngrab)) {
        return;
    }

    Client *c = windows[ev->xfocus.window];
    if (c) {
        unfocusAnyStrayClients();
        c->handleFocusInEvent(&ev->xfocus);
        focused_client = c;
        grabKeys(ev->xfocus.window);
    }

    foobar->handleFocusInEvent(&ev->xfocus);
}

void WindowManager::handleFocusOutEvent(XEvent *ev)
{
    Client *c = windows[ev->xfocus.window];
    if (c) {
        focused_client = NULL;
        ungrabKeys(ev->xfocus.window);
        return;
    }

    focusPreviousWindowInStackingOrder();
}

void WindowManager::handleClientMessageEvent(XEvent *ev)
{
    Client* c = findClient(ev->xclient.window);
    if (c) {
        c->handleClientMessage(&ev->xclient);
    }
}

void WindowManager::handleColormapNotifyEvent(XEvent *ev)
{
    Client* c = findClient(ev->xcolormap.window);
    if (c) {
        c->handleColormapChange(&ev->xcolormap);
    }
}

void WindowManager::handlePropertyNotifyEvent(XEvent *ev)
{
    Client* c = findClient(ev->xproperty.window);
    if (c) {
        c->handlePropertyChange(&ev->xproperty);
    }
}

void WindowManager::handleExposeEvent(XEvent *ev)
{
    Client* c = findClient(ev->xexpose.window);
    if (c) {
        c->handleExposeEvent(&ev->xexpose);
    }
    foobar->handleExposeEvent(&ev->xexpose);
}

void WindowManager::handleDefaultEvent(XEvent *ev)
{
    // not in use
}

void WindowManager::unfocusAnyStrayClients()
{
    // To prevent two windows titlebars from being painted with the focus color we
    // will prevent that from happening by setting all windows to false.
    set<Client *>::iterator iter;
    for (iter = clients.begin(); iter != clients.end(); iter++) {
        (*iter)->setFocus(false);
    }
}

void WindowManager::focusPreviousWindowInStackingOrder()
{
    unsigned int nwins, i;
    Window dummyw1, dummyw2, *wins;

    XSetInputFocus(dpy, _button_proxy_win, RevertToNone, CurrentTime);

    XQueryTree(dpy, root, &dummyw1, &dummyw2, &wins, &nwins);

    if (clients.size()) {
        list<Client *> client_list_for_current_workspace;

        for (i = 0; i < nwins; i++) {
            Client* c = findClient(wins[i]);
            if (c && c->isTagged(current_workspace) && c->hasWindowDecorations()) {
                client_list_for_current_workspace.push_back(c);
            }
        }

        if (client_list_for_current_workspace.size()) {
            list<Client*>::iterator iter = client_list_for_current_workspace.end();

            iter--;

            if ((*iter)) {
                XSetInputFocus(dpy, (*iter)->getAppWindow(), RevertToNone, CurrentTime);
            }
        }
    }

    XFree(wins);
}

void WindowManager::getMousePosition(int *x, int *y)
{
    Window mouse_root, mouse_win;
    int win_x, win_y;
    unsigned int mask;

    XQueryPointer(dpy, root, &mouse_root, &mouse_win, x, y, &win_x, &win_y, &mask);
}

void WindowManager::addClient(Client *c)
{
    clients.insert(c);
    windows[c->getAppWindow()] = c;
    windows[c->getFrameWindow()] = c;
    windows[c->getTitleWindow()] = c;
}

void WindowManager::removeClient(Client* c)
{
    windows.erase(c->getAppWindow());
    windows.erase(c->getFrameWindow());
    windows.erase(c->getTitleWindow());
    clients.erase(c);
}

Client *WindowManager::findClient(Window window)
{
    map<Window, Client *>::iterator iter = windows.find(window);
    return (iter != windows.end()) ? (*iter).second : NULL;
}

void WindowManager::findTransientsToMapOrUnmap(Window window, bool hide)
{
    if (clients.size()) {
        set<Client *>::iterator iter;
        for (iter = clients.begin(); iter != clients.end(); iter++) {
            if ((*iter)->getTransientWindow() == window && !hide) {
                (*iter)->unhide();
            }
        }
    }
}

void WindowManager::restart()
{
    cleanup();
    execl("/bin/sh", "sh", "-c", command_line.c_str(), (char *)NULL);
}

void WindowManager::quitNicely()
{
    cleanup();
    exit(0);
}

void WindowManager::cleanup()
{
    cerr << PROGRAM_NAME << " is cleaning up.... " << endl;

    unsigned int nwins, i;
    Window dummyw1, dummyw2, *wins;
    Client* c;

    XDestroyWindow(dpy, _button_proxy_win);

    ungrabKeys(root);

    // Preserve stacking order when removing the clients from the list.
    XQueryTree(dpy, root, &dummyw1, &dummyw2, &wins, &nwins);
    for (i = 0; i < nwins; i++) {
        c = findClient(wins[i]);
        if (c) {
            DEBUG("removing window: %ld\tclient: %ld\n", (long)wins[i], (long)c);
            XMapWindow(dpy, c->getAppWindow());
            delete c;
        }
    }
    XFree(wins);

    delete foobar;
    delete resources;

    XInstallColormap(dpy, DefaultColormap(dpy, screen));
    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    XCloseDisplay(dpy);

    cerr << PROGRAM_NAME << " is cleaned up!" << endl;
}

/* If we can't find a wm->wm_state we're going to have to assume
    * Withdrawn. This is not exactly optimal, since we can't really
    * distinguish between the case where no WM has run yet and when the
    * state was explicitly removed (Clients are allowed to either set the
    * atom to Withdrawn or just remove it... yuck.) */
long WindowManager::getWMState(Window window)
{
        Atom real_type; int real_format;
        unsigned long items_read, items_left;
        unsigned char *data, state = WithdrawnState;

        if (XGetWindowProperty(dpy, window, atom_wm_state, 0L, 2L, False,
                wm->atom_wm_state, &real_type, &real_format, &items_read, &items_left,
                &data) == Success && items_read) {
            state = *data;
            XFree(data);
        }

        return state;
}

/* Attempt to follow the ICCCM by explicity specifying 32 bits for
    * this property. Does this goof up on 64 bit systems? */
void WindowManager::setWMState(Window window, int state)
{
        CARD32 data[2];

        data[0] = state;
        data[1] = None; // Icon? We don't need no steenking icon.

        XChangeProperty(dpy, window, atom_wm_state, atom_wm_state,
            32, PropModeReplace, (unsigned char *)data, 2);
}

// The name of this function is a bit misleading: if the client
// doesn't listen to WM_DELETE then we just terminate it with extreme
// prejudice.
void WindowManager::sendWMDelete(Window window)
{
        int i, n, found = 0;
        Atom *protocols;

        if (XGetWMProtocols(dpy, window, &protocols, &n)) {
            for (i=0; i<n; i++) if (protocols[i] == atom_wm_delete) found++;
            XFree(protocols);
        }
        if (found)
            sendXMessage(window, atom_wm_protos, NoEventMask, atom_wm_delete);
        else
            XKillClient(dpy, window);
}

// Currently, only sendWMDelete uses this one...
int WindowManager::sendXMessage(Window window, Atom a, long mask, long x)
{
        XEvent e;

        e.type = ClientMessage;
        e.xclient.window = window;
        e.xclient.message_type = a;
        e.xclient.format = 32;
        e.xclient.data.l[0] = x;
        e.xclient.data.l[1] = CurrentTime;

        return XSendEvent(dpy, window, False, mask, &e);
}

Client *WindowManager::focusedClient() {
    set<Client *>::iterator iter;
    for (iter = clients.begin(); iter != clients.end(); iter++) {
        if ((*iter)->hasFocus() && (*iter)->isTagged(current_workspace)) {
            return (*iter);
        }
    }
    return NULL;
}

void WindowManager::nextClient() {
    XCirculateSubwindows(dpy, root, LowerHighest);
}

void WindowManager::runDialog() {
    foobar->setRunfield(true);
}

void WindowManager::closeFocusedClient() {
    if (clients.size()) {
        Client *focused = focusedClient();
        if (focused) {
            sendWMDelete(focused->getAppWindow());
        }
    }
}
