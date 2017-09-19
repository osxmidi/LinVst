/*  dssi-vst: a DSSI plugin wrapper for VST effects and instruments
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
#include "rdwrops.h"
#include <string>


class RemotePluginServer
{
public:
    virtual                 ~RemotePluginServer();

    virtual float           getVersion() { return RemotePluginVersion; }
    virtual std::string     getName() = 0;
    virtual std::string     getMaker() = 0;

    virtual void            setBufferSize(int) = 0;
    virtual void            setSampleRate(int) = 0;

    virtual void            reset() = 0;
    virtual void            terminate() = 0;

    virtual int             getInputCount() = 0;
    virtual int             getOutputCount() = 0;
    virtual int             getFlags() = 0;
    virtual int             getinitialDelay() = 0;

    virtual int             processVstEvents() = 0;
    virtual void            getChunk() = 0;
    virtual void            setChunk() = 0;
    // virtual void            canBeAutomated() = 0;
    virtual void            getProgram() = 0;
    virtual void            EffectOpen() = 0;

    // virtual int             getUniqueID() = 0;
    // virtual int             getVersion() = 0;
    // virtual void            eff_mainsChanged(int v) = 0;

    virtual int             getUID()                                { return 0; }
    virtual int             getParameterCount()                     { return 0; }
    virtual std::string     getParameterName(int)                   { return ""; }
    virtual void            setParameter(int, float)                { return; }
    virtual float           getParameter(int)                       { return 0.0f; }
    virtual float           getParameterDefault(int)                { return 0.0f; }
    virtual void            getParameters(int p0, int pn, float *v) { for (int i = p0; i <= pn; ++i) v[i - p0] = 0.0f; }

    virtual int             getProgramCount()                       { return 0; }
    virtual std::string     getProgramNameIndexed(int)              { return ""; }
    virtual std::string     getProgramName()                        { return ""; }
    virtual void            setCurrentProgram(int)                  { return; }

    virtual int             getEffInt(int opcode)                   { return 0; }
    virtual std::string     getEffString(int opcode, int index)     { return ""; }
    virtual void            effDoVoid(int opcode)                   { return; }

    virtual void            process(float **inputs, float **outputs, int sampleFrames) = 0;

    virtual void            setDebugLevel(RemotePluginDebugLevel)   { return; }
    virtual bool            warn(std::string) = 0;

    virtual void            showGUI()                               { }
    virtual void            hideGUI()                               { }

#ifdef EMBED
    virtual void            openGUI()                               { }
#endif

    void                    dispatch(int timeout = -1); // may throw RemotePluginClosedException
    void                    dispatchControl(int timeout = -1); // may throw RemotePluginClosedException
    void                    dispatchProcess(int timeout = -1); // may throw RemotePluginClosedException
    void                    dispatchGetSet(int timeout = -1); // may throw RemotePluginClosedException
    void                    dispatchPar(int timeout = -1); // may throw RemotePluginClosedException

    void                     sizeShm();
    char                     *m_shm;
    char                     *m_shm2;
    char                     *m_shm3;
    int                     m_shmControlFd;

    int                     m_threadsfinish;
    
    void                waitForClient2exit();
    void                waitForClient3exit();
    void                waitForClient4exit();
    void                waitForClient5exit();

protected:
                            RemotePluginServer(std::string fileIdentifiers);
    void                    cleanup();

private:
    void                    dispatchControlEvents();
    void                    dispatchProcessEvents();
    void                    dispatchGetSetEvents();
    void                    dispatchParEvents();

    int                     m_bufferSize;
    int                     m_numInputs;
    int                     m_numOutputs;
    int                     m_flags;
    int                     m_delay;
    int                     m_shmFd;
    int                     m_shmFd2;
    int                     m_shmFd3;
    size_t                  m_shmSize;
    size_t                  m_shmSize2;
    size_t                  m_shmSize3;
    char                    *m_shmFileName;
    char                    *m_shmFileName2;
    char                    *m_shmFileName3;

    float                   **m_inputs;
    float                   **m_outputs;

    RemotePluginDebugLevel  m_debugLevel;

public:

    char                    *m_shmControlFileName;
    ShmControl              *m_shmControl;
    int                     m_shmControl2Fd;
    char                    *m_shmControl2FileName;
    ShmControl              *m_shmControl2;
    int                     m_shmControl3Fd;
    char                    *m_shmControl3FileName;
    ShmControl              *m_shmControl3;
    int                     m_shmControl4Fd;
    char                    *m_shmControl4FileName;
    ShmControl              *m_shmControl4;
    int                     m_shmControl5Fd;
    char                    *m_shmControl5FileName;
    ShmControl              *m_shmControl5;

    void                     waitForServer();

    void                     waitForServerexit();

void RemotePluginClosedException();

int m_inexcept;

int m_runok;
int m_386run;

int starterror;

bool fwait(int *fcount, int ms);
bool fpost(int *fcount);

VstTimeInfo *timeinfo;

void rdwr_tryReadring(RingBuffer *ringbuf, void *buf, size_t count, const char *file, int line);
void rdwr_tryWritering(RingBuffer *ringbuf, const void *buf, size_t count, const char *file, int line);
void rdwr_writeOpcodering(RingBuffer *ringbuf, RemotePluginOpcode opcode, const char *file, int line);
int rdwr_readIntring(RingBuffer *ringbuf, const char *file, int line);
void rdwr_writeIntring(RingBuffer *ringbuf, int i, const char *file, int line);
void rdwr_writeFloatring(RingBuffer *ringbuf, float f, const char *file, int line);
float rdwr_readFloatring(RingBuffer *ringbuf, const char *file, int line);
void rdwr_writeStringring(RingBuffer *ringbuf, const std::string &str, const char *file, int line);
std::string rdwr_readStringring(RingBuffer *ringbuf, const char *file, int line);

void rdwr_commitWrite(RingBuffer *ringbuf, const char *file, int line);
bool dataAvailable(RingBuffer *ringbuf);

void rdwr_tryRead(char *ptr, void *buf, size_t count, const char *file, int line);
void rdwr_tryWrite(char *ptr, const void *buf, size_t count, const char *file, int line);
void rdwr_writeOpcode(char *ptr, RemotePluginOpcode opcode, const char *file, int line);
void rdwr_writeString(char *ptr, const std::string &str, const char *file, int line);
std::string rdwr_readString(char *ptr, const char *file, int line);
void rdwr_writeInt(char *ptr, int i, const char *file, int line);
int rdwr_readInt(char *ptr, const char *file, int line);
void rdwr_writeFloat(char *ptr, float f, const char *file, int line);
float rdwr_readFloat(char *ptr, const char *file, int line);

};
#endif
