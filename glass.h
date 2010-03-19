#ifndef _GLASS_H_
#define _GLASS_H_

#define WINDOW_MANAGER_NAME "aewm++"
#define VERSION "xxx"
#define DATE "28 December 2008"

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <list>
#include <set>

#include <X11/cursorfont.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>
#include <X11/extensions/shape.h>

using namespace std;

#define DEF_FONT        "Fixed"
#define DEF_FG          "#ffffff"
#define DEF_BG          "#000000"
#define DEF_FC          "#000000" // unfocused title bg
#define DEF_BD          "#555555" // title border

#define FOCUSED_BORDER_COLOR "#ff0000" // window border
#define UNFOCUSED_BORDER_COLOR "#555555" // window border
#define FOCUSED_WINDOW_TITLE_COLOR "#dddddd"

#define DEF_NEW1        "xterm -ls -sb -bg white -fg black"
#define DEF_BW          1
#define SPACE           3
#define MINSIZE         15
#define EDGE_SNAP       "true"
#define SNAP            5
#define TEXT_JUSTIFY    "right"
#define WIRE_MOVE       "false"
#define DEFAULT_WORKSPACE_COUNT    4
#define DEF_FM          "click"
#define DEF_WP          "mouse"

#define TRANSIENT_WINDOW_HEIGHT 8

enum { LEFT_JUSTIFY, CENTER_JUSTIFY, RIGHT_JUSTIFY };
enum { FOCUS_FOLLOW, FOCUS_SLOPPY, FOCUS_CLICK };
enum { APPLY_GRAVITY=1, REMOVE_GRAVITY=-1 };
enum { PIXELS=0, INCREMENTS=1 };

// Border width accessor to handle hints/no hints
#define BW (has_border ? wm->getOptBW() : 0)

// defined in main.cc
void forkExec(char *);
int handleXError(Display *, XErrorEvent *);

class Client;

//#include "basemenu.h"
//#include "genericmenu.h"
//#include "windowmenu.h"
//#include "iconmenu.h"
#include "client.h"
#include "windowmanager.h"

#endif // _GLASS_H_
