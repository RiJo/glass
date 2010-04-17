/*******************************************************************************
 * Description:
 *   Common data to be used by all other classes. Simple modifications of the
 *   windowmanager can be done here, such as colors, keybindings and definition
 *   of executables.
 ******************************************************************************/

#ifndef _GLASS_H_
#define _GLASS_H_

#include "point.h"

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
#define PROGRAM_VERSION             "0.8.0"
#define PROGRAM_DATE                "2010-04-10"

#define EXEC_TERMINAL               (char *)"xterm -geometry 110x30"
#define EXEC_WEBBROWSER             (char *)"firefox &> /dev/null"
#define EXEC_EDITOR                 (char *)"scite &> /dev/null"

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
#define INITIAL_WORKSPACE           1
#define RANDOM_WINDOW_PLACEMENT     false
#define WIRE_MOVE                   false
#define EDGE_SNAP                   5
#define TITLE_MINIMUM_HEIGHT        15

#ifdef _DEBUG_
#define DEBUG printf("[debug] ");printf
#else
#define DEBUG(arg1,...)
#endif



#endif // _GLASS_H_
