#include "client.h"

Client::Client(Display *d, Window new_client, character *c)
{
    DEBUG("new client:\tpid: %ld\tpos: %d:%d\tsize: %d:%d\tcmd: \"%s\"\n", (long)c->pid, c->position.x, c->position.y, c->size.x, c->size.y, c->command.c_str());
    initialize(d);
    wm->addClient(this);
    makeNewClient(new_client, c);
}

Client::~Client()
{
    removeClient();
}

void Client::initialize(Display *d)
{
    dpy                     = d;

    frame                   = None;
    title                   = None;
    trans                   = None;

    current_window          = 0;
    ignore_unmap            = 0;

    has_been_shaped         = false;
    has_title               = true;
    has_border              = true;
    has_focus               = false;

    // Extra Window States
    is_shaded               = false;
    is_maximized            = false;
    is_visible              = false;

    is_being_dragged        = false;
    do_drawoutline_once     = false;
    is_being_resized        = false;

    last_button1_time       = 0;
    direction               = 0;
    ascent                  = 0;
    descent                 = 0;

    screen                  = DefaultScreen(dpy);
    root                    = RootWindow(dpy, screen);
}

void Client::getXClientName(win *window)
{
    // window name
    if(window->name) {
        XFree(window->name);
    }
    XFetchName(dpy, window->window, &(window->name));
    // title
    generateTitle(window);
}

void Client::makeNewClient(Window w, character *c)
{
    XWindowAttributes attr;
    XWMHints *hints;

    XGrabServer(dpy);

    win *new_window = new win;
    new_window->pid = c->pid;
    new_window->name = NULL;
    new_window->title = NULL;
    new_window->title_width = 0;
    new_window->window = w;

    getXClientName(new_window);

    windows.push_back(new_window);
    current_window = windows.size() - 1;

    XGetTransientForHint(dpy, w, &trans);
    XGetWindowAttributes(dpy, w, &attr);

    if (c->position.zero()) {
        position.x = attr.x;
        position.y = attr.y;
    }
    else {
        position = c->position;
    }

    if (c->size.zero()) {
        size.x = attr.width;
        size.y = attr.height;
    }
    else {
        size = c->size;
    }

    border_width = attr.border_width;
    cmap = attr.colormap;
    xsize = XAllocSizeHints();

    long dummy;
    XGetWMNormalHints(dpy, w, xsize, &dummy);

    old_position = position;
    old_size = size;

    if (attr.map_state == IsViewable) {
        ignore_unmap++;
        initPosition();

        if ((hints = XGetWMHints(dpy, w))) {
            if (hints->flags & StateHint)
                wm->setWMState(w, hints->initial_state);
            else wm->setWMState(w, NormalState);
                XFree(hints);
        }
    }

    gravitate(APPLY_GRAVITY);
    reparent();

    if (c->tags.size() == 0) {
        tags.insert(wm->getCurrentWorkspace());
    }
    else {
        set<char>::iterator i;
        for (i = c->tags.begin(); i != c->tags.end(); i++) {
            tags.insert(*i);
        }
    }
    
    c->durability -= 1;

    unhide();

    XSetInputFocus(dpy, w, RevertToNone, CurrentTime);

    XSync(dpy, False);
    XUngrabServer(dpy);
    DEBUG("window created, number of windows: %d\n", (int)windows.size());
}

void Client::removeClient()
{
    XGrabServer(dpy);

    if (trans) {
        XSetInputFocus(dpy, trans, RevertToNone, CurrentTime);
    }

    XUngrabButton(dpy, AnyButton, AnyModifier, frame);

    gravitate(REMOVE_GRAVITY);

    XReparentWindow(dpy, windows[current_window]->window, root, position.x, position.y);

    if (windows[current_window]->name) {
        XFree(windows[current_window]->name);
    }

    if (windows.size() == 1) {
        // Last window in this client => destroy client
        XDestroyWindow(dpy, title);
        XDestroyWindow(dpy, frame);

        if (xsize) {
            XFree(xsize);
        }

        XSync(dpy, False);
        XUngrabServer(dpy);

        wm->removeClient(this);
    }

    // remove window from client
    delete windows[current_window];
    windows.erase(windows.begin() + current_window);
    current_window = 0;
    DEBUG("window removed, number of windows: %d\n", (int)windows.size());
}

int Client::titleHeight()
{
    if (!has_title) {
        return 0;
    }

    int title_size = wm->getResources()->getFont(FONT_NORMAL)->ascent + wm->getResources()->getFont(FONT_NORMAL)->descent + 4;
    return (title_size > TITLE_MINIMUM_HEIGHT) ? title_size : TITLE_MINIMUM_HEIGHT;
}

void Client::sendConfig()
{
    XConfigureEvent ce;

    ce.type = ConfigureNotify;
    ce.event = windows[current_window]->window;
    ce.window = windows[current_window]->window;
    ce.x = position.x;
    ce.y = position.y;
    ce.width = size.x;
    ce.height = size.y;
    ce.border_width = border_width;
    //ce.above = (is_always_on_top) ? Above : Below;
    ce.override_redirect = False;

    XSendEvent(dpy, getAppWindow(), False, StructureNotifyMask, (XEvent *)&ce);
}

void Client::redraw()
{
    if (!has_title) {
        return;
    }

    // Title decoration
    GC gc;
    if(has_focus)
        gc = wm->getResources()->getGC(COLOR_DECORATION_FOCUSED);
    else
        gc = wm->getResources()->getGC(COLOR_DECORATION_UNFOCUSED);

    XDrawLine(dpy, title, gc, 0, titleHeight() - border_width, size.x, titleHeight() - border_width);
    XDrawLine(dpy, title, gc, size.x - titleHeight(), 0, size.x - titleHeight(), titleHeight());

    // Title text
    if(has_focus)
        gc = wm->getResources()->getGC(COLOR_FOREGROUND_FOCUSED);
    else
        gc = wm->getResources()->getGC(COLOR_FOREGROUND_UNFOCUSED);

    const char *title_text = windows[current_window]->title;
    if (!trans && title_text) {
        int text_justify = size.x - windows[current_window]->title_width - 25;
        XDrawString(dpy, title, gc, text_justify, wm->getResources()->getFont(FONT_NORMAL)->ascent+1,title_text, strlen(title_text));
    }
}

void Client::generateTitle(win *window) {
    if (window == NULL) {
        fprintf(stderr, "Warning: cannot get title from NULL window\n");
        return;
    }
    if (window->name == NULL) {
        fprintf(stderr, "Warning: there is no name set on current window\n");
        return;
    }
    if (window->title) {
        free(window->title);
    }

    char buffer[1024];
    int length = sprintf(buffer, "[%d] %s", (int)window->pid, window->name);

    window->title = (char *)malloc(length + 1);
    strcpy(window->title, buffer);

    // title width
    XTextExtents(wm->getResources()->getFont(FONT_NORMAL), window->title, strlen(window->title), &direction, &ascent, &descent, &overall);
    window->title_width = overall.width;
}

void Client::gravitate(int multiplier)
{
        int dy = 0;
        int gravity = (xsize->flags & PWinGravity) ? xsize->win_gravity : NorthWestGravity;

        switch (gravity) {
            case NorthWestGravity:
            case NorthEastGravity:
            case NorthGravity:
                dy = titleHeight();
            break;

            case CenterGravity:
                dy = titleHeight() / 2;
            break;
        }
        position.y += multiplier * dy;
}

void Client::setShape()
{
    int n=0, order=0;
    XRectangle temp, *dummy;

    dummy = XShapeGetRectangles(dpy, getAppWindow(), ShapeBounding, &n, &order);

    if (n > 1) {
        XShapeCombineShape(dpy, frame, ShapeBounding,
                0, titleHeight(), getAppWindow(), ShapeBounding, ShapeSet);

        temp.x = -border_width;
        temp.y = -border_width;
        temp.width = size.x + 2 * border_width;
        temp.height = titleHeight() + border_width;

        XShapeCombineRectangles(dpy, frame, ShapeBounding,
                0, 0, &temp, 1, ShapeUnion, YXBanded);

        temp.x = 0;
        temp.y = 0;
        temp.width = size.x;
        temp.height = titleHeight() - border_width;

        XShapeCombineRectangles(dpy, frame, ShapeClip,
                0, titleHeight(), &temp, 1, ShapeUnion, YXBanded);

        has_been_shaped = 1;
    }
    else if (has_been_shaped) {
        temp.x = -border_width;
        temp.y = -border_width;
        temp.width = size.x + 2 * border_width;
        temp.height = size.y + titleHeight() + 2 * border_width;

        XShapeCombineRectangles(dpy, frame, ShapeBounding,
                0, 0, &temp, 1, ShapeSet, YXBanded);
    }

    XFree(dummy);
}

bool Client::isTagged(char workspace) const
{
    return (workspace <= 0) ? true : (tags.find(workspace) != tags.end());
}

void Client::hide()
{
    if (!ignore_unmap) {
        ignore_unmap++;
    }

    if(has_focus) {
        setFocus(false);
    }

    XUnmapSubwindows(dpy, frame);
    XUnmapWindow(dpy, frame);

    wm->setWMState(getAppWindow(), WithdrawnState);

    is_visible = false;
}

void Client::unhide()
{
    if(isTagged(wm->getCurrentWorkspace())) {
        if(trans) {
            wm->findTransientsToMapOrUnmap(getAppWindow(), false);
        }

        XMapSubwindows(dpy, frame);
        XMapRaised(dpy, frame);

        wm->setWMState(getAppWindow(), NormalState);

        XSetInputFocus(dpy, getAppWindow(), RevertToNone, CurrentTime);

        is_visible = true;
    }
}

void Client::initPosition()
{
    int mouse_x, mouse_y;

    unsigned int w, h;
    unsigned int border_width, depth;

    XWindowAttributes attr;

    XGetWindowAttributes(dpy, getAppWindow(), &attr);

    if (attr.map_state == IsViewable)
        return;

    XGetGeometry(dpy, getAppWindow(), &root, &position.x, &position.y, &w, &h, &border_width, &depth);

    size.x = w;
    size.y = h;

    if (xsize->flags & PPosition) {
            if(!position.x) position.x = xsize->x;
            if(!position.y) position.y = xsize->y;
    }
    else {
        if (xsize->flags & USPosition) {
            if(!position.x)
                position.x = xsize->x;
            if(!position.y)
                position.y = xsize->y;
        }
        else if ( (position.x == 0) || (position.y == 0)  ) {
            if(size.x >= wm->getXRes() && size.y >= wm->getYRes()) {
                position.reset();
                size.x = wm->getXRes();
                size.y = wm->getYRes()-titleHeight();
            }
            else {
                wm->getMousePosition(&mouse_x, &mouse_y);

                if(mouse_x && mouse_y) {
                    if (wm->getRandPlacement()) {
                        position.x = (rand() % (unsigned int) ((wm->getXRes() - size.x) * 0.94)) + ((unsigned int) (wm->getXRes() * 0.03));
                        position.y = (rand() % (unsigned int) ((wm->getYRes() - size.y) * 0.94)) + ((unsigned int) (wm->getYRes() * 0.03));
                    }
                    else {
                        position.x = (int) (((long) (wm->getXRes() - size.x) * (long) mouse_x) / (long) wm->getXRes());
                        position.y = (int) (((long) (wm->getYRes() - size.y - titleHeight()) * (long) mouse_y) / (long) wm->getYRes());
                        position.y = (position.y < titleHeight()) ? titleHeight() : position.y;
                    }
                    gravitate(REMOVE_GRAVITY);
                }
            }
        }
    }
}

void Client::maximize()
{
    if(trans) return;

    if(is_shaded) {
        shade();
        return;
    }

    if(! is_maximized) {
        old_position = position;
        old_size = size;

        if (xsize->flags & PMaxSize) {
            size.x = xsize->max_width;
            size.y = xsize->max_height;

            XMoveResizeWindow(dpy, frame, position.x, position.y-titleHeight(), size.x, size.y+titleHeight());
        }
        else {
            position.reset();
            size.x = wm->getXRes()-2;
            size.y = wm->getYRes()-2;

            XMoveResizeWindow(dpy, frame, position.x, position.y, size.x, size.y);

            position.y = titleHeight();
            size.y -= titleHeight();
        }
        is_maximized = true;
    }
    else {
        position = old_position;
        size = old_size;

        XMoveResizeWindow(dpy, frame, old_position.x, old_position.y - titleHeight(), old_size.x, old_size.y + titleHeight());

        is_maximized = false;

        if (is_shaded) {
            is_shaded = false;
        }
    }

    XResizeWindow(dpy, title, size.x, titleHeight());
    XResizeWindow(dpy, getAppWindow(), size.x, size.y);

    sendConfig();
}

void Client::handleMotionNotifyEvent(XMotionEvent *ev)
{
    if((ev->state & Button1Mask) && (wm->getFocusedClient() == this)) {
        if(! do_drawoutline_once && wm->getWireMove()) {
            XGrabServer(dpy);
            drawOutline();
            do_drawoutline_once=true;
            is_being_dragged=true;
        }

        if(wm->getWireMove()) {
            drawOutline();
        }

        Point new_cursor;
        new_cursor.x = old_cursor.x + (ev->x_root - cursor.x);
        new_cursor.y = old_cursor.y + (ev->y_root - cursor.y);

        int snap_width = wm->getEdgeSnap();
        if(snap_width) {
            // Move beyond edges of screen
            if (new_cursor.x == wm->getXRes() - size.x)
                new_cursor.x = wm->getXRes() - size.y + 1;
            else if (new_cursor.x == 0)
                new_cursor.x = -1;

            if (new_cursor.y == wm->getYRes() - snap_width)
                new_cursor.y = wm->getYRes() - snap_width - 1;
            else if (new_cursor.y == titleHeight())
                new_cursor.y = titleHeight() - 1;

            // Snap to edges of screen
            if ( (new_cursor.x + size.x >= wm->getXRes() - snap_width) && (new_cursor.x + size.x <= wm->getXRes()) )
                new_cursor.x = wm->getXRes() - size.x;
            else if ( (new_cursor.x <= snap_width) && (new_cursor.x >= 0) )
                new_cursor.x = 0;

            if (is_shaded) {
                if ( (new_cursor.y  >= wm->getYRes() - snap_width) && (new_cursor.y  <= wm->getYRes()) )
                    new_cursor.y = wm->getYRes();
                else if ( (new_cursor.y - titleHeight() <= snap_width) && (new_cursor.y - titleHeight() >= 0))
                    new_cursor.y = titleHeight();
            }
            else {
                if ( (new_cursor.y + size.y >= wm->getYRes() - snap_width) && (new_cursor.y + size.y <= wm->getYRes()) )
                    new_cursor.y = wm->getYRes() - size.y;
                else if ( (new_cursor.y - titleHeight() <= snap_width) && (new_cursor.y - titleHeight() >= 0))
                    new_cursor.y = titleHeight();
            }
        }

        position = new_cursor;

        if(!wm->getWireMove()) {
            XMoveWindow(dpy, frame, new_cursor.x, new_cursor.y - titleHeight());
            sendConfig();
        }

        if(wm->getWireMove()) {
            drawOutline();
        }
    }
    else if(ev->state & Button3Mask) {
        if(! is_being_resized) {
            int in_box = (ev->x >= size.x - titleHeight()) && (ev->y <= titleHeight());

            if(! in_box)
                return;
        }

        if(! do_drawoutline_once) {
            XGrabServer(dpy);
            is_being_resized=true;
            do_drawoutline_once=true;
            drawOutline();
            XWarpPointer(dpy, None, frame, 0, 0, 0, 0, size.x, size.y + titleHeight());
        }
        else {
            if((ev->x > 50) && (ev->y > 50)) {
                drawOutline();

                size.x = ev->x;
                size.y = ev->y - titleHeight();

                int pixels = 0; // what is this? old enum...
                getIncsize(&size.x, &size.y, pixels);

                if (xsize->flags & PMinSize) {
                    if (size.x < xsize->min_width)
                        size.x = xsize->min_width;
                    if (size.y < xsize->min_height)
                        size.y = xsize->min_height;

                    if(size.x < 100)
                        size.x = 100;
                    if(size.y < 50)
                        size.y = 50;
                }

                if (xsize->flags & PMaxSize) {
                    if (size.x > xsize->max_width)
                        size.x = xsize->max_width;
                    if (size.y > xsize->max_height)
                        size.y = xsize->max_height;

                    if(size.x > wm->getXRes())
                        size.x = wm->getXRes();
                    if(size.y > wm->getYRes())
                        size.y = wm->getYRes();
                }

                drawOutline();
            }
        }
    }
}

void Client::drawOutline()
{
    if (!is_shaded) {
        XDrawRectangle(dpy, root, wm->getResources()->getGC(COLOR_BORDER_FOCUSED),
                position.x + border_width/2, position.y - titleHeight() + border_width/2,
                size.x + border_width, size.y + titleHeight() + border_width);

        XDrawRectangle(dpy, root, wm->getResources()->getGC(COLOR_BORDER_FOCUSED),
                position.x + border_width/2 + 4, position.y - titleHeight() + border_width/2 + 4,
                size.x + border_width - 8, size.y + titleHeight() + border_width - 8);
    }
    else {
        XDrawRectangle(dpy, root, wm->getResources()->getGC(COLOR_BORDER_FOCUSED),
                position.x + border_width/2,
                position.y - titleHeight() + border_width/2,
                size.x + border_width,
                titleHeight() + border_width);
    }
}

int Client::getIncsize(int *x_ret, int *y_ret, int mode)
{
    int basex, basey;

    if (xsize->flags & PResizeInc) {
        basex = (xsize->flags & PBaseSize) ? xsize->base_width :
                (xsize->flags & PMinSize) ? xsize->min_width : 0;

        basey = (xsize->flags & PBaseSize) ? xsize->base_height :
                (xsize->flags & PMinSize) ? xsize->min_height : 0;

        if (mode == 0) {
            *x_ret = size.x - ((size.x - basex) % xsize->width_inc);
            *y_ret = size.y - ((size.y - basey) % xsize->height_inc);
        }
        else { // INCREMENTS
            *x_ret = (size.x - basex) / xsize->width_inc;
            *y_ret = (size.y - basey) / xsize->height_inc;
        }
        return 1;
    }
    return 0;
}

void Client::shade()
{
    XRaiseWindow(dpy, frame);

    if(! is_shaded) {
        XResizeWindow(dpy, frame, size.x, titleHeight() - 1);
        is_shaded=true;
    }
    else {
        XResizeWindow(dpy, frame, size.x, size.y + titleHeight());
        is_shaded=false;
    }
}

void Client::handleConfigureRequest(XConfigureRequestEvent *e)
{
        gravitate(REMOVE_GRAVITY);
        if (e->value_mask & CWX) position.x = e->x;
        if (e->value_mask & CWY) position.y = e->y;
        if (e->value_mask & CWWidth) size.x = e->width;
        if (e->value_mask & CWHeight) size.y = e->height;
        gravitate(APPLY_GRAVITY);

    if(!is_shaded) {
        XMoveResizeWindow(dpy, frame, position.x, position.y - titleHeight(), size.x, size.y + titleHeight());
        XResizeWindow(dpy, title, size.x, titleHeight());
        XMoveResizeWindow(dpy, getAppWindow(),0,titleHeight(), size.x, size.y);
    }

    if ( (position.x + size.x > wm->getXRes()) ||
        (size.y + titleHeight() > wm->getYRes()) ||
        (position.x > wm->getXRes()) ||
        (position.y > wm->getYRes()) ||
        (position.x < 0) ||
        (position.y < 0)
    )
    initPosition();

    if (e->value_mask & (CWWidth|CWHeight)) {
        setShape();
    }
}

void Client::handleMapRequest(XMapRequestEvent *e)
{
    unhide();
}

void Client::handleUnmapEvent(XUnmapEvent *e)
{
    if (!ignore_unmap) {
        delete this;
    }
}

void Client::handleDestroyEvent(XDestroyWindowEvent *e)
{
    delete this;
}

void Client::handleClientMessage(XClientMessageEvent *e)
{
    /*if (e->message_type == wm->getWMChangeStateAtom() && e->format == 32 && e->data.l[0] == IconicState)
        iconify();*/
}

void Client::handlePropertyChange(XPropertyEvent *e)
{
    switch (e->atom) {
        case XA_WM_NAME:
            getXClientName(windows[current_window]);
            XClearWindow(dpy, title);
            redraw();
            break;
        case XA_WM_NORMAL_HINTS:
            long dummy;
            XGetWMNormalHints(dpy, getAppWindow(), xsize, &dummy);
            break;
    }
}

void Client::reparent()
{
    XGrabServer(dpy);

    XSetWindowAttributes pattr;
    pattr.background_pixel = wm->getResources()->getColor(COLOR_BACKGROUND_FOCUSED).pixel;
    pattr.border_pixel = wm->getResources()->getColor(COLOR_BORDER_FOCUSED).pixel;
    pattr.do_not_propagate_mask = ButtonPressMask|ButtonReleaseMask|ButtonMotionMask;
    pattr.override_redirect=False;
    pattr.event_mask = ButtonMotionMask|
            SubstructureRedirectMask|
            SubstructureNotifyMask|
            ButtonPressMask|
            ButtonReleaseMask|
            ExposureMask|
            EnterWindowMask|
            LeaveWindowMask;

    int b_w = 1;
    if(border_width) {
        b_w = border_width;
        XSetWindowBorderWidth(dpy, getAppWindow(), 0);
    }

    frame = XCreateWindow(
            dpy,
            root,
            position.x,
            position.y - titleHeight(),
            size.x,
            size.y + titleHeight(),
            b_w,
            DefaultDepth(dpy, screen),
            CopyFromParent,
            DefaultVisual(dpy, screen),
            CWOverrideRedirect|CWDontPropagate|CWBackPixel|CWBorderPixel|CWEventMask,
            &pattr
    );

    title = XCreateWindow(
            dpy,
            frame,
            0,
            0,
            size.x,
            titleHeight(),
            0,
            DefaultDepth(dpy,  screen),
            CopyFromParent,
            DefaultVisual(dpy,  screen),
            CWOverrideRedirect|CWDontPropagate|CWBackPixel|CWBorderPixel|CWEventMask,
            &pattr
    );

    if (wm->getShape()) {
        XShapeSelectInput(dpy, getAppWindow(), ShapeNotifyMask);
        setShape();
    }

    XChangeWindowAttributes(dpy, getAppWindow(), CWDontPropagate, &pattr);
    XSelectInput(dpy, getAppWindow(), FocusChangeMask|PropertyChangeMask);
    XReparentWindow(dpy, getAppWindow(), frame, 0, titleHeight());
    XGrabButton(dpy, Button1, AnyModifier, frame, 1,  ButtonPressMask|ButtonReleaseMask, GrabModeSync, GrabModeAsync, None, None);

    sendConfig();

    XSync(dpy, false);
    XUngrabServer(dpy);
}

void Client::handleButtonEvent(XButtonEvent *e)
{
    int in_box;

    in_box = (e->x >= size.x - titleHeight()) && (e->y <= titleHeight());

    // Used to compute the pointer position on click
    // used in the motion handler when doing a window move.
    old_cursor = position;
    cursor.x = e->x_root;
    cursor.y = e->y_root;

    // Allow us to get clicks from anywhere on the window
    // so click to raise works.
    XAllowEvents(dpy, ReplayPointer, CurrentTime);

    switch (e->button)
    {
        case Button1: {
            if (e->type == ButtonPress) {
                if(e->window == getAppWindow() || e->subwindow == getAppWindow()) {
                    XRaiseWindow(dpy, frame);
                }
                if (e->window == title) {
                    if (in_box)
                        wm->sendWMDelete(getAppWindow());
                    else
                        XRaiseWindow(dpy, frame);
                }
            }

            if (e->type == ButtonRelease) {
                if(is_being_dragged) {
                    is_being_dragged=false;
                    do_drawoutline_once=false;
                    drawOutline();
                    XMoveWindow(dpy, frame, position.x, position.y - titleHeight());
                    sendConfig();

                    XUngrabServer(dpy);
                    XSync(dpy, False);
                }

                // Check for a double click then maximize the window.
                if(e->time-last_button1_time<250) {
                    maximize();
                    last_button1_time = 0;
                    return;
                }
                else {
                    last_button1_time=e->time;
                }
            }

        }
        break;

        case Button2: {
            if(e->window == title) {
                if(in_box) {
                    if(e->type == ButtonPress) {
                        if(is_shaded)
                            shade();
                        XRaiseWindow(dpy, frame);
                    }
                }
            }

            if(e->type == ButtonRelease) {
                shade();
            }
        }
        break;

        case Button3: {
            if(e->window == title) {
                if (e->type == ButtonRelease) {
                    if(is_being_resized) {
                        drawOutline();
                        do_drawoutline_once=false;
                        is_being_resized=false;

                        XResizeWindow(dpy, frame, size.x, size.y + titleHeight());
                        XResizeWindow(dpy, title, size.x, titleHeight());
                        XResizeWindow(dpy, getAppWindow(), size.x, size.y);

                        sendConfig();

                        XUngrabServer(dpy);
                        XSync(dpy, False);

                        return;
                    }
                }
            }
        }
        break;
    }
}

void Client::handleEnterEvent(XCrossingEvent *e)
{
    XSetInputFocus(dpy, getAppWindow(), RevertToNone, CurrentTime);
}

void Client::handleFocusInEvent(XFocusChangeEvent *e)
{
    wm->sendXMessage(getAppWindow(), wm->getWMProtosAtom(), SubstructureRedirectMask, wm->getWMTakeFocusAtom());
    XInstallColormap(dpy, cmap);
    setFocus(true);
}

void Client::setFocus(bool focus)
{
    has_focus=focus;

    if (has_title) {
        if(has_focus) {
            XSetWindowBackground(dpy, title, wm->getResources()->getColor(COLOR_BACKGROUND_FOCUSED).pixel);
            XSetWindowBorder(dpy, frame, wm->getResources()->getColor(COLOR_BORDER_FOCUSED).pixel);
        }
        else {
            XSetWindowBackground(dpy, title, wm->getResources()->getColor(COLOR_BACKGROUND_UNFOCUSED).pixel);
            XSetWindowBorder(dpy, frame, wm->getResources()->getColor(COLOR_BORDER_UNFOCUSED).pixel);
        }
        XClearWindow(dpy, title);
        redraw();
    }
}

void Client::handleColormapChange(XColormapEvent *e)
{
    if (e->c_new) {
        cmap = e->colormap;
        XInstallColormap(dpy, cmap);
    }
}

void Client::handleExposeEvent(XExposeEvent *e)
{
    if (e->count == 0) {
        redraw();
    }
}

void Client::handleShapeChange(XShapeEvent *e)
{
    setShape();
}

void Client::setTag(char tag)
{
    tags.clear();
    tags.insert(tag);

    if(!isTagged(wm->getCurrentWorkspace())) {
        hide();
    }
}

void Client::addTag(char tag)
{
    tags.insert(tag);

    if(!isTagged(wm->getCurrentWorkspace())) {
        hide();
    }
}

void Client::removeTag(char tag)
{
    if (tags.size() > 1) {
        set<char>::iterator item = tags.find(tag);
        tags.erase(item);

        if(!isTagged(wm->getCurrentWorkspace())) {
            hide();
        }
    }
}

void Client::toggleTag(char tag)
{
    if (isTagged(tag))
        removeTag(tag);
    else
        addTag(tag);
}