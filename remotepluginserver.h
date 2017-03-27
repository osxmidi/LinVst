/*
  dssi-vst: a DSSI plugin wrapper for VST effects and instruments
  Copyright 2004-2007 Chris Cannam
*/

#ifndef REMOTE_PLUGIN_SERVER_H
#define REMOTE_PLUGIN_SERVER_H

#ifdef __WINE__
#else
#define __cdecl
#endif

#include "pluginterfaces/vst2.x/aeffectx.h"

#include "remoteplugin.h"
#include <string>


class RemotePluginServer
{
public:
    virtual ~RemotePluginServer();

    virtual float        getVersion() { return RemotePluginVersion; }
    virtual std::string  getName() = 0;
    virtual std::string  getMaker() = 0;

    virtual void         setBufferSize(int) = 0;
    virtual void         setSampleRate(int) = 0;

    virtual void         reset() = 0;
    virtual void         terminate() = 0;
    
    virtual int          getInputCount() = 0;
    virtual int          getOutputCount() = 0;
    virtual int			 getFlags() = 0;
    virtual int			 getinitialDelay() = 0;
	
	virtual int			 processVstEvents() = 0;
	virtual void		 getChunk() = 0;
	virtual void		 setChunk() = 0;
//	virtual void	     canBeAutomated() = 0;
	virtual void	     getProgram() = 0;


//	virtual int			 getUniqueID() = 0;
//	virtual int			 getVersion() = 0;
//    virtual void         eff_mainsChanged(int v) = 0;

	
    virtual int          getParameterCount()                  { return 0; }
    virtual std::string  getParameterName(int)                { return ""; }
    virtual void         setParameter(int, float)             { return; }
    virtual float        getParameter(int)                    { return 0.0f; }
    virtual float        getParameterDefault(int)             { return 0.0f; }
    virtual void         getParameters(int p0, int pn, float *v) {
	for (int i = p0; i <= pn; ++i) v[i - p0] = 0.0f;
    }

    virtual int          getProgramCount()                    { return 0; }
    virtual std::string  getProgramName(int)                  { return ""; }
    virtual void         setCurrentProgram(int)               { return; }

    virtual int          getEffInt(int opcode) { return 0; }
    virtual std::string  getEffString(int opcode, int index) { return ""; }
    virtual void         effDoVoid(int opcode) {return;}

    virtual void         process(float **inputs, float **outputs, int sampleFrames) = 0;

    virtual void         setDebugLevel(RemotePluginDebugLevel) { return; } 
    virtual bool         warn(std::string) = 0;

    virtual void         showGUI() { } 
    virtual void         hideGUI() { }

    void dispatch(int timeout = -1); // may throw RemotePluginClosedException
    void dispatchControl(int timeout = -1); // may throw RemotePluginClosedException
    void dispatchProcess(int timeout = -1); // may throw RemotePluginClosedException
    void dispatchPar(int timeout = -1); // may throw RemotePluginClosedException

protected:
    RemotePluginServer(std::string fileIdentifiers);

    void cleanup();
    
    int m_controlRequestFd;
    int m_controlResponseFd;
    int m_parRequestFd;
    int m_parResponseFd;


private:
    RemotePluginServer(const RemotePluginServer &); // not provided
    RemotePluginServer &operator=(const RemotePluginServer &); // not provided

    void dispatchControlEvents();
    void dispatchProcessEvents();
 

    int m_bufferSize;
    int m_numInputs;
    int m_numOutputs;
    int m_flags;
    int m_delay;
    int m_processFd;
    int m_processResponseFd;
    int m_shmFd;
    int m_shmFd2;
    int m_shmFd3;

    char *m_controlRequestFileName;
    char *m_controlResponseFileName;
    char *m_parRequestFileName;
    char *m_parResponseFileName;
    char *m_processFileName;
    char *m_processResponseFileName;

    char *m_shmFileName;
    char *m_shm;
    size_t m_shmSize;
        
    char *m_shmFileName2;
    size_t m_shmSize2;

    char *m_shmFileName3;
    char *m_shm3;
    size_t m_shmSize3;

    float **m_inputs;
    float **m_outputs;

    RemotePluginDebugLevel m_debugLevel;

    void sizeShm();

public:
   char *m_shm2;

   void dispatchParEvents();
 

};

#endif
