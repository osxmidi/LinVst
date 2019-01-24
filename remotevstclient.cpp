/*  dssi-vst: a DSSI plugin wrapper for VST effects and instruments
    Copyright 2004-2007 Chris Cannam

    This file is part of linvst.

    linvst is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "remotevstclient.h"

#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>

#include "paths.h"

#include <dlfcn.h>

#include <iostream>
#include <string>
#include <fstream>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

void errwin(std::string dllname)
{
static Window window = 0;
static Window ignored = 0;
static Display* display = 0;
static int screen = 0;
static Atom winstate;
static Atom winmodal;
    
std::string filename;
std::string filename2;

  size_t found2 = dllname.find_last_of("/");
  filename = dllname.substr(found2 + 1, strlen(dllname.c_str()) - (found2 +1));
  filename2 = "VST dll file not found or timeout:  " + filename;
      
  XInitThreads();
  display = XOpenDisplay(NULL);  
  if (!display) 
  return;  
  screen = DefaultScreen(display);
  window = XCreateSimpleWindow(display, RootWindow(display, screen), 10, 10, 480, 20, 0, BlackPixel(display, screen), WhitePixel(display, screen));
  if (!window) 
  return;
  winstate = XInternAtom(display, "_NET_WM_STATE", True);
  winmodal = XInternAtom(display, "_NET_WM_STATE_ABOVE", True);
  XChangeProperty(display, window, winstate, XA_ATOM, 32, PropModeReplace, (unsigned char*)&winmodal, 1);
  XStoreName(display, window, filename2.c_str()); 
  XMapWindow(display, window);
  XSync (display, false);
  XFlush(display);
  sleep(10);
  XSync (display, false);
  XFlush(display);
  XDestroyWindow(display, window);
  XCloseDisplay(display);  
  }

const char *selfname()
{
    int i = 5;
}

RemoteVSTClient::RemoteVSTClient(audioMasterCallback theMaster) : RemotePluginClient(theMaster)
{
    pid_t       child;
    Dl_info     info;
    std::string dllName;
    size_t len;
    char        newname[4096];

    if (!dladdr((const char*) selfname, &info))
    {
        m_runok = 1;
        cleanup();
        return;
    }

    if (!info.dli_fname)
    {
        m_runok = 1;
        cleanup();
        return;
    }

    if (realpath(info.dli_fname, newname) == 0)
    {
        m_runok = 1;
        cleanup();
        return;
    }

len = strlen(info.dli_fname);

newname[len - 4] = '.';

newname[len - 3] = 's';

newname[len - 2] = 'o';

newname[len - 1] = '\0';


    dllName = newname;

    std::string arg = dllName + "," + getFileIdentifiers();

    const char *argStr = arg.c_str();

    #ifdef LVRT
    struct sched_param param;
    param.sched_priority = 1;

    int result = sched_setscheduler(0, SCHED_FIFO, &param);

    if (result < 0)
    {
        perror("Failed to set realtime priority");
    }
#endif

				if ((child = fork()) < 0) 
                {
                 m_runok = 1;
                 cleanup();
                 return;
				} 
                else if (child == 0) 
                { 
			    if (execlp("/usr/bin/lin-vst-server", "/usr/bin/lin-vst-server", argStr, NULL)) 
                {
                m_runok = 1;
                cleanup();
                return;
			    }  
                }

                syncStartup();
}

RemoteVSTClient::~RemoteVSTClient()
{
      wait(NULL);
//    while (waitpid(-1, NULL, WNOHANG) > 0) {}
/*
    for (int i=0;i<5000;i++)
    {
        if (waitpid(-1, NULL, WNOHANG))
        break;
        usleep(100);
    }
*/
}
