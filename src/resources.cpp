#include "resources.h"

Resources::Resources(Display *d, Window w, int s)
{
    dpy = d;
    root = w;
    screen = s;
    
    loadResources();
}

Resources::~Resources() {
    // fonts
    for( map<FONT, XFontStruct *>::iterator iter = fonts.begin(); iter != fonts.end(); iter++) {
        cout << "freeing XFontStruct\n";
        XFreeFont(dpy, (*iter).second);
    }
    // gcs
    for( map<COLOR, GC>::iterator iter = gcs.begin(); iter != gcs.end(); iter++) {
        cout << "freeing GC\n";
        XFreeGC(dpy, (*iter).second);
    }
    // cursors
    for( map<CURSOR, Cursor>::iterator iter = cursors.begin(); iter != cursors.end(); iter++) {
        cout << "freeing Cursor\n";
        XFreeCursor(dpy, (*iter).second);
    }

    cout << "all X resources freed\n";
}

void Resources::loadResources()
{
    loadCursors();
    loadFonts();
    loadColors();
    loadGCs();
}

void Resources::loadColors()
{
    loadColor(COLOR_FOREGROUND_FOCUSED, COLOR_TITLE_FG_FOCUS);
    loadColor(COLOR_FOREGROUND_UNFOCUSED, COLOR_TITLE_FG_UNFOCUS);
    loadColor(COLOR_BACKGROUND_FOCUSED, COLOR_TITLE_BG_FOCUS);
    loadColor(COLOR_BACKGROUND_UNFOCUSED, COLOR_TITLE_BG_UNFOCUS);
    loadColor(COLOR_DECORATION_FOCUSED, COLOR_TITLE_BD_FOCUS);
    loadColor(COLOR_DECORATION_UNFOCUSED, COLOR_TITLE_BD_UNFOCUS);
    loadColor(COLOR_BORDER_FOCUSED, COLOR_CLIENT_BD_FOCUS);
    loadColor(COLOR_BORDER_UNFOCUSED, COLOR_CLIENT_BD_UNFOCUS);
}

void Resources::loadColor(COLOR key, char *color_code) {
    XColor dummyc;
    XAllocNamedColor(dpy, DefaultColormap(dpy, screen), color_code, &colors[key], &dummyc);
}

void Resources::loadGCs()
{
    loadGC(COLOR_FOREGROUND_FOCUSED);
    loadGC(COLOR_FOREGROUND_UNFOCUSED);
    loadGC(COLOR_BACKGROUND_FOCUSED);
    loadGC(COLOR_BACKGROUND_UNFOCUSED);
    loadGC(COLOR_DECORATION_FOCUSED);
    loadGC(COLOR_DECORATION_UNFOCUSED);
    loadGC(COLOR_BORDER_FOCUSED);
    loadGC(COLOR_BORDER_UNFOCUSED);
}

void Resources::loadGC(COLOR key)
{
    XGCValues values;
    values.font = getFont(FONT_NORMAL)->fid;
    values.line_width = 1;
    values.foreground = getColor(key).pixel;
    gcs[key] = XCreateGC(dpy, root, GCForeground|GCFont|GCLineWidth, &values);

/*
    gv.function = GXcopy;
    gv.foreground = col_fg_focus.pixel;
    gv.font = font->fid;
    string_gc = XCreateGC(dpy, root, GCFunction|GCForeground|GCFont, &gv);

    gv.foreground = col_fg_focus.pixel;
    gv.function = GXinvert;
    gv.subwindow_mode = IncludeInferiors;
    invert_gc = XCreateGC(dpy, root, GCForeground|GCFunction|GCSubwindowMode|GCLineWidth|GCFont, &gv);
*/
}

void Resources::loadCursors()
{
    loadCursor(CURSOR_MOVE, XC_fleur);
    loadCursor(CURSOR_ARROW, XC_left_ptr);
}

void Resources::loadCursor(CURSOR key, unsigned int shape)
{
    cursors[key] = XCreateFontCursor(dpy, shape);
}

void Resources::loadFonts()
{
    loadFont(FONT_NORMAL, FONT_DEFAULT);
}

void Resources::loadFont(FONT key, char *font_name)
{
    fonts[key] = XLoadQueryFont(dpy, font_name);
    if (!fonts[key]) {
        cerr << "The font cannot be found, exiting..." << endl;
        exit(EXIT_FAILURE);
    }
}