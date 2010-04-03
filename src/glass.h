#ifndef _GLASS_H_
#define _GLASS_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <getopt.h> /* getopt_long() */
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

#define PROGRAM_NAME                "glass"
#define PROGRAM_VERSION             "0.6.0"
#define PROGRAM_DATE                "2010-04-03"

#define EXEC_TERMINAL               "xterm -geometry 100x30"
#define EXEC_WEBBROWSER             "firefox"
#define EXEC_EDITOR                 "scite"

#define FONT_DEFAULT                (char *)"Fixed"

#define COLOR_TITLE_FG_FOCUS        (char *)"#dddddd"
#define COLOR_TITLE_FG_UNFOCUS      (char *)"#bbbbbb"
#define COLOR_TITLE_BG_FOCUS        (char *)"#000000"
#define COLOR_TITLE_BG_UNFOCUS      (char *)"#000000"
#define COLOR_TITLE_BD_FOCUS        (char *)"#333333"
#define COLOR_TITLE_BD_UNFOCUS      (char *)"#333333"
#define COLOR_CLIENT_BD_FOCUS       (char *)"#ff0000"
#define COLOR_CLIENT_BD_UNFOCUS     (char *)"#333333"

#define SPACE                       3
#define MINSIZE                     15
#define EDGE_SNAP                   "true"
#define SNAP                        5
#define TEXT_JUSTIFY                "right"
#define WIRE_MOVE                   "false"
#define DEFAULT_WORKSPACE_COUNT     4
#define TRANSIENT_WINDOW_HEIGHT     8

#ifdef _DEBUG_
#define DEBUG printf("[debug] ");printf
#else
#define DEBUG(arg1,...)
#endif

// Border width accessor to handle hints/no hints
#define BW                          (has_border ? 1 : 0)

enum { LEFT_JUSTIFY, CENTER_JUSTIFY, RIGHT_JUSTIFY };
enum { APPLY_GRAVITY=1, REMOVE_GRAVITY=-1 };
enum { PIXELS=0, INCREMENTS=1 };

void forkExec(char *);
int handleXError(Display *, XErrorEvent *);

#include "client.h"


#endif // _GLASS_H_
