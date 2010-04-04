#ifndef _RESOURCES_H_
#define _RESOURCES_H_

#include "glass.h"

#include <map>

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