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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>

#include "paths.h"

#include <dlfcn.h>

#include <fstream>
#include <iostream>
#include <string>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

void errwin(std::string dllname) {
  Window window = 0;
  Window ignored = 0;
  Display *display = 0;
  int screen = 0;
  Atom winstate;
  Atom winmodal;

  std::string filename;
  std::string filename2;

  size_t found2 = dllname.find_last_of("/");
  filename = dllname.substr(found2 + 1, strlen(dllname.c_str()) - (found2 + 1));
  filename2 = "LinVst Error: VST dll file not found:  " + filename;

  XInitThreads();
  display = XOpenDisplay(NULL);
  if (!display)
    return;
  screen = DefaultScreen(display);
  window = XCreateSimpleWindow(display, RootWindow(display, screen), 10, 10,
                               480, 20, 0, BlackPixel(display, screen),
                               WhitePixel(display, screen));
  if (!window)
    return;
  winstate = XInternAtom(display, "_NET_WM_STATE", True);
  winmodal = XInternAtom(display, "_NET_WM_STATE_ABOVE", True);
  XChangeProperty(display, window, winstate, XA_ATOM, 32, PropModeReplace,
                  (unsigned char *)&winmodal, 1);
  XStoreName(display, window, filename2.c_str());
  XMapWindow(display, window);
  XSync(display, false);
  XFlush(display);
  sleep(10);
  XSync(display, false);
  XFlush(display);
  XDestroyWindow(display, window);
  XCloseDisplay(display);
}

const char *selfname() { int i = 5; }

RemoteVSTClient::RemoteVSTClient(audioMasterCallback theMaster)
    : RemotePluginClient(theMaster) {
  pid_t child;
  Dl_info info;
  std::string dllName;
  std::string LinVstName;
  bool test;

#ifdef VST6432
  int dlltype;
  unsigned int offset;
  char buffer[256];
#endif

  char hit2[4096];

  if (!dladdr((const char *)selfname, &info)) {
    m_runok = 1;
    cleanup();
    return;
  }

  if (!info.dli_fname) {
    m_runok = 1;
    cleanup();
    return;
  }

  if (realpath(info.dli_fname, hit2) == 0) {
    m_runok = 1;
    cleanup();
    return;
  }

  dllName = hit2;
  dllName.replace(dllName.begin() + dllName.find(".so"), dllName.end(), ".dll");
  test = std::ifstream(dllName.c_str()).good();

  if (!test) {
    dllName = hit2;
    dllName.replace(dllName.begin() + dllName.find(".so"), dllName.end(),
                    ".Dll");
    test = std::ifstream(dllName.c_str()).good();

    if (!test) {
      dllName = hit2;
      dllName.replace(dllName.begin() + dllName.find(".so"), dllName.end(),
                      ".DLL");
      test = std::ifstream(dllName.c_str()).good();
    }

    if (!test) {
      dllName = hit2;
      dllName.replace(dllName.begin() + dllName.find(".so"), dllName.end(),
                      ".dll");
      errwin(dllName);
      m_runok = 1;
      cleanup();
      return;
    }
  }

#ifdef VST6432
  std::ifstream mfile(dllName.c_str(), std::ifstream::binary);

  if (!mfile) {
    m_runok = 1;
    cleanup();
    return;
  }

  mfile.read(&buffer[0], 2);
  short *ptr;
  ptr = (short *)&buffer[0];

  if (*ptr != 0x5a4d) {
    mfile.close();
    m_runok = 1;
    cleanup();
    return;
  }

  mfile.seekg(60, mfile.beg);
  mfile.read(&buffer[0], 4);

  int *ptr2;
  ptr2 = (int *)&buffer[0];
  offset = *ptr2;
  offset += 4;

  mfile.seekg(offset, mfile.beg);
  mfile.read(&buffer[0], 2);

  unsigned short *ptr3;
  ptr3 = (unsigned short *)&buffer[0];

  dlltype = 0;
  if (*ptr3 == 0x8664)
    dlltype = 1;
  else if (*ptr3 == 0x014c)
    dlltype = 2;
  else if (*ptr3 == 0x0200)
    dlltype = 3;

  if (dlltype == 0) {
    mfile.close();
    m_runok = 1;
    cleanup();
    return;
  }

  mfile.close();
#endif

#ifndef VST32
#ifdef VST6432
#ifdef TRACKTIONWM
  if (dlltype == 2) {
#ifdef EMBED
#ifdef BITWIG
    LinVstName = BIN_DIR "/lin-vst-servertrack32-bw.exe";
#else
    LinVstName = BIN_DIR "/lin-vst-servertrack32.exe";
#endif
#else
    LinVstName = BIN_DIR "/lin-vst-servertrack32st.exe";
#endif
    test = std::ifstream(LinVstName.c_str()).good();
    if (!test) {
      m_runok = 1;
      cleanup();
      return;
    }
#ifdef EMBED
#ifdef BITWIG
    LinVstName = BIN_DIR "/lin-vst-servertrack32-bw.exe.so";
#else
    LinVstName = BIN_DIR "/lin-vst-servertrack32.exe.so";
#endif
#else
    LinVstName = BIN_DIR "/lin-vst-servertrack32st.exe.so";
#endif
    test = std::ifstream(LinVstName.c_str()).good();
    if (!test) {
      m_runok = 1;
      cleanup();
      return;
    }
  } else {
#ifdef EMBED
#ifdef BITWIG
    LinVstName = BIN_DIR "/lin-vst-servertrack-bw.exe";
#else
    LinVstName = BIN_DIR "/lin-vst-servertrack.exe";
#endif
#else
    LinVstName = BIN_DIR "/lin-vst-servertrackst.exe";
#endif
    test = std::ifstream(LinVstName.c_str()).good();
    if (!test) {
      m_runok = 1;
      cleanup();
      return;
    }
#ifdef EMBED
#ifdef BITWIG
    LinVstName = BIN_DIR "/lin-vst-servertrack-bw.exe.so";
#else
    LinVstName = BIN_DIR "/lin-vst-servertrack.exe.so";
#endif
#else
    LinVstName = BIN_DIR "/lin-vst-servertrackst.exe.so";
#endif
    test = std::ifstream(LinVstName.c_str()).good();
    if (!test) {
      m_runok = 1;
      cleanup();
      return;
    }
  }
#else
  if (dlltype == 2) {
#ifdef EMBED
#ifdef BITWIG
    LinVstName = BIN_DIR "/lin-vst-server32-bw.exe";
#else
    LinVstName = BIN_DIR "/lin-vst-server32.exe";
#endif
#else
    LinVstName = BIN_DIR "/lin-vst-server32st.exe";
#endif
    test = std::ifstream(LinVstName.c_str()).good();
    if (!test) {
      m_runok = 1;
      cleanup();
      return;
    }
#ifdef EMBED
#ifdef BITWIG
    LinVstName = BIN_DIR "/lin-vst-server32-bw.exe.so";
#else
    LinVstName = BIN_DIR "/lin-vst-server32.exe.so";
#endif
#else
    LinVstName = BIN_DIR "/lin-vst-server32st.exe.so";
#endif
    test = std::ifstream(LinVstName.c_str()).good();
    if (!test) {
      m_runok = 1;
      cleanup();
      return;
    }
  } else {
#ifdef EMBED
#ifdef BITWIG
    LinVstName = BIN_DIR "/lin-vst-server-bw.exe";
#else
    LinVstName = BIN_DIR "/lin-vst-server.exe";
#endif
#else
    LinVstName = BIN_DIR "/lin-vst-serverst.exe";
#endif
    test = std::ifstream(LinVstName.c_str()).good();
    if (!test) {
      m_runok = 1;
      cleanup();
      return;
    }
#ifdef EMBED
#ifdef BITWIG
    LinVstName = BIN_DIR "/lin-vst-server-bw.exe.so";
#else
    LinVstName = BIN_DIR "/lin-vst-server.exe.so";
#endif
#else
    LinVstName = BIN_DIR "/lin-vst-serverst.exe.so";
#endif
    test = std::ifstream(LinVstName.c_str()).good();
    if (!test) {
      m_runok = 1;
      cleanup();
      return;
    }
  }
#endif
#else
#ifdef TRACKTIONWM
#ifdef EMBED
#ifdef BITWIG
  LinVstName = BIN_DIR "/lin-vst-servertrack-bw.exe";
#else
  LinVstName = BIN_DIR "/lin-vst-servertrack.exe";
#endif
#else
  LinVstName = BIN_DIR "/lin-vst-servertrackst.exe";
#endif
  test = std::ifstream(LinVstName.c_str()).good();
  if (!test) {
    m_runok = 1;
    cleanup();
    return;
  }
#ifdef EMBED
#ifdef BITWIG
  LinVstName = BIN_DIR "/lin-vst-servertrack-bw.exe.so";
#else
  LinVstName = BIN_DIR "/lin-vst-servertrack.exe.so";
#endif
#else
  LinVstName = BIN_DIR "/lin-vst-servertrackst.exe.so";
#endif
  test = std::ifstream(LinVstName.c_str()).good();
  if (!test) {
    m_runok = 1;
    cleanup();
    return;
  }
#else
#ifdef EMBED
#ifdef BITWIG
  LinVstName = BIN_DIR "/lin-vst-server-bw.exe";
#else
  LinVstName = BIN_DIR "/lin-vst-server.exe";
#endif
#else
  LinVstName = BIN_DIR "/lin-vst-serverst.exe";
#endif
  test = std::ifstream(LinVstName.c_str()).good();
  if (!test) {
    m_runok = 1;
    cleanup();
    return;
  }
#ifdef EMBED
#ifdef BITWIG
  LinVstName = BIN_DIR "/lin-vst-server-bw.exe.so";
#else
  LinVstName = BIN_DIR "/lin-vst-server.exe.so";
#endif
#else
  LinVstName = BIN_DIR "/lin-vst-serverst.exe.so";
#endif
  test = std::ifstream(LinVstName.c_str()).good();
  if (!test) {
    m_runok = 1;
    cleanup();
    return;
  }
#endif
#endif
#endif

#ifdef VST32
#ifdef EMBED
  LinVstName = BIN_DIR "/lin-vst-server32lx.exe";
#else
  LinVstName = BIN_DIR "/lin-vst-server32lxst.exe";
#endif
  test = std::ifstream(LinVstName.c_str()).good();
  if (!test) {
    m_runok = 1;
    cleanup();
    return;
  }
#ifdef EMBED
  LinVstName = BIN_DIR "/lin-vst-server32lx.exe.so";
#else
  LinVstName = BIN_DIR "/lin-vst-server32lxst.exe.so";
#endif
  test = std::ifstream(LinVstName.c_str()).good();
  if (!test) {
    m_runok = 1;
    cleanup();
    return;
  }
#endif

  hit2[0] = '\0';

  std::string dllNamewin = dllName;
  std::size_t idx = dllNamewin.find("drive_c");

  if (idx != std::string::npos) {
    const char *hit = dllNamewin.c_str();
    strcpy(hit2, hit);
    hit2[idx - 1] = '\0';
    setenv("WINEPREFIX", hit2, 1);
  }

  std::string arg = dllName + "," + getFileIdentifiers();
  const char *argStr = arg.c_str();

  //    signal(SIGCHLD, SIG_IGN);

#ifdef LVRT
  struct sched_param param;
  param.sched_priority = 1;

  int result = sched_setscheduler(0, SCHED_FIFO, &param);

  if (result < 0) {
    perror("Failed to set realtime priority");
  }
#endif

  if ((child = vfork()) < 0) {
    m_runok = 1;
    cleanup();
    return;
  } else if (child == 0) {
// for (int fd=3; fd<256; fd++) (void) close(fd);
/*
int maxfd=sysconf(_SC_OPEN_MAX);
for(int fd=3; fd<maxfd; fd++)
    close(fd);
*/
#ifndef VST32
#ifdef VST6432
#ifdef TRACKTIONWM
    if (dlltype == 2) {
      m_386run = 1;
#ifdef EMBED
#ifdef BITWIG
      if (execlp(BIN_DIR "/lin-vst-servertrack32-bw.exe",
                 BIN_DIR "/lin-vst-servertrack32-bw.exe", argStr, NULL))
#else
      if (execlp(BIN_DIR "/lin-vst-servertrack32.exe",
                 BIN_DIR "/lin-vst-servertrack32.exe", argStr, NULL))
#endif
#else
      if (execlp(BIN_DIR "/lin-vst-servertrack32st.exe",
                 BIN_DIR "/lin-vst-servertrack32st.exe", argStr, NULL))
#endif
      {
        m_runok = 1;
        cleanup();
        return;
      }
    } else {
#ifdef EMBED
#ifdef BITWIG
      if (execlp(BIN_DIR "/lin-vst-servertrack-bw.exe",
                 BIN_DIR "/lin-vst-servertrack-bw.exe", argStr, NULL))
#else
      if (execlp(BIN_DIR "/lin-vst-servertrack.exe",
                 BIN_DIR "/lin-vst-servertrack.exe", argStr, NULL))
#endif
#else
      if (execlp(BIN_DIR "/lin-vst-servertrackst.exe",
                 BIN_DIR "/lin-vst-servertrackst.exe", argStr, NULL))
#endif
      {
        m_runok = 1;
        cleanup();
        return;
      }
    }
#else
    if (dlltype == 2) {
      m_386run = 1;
#ifdef EMBED
#ifdef BITWIG
      if (execlp(BIN_DIR "/lin-vst-server32-bw.exe",
                 BIN_DIR "/lin-vst-server32-bw.exe", argStr, NULL))
#else
      if (execlp(BIN_DIR "/lin-vst-server32.exe",
                 BIN_DIR "/lin-vst-server32.exe", argStr, NULL))
#endif
#else
      if (execlp(BIN_DIR "/lin-vst-server32st.exe",
                 BIN_DIR "/lin-vst-server32st.exe", argStr, NULL))
#endif
      {
        m_runok = 1;
        cleanup();
        return;
      }
    } else {
#ifdef EMBED
#ifdef BITWIG
      if (execlp(BIN_DIR "/lin-vst-server-bw.exe",
                 BIN_DIR "/lin-vst-server-bw.exe", argStr, NULL))
#else
      if (execlp(BIN_DIR "/lin-vst-server.exe", BIN_DIR "/lin-vst-server.exe",
                 argStr, NULL))
#endif
#else
      if (execlp(BIN_DIR "/lin-vst-serverst.exe",
                 BIN_DIR "/lin-vst-serverst.exe", argStr, NULL))
#endif
      {
        m_runok = 1;
        cleanup();
        return;
      }
    }
#endif
#else
#ifdef TRACKTIONWM
#ifdef EMBED
#ifdef BITWIG
    if (execlp(BIN_DIR "/lin-vst-servertrack-bw.exe",
               BIN_DIR "/lin-vst-servertrack-bw.exe", argStr, NULL))
#else
    if (execlp(BIN_DIR "/lin-vst-servertrack.exe",
               BIN_DIR "/lin-vst-servertrack.exe", argStr, NULL))
#endif
#else
    if (execlp(BIN_DIR "/lin-vst-servertrackst.exe",
               BIN_DIR "/lin-vst-servertrackst.exe", argStr, NULL))
#endif
    {
      m_runok = 1;
      cleanup();
      return;
    }
#else
#ifdef EMBED
#ifdef BITWIG
    if (execlp(BIN_DIR "/lin-vst-server-bw.exe",
               BIN_DIR "/lin-vst-server-bw.exe", argStr, NULL))
#else
    if (execlp(BIN_DIR "/lin-vst-server.exe", BIN_DIR "/lin-vst-server.exe",
               argStr, NULL))
#endif
#else
    if (execlp(BIN_DIR "/lin-vst-serverst.exe", BIN_DIR "/lin-vst-serverst.exe",
               argStr, NULL))
#endif
    {
      m_runok = 1;
      cleanup();
      return;
    }
#endif
#endif
#else
#ifdef EMBED
    if (execlp(BIN_DIR "/lin-vst-server32lx.exe",
               BIN_DIR "/lin-vst-server32lx.exe", argStr, NULL))
#else
    if (execlp(BIN_DIR "/lin-vst-server32lxst.exe",
               BIN_DIR "/lin-vst-server32lxst.exe", argStr, NULL))
#endif
    {
      m_runok = 1;
      cleanup();
      return;
    }
#endif
  }
  syncStartup();
}

RemoteVSTClient::~RemoteVSTClient() {
  //     wait(NULL);
}
