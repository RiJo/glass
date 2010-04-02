/*
    Copyright (C) 2010 Rikard Johansson

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    This program is forked from aewm++ (timestamped 28 December 2008).
    http://code.google.com/p/aewmpp
*/

#include "windowmanager.h"

void forkExec(char *cmd) {
    if (!cmd || strlen(cmd) == 0) {
        return;
    }

    DEBUG("executing: \"%s\"\n", cmd);

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Error: could not fork\n");
    }
    else if (pid == 0) {
        execlp("/bin/sh", "sh", "-c", cmd, NULL);
        fprintf(stderr, "Error: exec failed, cleaning up child\n");
        exit(EXIT_FAILURE);
    }
}

int handleXError(Display *dpy, XErrorEvent *e) {
    if (e->error_code == BadAccess && e->resourceid == RootWindow(dpy, DefaultScreen(dpy)) ) {
        fprintf(stderr, "Error: root window unavailable (maybe another wm is running?)\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int main(int argc, char **argv) {
    WindowManager wm(argc, argv);
    return 0;
}