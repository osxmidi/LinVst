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


#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <dlfcn.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <time.h>

#include <sched.h>

#include "remotepluginserver.h"

#include "paths.h"

#define APPLICATION_CLASS_NAME "dssi_vst"
#define OLD_PLUGIN_ENTRY_POINT "main"
#define NEW_PLUGIN_ENTRY_POINT "VSTPluginMain"

#if VST_FORCE_DEPRECATED
#define DEPRECATED_VST_SYMBOL(x) __##x##Deprecated
#else
#define DEPRECATED_VST_SYMBOL(x) x
#endif

typedef AEffect* (*PluginEntryProc) (audioMasterCallback audioMaster);

RemotePluginDebugLevel  debugLevel = RemotePluginDebugNone;

#define disconnectserver 32143215

using namespace std;

class RemoteVSTServer : public RemotePluginServer
{
public:
                        RemoteVSTServer(std::string fileIdentifiers, std::string fallbackName);
    virtual             ~RemoteVSTServer();

    virtual std::string getName() { return m_name; }
    virtual std::string getMaker() { return m_maker; }
    virtual void        setBufferSize(int);
    virtual void        setSampleRate(int);
    virtual void        reset();
    virtual void        terminate();

    virtual int         getInputCount() { return m_plugin->numInputs; }
    virtual int         getOutputCount() { return m_plugin->numOutputs; }
    virtual int         getFlags() { return m_plugin->flags; }
    virtual int         getinitialDelay() { return m_plugin->initialDelay; }
    virtual int         getUID() { return m_plugin->uniqueID; }
    virtual int         getParameterCount() { return m_plugin->numParams; }
    virtual std::string getParameterName(int);
    virtual void        setParameter(int, float);
    virtual float       getParameter(int);
    virtual void        getParameters(int, int, float *);

    virtual int         getProgramCount() { return m_plugin->numPrograms; }
    virtual std::string getProgramNameIndexed(int);
    virtual std::string getProgramName();

    virtual void        setCurrentProgram(int);

    virtual void        showGUI();
    virtual void        hideGUI();
    virtual void        hideGUI2();
#ifdef EMBED
    virtual void        openGUI();
#endif

    virtual void        GetRect();

    virtual int         getEffInt(int opcode);
    virtual std::string getEffString(int opcode, int index);
    virtual void        effDoVoid(int opcode);
    virtual int         effDoVoid2(int opcode, int index, int value, float opt);
  
//    virtual int         getInitialDelay() {return m_plugin->initialDelay;}
//    virtual int         getUniqueID() { return m_plugin->uniqueID;}
//    virtual int         getVersion() { return m_plugin->version;}

    virtual int         processVstEvents();
    virtual void        getChunk();
    virtual void        setChunk();
    virtual void        canBeAutomated();
    virtual void        getProgram();
    virtual void        EffectOpen();
//    virtual void        eff_mainsChanged(int v);

    virtual void        process(float **inputs, float **outputs, int sampleFrames);
#ifdef DOUBLEP
    virtual void        processdouble(double **inputs, double **outputs, int sampleFrames);
    virtual bool        setPrecision(int);  
#endif
#ifdef MIDIEFF
    virtual bool        getOutProp(int);
    virtual bool        getInProp(int);
    virtual bool        getMidiKey(int);
    virtual bool        getMidiProgName(int);
    virtual bool        getMidiCurProg(int);
    virtual bool        getMidiProgCat(int);
    virtual bool        getMidiProgCh(int);
    virtual bool        setSpeaker();
    virtual bool        getSpeaker();
#endif
#ifdef CANDOEFF 
    virtual bool        getEffCanDo(std::string);
#endif 

    virtual void        setDebugLevel(RemotePluginDebugLevel level) { debugLevel = level; }

    virtual bool        warn(std::string);

    virtual void        waitForServer();
    virtual void        waitForServerexit();

    Display *x11_dpy;
    Window x11_win;

    bool                haveGui;
    int                 hideguival;
#ifdef EMBED
    struct winmessage
    {
        int handle;
        int width;
        int height;
    } winm;
#endif
    int guiupdate;
    int guiupdatecount;
    int guiresizewidth;
    int guiresizeheight;
    ERect               *rect;
    int                 setprogrammiss;
    int                 hostreaper;
    AEffect             *m_plugin;
    VstEvents           vstev[VSTSIZE];
    int                 bufferSize;
    int                 sampleRate;
    bool                exiting;
    bool                effectrun;
    bool                inProcessThread;
    bool                guiVisible;
    int                 parfin;
    int                 audfin;
    int                 getfin;

private:
    std::string         m_name;
    std::string         m_maker;
};

RemoteVSTServer         *remoteVSTServerInstance = 0;


void *AudioThreadMain(void * parm)
{
/*
    struct sched_param param;
    param.sched_priority = 1;

    int result = sched_setscheduler(0, SCHED_FIFO, &param);

    if (result < 0)
    {
        perror("Failed to set realtime priority for audio thread");
    }
*/
    while (!remoteVSTServerInstance->exiting)
    {
    remoteVSTServerInstance->dispatchProcess(100);
    }
    // param.sched_priority = 0;
    // (void)sched_setscheduler(0, SCHED_OTHER, &param);
    remoteVSTServerInstance->audfin = 1;
    return 0;
}

void *GetSetThreadMain(void * parm)
{
/*
    struct sched_param param;
    param.sched_priority = 1;

    int result = sched_setscheduler(0, SCHED_FIFO, &param);

    if (result < 0)
    {
        perror("Failed to set realtime priority for audio thread");
    }
*/
    while (!remoteVSTServerInstance->exiting)
    {
    remoteVSTServerInstance->dispatchGetSet(100);
    }
    // param.sched_priority = 0;
    // (void)sched_setscheduler(0, SCHED_OTHER, &param);
    remoteVSTServerInstance->getfin = 1;
    return 0;
}

void *ParThreadMain(void * parm)
{
/*
    struct sched_param param;
    param.sched_priority = 1;

    int result = sched_setscheduler(0, SCHED_FIFO, &param);

    if (result < 0)
    {
        perror("Failed to set realtime priority for audio thread");
    }
*/
    while (!remoteVSTServerInstance->exiting)
    {
    remoteVSTServerInstance->dispatchPar(100);
    }
    // param.sched_priority = 0;
    // (void)sched_setscheduler(0, SCHED_OTHER, &param);
     remoteVSTServerInstance->parfin = 1;
    return 0;
}

RemoteVSTServer::RemoteVSTServer(std::string fileIdentifiers, std::string fallbackName) :
    RemotePluginServer(fileIdentifiers),
    m_plugin(0),
    m_name(fallbackName),
    m_maker(""),
    bufferSize(0),
    sampleRate(0),
    setprogrammiss(0),
    hostreaper(0),
    haveGui(true),
    exiting(false),
    effectrun(false),
    inProcessThread(false),
    guiVisible(false),
    hideguival(0),
    parfin(0),
    audfin(0),
    getfin(0),
    guiupdate(0),
    guiupdatecount(0),
    guiresizewidth(500),
    guiresizeheight(200)
{   

}

void RemoteVSTServer::EffectOpen()
{
    if (debugLevel > 0)
        cerr << "dssi-vst-server[1]: opening plugin" << endl;	
	
    m_plugin->dispatcher(m_plugin, effOpen, 0, 0, NULL, 0);

    m_plugin->dispatcher(m_plugin, effMainsChanged, 0, 0, NULL, 0);

    if (m_plugin->dispatcher(m_plugin, effGetVstVersion, 0, 0, NULL, 0) < 2)
    {
        if (debugLevel > 0)
            cerr << "dssi-vst-server[1]: plugin is VST 1.x" << endl;
    }
    else
    {
        if (debugLevel > 0)
            cerr << "dssi-vst-server[1]: plugin is VST 2.0 or newer" << endl;
    }

    char buffer[512];
    memset(buffer, 0, sizeof(buffer));

    m_plugin->dispatcher(m_plugin, effGetEffectName, 0, 0, buffer, 0);
    if (debugLevel > 0)
        cerr << "dssi-vst-server[1]: plugin name is \"" << buffer << "\"" << endl;
    if (buffer[0]) 
    m_name = buffer;

/*
    if (strncmp(buffer, "Guitar Rig 5", 12) == 0)
        setprogrammiss = 1;
    if (strncmp(buffer, "T-Rack", 6) == 0)
        setprogrammiss = 1;
*/

    memset(buffer, 0, sizeof(buffer));

    m_plugin->dispatcher(m_plugin, effGetVendorString, 0, 0, buffer, 0);
    if (debugLevel > 0)
        cerr << "dssi-vst-server[1]: vendor string is \"" << buffer << "\"" << endl;
    if (buffer[0]) 
    m_maker = buffer;

/*
    if (strncmp(buffer, "IK", 2) == 0)
        setprogrammiss = 1;
*/
	
    m_plugin->dispatcher(m_plugin, effMainsChanged, 0, 1, NULL, 0);	
		
    struct amessage
    {
        int flags;
        int pcount;
        int parcount;
        int incount;
        int outcount;
        int delay;
    } am;

        am.flags = m_plugin->flags;
        am.pcount = m_plugin->numPrograms;
        am.parcount = m_plugin->numParams;
        am.incount = m_plugin->numInputs;
        am.outcount = m_plugin->numOutputs;
        am.delay = m_plugin->initialDelay;
#ifndef DOUBLEP
        am.flags &= ~effFlagsCanDoubleReplacing;
#endif

    memcpy(&remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3], &am, sizeof(am));

    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)audioMasterIOChanged);
   
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();
	
    effectrun = true;	
}

RemoteVSTServer::~RemoteVSTServer()
{
    if(effectrun == true)
    {
    if (haveGui == true)
    {
    if (guiVisible)
    {
    if(m_plugin)
    m_plugin->dispatcher(m_plugin, effEditClose, 0, 0, 0, 0);
    }
    }
	
    if(m_plugin)
    {
    m_plugin->dispatcher(m_plugin, effMainsChanged, 0, 0, NULL, 0);
    m_plugin->dispatcher(m_plugin, effClose, 0, 0, NULL, 0);
    }
    }    
}

void RemoteVSTServer::process(float **inputs, float **outputs, int sampleFrames)
{
    inProcessThread = true;
    m_plugin->processReplacing(m_plugin, inputs, outputs, sampleFrames);
    inProcessThread = false;
}

#ifdef DOUBLEP
void RemoteVSTServer::processdouble(double **inputs, double **outputs, int sampleFrames)
{
    inProcessThread = true;
    m_plugin->processDoubleReplacing(m_plugin, inputs, outputs, sampleFrames);
    inProcessThread = false;
}

bool RemoteVSTServer::setPrecision(int value)
{
bool retval;

        retval = m_plugin->dispatcher(m_plugin, effSetProcessPrecision, 0, value, 0, 0);

        return retval;       
}    
#endif

#ifdef MIDIEFF
bool RemoteVSTServer::getInProp(int index)
{
VstPinProperties ptr;
bool retval;

        retval = m_plugin->dispatcher(m_plugin, effGetInputProperties, index, 0, &ptr, 0);

        tryWrite(&m_shm2[FIXED_SHM_SIZE2 - sizeof(VstPinProperties)], &ptr, sizeof(VstPinProperties));

        return retval;       
}

bool RemoteVSTServer::getOutProp(int index)
{
VstPinProperties ptr;
bool retval;

        retval = m_plugin->dispatcher(m_plugin, effGetOutputProperties, index, 0, &ptr, 0);

        tryWrite(&m_shm2[FIXED_SHM_SIZE2 - sizeof(VstPinProperties)], &ptr, sizeof(VstPinProperties));

        return retval;         
}

bool RemoteVSTServer::getMidiKey(int index)
{
MidiKeyName ptr;
bool retval;

        retval = m_plugin->dispatcher(m_plugin, effGetMidiKeyName, index, 0, &ptr, 0);
        tryWrite(&m_shm2[FIXED_SHM_SIZE2 - sizeof(MidiKeyName)], &ptr, sizeof(MidiKeyName));

        return retval;       
}

bool RemoteVSTServer::getMidiProgName(int index)
{
MidiProgramName ptr;
bool retval;

        retval = m_plugin->dispatcher(m_plugin, effGetMidiProgramName, index, 0, &ptr, 0);
        tryWrite(&m_shm2[FIXED_SHM_SIZE2 - sizeof(MidiProgramName)], &ptr, sizeof(MidiProgramName));

        return retval;       
}

bool RemoteVSTServer::getMidiCurProg(int index)
{
MidiProgramName ptr;
bool retval;

        retval = m_plugin->dispatcher(m_plugin, effGetCurrentMidiProgram, index, 0, &ptr, 0);
        tryWrite(&m_shm2[FIXED_SHM_SIZE2 - sizeof(MidiProgramName)], &ptr, sizeof(MidiProgramName));

        return retval;       
}

bool RemoteVSTServer::getMidiProgCat(int index)
{
MidiProgramCategory ptr;
bool retval;

        retval = m_plugin->dispatcher(m_plugin, effGetMidiProgramCategory, index, 0, &ptr, 0);
        tryWrite(&m_shm2[FIXED_SHM_SIZE2 - sizeof(MidiProgramCategory)], &ptr, sizeof(MidiProgramCategory));

        return retval;       
}

bool RemoteVSTServer::getMidiProgCh(int index)
{
bool retval;

        retval = m_plugin->dispatcher(m_plugin, effHasMidiProgramsChanged, index, 0, 0, 0);

        return retval;       
}

bool RemoteVSTServer::setSpeaker() 
{
VstSpeakerArrangement ptr;
VstSpeakerArrangement value;
bool retval;

       tryRead(&m_shm2[FIXED_SHM_SIZE2 - (sizeof(VstSpeakerArrangement)*2)], &ptr, sizeof(VstSpeakerArrangement));

       tryRead(&m_shm2[FIXED_SHM_SIZE2 - sizeof(VstSpeakerArrangement)], &value, sizeof(VstSpeakerArrangement));

       retval = m_plugin->dispatcher(m_plugin, effSetSpeakerArrangement, 0, (VstIntPtr)&value, &ptr, 0);

       return retval;  
}       

bool RemoteVSTServer::getSpeaker() 
{
VstSpeakerArrangement ptr;
VstSpeakerArrangement value;
bool retval;

       retval = m_plugin->dispatcher(m_plugin, effSetSpeakerArrangement, 0, (VstIntPtr)&value, &ptr, 0);

       tryWrite(&m_shm2[FIXED_SHM_SIZE2 - (sizeof(VstSpeakerArrangement)*2)], &ptr, sizeof(VstSpeakerArrangement));

       tryWrite(&m_shm2[FIXED_SHM_SIZE2 - sizeof(VstSpeakerArrangement)], &value, sizeof(VstSpeakerArrangement));
 
       return retval;  
}     
#endif	

#ifdef CANDOEFF
bool RemoteVSTServer::getEffCanDo(std::string ptr)
{
        if(m_plugin->dispatcher(m_plugin, effCanDo, 0, 0, (char*)ptr.c_str(), 0))
        return true;
        else
        return false;
}
#endif

int RemoteVSTServer::getEffInt(int opcode)
{
    return m_plugin->dispatcher(m_plugin, opcode, 0, 0, NULL, 0);
}

void RemoteVSTServer::effDoVoid(int opcode)
{
/*
    if (opcode == effCanDo)
    {
        hostreaper = 1;
         return;
    }
*/
	
    if (opcode == effClose)
    {
         // usleep(500000);
        waitForServerexit();
        terminate();
    }
    else
    {
        m_plugin->dispatcher(m_plugin, opcode, 0, 0, NULL, 0);
    }
}

int RemoteVSTServer::effDoVoid2(int opcode, int index, int value, float opt)
{
int ret;

    ret = 0;
    if(opcode == effEditIdle)
    {
    ret = m_plugin->dispatcher(m_plugin, opcode, index, value, NULL, opt);
    }
    return ret;
}

std::string RemoteVSTServer::getEffString(int opcode, int index)
{
    char name[512];
    memset(name, 0, sizeof(name));

    m_plugin->dispatcher(m_plugin, opcode, index, 0, name, 0);
    return name;
}

void RemoteVSTServer::setBufferSize(int sz)
{
    if (bufferSize != sz)
    {
        m_plugin->dispatcher(m_plugin, effMainsChanged, 0, 0, NULL, 0);
        m_plugin->dispatcher(m_plugin, effSetBlockSize, 0, sz, NULL, 0);
        m_plugin->dispatcher(m_plugin, effMainsChanged, 0, 1, NULL, 0);
        bufferSize = sz;
    }
   
    if (debugLevel > 0)
        cerr << "dssi-vst-server[1]: set buffer size to " << sz << endl;
}

void RemoteVSTServer::setSampleRate(int sr)
{
    if (sampleRate != sr)
    {
        m_plugin->dispatcher(m_plugin, effMainsChanged, 0, 0, NULL, 0);
        m_plugin->dispatcher(m_plugin, effSetSampleRate, 0, 0, NULL, (float)sr);
        m_plugin->dispatcher(m_plugin, effMainsChanged, 0, 1, NULL, 0);
        sampleRate = sr;
    }

    if (debugLevel > 0)
        cerr << "dssi-vst-server[1]: set sample rate to " << sr << endl;
}

void RemoteVSTServer::reset()
{
    cerr << "dssi-vst-server[1]: reset" << endl;

    m_plugin->dispatcher(m_plugin, effMainsChanged, 0, 0, NULL, 0);
    m_plugin->dispatcher(m_plugin, effMainsChanged, 0, 1, NULL, 0);
}

void RemoteVSTServer::terminate()
{
    cerr << "RemoteVSTServer::terminate: setting exiting flag" << endl;

    exiting = true;
}

std::string RemoteVSTServer::getParameterName(int p)
{
    char name[512];
    memset(name, 0, sizeof(name));

    m_plugin->dispatcher(m_plugin, effGetParamName, p, 0, name, 0);
    return name;
}

void RemoteVSTServer::setParameter(int p, float v)
{
    m_plugin->setParameter(m_plugin, p, v);
}

float RemoteVSTServer::getParameter(int p)
{
    return m_plugin->getParameter(m_plugin, p);
}

void RemoteVSTServer::getParameters(int p0, int pn, float *v)
{
    for (int i = p0; i <= pn; ++i)
        v[i - p0] = m_plugin->getParameter(m_plugin, i);
}

std::string RemoteVSTServer::getProgramNameIndexed(int p)
{
    if (debugLevel > 1)
        cerr << "dssi-vst-server[2]: getProgramName(" << p << ")" << endl;

    char name[512];
    memset(name, 0, sizeof(name));

    m_plugin->dispatcher(m_plugin, effGetProgramNameIndexed, p, 0, name, 0);
    return name;
}

std::string
RemoteVSTServer::getProgramName()
{
    if (debugLevel > 1)
        cerr << "dssi-vst-server[2]: getProgramName()" << endl;

    char name[512];
    memset(name, 0, sizeof(name));

    m_plugin->dispatcher(m_plugin, effGetProgramName, 0, 0, name, 0);
    return name;
}

void
RemoteVSTServer::setCurrentProgram(int p)
{
    if (debugLevel > 1)
        cerr << "dssi-vst-server[2]: setCurrentProgram(" << p << ")" << endl;

/*
    if ((hostreaper == 1) && (setprogrammiss == 1))
    {
        writeIntring(&m_shmControl5->ringBuffer, 1);
        return;
    }
*/

    if (p < m_plugin->numPrograms)
        m_plugin->dispatcher(m_plugin, effSetProgram, 0, p, 0, 0);
}

bool RemoteVSTServer::warn(std::string warning)
{
    return true;
}

void RemoteVSTServer::GetRect()
{
#ifdef EMBED
        m_plugin->dispatcher(m_plugin, effEditGetRect, 0, 0, &rect, 0);

        winm.width = rect->right - rect->left;
        winm.height = rect->bottom - rect->top;
        winm.handle = 0;  
        tryWrite(&m_shm[FIXED_SHM_SIZE], &winm, sizeof(winm));
#endif
}

void RemoteVSTServer::showGUI()
{
#ifdef STANDALONE
    Atom dmessage;
    Atom winstate;
    Atom winmodal;
    ERect *eRect;
    int height = 0;
    int width = 0;
    int ret = 0;

    if (debugLevel > 0) {
	cerr << "RemoteVSTServer::showGUI(" << "): guiVisible is " << guiVisible << endl;
    }

    if(haveGui == false)
    {
    ret = 1;
    tryWrite(&m_shm[FIXED_SHM_SIZE], &ret, sizeof(int));
    return;
    }

    if (guiVisible)
    {
    ret = 1;
    tryWrite(&m_shm[FIXED_SHM_SIZE], &ret, sizeof(int));
    return;
    }

    x11_dpy = 0;

    x11_dpy = XOpenDisplay(0);

    if(x11_dpy == 0)
    {
    ret = 1;
    tryWrite(&m_shm[FIXED_SHM_SIZE], &ret, sizeof(int));
    return;       
    }

    x11_win = 0;

    x11_win = XCreateSimpleWindow(x11_dpy, DefaultRootWindow(x11_dpy), 0, 0, 300, 300, 0, 0, 0);

     if(x11_win == 0)
     {
    
    XCloseDisplay(x11_dpy);

    ret = 1;
    tryWrite(&m_shm[FIXED_SHM_SIZE], &ret, sizeof(int));
    return;       
     }

    winstate = XInternAtom(x11_dpy, "_NET_WM_STATE", True);
    winmodal = XInternAtom(x11_dpy, "_NET_WM_STATE_ABOVE", True);
    XChangeProperty(x11_dpy, x11_win, winstate, XA_ATOM, 32, PropModeReplace, (unsigned char*)&winmodal, 1);

    XMapWindow(x11_dpy, x11_win);

    XFlush(x11_dpy);
 
    XSelectInput(x11_dpy, x11_win, SubstructureNotifyMask | ButtonPressMask | ButtonReleaseMask
                 | ButtonMotionMask | ExposureMask | KeyPressMask);

    dmessage = XInternAtom(x11_dpy, "WM_DELETE_WINDOW", false);
     
    XSetWMProtocols(x11_dpy, x11_win, &dmessage, 1);

    m_plugin->dispatcher(m_plugin, effEditGetRect, 0, 0, &eRect, 0);

    m_plugin->dispatcher(m_plugin, effEditOpen, 0, (VstIntPtr) x11_dpy, (void *) x11_win, 0);
 
    m_plugin->dispatcher(m_plugin, effEditGetRect, 0, 0, &eRect, 0);
 
    if (eRect) {
        width = eRect->right - eRect->left;
        height = eRect->bottom - eRect->top;
        
        if((width == 0) || (height == 0))
        {
        
        XDestroyWindow (x11_dpy, x11_win);
        XCloseDisplay(x11_dpy);

        x11_dpy = NULL;

        ret = 1;
        tryWrite(&m_shm[FIXED_SHM_SIZE], &ret, sizeof(int));   
        return;
        }
        
        XResizeWindow(x11_dpy, x11_win, width, height);
        
        }
        else
        {       
        XDestroyWindow (x11_dpy, x11_win);
        XCloseDisplay(x11_dpy);
  
        ret = 1;
        tryWrite(&m_shm[FIXED_SHM_SIZE], &ret, sizeof(int));        
        return;
          
        }

   XStoreName(x11_dpy, x11_win, "LinVst-Linux");

   XFlush(x11_dpy);

   XSync(x11_dpy, false);

   guiVisible = true;


// usleep(100000);

    tryWrite(&m_shm[FIXED_SHM_SIZE], &ret, sizeof(int));
#else
        winm.handle = 0;
        winm.width = 0;
        winm.height = 0;

    if (debugLevel > 0)
        cerr << "RemoteVSTServer::showGUI(" << "): guiVisible is " << guiVisible << endl;

    if (haveGui == false)
    {
        winm.handle = 0;
        winm.width = 0;
        winm.height = 0;
        tryRead(&m_shm[FIXED_SHM_SIZE], &winm, sizeof(winm));
        tryWrite(&m_shm[FIXED_SHM_SIZE], &winm, sizeof(winm));
        return;
    }

    if (guiVisible)
    {
        winm.handle = 0;
        winm.width = 0;
        winm.height = 0;
        tryRead(&m_shm[FIXED_SHM_SIZE], &winm, sizeof(winm));
        tryWrite(&m_shm[FIXED_SHM_SIZE], &winm, sizeof(winm));
        return;
    }

    x11_dpy = 0;

    x11_dpy = XOpenDisplay(0);

    if(!x11_dpy)
    {
        winm.handle = 0;
        winm.width = 0;
        winm.height = 0;
        tryRead(&m_shm[FIXED_SHM_SIZE], &winm, sizeof(winm));
        tryWrite(&m_shm[FIXED_SHM_SIZE], &winm, sizeof(winm));
        return;
    }

    tryRead(&m_shm[FIXED_SHM_SIZE], &winm, sizeof(winm));

    x11_win = winm.handle;

    if(!x11_win)
    {
        winm.handle = 0;
        winm.width = 0;
        winm.height = 0;
        tryRead(&m_shm[FIXED_SHM_SIZE], &winm, sizeof(winm));
        tryWrite(&m_shm[FIXED_SHM_SIZE], &winm, sizeof(winm));
        return;
   }

    XMapWindow(x11_dpy, x11_win);

    XFlush(x11_dpy);
 
    XSelectInput(x11_dpy, x11_win, SubstructureNotifyMask | ButtonPressMask | ButtonReleaseMask
                 | ButtonMotionMask | ExposureMask | KeyPressMask);

    m_plugin->dispatcher(m_plugin, effEditGetRect, 0, 0, &rect, 0);

    m_plugin->dispatcher(m_plugin, effEditOpen, 0, (VstIntPtr) x11_dpy, (void *) x11_win, 0);
 
    m_plugin->dispatcher(m_plugin, effEditGetRect, 0, 0, &rect, 0);

    if (!rect)
    {
        // cerr << "dssi-vst-server: ERROR: Plugin failed to report window size\n" << endl;
     //   XDestroyWindow (x11_dpy, x11_win);
        XCloseDisplay(x11_dpy);
        guiVisible = false;
        winm.handle = 0;
        winm.width = 0;
        winm.height = 0;
        tryWrite(&m_shm[FIXED_SHM_SIZE], &winm, sizeof(winm));
        return;
    }

        if (debugLevel > 0)
            cerr << "dssi-vst-server[1]: sized window" << endl;

        winm.width = rect->right - rect->left;
        winm.height = rect->bottom - rect->top;
        winm.handle = 0;

        XFlush(x11_dpy);

        XSync(x11_dpy, false);

        guiVisible = true;
   
        tryWrite(&m_shm[FIXED_SHM_SIZE], &winm, sizeof(winm));
   
#endif
}

void RemoteVSTServer::hideGUI2()
{
#ifdef STANDALONE
int ret;

    if(haveGui == false)
    {
    ret = 1;
    tryWrite(&m_shm[FIXED_SHM_SIZE], &ret, sizeof(int));  
    return;
    }

    if (guiVisible == false)
    {
    ret = 1;
    tryWrite(&m_shm[FIXED_SHM_SIZE], &ret, sizeof(int));  
    return;
    }

if(x11_dpy)
{
m_plugin->dispatcher(m_plugin, effEditClose, 0, 0, 0, 0);

XDestroyWindow (x11_dpy, x11_win);

XCloseDisplay(x11_dpy);

x11_dpy = NULL;

}
#else
    // if (!hWnd)
        // return;
/*
    if ((haveGui == false) || (guiVisible == false))
    {
    hideguival = 0;
    return;
    }
    
*/   
 
if(x11_dpy && guiVisible && hideguival)
{
XCloseDisplay(x11_dpy);

x11_dpy = 0;
x11_win = 0;
}

#endif

    guiVisible = false;

    hideguival = 0;

   // if (!exiting)
    //    usleep(50000);
}

void RemoteVSTServer::hideGUI()
{
if ((haveGui) && guiVisible)
     hideguival = 1;
}

#ifdef EMBED
void RemoteVSTServer::openGUI()
{
    guiVisible = true;

        XFlush(x11_dpy);

        XSync(x11_dpy, false);
}
#endif

int RemoteVSTServer::processVstEvents()
{
    int         els;
    int         *ptr;
    int         sizeidx = 0;
    int         size;
    VstEvents   *evptr;

    ptr = (int *) m_shm2;
    els = *ptr;
    sizeidx = sizeof(int);

    if (els > VSTSIZE)
        els = VSTSIZE;

    evptr = &vstev[0];
    evptr->numEvents = els;
    evptr->reserved = 0;

    for (int i = 0; i < els; i++)
    {
        VstEvent* bsize = (VstEvent*) &m_shm2[sizeidx];
        size = bsize->byteSize + (2*sizeof(VstInt32));
        evptr->events[i] = bsize;
        sizeidx += size;
    }

    m_plugin->dispatcher(m_plugin, effProcessEvents, 0, 0, evptr, 0);

    return 1;
}

void RemoteVSTServer::getChunk()
{
#ifdef CHUNKBUF
    int bnk_prg = readIntring(&m_shmControl5->ringBuffer);
    int sz = m_plugin->dispatcher(m_plugin, effGetChunk, bnk_prg, 0, &chunkptr, 0);

if(sz >= CHUNKSIZEMAX)
{
    writeInt(&m_shm[FIXED_SHM_SIZE], sz);
    return;
}
else
{
    if(sz < CHUNKSIZEMAX)    
    tryWrite(&m_shm[FIXED_SHM_SIZECHUNKSTART], chunkptr, sz);
    writeInt(&m_shm[FIXED_SHM_SIZE], sz);
    return;
}
#else
    void *ptr;
    int bnk_prg = readIntring(&m_shmControl5->ringBuffer);
    int sz = m_plugin->dispatcher(m_plugin, effGetChunk, bnk_prg, 0, &ptr, 0);
    if(sz < CHUNKSIZEMAX)
    tryWrite(&m_shm[FIXED_SHM_SIZECHUNKSTART], ptr, sz);
    writeInt(&m_shm[FIXED_SHM_SIZE], sz);
    return;
#endif
}

void RemoteVSTServer::setChunk()
{
#ifdef CHUNKBUF
    int sz = readIntring(&m_shmControl5->ringBuffer);
    if(sz >= CHUNKSIZEMAX)
{
    int bnk_prg = readIntring(&m_shmControl5->ringBuffer);
    void *ptr = chunkptr2;
    int r = m_plugin->dispatcher(m_plugin, effSetChunk, bnk_prg, sz, ptr, 0);
    free(chunkptr2);
    writeInt(&m_shm[FIXED_SHM_SIZE], r);
    return;
}
else
{
    int bnk_prg = readIntring(&m_shmControl5->ringBuffer);
    void *ptr = &m_shm[FIXED_SHM_SIZECHUNKSTART];
    int r = m_plugin->dispatcher(m_plugin, effSetChunk, bnk_prg, sz, ptr, 0);
    writeInt(&m_shm[FIXED_SHM_SIZE], r);
    return;
}
#else
    int sz = readIntring(&m_shmControl5->ringBuffer);
    int bnk_prg = readIntring(&m_shmControl5->ringBuffer);
    void *ptr = &m_shm[FIXED_SHM_SIZECHUNKSTART];
    int r = m_plugin->dispatcher(m_plugin, effSetChunk, bnk_prg, sz, ptr, 0);
    writeInt(&m_shm[FIXED_SHM_SIZE], r);
    return;
#endif
}

void RemoteVSTServer::canBeAutomated()
{
    int param = readIntring(&m_shmControl5->ringBuffer);
    int r = m_plugin->dispatcher(m_plugin, effCanBeAutomated, param, 0, 0, 0);
    writeInt(&m_shm[FIXED_SHM_SIZE], r);
}

void RemoteVSTServer::getProgram()
{
    int r = m_plugin->dispatcher(m_plugin, effGetProgram, 0, 0, 0, 0);
    writeInt(&m_shm[FIXED_SHM_SIZE], r);
}

#ifdef SEM

void
RemoteVSTServer::waitForServer()
{
    if(m_386run == 0)
    {
    sem_post(&m_shmControl->runServer);

    timespec ts_timeout;
    clock_gettime(CLOCK_REALTIME, &ts_timeout);
    ts_timeout.tv_sec += 60;
    if (sem_timedwait(&m_shmControl->runClient, &ts_timeout) != 0) {
         if(m_inexcept == 0)
         RemotePluginClosedException();
    }
    }
    else
    {
    fpost(&m_shmControl->runServer386);

    if (fwait(&m_shmControl->runClient386, 60000)) {
         if(m_inexcept == 0)
	 RemotePluginClosedException();
    }
   }
}


void
RemoteVSTServer::waitForServerexit()
{
    if(m_386run == 0)
    {
    sem_post(&m_shmControl->runServer);
    }
    else
    {
    fpost(&m_shmControl->runServer386);
   }
}


#else

void
RemoteVSTServer::waitForServer()
{
    fpost(&m_shmControl->runServer);

    if (fwait(&m_shmControl->runClient, 60000)) {
         if(m_inexcept == 0)
	 RemotePluginClosedException();
    }
}

void
RemoteVSTServer::waitForServerexit()
{
    fpost(&m_shmControl->runServer);
}

#endif

#ifdef VESTIGE
VstIntPtr VESTIGECALLBACK hostCallback(AEffect *plugin, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt)
#else
VstIntPtr VSTCALLBACK hostCallback(AEffect *plugin, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt)
#endif
{
    VstTimeInfo timeInfo;
    int rv = 0;
    int retval = 0;

    switch (opcode)
    {
    case audioMasterAutomate:
   //     plugin->setParameter(plugin, index, opt);
    if(remoteVSTServerInstance)
    {	    
    if(!remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun)
    {	    
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)opcode);
    remoteVSTServerInstance->writeIntring(&remoteVSTServerInstance->m_shmControl->ringBuffer, index);
    remoteVSTServerInstance->writeFloatring(&remoteVSTServerInstance->m_shmControl->ringBuffer, opt);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();
    retval = 0;
    memcpy(&retval, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3], sizeof(int));
    rv = retval;
      }
     }
        break;

    case audioMasterVersion:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterVersion requested" << endl;
        rv = 2400;
        break;

    case audioMasterCurrentId:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterCurrentId requested" << endl;
        break;

    case audioMasterIdle:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterIdle requested " << endl;
        // plugin->dispatcher(plugin, effEditIdle, 0, 0, 0, 0);
        break;

    case DEPRECATED_VST_SYMBOL(audioMasterPinConnected):
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterPinConnected requested" << endl;
        break;

    case DEPRECATED_VST_SYMBOL(audioMasterWantMidi):
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterWantMidi requested" << endl;
        // happy to oblige
        rv = 1;
        break;

    case audioMasterGetTime:
    
    if(remoteVSTServerInstance)
    {	
    if (!remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun)
    {
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)opcode);
    remoteVSTServerInstance->writeIntring(&remoteVSTServerInstance->m_shmControl->ringBuffer, value);   
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();

    if(remoteVSTServerInstance->timeinfo)
    {
    memcpy(remoteVSTServerInstance->timeinfo, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3 - sizeof(VstTimeInfo)], sizeof(VstTimeInfo));

    // printf("%f\n", timeInfo.sampleRate);

    rv = (long)remoteVSTServerInstance->timeinfo;
    }
    }
    }
    
        break;

    case audioMasterProcessEvents:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterProcessEvents requested" << endl;
        {
            VstEvents   *evnts;
            int         eventnum;
            int         *ptr2;
            int         sizeidx = 0;
            int         ok;

	    if(remoteVSTServerInstance)
            {	
            if (!remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun)
            {
                evnts = (VstEvents*)ptr;

                if ((!evnts) || (evnts->numEvents <= 0))
                    break;

                eventnum = evnts->numEvents;

                ptr2 = (int *)remoteVSTServerInstance->m_shm3;

                sizeidx = sizeof(int);

                if (eventnum > VSTSIZE)
                    eventnum = VSTSIZE;            

                for (int i = 0; i < evnts->numEvents; i++)
                {

                    VstEvent *pEvent = evnts->events[i];
                    if (pEvent->type == kVstSysExType)
                        eventnum--;
                    else
                    {
                        unsigned int size = (2*sizeof(VstInt32)) + evnts->events[i]->byteSize;
                        memcpy(&remoteVSTServerInstance->m_shm3[sizeidx], evnts->events[i], size);
                        sizeidx += size;
                    }
                }
                *ptr2 = eventnum;

    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)opcode);
  //  remoteVSTServerInstance->writeIntring(&remoteVSTServerInstance->m_shmControl->ringBuffer, value);
   
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();
    retval = 0;
    memcpy(&retval, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3], sizeof(int));
    rv = retval;
        }       
       }
      }
        break;

    case DEPRECATED_VST_SYMBOL(audioMasterSetTime):
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterSetTime requested" << endl;
        break;

    case DEPRECATED_VST_SYMBOL(audioMasterTempoAt):
        // if (debugLevel > 1)
            // cerr << "dssi-vst-server[2]: audioMasterTempoAt requested" << endl;
        // can't support this, return 120bpm
        rv = 120 * 10000;
        break;

    case DEPRECATED_VST_SYMBOL(audioMasterGetNumAutomatableParameters):
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterGetNumAutomatableParameters requested" << endl;
        rv = 5000;
        break;

    case DEPRECATED_VST_SYMBOL(audioMasterGetParameterQuantization):
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterGetParameterQuantization requested" << endl;
        rv = 1;
        break;

    case audioMasterIOChanged:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterIOChanged requested" << endl;
    {        
    struct amessage
    {
        int flags;
        int pcount;
        int parcount;
        int incount;
        int outcount;
        int delay;
    } am;

    if(remoteVSTServerInstance)
    {		    
    if (!remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun)
    {
        am.flags = plugin->flags;
        am.pcount = plugin->numPrograms;
        am.parcount = plugin->numParams;
        am.incount = plugin->numInputs;
        am.outcount = plugin->numOutputs;
        am.delay = plugin->initialDelay;
#ifndef DOUBLEP
        am.flags &= ~effFlagsCanDoubleReplacing;
#endif

        memcpy(&remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3], &am, sizeof(am));

    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)opcode);
   
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();
    retval = 0;
    memcpy(&retval, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3], sizeof(int));
    rv = retval;
	    
    if((am.incount != remoteVSTServerInstance->m_numInputs) || (am.outcount != remoteVSTServerInstance->m_numOutputs))
    {
    if ((am.incount + am.outcount) * remoteVSTServerInstance->m_bufferSize * sizeof(float) < (PROCESSSIZE))
    {
    remoteVSTServerInstance->m_updateio = 1;
    remoteVSTServerInstance->m_updatein = am.incount;
    remoteVSTServerInstance->m_updateout = am.outcount;
    }
    }
/*
        AEffect* update = remoteVSTServerInstance->m_plugin;
        update->flags = am.flags;
        update->numPrograms = am.pcount;
        update->numParams = am.parcount;
        update->numInputs = am.incount;
        update->numOutputs = am.outcount;
        update->initialDelay = am.delay;
*/
       }
      }
     }
        break;

    case DEPRECATED_VST_SYMBOL(audioMasterNeedIdle):
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterNeedIdle requested" << endl;
        // might be nice to handle this better
        rv = 1;
        break;

    case audioMasterSizeWindow:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterSizeWindow requested" << endl;

       XResizeWindow(remoteVSTServerInstance->x11_dpy, remoteVSTServerInstance->x11_win, index, value);
       XFlush(remoteVSTServerInstance->x11_dpy);
       XSync(remoteVSTServerInstance->x11_dpy, false);

       rv = 1;   
       break;

    case audioMasterGetSampleRate:
          if (debugLevel > 1)
           cerr << "dssi-vst-server[2]: audioMasterGetSampleRate requested" << endl;
/*		
    if(remoteVSTServerInstance)
    {	
        if (!remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun)
        {
        if (!remoteVSTServerInstance->sampleRate)
        {
            //  cerr << "WARNING: Sample rate requested but not yet set" << endl;
            break;
        }
        plugin->dispatcher(plugin, effSetSampleRate, 0, 0, NULL, (float)remoteVSTServerInstance->sampleRate);
        }
	}
*/	
/*   
    if(remoteVSTServerInstance)
    {	
    if (!remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun)
    {
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)opcode);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();
    retval = 0;
    memcpy(&retval, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3], sizeof(int));
    rv = retval;
      }
     }
    */
    if(remoteVSTServerInstance)
    {	    
    if (!remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun)
    {
    rv = remoteVSTServerInstance->sampleRate;
    }
    }
        break;

    case audioMasterGetBlockSize:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterGetBlockSize requested" << endl;
/*
    if(remoteVSTServerInstance)
    {	
        if (!remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun)
        {
        if (!remoteVSTServerInstance->bufferSize)
        {
            // cerr << "WARNING: Buffer size requested but not yet set" << endl;
            break;
        }
        plugin->dispatcher(plugin, effSetBlockSize, 0, remoteVSTServerInstance->bufferSize, NULL, 0);
        }
	}
*/	
/*
    if(remoteVSTServerInstance)
    {	
    if (!remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun)
    {
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)opcode);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();
    retval = 0;
    memcpy(&retval, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3], sizeof(int));
    rv = retval;
      }
     }
 */
    if(remoteVSTServerInstance)
    {
    if (!remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun)
    {
    rv = remoteVSTServerInstance->bufferSize;
    }
    }
        break;

    case audioMasterGetInputLatency:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterGetInputLatency requested" << endl;
/*		    
    if(remoteVSTServerInstance)
    {	
    if (!remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun)
    {
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)opcode);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();
    retval = 0;
    memcpy(&retval, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3], sizeof(int));
    rv = retval;
      }
     }
*/
        break;

    case audioMasterGetOutputLatency:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterGetOutputLatency requested" << endl;
/*		    
    if(remoteVSTServerInstance)
    {	
    if (!remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun)
    {
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)opcode);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();
    retval = 0;
    memcpy(&retval, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3], sizeof(int));
    rv = retval;
      }
     }
*/
        break;

    case DEPRECATED_VST_SYMBOL(audioMasterGetPreviousPlug):
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterGetPreviousPlug requested" << endl;
        break;

    case DEPRECATED_VST_SYMBOL(audioMasterGetNextPlug):
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterGetNextPlug requested" << endl;
        break;

    case DEPRECATED_VST_SYMBOL(audioMasterWillReplaceOrAccumulate):
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterWillReplaceOrAccumulate requested" << endl;
        // 0 -> unsupported, 1 -> replace, 2 -> accumulate
        rv = 1;
        break;

    case audioMasterGetCurrentProcessLevel:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterGetCurrentProcessLevel requested" << endl;
        // 0 -> unsupported, 1 -> gui, 2 -> process, 3 -> midi/timer, 4 -> offline
		    
    if(remoteVSTServerInstance)
    {	
    if (!remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun)
    {
        if (remoteVSTServerInstance->inProcessThread)
            rv = 2;
        else
            rv = 1;
    }
    }
        break;

    case audioMasterGetAutomationState:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterGetAutomationState requested" << endl;

    if(remoteVSTServerInstance)
    {	
    if (!remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun)
    {
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)opcode);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();
    retval = 0;
    memcpy(&retval, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3], sizeof(int));
    rv = retval;
      }
     }
   //     rv = 4; // read/write
        break;

    case audioMasterOfflineStart:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterOfflineStart requested" << endl;
        break;

    case audioMasterOfflineRead:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterOfflineRead requested" << endl;
        break;

    case audioMasterOfflineWrite:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterOfflineWrite requested" << endl;
        break;

    case audioMasterOfflineGetCurrentPass:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterOfflineGetCurrentPass requested" << endl;
        break;

    case audioMasterOfflineGetCurrentMetaPass:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterOfflineGetCurrentMetaPass requested" << endl;
        break;

    case DEPRECATED_VST_SYMBOL(audioMasterSetOutputSampleRate):
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterSetOutputSampleRate requested" << endl;
        break;

/* Deprecated in VST 2.4 and also (accidentally?) renamed in the SDK header,
   so we won't retain it here
    case audioMasterGetSpeakerArrangement:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterGetSpeakerArrangement requested" << endl;
        break;
*/
    case audioMasterGetVendorString:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterGetVendorString requested" << endl;
{
    char retstr[512];

    if(remoteVSTServerInstance)
    {	
    if (!remoteVSTServerInstance->exiting)
    {
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)opcode);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();
    strcpy(retstr, remoteVSTServerInstance->readString(&remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3]).c_str()); 
    strcpy((char *)ptr, retstr);
    memcpy(&retval, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3 + 512], sizeof(int));
    rv = retval;
      }
     }
   }
        break;

    case audioMasterGetProductString:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterGetProductString requested" << endl;
{
    char retstr[512];

    if(remoteVSTServerInstance)
    {	
    if (!remoteVSTServerInstance->exiting)
    {
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)opcode);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();
    strcpy(retstr, remoteVSTServerInstance->readString(&remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3]).c_str()); 
    strcpy((char *)ptr, retstr);
    memcpy(&retval, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3 + 512], sizeof(int));
    rv = retval;
      }
     }
    }
        break;

    case audioMasterGetVendorVersion:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterGetVendorVersion requested" << endl;
  
    if(remoteVSTServerInstance)
    {	
    if (!remoteVSTServerInstance->exiting)
    {
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)opcode);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();
    retval = 0;
    memcpy(&retval, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3], sizeof(int));
    rv = retval;
    }
    }	    
        break;

    case audioMasterVendorSpecific:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterVendorSpecific requested" << endl;
        break;

    case DEPRECATED_VST_SYMBOL(audioMasterSetIcon):
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterSetIcon requested" << endl;
        break;

    case audioMasterCanDo:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterCanDo(" << (char *)ptr << ") requested" << endl;
#ifdef CANDOEFF
    {
    int retval;

    if(remoteVSTServerInstance && !remoteVSTServerInstance->exiting)
    {	
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)opcode);
    remoteVSTServerInstance-> writeString(&remoteVSTServerInstance->m_shm[FIXED_SHM_SIZE3], (char*)ptr);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();

    memcpy(&retval, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3], sizeof(int));
    rv = retval;  
    }
    }
#else		    
        if (!strcmp((char*)ptr, "sendVstEvents")
                    || !strcmp((char*)ptr, "sendVstMidiEvent")
                    || !strcmp((char*)ptr, "sendVstMidiEvent")
                    || !strcmp((char*)ptr, "receiveVstEvents")
                    || !strcmp((char*)ptr, "receiveVstMidiEvents")
                    || !strcmp((char*)ptr, "acceptIOChanges")
                    || !strcmp((char*)ptr, "startStopProcess")
#ifdef EMBED
#ifdef EMBEDRESIZE
                    || !strcmp((char*)ptr, "sizeWindow")
#endif
#else
                    || !strcmp((char*)ptr, "sizeWindow")
#endif
                    // || !strcmp((char*)ptr, "supplyIdle")
                    )
            rv = 1;
#endif 		    
        break;

    case audioMasterGetLanguage:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterGetLanguage requested" << endl;
        rv = kVstLangEnglish;
        break;

    case DEPRECATED_VST_SYMBOL(audioMasterOpenWindow):
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterOpenWindow requested" << endl;
        break;

    case DEPRECATED_VST_SYMBOL(audioMasterCloseWindow):
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterCloseWindow requested" << endl;
        break;

    case audioMasterGetDirectory:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterGetDirectory requested" << endl;
        break;

    case audioMasterUpdateDisplay:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterUpdateDisplay requested" << endl;
        break;

    case audioMasterBeginEdit:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterBeginEdit requested" << endl;
    
    if(remoteVSTServerInstance)
    {	
    if (!remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun)
    {
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)opcode);
    remoteVSTServerInstance->writeIntring(&remoteVSTServerInstance->m_shmControl->ringBuffer, index);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();
    retval = 0;
    memcpy(&retval, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3], sizeof(int));
    rv = retval;
      }
     }	    
        break;

    case audioMasterEndEdit:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterEndEdit requested" << endl;
    
    if(remoteVSTServerInstance)
    {	
    if (!remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun)
    {
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)opcode);
    remoteVSTServerInstance->writeIntring(&remoteVSTServerInstance->m_shmControl->ringBuffer, index);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();
    retval = 0;
    memcpy(&retval, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3], sizeof(int));
    rv = retval;
      }
     }
        break;

    case audioMasterOpenFileSelector:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterOpenFileSelector requested" << endl;
        break;

    case audioMasterCloseFileSelector:
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterCloseFileSelector requested" << endl;
        break;

    case DEPRECATED_VST_SYMBOL(audioMasterEditFile):
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterEditFile requested" << endl;
        break;

    case DEPRECATED_VST_SYMBOL(audioMasterGetChunkFile):
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterGetChunkFile requested" << endl;
        break;

    case DEPRECATED_VST_SYMBOL(audioMasterGetInputSpeakerArrangement):
        if (debugLevel > 1)
            cerr << "dssi-vst-server[2]: audioMasterGetInputSpeakerArrangement requested" << endl;
        break;

    default:
        if (debugLevel > 0)
            cerr << "dssi-vst-server[0]: unsupported audioMaster callback opcode " << opcode << endl;
    }
    return rv;
}

int main(int argc, char **argv)
{
    string pathName;
    string fileName;
    char cdpath[4096];
    char *libname = 0;
    char *libname2 = 0;
    char *fileInfo = 0;
    char **cmdline2;
    char *cmdline;

    cout << "DSSI VST plugin server v" << RemotePluginVersion << endl;
    cout << "Copyright (c) 2012-2013 Filipe Coelho" << endl;
    cout << "Copyright (c) 2010-2011 Kristian Amlie" << endl;
    cout << "Copyright (c) 2004-2006 Chris Cannam" << endl;
    cout << "LinVst version 2.32" << endl;

   cmdline2 = &argv[1];
   cmdline = *cmdline2;

    if (cmdline)
    {
        int offset = 0;
        if (cmdline[0] == '"' || cmdline[0] == '\'') offset = 1;
            for (int ci = offset; cmdline[ci]; ++ci)
            {
                if (cmdline[ci] == ',')
                {
                    libname2 = strndup(cmdline + offset, ci - offset);
                    ++ci;
                    if (cmdline[ci])
                    {
                        fileInfo = strdup(cmdline + ci);
                        int l = strlen(fileInfo);
                        if (fileInfo[l-1] == '"' || fileInfo[l-1] == '\'')
                            fileInfo[l-1] = '\0';
                    }
                }
            }
    }

    if (libname2 != NULL)
    {
        if ((libname2[0] == '/') && (libname2[1] == '/'))
            libname = strdup(&libname2[1]);
        else
            libname = strdup(libname2);
    }
    else
    {
        cerr << "Usage: dssi-vst-server <vstname.dll>,<tmpfilebase>" << endl;
        cerr << "(Command line was: " << cmdline << ")" << endl;
        return 1;
    }

    if (!libname || !libname[0] || !fileInfo || !fileInfo[0])
    {
        cerr << "Usage: dssi-vst-server <vstname.dll>,<tmpfilebase>" << endl;
        cerr << "(Command line was: " << cmdline << ")" << endl;
        return 1;
    }

    cout << "Loading  " << libname << endl;

    void* libHandle = 0;

    libHandle = dlopen(libname, RTLD_LOCAL | RTLD_LAZY);

    if (!libHandle) {
	cerr << "dssi-vst-server: ERROR: Couldn't load VST DLL \"" << libname << "\"" << endl;
	return 1;
    }

    PluginEntryProc g_entry;

    g_entry = (PluginEntryProc) dlsym(libHandle, "VSTPluginMain");
		if (!g_entry) g_entry = (PluginEntryProc) dlsym(libHandle, "main");
		
	if (!g_entry) {
	    cerr << "dssi-vst-server: ERROR: VST entrypoints \""
		 << NEW_PLUGIN_ENTRY_POINT << "\" or \"" 
		 << OLD_PLUGIN_ENTRY_POINT << "\" not found in DLL \""
		 << libname << "\"" << endl;
	    
          dlclose(libHandle);
          return 1;

	} 
		
   AEffect *plugin = g_entry(hostCallback);

    if (!plugin) {
	cerr << "dssi-vst-server: ERROR: Failed to instantiate plugin in VST DLL \""
	     << libname << "\"" << endl;
	
          dlclose(libHandle);
          return 1;
    } 
	
	remoteVSTServerInstance = 0;
	
        remoteVSTServerInstance = new RemoteVSTServer(fileInfo, libname);
    
        if(!remoteVSTServerInstance)
        {
        cerr << "ERROR: Remote VST startup failed" << endl;
        if(libHandle)
        dlclose(libHandle);
        return 1; 
        }

        if(remoteVSTServerInstance->starterror == 1)
        {
        cerr << "ERROR: Remote VST startup failed and/or mismatched LinVst versions" << endl;
 
	if(remoteVSTServerInstance)
	delete remoteVSTServerInstance;
	if(libHandle)
    dlclose(libHandle);
    return 1; 
        }

    remoteVSTServerInstance->m_plugin = g_entry(hostCallback);
    if (!remoteVSTServerInstance->m_plugin)
    {
    cerr << "dssi-vst-server: ERROR: Failed to instantiate plugin in VST DLL \"" << libname << "\"" << endl;
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)disconnectserver);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();  
    remoteVSTServerInstance->waitForClient2exit();
    remoteVSTServerInstance->waitForClient3exit();
    remoteVSTServerInstance->waitForClient4exit();
    remoteVSTServerInstance->waitForClient5exit();
	if(remoteVSTServerInstance)
	delete remoteVSTServerInstance;
	if(libHandle)
    dlclose(libHandle);
    return 1;
    }

    if (remoteVSTServerInstance->m_plugin->magic != kEffectMagic)
    {
     cerr << "dssi-vst-server: ERROR: Not a VST plugin in DLL \"" << libname << "\"" << endl;
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)disconnectserver);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();  
    remoteVSTServerInstance->waitForClient2exit();
    remoteVSTServerInstance->waitForClient3exit();
    remoteVSTServerInstance->waitForClient4exit();
    remoteVSTServerInstance->waitForClient5exit();
	if(remoteVSTServerInstance)
	delete remoteVSTServerInstance;
	if(libHandle)
    dlclose(libHandle);
    return 1;
    }

    if (!(remoteVSTServerInstance->m_plugin->flags & effFlagsCanReplacing))
    {
    cerr << "dssi-vst-server: ERROR: Plugin does not support processReplacing (required)" << endl;
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)disconnectserver);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();  
    remoteVSTServerInstance->waitForClient2exit();
    remoteVSTServerInstance->waitForClient3exit();
    remoteVSTServerInstance->waitForClient4exit();
    remoteVSTServerInstance->waitForClient5exit();
	if(remoteVSTServerInstance)
	delete remoteVSTServerInstance;
	if(libHandle)
    dlclose(libHandle);
    return 1;
    }

    if(remoteVSTServerInstance->m_plugin->flags & effFlagsHasEditor)
    remoteVSTServerInstance->haveGui = true;
    else 
    remoteVSTServerInstance->haveGui = false;
	
pthread_t m_ParThread, m_GetSetThread, m_AudioThread;

if(pthread_create(&m_ParThread, NULL, ParThreadMain, NULL))
    {
    cerr << "Failed to create par thread!" << endl;
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)disconnectserver);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();  
    remoteVSTServerInstance->waitForClient2exit();
    remoteVSTServerInstance->waitForClient3exit();
    remoteVSTServerInstance->waitForClient4exit();
    remoteVSTServerInstance->waitForClient5exit();
	if(remoteVSTServerInstance)
	delete remoteVSTServerInstance;
    dlclose(libHandle);
    return 1;
    }  

if(pthread_create(&m_GetSetThread, NULL, GetSetThreadMain, NULL))
    {
    cerr << "Failed to create getset thread!" << endl;

    if(m_ParThread)
    pthread_join(m_ParThread, NULL);

    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)disconnectserver);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();  
    remoteVSTServerInstance->waitForClient2exit();
    remoteVSTServerInstance->waitForClient3exit();
    remoteVSTServerInstance->waitForClient4exit();
    remoteVSTServerInstance->waitForClient5exit();
	if(remoteVSTServerInstance)
	delete remoteVSTServerInstance;
    dlclose(libHandle);
    return 1;
    }  
 
if(pthread_create(&m_AudioThread, NULL, AudioThreadMain, NULL))
     {
    cerr << "Failed to create audio thread!" << endl;

    if(m_ParThread)
    pthread_join(m_ParThread, NULL);

    if(m_GetSetThread)
    pthread_join(m_GetSetThread, NULL);

    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)disconnectserver);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();  
    remoteVSTServerInstance->waitForClient2exit();
    remoteVSTServerInstance->waitForClient3exit();
    remoteVSTServerInstance->waitForClient4exit();
    remoteVSTServerInstance->waitForClient5exit();
	if(remoteVSTServerInstance)
	delete remoteVSTServerInstance;
    dlclose(libHandle);
    return 1;
    }  

    int tcount = 0;
    remoteVSTServerInstance->exiting = false;

   while (!remoteVSTServerInstance->exiting) {

	if (remoteVSTServerInstance->exiting) 
    break;


	   if(remoteVSTServerInstance->hideguival == 1)
           {
           remoteVSTServerInstance->hideGUI2();
//         remoteVSTServerInstance->hideguival = 0;
           }        

           remoteVSTServerInstance->dispatchControl(100);

    }

    // wait for audio thread to catch up
    // sleep(1);

    for (int i=0;i<1000;i++)
    {
        usleep(10000);
        if (remoteVSTServerInstance->parfin && remoteVSTServerInstance->audfin && remoteVSTServerInstance->getfin)
            break;
    }

    if (debugLevel > 0)
        cerr << "dssi-vst-server[1]: cleaning up" << endl;

    if(m_ParThread)
    pthread_join(m_ParThread, NULL);

    if(m_GetSetThread)
    pthread_join(m_ParThread, NULL);

    if(m_AudioThread)
    pthread_join(m_AudioThread, NULL);
    if (debugLevel > 0)
        cerr << "dssi-vst-server[1]: closed threads" << endl;

    if(remoteVSTServerInstance)
    delete remoteVSTServerInstance;
	
    if(libHandle)
    dlclose(libHandle);

    if (debugLevel > 0)
        cerr << "dssi-vst-server[1]: freed dll" << endl;
    if (debugLevel > 0)
        cerr << "dssi-vst-server[1]: exiting" << endl;
    return 0;
}
