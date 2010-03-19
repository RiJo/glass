/*
 * aewm++ - A small C++ window manager developed from aewm 0.9.6 around 2000
 *
 * Frank Hale
 * frankhale@gmail.com
 *
 * http://code.google.com/p/aewmpp/
 *
 * Date: 28 December 2008
 *
 * This code is released under the GPL license www.gnu.org
 *
 * See LICENSE.txt which is included with the source code files.
 */

#include "glass.h"

// Dunno where I ripped this from. Kudos to the author whoever he is!
void forkExec(char *cmd)
{
    if(! (strlen(cmd)>0)) return;

    pid_t pid = fork();

    switch (pid) {
        case 0:
            execlp("/bin/sh", "sh", "-c", cmd, NULL);
            cerr << "exec failed, cleaning up child" << endl;
            exit(1);
        case -1:
            cerr << "can't fork" << endl;
    }
}

int handleXError(Display *dpy, XErrorEvent *e)
{
    if (e->error_code == BadAccess && e->resourceid == RootWindow(dpy, DefaultScreen(dpy)) ) {
        cerr << "root window unavailable (maybe another wm is running?)" << endl;
        exit(1);
    }

    return 0;
}

int main(int argc, char **argv)
{
    WindowManager aewm(argc, argv);
    return 0;
}
