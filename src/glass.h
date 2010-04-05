/*******************************************************************************
 * Description:
 *   Generic data to be used by all other classes. Simple modifications of the
 *   windowmanager can be done here, such as colors, keybindings and definition
 *   of executables.
 ******************************************************************************/

#ifndef _GLASS_H_
#define _GLASS_H_

/*
    TODO:
      Minor:
        * comment all classes and explain their responsibilities
        * change all x and y to point (or similar)
        * create own class for point, with calculations
        * dynamic/static allocations of all strings (EXEC_*. etc...)
        * negative egde_snap: snap outside screen
        * add screenshot to repository
      Major
        * make foobar a window
        * notify icons in foobar
        * multiple windows in one client (grouping)
        * autocomplete in runfield
      Bugs
        * glass does not terminate if there is clients alive
        * wire move does not redraw properly

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
#define PROGRAM_VERSION             "0.7.0"
#define PROGRAM_DATE                "2010-04-05"

#define EXEC_TERMINAL               (char *)"xterm -geometry 100x30"
#define EXEC_WEBBROWSER             (char *)"firefox &> /dev/null"
#define EXEC_EDITOR                 (char *)"scite"

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
#define EDGE_SNAP                   5
#define TITLE_MINIMUM_HEIGHT        15

#ifdef _DEBUG_
#define DEBUG printf("[debug] ");printf
#else
#define DEBUG(arg1,...)
#endif



#endif // _GLASS_H_
