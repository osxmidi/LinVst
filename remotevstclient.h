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

#ifndef REMOTE_VST_CLIENT_H
#define REMOTE_VST_CLIENT_H

#include "remotepluginclient.h"

class RemoteVSTClient : public RemotePluginClient
{
public:
    // may throw a string exception
    RemoteVSTClient(audioMasterCallback theMaster);

    virtual ~RemoteVSTClient();


};    

#endif
