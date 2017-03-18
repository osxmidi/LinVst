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

#ifndef REMOTE_PLUGIN_CLIENT_H
#define REMOTE_PLUGIN_CLIENT_H

#ifdef __WINE__
#else
#define __cdecl
#endif

#include "pluginterfaces/vst2.x/aeffectx.h"

#include "remoteplugin.h"
#include <string>
#include <vector>
#include <sys/shm.h>

// Any of the methods in this file, including constructors, should be
// considered capable of throwing RemotePluginClosedException.  Do not
// call anything here without catching it.

class RemotePluginClient
{
public:
	RemotePluginClient(audioMasterCallback theMaster);
    virtual ~RemotePluginClient();

    std::string  getFileIdentifiers();

    float        getVersion();
    std::string  getName();
    std::string  getMaker();

    void         setBufferSize(int);
    void         setSampleRate(int);

    void         reset();
    void         terminate();
    
    int          getInputCount();
    int          getOutputCount();
    int		 getFlags();
    int          getinitialDelay();

    int          getParameterCount();
    std::string  getParameterName(int);
    void         setParameter(int, float);
    float        getParameter(int);
    float        getParameterDefault(int);
    void         getParameters(int, int, float *);

    int          getProgramCount();
    std::string  getProgramName(int);
    void         setCurrentProgram(int);

	int			 processVstEvents(VstEvents *);
	int			 getChunk(void **ptr, int bank_prog);
 	int			 setChunk(void *ptr, int sz, int bank_prog);
//	int			 canBeAutomated(int param);
	int			 getProgram();
	
//	void		effMainsChanged(int s);
//	int			 getUniqueID();


    // Either inputs or outputs may be NULL if (and only if) there are none
    void         process(float **inputs, float **outputs, int sampleFrames);

    void         setDebugLevel(RemotePluginDebugLevel);
    bool         warn(std::string);

    void         showGUI();
    void         hideGUI();

	int			getEffInt(int opcode);
	void		getEffString(int opcode, int index, char *ptr, int len);
	void		effVoidOp(int opcode);
	

protected:
    RemotePluginClient();

    void         cleanup();
    void         syncStartup();

private:
    RemotePluginClient(const RemotePluginClient &); // not provided
    RemotePluginClient &operator=(const RemotePluginClient &); // not provided

    int m_controlRequestFd;
    int m_controlResponseFd;
    int m_processFd;
    int m_processResponseFd;
    int m_shmFd;
    int m_shmFd2;
    int m_shmFd3;
    char *m_controlRequestFileName;
    char *m_controlResponseFileName;
    char *m_processFileName;
    char *m_processResponseFileName;

    char *m_shmFileName;
    char *m_shm;
    size_t m_shmSize;

    char *m_shmFileName2;
    char *m_shm2;
    size_t m_shmSize2;

    char *m_shmFileName3;
    char *m_shm3;
    size_t m_shmSize3;

    void sizeShm();

   public:
    int m_bufferSize;
    int m_numInputs;
    int m_numOutputs;
    AEffect *theEffect;
  

};


#endif

