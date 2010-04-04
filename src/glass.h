#ifndef _GLASS_H_
#define _GLASS_H_

/*
    TODO:
        * change all x and y to point (or similar)
        * create own class for point, with calculations
        * dynamic/static allocations of all strings (EXEC_*. etc...)
        * fix command_line in windowmanager.h

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

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
#define EXEC_WEBBROWSER             "firefox &> /dev/null"
#define EXEC_EDITOR                 "scite"

#define MODIFIER_GOTO_WORKSPACE     Mod4Mask
#define MODIFIER_TAG_CLIENT         (ControlMask|Mod1Mask)

#define FONT_DEFAULT                (char *)"Fixed"

#define COLOR_TITLE_FG_FOCUS        (char *)"#dddddd"
#define COLOR_TITLE_FG_UNFOCUS      (char *)"#bbbbbb"
#define COLOR_TITLE_BG_FOCUS        (char *)"#000000"
#define COLOR_TITLE_BG_UNFOCUS      (char *)"#000000"
#define COLOR_TITLE_BD_FOCUS        (char *)"#333333"
#define COLOR_TITLE_BD_UNFOCUS      (char *)"#333333"
#define COLOR_CLIENT_BD_FOCUS       (char *)"#ff0000"
#define COLOR_CLIENT_BD_UNFOCUS     (char *)"#333333"

#define DEFAULT_WORKSPACE_COUNT     4
#define WIRE_MOVE                   false
#define EDGE_SNAP                   true


#define SPACE                       3
#define MINSIZE                     15
#define SNAP                        5
#define TEXT_JUSTIFY                "right"
#define TRANSIENT_WINDOW_HEIGHT     8

#ifdef _DEBUG_
#define DEBUG printf("[debug] ");printf
#else
#define DEBUG(arg1,...)
#endif



#endif // _GLASS_H_
