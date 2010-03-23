#ifndef _RESOURCES_H_
#define _RESOURCES_H_



#include <X11/cursorfont.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>
#include <X11/extensions/shape.h>
#include <stdlib.h>
#include <iostream>
#include <map>

using namespace std;

#define FONT_DEFAULT                (char *)"Fixed"

#define COLOR_TITLE_FG_FOCUS        (char *)"#dddddd"
#define COLOR_TITLE_FG_UNFOCUS      (char *)"#bbbbbb"
#define COLOR_TITLE_BG_FOCUS        (char *)"#000000"
#define COLOR_TITLE_BG_UNFOCUS      (char *)"#000000"
#define COLOR_TITLE_BD_FOCUS        (char *)"#333333"
#define COLOR_TITLE_BD_UNFOCUS      (char *)"#333333"
#define COLOR_CLIENT_BD_FOCUS       (char *)"#ff0000"
#define COLOR_CLIENT_BD_UNFOCUS     (char *)"#333333"

enum COLOR {
    COLOR_FOREGROUND_FOCUSED,
    COLOR_FOREGROUND_UNFOCUSED,
    COLOR_BACKGROUND_FOCUSED,
    COLOR_BACKGROUND_UNFOCUSED,
    COLOR_DECORATION_FOCUSED,
    COLOR_DECORATION_UNFOCUSED,
    COLOR_BORDER_FOCUSED,
    COLOR_BORDER_UNFOCUSED
};

enum CURSOR {
    CURSOR_MOVE,
    CURSOR_ARROW
};

enum FONT {
    FONT_NORMAL
};

class Resources
{
private: /* Member Variables */
    Display *dpy;
    Window root;
    int screen;

    map<COLOR, GC> gcs;
    map<COLOR, XColor> colors;
    map<CURSOR, Cursor> cursors;
    map<FONT, XFontStruct *> fonts;

private: /* Member Functions */
    void loadResources();
    void loadColor(COLOR, char *);
    void loadColors();
    void loadGC(COLOR);
    void loadGCs();
    void loadCursor(CURSOR, unsigned int);
    void loadCursors();
    void loadFont(FONT, char *);
    void loadFonts();


public: /* Member Functions */
    Resources(Display *, Window, int);
    ~Resources();

    XColor getColor(COLOR key)      { return colors[key]; }
    GC getGC(COLOR key)             { return gcs[key]; }
    Cursor getCursor(CURSOR key)    { return cursors[key]; }
    XFontStruct *getFont(FONT key)  { return fonts[key]; }
};


#endif