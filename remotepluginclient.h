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


#ifndef REMOTE_PLUGIN_CLIENT_H
#define REMOTE_PLUGIN_CLIENT_H

#define __cdecl

#ifdef VESTIGE
typedef int16_t VstInt16;	
typedef int32_t VstInt32;		
typedef int64_t VstInt64;		
typedef intptr_t VstIntPtr;
#define VESTIGECALLBACK __cdecl
#include "vestige.h"
#else
#include "pluginterfaces/vst2.x/aeffectx.h"
#endif

#include "remoteplugin.h"
#include "rdwrops.h"
#include <string>
#include <vector>
#include <sys/shm.h>

#ifdef EMBED
#include <X11/Xlib.h>
#endif

// Any of the methods in this file, including constructors, should be
// considered capable of throwing RemotePluginClosedException.  Do not
// call anything here without catching it.

class RemotePluginClient
{
public:
                        RemotePluginClient(audioMasterCallback theMaster);
    virtual             ~RemotePluginClient();

    std::string         getFileIdentifiers();

    float               getVersion();
    int                 getUID();

    std::string         getName();
    std::string         getMaker();

    void                setBufferSize(int);
    void                setSampleRate(int);

    void                reset();
    void                terminate();

    int                 getInputCount();
    int                 getOutputCount();
    int                 getFlags();
    int                 getinitialDelay();

    int                 getParameterCount();
    std::string         getParameterName(int);
#ifdef WAVES
    int                 getShellName(char *ptr);
#endif
    void                setParameter(int, float);
    float               getParameter(int);
    float               getParameterDefault(int);
    void                getParameters(int, int, float *);

    int                 getProgramCount();
    int                 getProgramNameIndexed(int, char *ptr);
    std::string         getProgramName();
    void                setCurrentProgram(int);

    int                 processVstEvents(VstEvents *);
#ifdef DOUBLEP
    void                processdouble(double **inputs, double **outputs, int sampleFrames);
    bool                setPrecision(int value);
#endif 
    
    bool                getEffInProp(int index, void *ptr);
    bool                getEffOutProp(int index, void *ptr);
        
#ifdef MIDIEFF
//    bool                getEffInProp(int index, void *ptr);
//    bool                getEffOutProp(int index, void *ptr);
    bool                getEffMidiKey(int index, void *ptr);
    bool                getEffMidiProgName(int index, void *ptr);
    bool                getEffMidiCurProg(int index, void *ptr);
    bool                getEffMidiProgCat(int index, void *ptr);
    bool                getEffMidiProgCh(int index);
    bool                setEffSpeaker(VstIntPtr value, void *ptr);
    bool                getEffSpeaker(VstIntPtr value, void *ptr);
#endif 
#ifdef CANDOEFF   
    bool                getEffCanDo(char *ptr);
#endif    
    int                 getChunk(void **ptr, int bank_prog);
    int                 setChunk(void *ptr, int sz, int bank_prog);
    int                 canBeAutomated(int param);
    int                 getProgram();
    int                 EffectOpen();
 
    // void                effMainsChanged(int s);
    // int                 getUniqueID();

    // Either inputs or outputs may be NULL if (and only if) there are none
    void                process(float **inputs, float **outputs, int sampleFrames);

    void                waitForServer2();
    void                waitForServer3();
    void                waitForServer4();
    void                waitForServer5();

    void                waitForServer2exit();
    void                waitForServer3exit();
    void                waitForServer4exit();
    void                waitForServer5exit();
    
    void                waitForClientexit();

    void                setDebugLevel(RemotePluginDebugLevel);
    bool                warn(std::string);

    void                showGUI();
    void                hideGUI();

#ifdef EMBED
    void                openGUI();
#endif

    int                 getEffInt(int opcode);
    std::string         getEffString(int opcode, int index);
    void                effVoidOp(int opcode);
    int                 effVoidOp2(int opcode, int index, int value, float opt);

    int                 m_bufferSize;
    int                 m_numInputs;
    int                 m_numOutputs;
    int                 m_finishaudio;
    int                 m_runok;
    int                 m_syncok;
    int                 m_386run;
    AEffect             *theEffect;
    AEffect             theEffect2;
    audioMasterCallback m_audioMaster;

    int                 m_threadbreak;
    int                 m_threadbreakexit;
    
    int                 editopen;
#ifdef EMBED
#ifdef XECLOSE
    int                 xeclose;  
#endif    
#ifdef EMBEDTHREAD
    int                 m_threadbreakembed;
    int                 m_threadbreakexitembed;
    pthread_mutex_t     mutex2;
#ifdef XEMBED
    int                 mapok;
#endif    
#endif
#endif    
    VstEvents           vstev[VSTSIZE];
    ERect               retRect = {0,0,200,500};    
    int                 reaperid;
    int                 m_updateio;
    int                 m_updatein;
    int                 m_updateout;
#ifdef CHUNKBUF
    char *chunk_ptr;
#endif
    
#ifdef EMBED
   Window  child;
   Window  parent;
   Display *display;
   int handle;
   int width;
   int height;
   struct winmessage
   {
    int handle;
    int width;
    int height;
   } winm2;        
   winmessage *winm;
  int displayerr;
#ifdef EMBEDTHREAD
   pthread_t           m_EMBEDThread;
   static void         *callEMBEDThread(void *arg) { return ((RemotePluginClient*)arg)->EMBEDThread(); }
   void                *EMBEDThread();
   int runembed;
#endif
#ifdef EMBEDDRAG
   Window x11_win;
   Window pparent;
   Window root;
   Window *children;
   unsigned int numchildren;
   int parentok;    
#endif
   int eventrun;    
   int eventfinish;    
#endif    

char *m_shm3;
    
int m_inexcept;
    
VstTimeInfo *timeInfo;   
    
#ifdef WAVES
int wavesthread;
#endif
    
struct vinfo
{
char a[96];
};    

protected:
    void                cleanup();
    void                syncStartup();

private:
    int                 m_shmFd;
    int                 m_shmFd2;
    int                 m_shmFd3;
    int                 m_shmControlFd;
    char                *m_shmControlFileName;
    ShmControl          *m_shmControl;
    int                 m_shmControl2Fd;
    char                *m_shmControl2FileName;
    ShmControl          *m_shmControl2;
    int                 m_shmControl3Fd;
    char                *m_shmControl3FileName;
    ShmControl          *m_shmControl3;
    int                 m_shmControl4Fd;
    char                *m_shmControl4FileName;
    ShmControl          *m_shmControl4;
    int                 m_shmControl5Fd;
    char                *m_shmControl5FileName;
    ShmControl          *m_shmControl5;
    int                 m_AMRequestFd;
    int                 m_AMResponseFd;
    char                *m_AMRequestFileName;
    char                *m_AMResponseFileName;
    char                *m_shmFileName;
    char                *m_shm;
    size_t              m_shmSize;
    char                *m_shmFileName2;
    char                *m_shm2;
    size_t              m_shmSize2;
    char                *m_shmFileName3;
    size_t              m_shmSize3;

    int                 sizeShm();

    pthread_t           m_AMThread;
    static void         *callAMThread(void *arg) { return ((RemotePluginClient*)arg)->AMThread(); }
    void                *AMThread();
    int                 m_threadinit;

void RemotePluginClosedException();

bool fwait(int *fcount, int ms);
bool fpost(int *fcount);

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

