/*
  dssi-vst: a DSSI plugin wrapper for VST effects and instruments
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
#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>

#include "rdwrops.h"
#include "paths.h"

#include <dlfcn.h> 

#include <iostream>
#include <string>
#include <fstream>

const char *selfname()
{

}

using std::string;

void addextension(string& name, const string& ext) {

   string::size_type idx = name.rfind('.', name.length());

   if (idx != string::npos) {
      name.replace(idx+1, ext.length(), ext);
   }
}


RemoteVSTClient::RemoteVSTClient(audioMasterCallback theMaster) :
    RemotePluginClient(theMaster)
{

    pid_t child;

    int runok;

    Dl_info info;

    dladdr( (const char*)&selfname, &info );

    std::string dllName = info.dli_fname;

    addextension(dllName, "dll");

    std::string arg = dllName2 + "," + getFileIdentifiers();

    const char *argStr = arg.c_str();

    runok = 1;


	if ((child = vfork()) < 0) 
         {
	 perror("Fork failed");
         runok = 0;
         } 
        else if (child == 0) 
        { 
     
       if (execlp("/usr/bin/lin-vst-server.exe", "/usr/bin/lin-vst-server.exe", argStr, NULL)) 
       {
       perror("Exec failed");
       runok = 0;
       }

       }
			
       else 
       {
       syncStartup();
       }

       if (!runok) {
       cleanup();
       throw(std::string("Failed to run lin-vst-server.exe"));
       }
	
    return;
 }

RemoteVSTClient::~RemoteVSTClient()
{
    for (int i = 0; i < 3; ++i) {
	if (waitpid(-1, NULL, WNOHANG)) break;
	sleep(1);
    }
}


