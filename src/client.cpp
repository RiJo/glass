#include "client.h"

Client::Client(Display *d, Window new_client, character *c)
{
    printf("New window: pid: %ld\tposition: %d:%d\tsize: %d:%d\n", (long)c->pid, c->position.x, c->position.y, c->size.x, c->size.y);
    initialize(d, c);
    wm->addClient(this);
    makeNewClient(new_client);
}

Client::~Client()
{
    removeClient();
}

void Client::initialize(Display *d, character *c)
{
    dpy                     = d;

    name                    = NULL;

    window                  = None;
    frame                   = None;
    title                   = None;
    trans                   = None;

    x                       = 1;
    y                       = 1;
    width                   = 1;
    height                  = 1;
    ignore_unmap            = 0;

    pointer_x               = 0;
    pointer_y               = 0;

    old_cx                  = 0;
    old_cy                  = 0;

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
    old_x                   = 0;
    old_y                   = 0;
    old_width               = 1;
    old_height              = 1;

    direction               = 0;
    ascent                  = 0;
    descent                 = 0;
    text_width              = 0;
//    text_justify            = 0;
//    justify_style           = RIGHT_JUSTIFY;

    screen                  = DefaultScreen(dpy);
    root                    = RootWindow(dpy, screen);
}

void Client::getXClientName()
{
    if(name) {
        XFree(name);
    }

    XFetchName(dpy, window, &name);

    if(name) {
        XTextExtents(wm->getResources()->getFont(FONT_NORMAL), name , strlen(name),
                &direction, &ascent, &descent, &overall);
        text_width = overall.width;
    }
}

void Client::makeNewClient(Window w)
{
    XWindowAttributes attr;
    XWMHints *hints;

    XGrabServer(dpy);

    window = w;

    getXClientName();

    XGetTransientForHint(dpy, window, &trans);
    XGetWindowAttributes(dpy, window, &attr);

    x = attr.x;
    y = attr.y;
    width = attr.width;
    height = attr.height;
    border_width = attr.border_width;
    cmap = attr.colormap;
    size = XAllocSizeHints();

    long dummy;
    XGetWMNormalHints(dpy, window, size, &dummy);

    old_x = x;
    old_y = y;
    old_width = width;
    old_height = height;

    if (attr.map_state == IsViewable) {
        ignore_unmap++;
        initPosition();

        if ((hints = XGetWMHints(dpy, w))) {
            if (hints->flags & StateHint)
                wm->setWMState(window, hints->initial_state);
            else wm->setWMState(window, NormalState);
                XFree(hints);
        }
    }

    gravitate(APPLY_GRAVITY);
    reparent();

    tags.insert(wm->getCurrentWorkspace());

    unhide();

    XSetInputFocus(dpy, window, RevertToNone, CurrentTime);

    XSync(dpy, False);
    XUngrabServer(dpy);
}

void Client::removeClient()
{
    XGrabServer(dpy);

    if(trans) XSetInputFocus(dpy, trans, RevertToNone, CurrentTime);

    XUngrabButton(dpy, AnyButton, AnyModifier, frame);

    gravitate(REMOVE_GRAVITY);

    XReparentWindow(dpy, window, root, x, y);

    XDestroyWindow(dpy, title);
    XDestroyWindow(dpy, frame);

    if (name) {
        XFree(name);
    }

    if (size) {
        XFree(size);
    }

    XSync(dpy, False);
    XUngrabServer(dpy);

    wm->removeClient(this);
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
    ce.event = window;
    ce.window = window;
    ce.x = x;
    ce.y = y;
    ce.width = width;
    ce.height = height;
    ce.border_width = border_width;
    //ce.above = (is_always_on_top) ? Above : Below;
    ce.override_redirect = False;

    XSendEvent(dpy, window, False, StructureNotifyMask, (XEvent *)&ce);
}

void Client::redraw()
{
    if (!has_title)
        return;

    // Title decoration
    GC gc;
    if(has_focus)
        gc = wm->getResources()->getGC(COLOR_DECORATION_FOCUSED);
    else
        gc = wm->getResources()->getGC(COLOR_DECORATION_UNFOCUSED);

    XDrawLine(dpy, title, gc, 0, titleHeight() - border_width, width, titleHeight() - border_width);
    XDrawLine(dpy, title, gc, width - titleHeight(), 0, width - titleHeight(), titleHeight());

    // Title text
    if(has_focus)
        gc = wm->getResources()->getGC(COLOR_FOREGROUND_FOCUSED);
    else
        gc = wm->getResources()->getGC(COLOR_FOREGROUND_UNFOCUSED);

    if (!trans && name) {
        //~ switch(justify_style) {
            //~ case LEFT_JUSTIFY:
                //~ text_justify = TITLE_MINIMUM_HEIGHT;
            //~ break;

            //~ case CENTER_JUSTIFY:
                //~ text_justify = ( (width / 2) - (text_width / 2) );
            //~ break;

            //~ case RIGHT_JUSTIFY:
                //~ text_justify = width - text_width - 25;
            //~ break;
        //~ }

        if(name != NULL) {
            int text_justify = width - text_width - 25;
            XDrawString(dpy, title, gc, text_justify, wm->getResources()->getFont(FONT_NORMAL)->ascent+1,name, strlen(name));
        }
    }
}

void Client::gravitate(int multiplier)
{
        int dy = 0;
        int gravity = (size->flags & PWinGravity) ? size->win_gravity : NorthWestGravity;

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
        y += multiplier * dy;
}

void Client::setShape()
{
    int n=0, order=0;
    XRectangle temp, *dummy;

    dummy = XShapeGetRectangles(dpy, window, ShapeBounding, &n, &order);

    if (n > 1) {
        XShapeCombineShape(dpy, frame, ShapeBounding,
                0, titleHeight(), window, ShapeBounding, ShapeSet);

        temp.x = -border_width;
        temp.y = -border_width;
        temp.width = width + 2 * border_width;
        temp.height = titleHeight() + border_width;

        XShapeCombineRectangles(dpy, frame, ShapeBounding,
                0, 0, &temp, 1, ShapeUnion, YXBanded);

        temp.x = 0;
        temp.y = 0;
        temp.width = width;
        temp.height = titleHeight() - border_width;

        XShapeCombineRectangles(dpy, frame, ShapeClip,
                0, titleHeight(), &temp, 1, ShapeUnion, YXBanded);

        has_been_shaped = 1;
    }
    else if (has_been_shaped) {
        temp.x = -border_width;
        temp.y = -border_width;
        temp.width = width + 2 * border_width;
        temp.height = height + titleHeight() + 2 * border_width;

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

    wm->setWMState(window, WithdrawnState);

    is_visible = false;
}

void Client::unhide()
{
    if(isTagged(wm->getCurrentWorkspace())) {
        if(trans) {
            wm->findTransientsToMapOrUnmap(window, false);
        }

        XMapSubwindows(dpy, frame);
        XMapRaised(dpy, frame);

        wm->setWMState(window, NormalState);

        XSetInputFocus(dpy, window, RevertToNone, CurrentTime);

        is_visible = true;
    }
}

void Client::initPosition()
{
    int mouse_x, mouse_y;

    unsigned int w, h;
    unsigned int border_width, depth;

    XWindowAttributes attr;

    XGetWindowAttributes(dpy, window, &attr);

    if (attr.map_state == IsViewable)
        return;

    XGetGeometry(dpy, window, &root, &x, &y, &w, &h, &border_width, &depth);

    width = (int)w;
    height = (int)h;

    if (size->flags & PPosition) {
            if(!x) x = size->x;
            if(!y) y = size->y;
    }
    else {
        if (size->flags & USPosition) {
            if(!x)
                x = size->x;
            if(!y)
                y = size->y;
        }
        else if ( (x==0) || (y==0)  ) {
            if(width>=wm->getXRes() && height>=wm->getYRes()) {
                x=0;
                y=0;
                width=wm->getXRes();
                height=wm->getYRes()-titleHeight();
            }
            else {
                wm->getMousePosition(&mouse_x, &mouse_y);

                if(mouse_x && mouse_y) {

                    if (wm->getRandPlacement()) {
                        x = (rand() % (unsigned int) ((wm->getXRes() - width) * 0.94)) + ((unsigned int) (wm->getXRes() * 0.03));
                        y = (rand() % (unsigned int) ((wm->getYRes() - height) * 0.94)) + ((unsigned int) (wm->getYRes() * 0.03));
                    }
                    else {
                        x = (int) (((long) (wm->getXRes() - width)
                                    * (long) mouse_x) / (long) wm->getXRes());
                        y = (int) (((long) (wm->getYRes() - height - titleHeight())
                                    * (long) mouse_y) / (long) wm->getYRes());
                        y = (y<titleHeight()) ? titleHeight() : y;
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
        old_x=x;
        old_y=y;
        old_width=width;
        old_height=height;

        if (size->flags & PMaxSize) {
            width = size->max_width;
            height = size->max_height;

            XMoveResizeWindow(dpy, frame, x, y-titleHeight(), width, height+titleHeight());

        }
        else {
            x=0;
            y=0;
            width=wm->getXRes()-2;
            height=wm->getYRes()-2;

            XMoveResizeWindow(dpy, frame, x, y, width, height);

            y = titleHeight();
            height -= titleHeight();
        }
        is_maximized=true;
    }
    else {
        x = old_x;
        y = old_y;
        width = old_width;
        height = old_height;

        XMoveResizeWindow(dpy, frame, old_x, old_y - titleHeight(), old_width, old_height + titleHeight());

        is_maximized = false;

        if(is_shaded)
            is_shaded = false;
    }

    XResizeWindow(dpy, title, width, titleHeight());
    XResizeWindow(dpy, window, width, height);

    sendConfig();
}

void Client::handleMotionNotifyEvent(XMotionEvent *ev)
{
    int nx=0, ny=0;

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

            nx = old_cx + (ev->x_root - pointer_x);
            ny = old_cy + (ev->y_root - pointer_y);

            int snap_width = wm->getEdgeSnap();
            if(snap_width) {
                // Move beyond edges of screen
                if (nx == wm->getXRes() - width)
                    nx = wm->getXRes() - width + 1;
                else if (nx == 0)
                    nx = -1;

                if (ny == wm->getYRes() - snap_width)
                    ny = wm->getYRes() - snap_width - 1;
                else if (ny == titleHeight())
                    ny = titleHeight() - 1;

                // Snap to edges of screen
                if ( (nx + width >= wm->getXRes() - snap_width) && (nx + width <= wm->getXRes()) )
                    nx = wm->getXRes() - width;
                else if ( (nx <= snap_width) && (nx >= 0) )
                    nx = 0;

                if (is_shaded) {
                    if ( (ny  >= wm->getYRes() - snap_width) && (ny  <= wm->getYRes()) )
                        ny = wm->getYRes();
                    else if ( (ny - titleHeight() <= snap_width) && (ny - titleHeight() >= 0))
                        ny = titleHeight();
                }
                else {
                    if ( (ny + height >= wm->getYRes() - snap_width) && (ny + height <= wm->getYRes()) )
                        ny = wm->getYRes() - height;
                    else if ( (ny - titleHeight() <= snap_width) && (ny - titleHeight() >= 0))
                        ny = titleHeight();
                }
            }

            x = nx;
            y = ny;

            if(!wm->getWireMove()) {
                XMoveWindow(dpy, frame, nx, ny-titleHeight());
                sendConfig();
            }

            if(wm->getWireMove()) {
                drawOutline();
            }
    }
    else if(ev->state & Button3Mask) {
        if(! is_being_resized) {
            int in_box = (ev->x >= width - titleHeight()) && (ev->y <= titleHeight());

            if(! in_box)
                return;
        }

        if(! do_drawoutline_once) {
            XGrabServer(dpy);
            is_being_resized=true;
            do_drawoutline_once=true;
            drawOutline();
            XWarpPointer(dpy, None, frame, 0, 0, 0, 0, width, height+titleHeight());
        }
        else {
            if((ev->x > 50) && (ev->y > 50)) {
                drawOutline();

                width =  ev->x;
                height = ev->y - titleHeight();

                int pixels = 0; // what is this? old enum...
                getIncsize(&width, &height, pixels);

                if (size->flags & PMinSize) {
                    if (width < size->min_width)
                        width = size->min_width;
                    if (height < size->min_height)
                        height = size->min_height;

                    if(width < 100)
                        width = 100;
                    if(height < 50)
                        height = 50;
                }

                if (size->flags & PMaxSize) {
                    if (width > size->max_width)
                        width = size->max_width;
                    if (height > size->max_height)
                        height = size->max_height;

                    if(width > wm->getXRes())
                        width = wm->getXRes();
                    if(height > wm->getYRes())
                        height = wm->getYRes();
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
                x + border_width/2, y - titleHeight() + border_width/2,
                width + border_width, height + titleHeight() + border_width);

        XDrawRectangle(dpy, root, wm->getResources()->getGC(COLOR_BORDER_FOCUSED),
                x + border_width/2 + 4, y - titleHeight() + border_width/2 + 4,
                width + border_width - 8, height + titleHeight() + border_width - 8);
    }
    else {
        XDrawRectangle(dpy, root, wm->getResources()->getGC(COLOR_BORDER_FOCUSED),
                x + border_width/2,
                y - titleHeight() + border_width/2,
                width + border_width,
                titleHeight() + border_width);
    }
}

int Client::getIncsize(int *x_ret, int *y_ret, int mode)
{
    int basex, basey;

    if (size->flags & PResizeInc) {
        basex = (size->flags & PBaseSize) ? size->base_width :
                (size->flags & PMinSize) ? size->min_width : 0;

        basey = (size->flags & PBaseSize) ? size->base_height :
                (size->flags & PMinSize) ? size->min_height : 0;

        if (mode == 0) {
            *x_ret = width - ((width - basex) % size->width_inc);
            *y_ret = height - ((height - basey) % size->height_inc);
        }
        else { // INCREMENTS
            *x_ret = (width - basex) / size->width_inc;
            *y_ret = (height - basey) / size->height_inc;
        }
        return 1;
    }
    return 0;
}

void Client::shade()
{
    XRaiseWindow(dpy, frame);

    if(! is_shaded) {
        XResizeWindow(dpy, frame, width, titleHeight() - 1);
        is_shaded=true;
    }
    else {
        XResizeWindow(dpy, frame, width, height + titleHeight());
        is_shaded=false;
    }
}

void Client::handleConfigureRequest(XConfigureRequestEvent *e)
{
        gravitate(REMOVE_GRAVITY);
        if (e->value_mask & CWX) x = e->x;
        if (e->value_mask & CWY) y = e->y;
        if (e->value_mask & CWWidth) width = e->width;
        if (e->value_mask & CWHeight) height = e->height;
        gravitate(APPLY_GRAVITY);

    if(!is_shaded) {
        XMoveResizeWindow(dpy, frame,x,y-titleHeight(), width, height+titleHeight());
        XResizeWindow(dpy, title, width, titleHeight());
        XMoveResizeWindow(dpy, window,0,titleHeight(),width,height);
    }

    if ( (x + width > wm->getXRes())         ||
        (height + titleHeight() > wm->getYRes())     ||
        (x > wm->getXRes())             ||
        (y > wm->getYRes())            ||
        (x < 0)            ||
        (y < 0)
    )
        initPosition();

    if (e->value_mask & (CWWidth|CWHeight))
        setShape();
}

void Client::handleMapRequest(XMapRequestEvent *e)
{
    unhide();
}

void Client::handleUnmapEvent(XUnmapEvent *e)
{
    if (! ignore_unmap)
        delete this;
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
    long dummy;

    switch (e->atom)
    {
            case XA_WM_NAME:
                getXClientName();
                XClearWindow(dpy, title);
                redraw();
                break;

            case XA_WM_NORMAL_HINTS:
                    XGetWMNormalHints(dpy, window, size, &dummy);
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
    pattr.event_mask = ButtonMotionMask        |
                SubstructureRedirectMask    |
                SubstructureNotifyMask    |
                ButtonPressMask        |
                ButtonReleaseMask        |
                ExposureMask            |
                EnterWindowMask         |
                LeaveWindowMask        ;

    int b_w = 1;
    if(border_width) {
        b_w = border_width; XSetWindowBorderWidth(dpy, window, 0);
    }

    frame = XCreateWindow(
            dpy,
            root,
            x,
            y - titleHeight(),
            width,
            height + titleHeight(),
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
            width,
            titleHeight(),
            0,
            DefaultDepth(dpy,  screen),
            CopyFromParent,
            DefaultVisual(dpy,  screen),
            CWOverrideRedirect|CWDontPropagate|CWBackPixel|CWBorderPixel|CWEventMask,
            &pattr
    );

    if (wm->getShape()) {
        XShapeSelectInput(dpy, window, ShapeNotifyMask);
        setShape();
    }

    XChangeWindowAttributes(dpy, window, CWDontPropagate, &pattr);

    XSelectInput(dpy, window, FocusChangeMask|PropertyChangeMask);

    XReparentWindow(dpy, window, frame, 0, titleHeight());

    XGrabButton(dpy, Button1, AnyModifier, frame, 1,  ButtonPressMask|ButtonReleaseMask,
            GrabModeSync, GrabModeAsync, None, None);

    sendConfig();

    XSync(dpy, false);
    XUngrabServer(dpy);
}

void Client::handleButtonEvent(XButtonEvent *e)
{
    int in_box;

    in_box = (e->x >= width - titleHeight()) && (e->y <= titleHeight());

    // Used to compute the pointer position on click
    // used in the motion handler when doing a window move.
    old_cx = x;
    old_cy = y;
    pointer_x = e->x_root;
    pointer_y = e->y_root;

    // Allow us to get clicks from anywhere on the window
    // so click to raise works.
    XAllowEvents(dpy, ReplayPointer, CurrentTime);

    switch (e->button)
    {
        case Button1: {
            if (e->type == ButtonPress) {
                if(e->window == window || e->subwindow == window) {
                    XRaiseWindow(dpy, frame);
                }

                if (e->window == title) {
                    if (in_box)
                        wm->sendWMDelete(window);
                    else
                        XRaiseWindow(dpy, frame);
                }
            }

            if (e->type == ButtonRelease) {
                if(is_being_dragged) {
                    is_being_dragged=false;
                    do_drawoutline_once=false;
                    drawOutline();
                    XMoveWindow(dpy, frame, x, y-titleHeight());
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

                        XResizeWindow(dpy, frame, width, height + titleHeight());
                        XResizeWindow(dpy, title, width, titleHeight());
                        XResizeWindow(dpy, window, width, height);

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
    XSetInputFocus(dpy, window, RevertToNone, CurrentTime);
}

void Client::handleFocusInEvent(XFocusChangeEvent *e)
{
    wm->sendXMessage(window, wm->getWMProtosAtom(), SubstructureRedirectMask,
            wm->getWMTakeFocusAtom());
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