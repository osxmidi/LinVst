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

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "remotepluginserver.h"

#include "paths.h"

#define APPLICATION_CLASS_NAME "dssi_vst"
#ifdef TRACKTIONWM 
#define APPLICATION_CLASS_NAME2 "dssi_vst2"
#endif
#define OLD_PLUGIN_ENTRY_POINT "main"
#define NEW_PLUGIN_ENTRY_POINT "VSTPluginMain"

#if VST_FORCE_DEPRECATED
#define DEPRECATED_VST_SYMBOL(x) __##x##Deprecated
#else
#define DEPRECATED_VST_SYMBOL(x) x
#endif

#ifdef VESTIGE
typedef AEffect *(VESTIGECALLBACK *VstEntry)(audioMasterCallback audioMaster);
#else
typedef AEffect *(VSTCALLBACK *VstEntry)(audioMasterCallback audioMaster);
#endif

RemotePluginDebugLevel  debugLevel = RemotePluginDebugNone;

#define disconnectserver 32143215

#define hidegui2 77775634

using namespace std;

class RemoteVSTServer : public RemotePluginServer
{
public:
                        RemoteVSTServer(std::string fileIdentifiers, std::string fallbackName);
    virtual             ~RemoteVSTServer();

     // virtual std::string getName() { return m_name; }
     // virtual std::string getMaker() { return m_maker; }
    virtual std::string getName();
    virtual std::string getMaker();
    
    virtual void        setBufferSize(int);
    virtual void        setSampleRate(int);
    virtual void        reset();
    virtual void        terminate();

    virtual int         getInputCount() { if(m_plugin) return m_plugin->numInputs; }
    virtual int         getOutputCount() { if(m_plugin) return m_plugin->numOutputs; }
    virtual int         getFlags() { if(m_plugin) return m_plugin->flags; }
    virtual int         getinitialDelay() { if(m_plugin) return m_plugin->initialDelay; }
    virtual int         getUID() { if(m_plugin) return m_plugin->uniqueID; }
    virtual int         getParameterCount() { if(m_plugin) return m_plugin->numParams; }
    virtual std::string getParameterName(int);
    virtual std::string getParameterLabel(int);
    virtual std::string getParameterDisplay(int);
    virtual void        setParameter(int, float);
    virtual float       getParameter(int);
    virtual void        getParameters(int, int, float *);

    virtual int         getProgramCount() { if(m_plugin) return m_plugin->numPrograms; }
    virtual int         getProgramNameIndexed(int, char *name);
    virtual std::string getProgramName();
#ifdef WAVES
    virtual int         getShellName(char *name);
#endif
    virtual void        setCurrentProgram(int);

    virtual void        showGUI();
    virtual void        hideGUI();
#ifdef EMBED
    virtual void        openGUI();
#endif
    virtual void        guiUpdate();
    
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

    virtual bool        getOutProp(int);
    virtual bool        getInProp(int);

#ifdef MIDIEFF
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

    HWND                hWnd;
    WNDCLASSEX          wclass;
#ifdef TRACKTIONWM  
    WNDCLASSEX          wclass2;
#endif    
    UINT_PTR            timerval;
    bool                haveGui;
#ifdef EMBED
    HANDLE handlewin;
    struct winmessage
    {
    int handle;
    int width;
    int height;
    } winm2;    
    winmessage *winm;
#endif
    int guiupdate;
    int guiupdatecount;
    int guiresizewidth;
    int guiresizeheight;
    ERect               *rect;
    int                 setprogrammiss;
    int                 hostreaper;
    int                 melda;      
    int                 wavesthread;
#ifdef EMBED
#ifdef TRACKTIONWM  
    int                 hosttracktion;
#endif
#endif
    AEffect             *m_plugin;
    VstEvents           vstev[VSTSIZE];
    int                 bufferSize;
    int                 sampleRate;
    bool                exiting;
    bool                exiting2;
    bool                effectrun;
    bool                inProcessThread;
    bool                guiVisible;
    int                 parfin;
    int                 audfin;
    int                 getfin;
    int                 hidegui;    
    
    std::string         deviceName2;
    std::string         bufferwaves;

private:
    std::string         m_name;
    std::string         m_maker;
};

RemoteVSTServer         *remoteVSTServerInstance = 0;


LRESULT WINAPI MainProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
#ifndef EMBED
         if(remoteVSTServerInstance)
	 {
         if (!remoteVSTServerInstance->exiting && remoteVSTServerInstance->guiVisible)
	 {
         remoteVSTServerInstance->hidegui = 1;
//         remoteVSTServerInstance->hideGUI();	 
         return 0;
	 }
         }
#endif
    break;
		    
    case WM_TIMER:
	/*
         if(wParam == 678)
         {
         remoteVSTServerInstance->m_plugin->dispatcher (remoteVSTServerInstance->m_plugin, effEditIdle, 0, 0, NULL, 0);
         }	
	*/
     break;
       	
    default:
    return DefWindowProc(hWnd, msg, wParam, lParam);		   
    }
 return 0;
}

#ifdef TRACKTIONWM 
LRESULT WINAPI MainProc2(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
    break;
		    
    case WM_TIMER:
    break;
       	
    default:
    return DefWindowProc(hWnd, msg, wParam, lParam);		   
    }
 return 0;
}
#endif

DWORD WINAPI AudioThreadMain(LPVOID parameter)
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
    remoteVSTServerInstance->dispatchProcess(50);
    }
    // param.sched_priority = 0;
    // (void)sched_setscheduler(0, SCHED_OTHER, &param);
    remoteVSTServerInstance->audfin = 1;
    ExitThread(0);
    return 0;
}

DWORD WINAPI GetSetThreadMain(LPVOID parameter)
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
    remoteVSTServerInstance->dispatchGetSet(50);
    }
    // param.sched_priority = 0;
    // (void)sched_setscheduler(0, SCHED_OTHER, &param);
    remoteVSTServerInstance->getfin = 1;
    ExitThread(0);
    return 0;
}

DWORD WINAPI ParThreadMain(LPVOID parameter)
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
    remoteVSTServerInstance->dispatchPar(50);
    }
    // param.sched_priority = 0;
    // (void)sched_setscheduler(0, SCHED_OTHER, &param);
     remoteVSTServerInstance->parfin = 1;
     ExitThread(0);
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
    wavesthread(0),
#ifdef EMBED
    winm(0),
#ifdef TRACKTIONWM  
    hosttracktion(0),
#endif
#endif
    haveGui(true),
    timerval(0),
    exiting(false),
    exiting2(false),
    effectrun(false),
    inProcessThread(false),
    guiVisible(false),
    parfin(0),
    audfin(0),
    getfin(0),
    guiupdate(0),
    guiupdatecount(0),
    guiresizewidth(500),
    guiresizeheight(200),
    melda(0),
    hWnd(0),
    hidegui(0)
{
#ifdef EMBED	
     /*
     winm = new winmessage;  
     if(!winm)
     starterror = 1;
     */
     winm = &winm2;      
#endif	    
}

std::string RemoteVSTServer::getName()
{
      char buffer[512];
      memset(buffer, 0, sizeof(buffer));
      m_plugin->dispatcher(m_plugin, effGetEffectName, 0, 0, buffer, 0);
      if (buffer[0])
      m_name = buffer;
      return m_name;
}
  
std::string RemoteVSTServer::getMaker()
{
      char buffer[512];
      memset(buffer, 0, sizeof(buffer));
      m_plugin->dispatcher(m_plugin, effGetVendorString, 0, 0, buffer, 0);
      if (buffer[0])
      m_maker = buffer;
      return m_maker;
}

void RemoteVSTServer::EffectOpen()
{
    if (debugLevel > 0)
        cerr << "dssi-vst-server[1]: opening plugin" << endl;	
	
    m_plugin->dispatcher(m_plugin, effOpen, 0, 0, NULL, 0);

    m_plugin->dispatcher(m_plugin, effMainsChanged, 0, 0, NULL, 0);
	
    m_plugin->dispatcher(m_plugin, effSetBlockSize, 0, 1024, NULL, 0);

    m_plugin->dispatcher(m_plugin, effSetSampleRate, 0, 0, NULL, (float)44100);
	
    char buffer[512];
    memset(buffer, 0, sizeof(buffer));
	
    string buffer2 = getMaker();
    strcpy(buffer, buffer2.c_str());

/*
    if (strncmp(buffer, "Guitar Rig 5", 12) == 0)
        setprogrammiss = 1;
    if (strncmp(buffer, "T-Rack", 6) == 0)
        setprogrammiss = 1;
*/
		
    if(strcmp("MeldaProduction", buffer) == 0)
    {    
    melda = 1;	
    wavesthread = 1;  
    }	    
	
#ifdef WAVES
    if(strcmp("Waves", buffer) == 0)
    {
    m_plugin->flags |= effFlagsHasEditor;
    haveGui = true;
    wavesthread = 1;
    bufferwaves = buffer;	    
    }

    writeInt(&m_shm[FIXED_SHM_SIZE], wavesthread);
#endif
/*
    if (strncmp(buffer, "IK", 2) == 0)
        setprogrammiss = 1;
*/	
#ifndef WCLASS
	    if (haveGui == true)
    {
	memset(&wclass, 0, sizeof(WNDCLASSEX));
        wclass.cbSize = sizeof(WNDCLASSEX);
        wclass.style = 0;
	    // CS_HREDRAW | CS_VREDRAW;
        wclass.lpfnWndProc = MainProc;
        wclass.cbClsExtra = 0;
        wclass.cbWndExtra = 0;
        wclass.hInstance = GetModuleHandle(0);
        wclass.hIcon = LoadIcon(GetModuleHandle(0), APPLICATION_CLASS_NAME);
        wclass.hCursor = LoadCursor(0, IDI_APPLICATION);
        // wclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wclass.lpszMenuName = "MENU_DSSI_VST";
        wclass.lpszClassName = APPLICATION_CLASS_NAME;
        wclass.hIconSm = 0;

        if (!RegisterClassEx(&wclass))
        {
            cerr << "dssi-vst-server: ERROR: Failed to register Windows application class!\n" << endl;
            haveGui = false;
        }
#ifdef TRACKTIONWM       
    	memset(&wclass2, 0, sizeof(WNDCLASSEX));
        wclass2.cbSize = sizeof(WNDCLASSEX);
        wclass2.style = 0;
	    // CS_HREDRAW | CS_VREDRAW;
        wclass2.lpfnWndProc = MainProc2;
        wclass2.cbClsExtra = 0;
        wclass2.cbWndExtra = 0;
        wclass2.hInstance = GetModuleHandle(0);
        wclass2.hIcon = LoadIcon(GetModuleHandle(0), APPLICATION_CLASS_NAME2);
        wclass2.hCursor = LoadCursor(0, IDI_APPLICATION);
        // wclass2.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wclass2.lpszMenuName = "MENU_DSSI_VST2";
        wclass2.lpszClassName = APPLICATION_CLASS_NAME2;
        wclass2.hIconSm = 0;

        if (!RegisterClassEx(&wclass2))
        {
            cerr << "dssi-vst-server: ERROR: Failed to register Windows application class!\n" << endl;
            haveGui = false;
        }
#endif        
    }
#endif
	
    struct amessage
    {
        int flags;
        int pcount;
        int parcount;
        int incount;
        int outcount;
        int delay;
    } am;

        am.pcount = m_plugin->numPrograms;
        am.parcount = m_plugin->numParams;
        am.incount = m_plugin->numInputs;
        am.outcount = m_plugin->numOutputs;
        am.delay = m_plugin->initialDelay;
#ifndef DOUBLEP
        am.flags = m_plugin->flags;	
        am.flags &= ~effFlagsCanDoubleReplacing;
#else
        am.flags = m_plugin->flags;	
#endif

    memcpy(&remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3], &am, sizeof(am));

    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)audioMasterIOChanged);
   
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();
	
    m_plugin->dispatcher(m_plugin, effMainsChanged, 0, 1, NULL, 0);		
	
    effectrun = true;	
}

RemoteVSTServer::~RemoteVSTServer()
{
    if(effectrun == true)
    {		    
    if(m_plugin)
    {
    m_plugin->dispatcher(m_plugin, effMainsChanged, 0, 0, NULL, 0); 
    
    if(((strncmp(remoteVSTServerInstance->deviceName2.c_str(), "vst3shell", 9) == 0) || (strncmp(remoteVSTServerInstance->deviceName2.c_str(), "vsti3shell", 10) == 0)) && (strcmp(bufferwaves.c_str(), "Waves") == 0)) 
    {
    }
    else
    m_plugin->dispatcher(m_plugin, effClose, 0, 0, NULL, 0);
    }
    } 
	
/*
#ifdef EMBED		
    if (winm)
    delete winm; 
#endif	
*/
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

#ifdef VESTIGE
bool RemoteVSTServer::getOutProp(int index)
{
char ptr[256];
bool retval;

        retval = m_plugin->dispatcher(m_plugin, effGetOutputProperties, index, 0, &ptr, 0);

        tryWrite(&m_shm2[FIXED_SHM_SIZE2 - 256], &ptr, 256);

        return retval;         
}

bool RemoteVSTServer::getInProp(int index)
{
char ptr[256];
bool retval;

        retval = m_plugin->dispatcher(m_plugin, effGetInputProperties, index, 0, &ptr, 0);

        tryWrite(&m_shm2[FIXED_SHM_SIZE2 - 256], &ptr, 256);

        return retval;       
}
#else
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
#endif

#ifdef MIDIEFF
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
    if (opcode == 78345432)
    {
//        hostreaper = 1;
         return;
    }
#ifdef EMBED
#ifdef TRACKTIONWM  
    if (opcode == 67584930)
    {
        hosttracktion = 1;
         return;
    }	
#endif	
#endif
	
    if (opcode == effClose)
    {
         // usleep(500000);
        waitForServerexit();
        terminate();
	return;    
    }
  
        m_plugin->dispatcher(m_plugin, opcode, 0, 0, NULL, 0);
}

int RemoteVSTServer::effDoVoid2(int opcode, int index, int value, float opt)
{
int ret;

    ret = 0;	
    if(opcode == hidegui2)
    {
    hidegui = 1;  
#ifdef XECLOSE    
    while(hidegui == 1)
    {    
    sched_yield();
    } 
#endif       
    }  	
    else
    ret = m_plugin->dispatcher(m_plugin, opcode, index, value, NULL, opt);    
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
    exiting = true;	
	
  //  cerr << "RemoteVSTServer::terminate: setting exiting flag" << endl;
}

std::string RemoteVSTServer::getParameterName(int p)
{
    char name[512];
    memset(name, 0, sizeof(name));

    m_plugin->dispatcher(m_plugin, effGetParamName, p, 0, name, 0);
    return name;
}

std::string RemoteVSTServer::getParameterLabel(int p)
{
    char name[512];
    memset(name, 0, sizeof(name));

    m_plugin->dispatcher(m_plugin, effGetParamLabel, p, 0, name, 0);
    return name;
}

std::string RemoteVSTServer::getParameterDisplay(int p)
{
    char name[512];
    memset(name, 0, sizeof(name));

    m_plugin->dispatcher(m_plugin, effGetParamDisplay, p, 0, name, 0);
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

int RemoteVSTServer::getProgramNameIndexed(int p, char *name)
{
    if (debugLevel > 1)
        cerr << "dssi-vst-server[2]: getProgramName(" << p << ")" << endl;

    int retval = 0;
    char nameret[512];
    memset(nameret, 0, sizeof(nameret));

    retval = m_plugin->dispatcher(m_plugin, effGetProgramNameIndexed, p, 0, nameret, 0);
    strcpy(name, nameret); 
    return retval;
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

#ifdef WAVES
int
RemoteVSTServer::getShellName(char *name)
{
int retval = 0;
char nameret[512];

    if (debugLevel > 1)
        cerr << "dssi-vst-server[2]: getProgramName()" << endl;

    memset(nameret, 0, sizeof(nameret));
    retval = m_plugin->dispatcher(m_plugin, effShellGetNextPlugin, 0, 0, nameret, 0);
    strcpy(name, nameret); 
    return retval;
}
#endif

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
    if (hWnd)
        MessageBox(hWnd, warning.c_str(), "Error", 0);
    return true;
}

void RemoteVSTServer::showGUI()
{	
#ifdef WCLASS
        memset(&wclass, 0, sizeof(WNDCLASSEX));
        wclass.cbSize = sizeof(WNDCLASSEX);
        wclass.style = 0;
	    // CS_HREDRAW | CS_VREDRAW;
        wclass.lpfnWndProc = MainProc;
        wclass.cbClsExtra = 0;
        wclass.cbWndExtra = 0;
        wclass.hInstance = GetModuleHandle(0);
        wclass.hIcon = LoadIcon(GetModuleHandle(0), APPLICATION_CLASS_NAME);
        wclass.hCursor = LoadCursor(0, IDI_APPLICATION);
        // wclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wclass.lpszMenuName = "MENU_DSSI_VST";
        wclass.lpszClassName = APPLICATION_CLASS_NAME;
        wclass.hIconSm = 0;

        if (!RegisterClassEx(&wclass))
        {
        guiVisible = false;
#ifdef EMBED        
        winm->handle = 0;
        winm->width = 0;
        winm->height = 0;
        tryWrite(&m_shm[FIXED_SHM_SIZE], winm, sizeof(winmessage));
#endif        
        return;
        }        
#ifdef EMBED         
#ifdef TRACKTIONWM       
    	memset(&wclass2, 0, sizeof(WNDCLASSEX));
        wclass2.cbSize = sizeof(WNDCLASSEX);
        wclass2.style = 0;
	    // CS_HREDRAW | CS_VREDRAW;
        wclass2.lpfnWndProc = MainProc2;
        wclass2.cbClsExtra = 0;
        wclass2.cbWndExtra = 0;
        wclass2.hInstance = GetModuleHandle(0);
        wclass2.hIcon = LoadIcon(GetModuleHandle(0), APPLICATION_CLASS_NAME2);
        wclass2.hCursor = LoadCursor(0, IDI_APPLICATION);
        // wclass2.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wclass2.lpszMenuName = "MENU_DSSI_VST2";
        wclass2.lpszClassName = APPLICATION_CLASS_NAME2;
        wclass2.hIconSm = 0;

        if (!RegisterClassEx(&wclass2))
        {       
        UnregisterClassA(APPLICATION_CLASS_NAME, GetModuleHandle(0)); 
        guiVisible = false;        
        winm->handle = 0;
        winm->width = 0;
        winm->height = 0;
        tryWrite(&m_shm[FIXED_SHM_SIZE], winm, sizeof(winmessage));       
        return;
        }
#endif 
#endif
#endif       
	
#ifdef EMBED
        winm->handle = 0;
        winm->width = 0;
        winm->height = 0;

        handlewin = 0; 

    if (debugLevel > 0)
        cerr << "RemoteVSTServer::showGUI(" << "): guiVisible is " << guiVisible << endl;

    if (haveGui == false)
    {
#ifdef WCLASS        
        UnregisterClassA(APPLICATION_CLASS_NAME, GetModuleHandle(0));
#ifdef TRACKTIONWM   
        UnregisterClassA(APPLICATION_CLASS_NAME2, GetModuleHandle(0));
#endif 
#endif          
        winm->handle = 0;
        winm->width = 0;
        winm->height = 0;
        tryWrite(&m_shm[FIXED_SHM_SIZE], winm, sizeof(winmessage));     
        return;
    }

    if (guiVisible)
    {
#ifdef WCLASS        
        UnregisterClassA(APPLICATION_CLASS_NAME, GetModuleHandle(0));
#ifdef TRACKTIONWM   
        UnregisterClassA(APPLICATION_CLASS_NAME2, GetModuleHandle(0));
#endif 
#endif      
        winm->handle = 0;
        winm->width = 0;
        winm->height = 0;
        tryWrite(&m_shm[FIXED_SHM_SIZE], winm, sizeof(winmessage));           
        return;
    }

    // if (hWnd)
   //     DestroyWindow(hWnd);
 //   hWnd = 0;
 
#ifdef EMBEDDRAG
    hWnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_ACCEPTFILES, APPLICATION_CLASS_NAME, "LinVst", WS_POPUP, 0, 0, 200, 200, 0, 0, GetModuleHandle(0), 0);
#else
    hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, APPLICATION_CLASS_NAME, "LinVst", WS_POPUP, 0, 0, 200, 200, 0, 0, GetModuleHandle(0), 0);
#endif
    if (!hWnd)
    {
        // cerr << "dssi-vst-server: ERROR: Failed to create window!\n" << endl;
#ifdef WCLASS 
        UnregisterClassA(APPLICATION_CLASS_NAME, GetModuleHandle(0));
#ifdef TRACKTIONWM   
        UnregisterClassA(APPLICATION_CLASS_NAME2, GetModuleHandle(0));
#endif 
#endif       
        guiVisible = false;
        winm->handle = 0;
        winm->width = 0;
        winm->height = 0;
        tryWrite(&m_shm[FIXED_SHM_SIZE], winm, sizeof(winmessage));
        return;
    }
	
    rect = 0;
    m_plugin->dispatcher(m_plugin, effEditGetRect, 0, 0, &rect, 0);
    m_plugin->dispatcher(m_plugin, effEditOpen, 0, 0, hWnd, 0);
    m_plugin->dispatcher(m_plugin, effEditGetRect, 0, 0, &rect, 0);
    if (!rect)
    {
        // cerr << "dssi-vst-server: ERROR: Plugin failed to report window size\n" << endl;
        DestroyWindow(hWnd);
#ifdef WCLASS         
        UnregisterClassA(APPLICATION_CLASS_NAME, GetModuleHandle(0));
#ifdef TRACKTIONWM   
        UnregisterClassA(APPLICATION_CLASS_NAME2, GetModuleHandle(0));
#endif 
#endif             
        guiVisible = false;
        winm->handle = 0;
        winm->width = 0;
        winm->height = 0;
        tryWrite(&m_shm[FIXED_SHM_SIZE], winm, sizeof(winmessage));
        return;
    }
#ifdef TRACKTIONWM
	if(hosttracktion == 1)
        {
        RECT offsetcl, offsetwin;
        POINT offset;

        HWND hWnd2 = CreateWindow(APPLICATION_CLASS_NAME2, "LinVst", WS_CAPTION, 0, 0, 200, 200, 0, 0, GetModuleHandle(0), 0);
        GetClientRect(hWnd2, &offsetcl);
        GetWindowRect(hWnd2, &offsetwin);
        DestroyWindow(hWnd2);

        offset.x = (offsetwin.right - offsetwin.left) - offsetcl.right;
        offset.y = (offsetwin.bottom - offsetwin.top) - offsetcl.bottom;

        // if(GetSystemMetrics(SM_CMONITORS) > 1)	
        SetWindowPos(hWnd, HWND_TOP, GetSystemMetrics(SM_XVIRTUALSCREEN) + offset.x, GetSystemMetrics(SM_YVIRTUALSCREEN) + offset.y, rect->right - rect->left, rect->bottom - rect->top, 0); 
	// else
	// SetWindowPos(hWnd, HWND_TOP, offset.x, offset.y, rect->right - rect->left, rect->bottom - rect->top, 0);  
        }
	else
	{
        // if(GetSystemMetrics(SM_CMONITORS) > 1)
        SetWindowPos(hWnd, HWND_TOP, GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN), rect->right - rect->left, rect->bottom - rect->top, 0);
        // else
        // SetWindowPos(hWnd, HWND_TOP, 0, 0, rect->right - rect->left, rect->bottom - rect->top, 0);
	}
#else
        // if(GetSystemMetrics(SM_CMONITORS) > 1)
        SetWindowPos(hWnd, HWND_TOP, GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN), rect->right - rect->left, rect->bottom - rect->top, 0);
        // else
        // SetWindowPos(hWnd, HWND_TOP, 0, 0, rect->right - rect->left, rect->bottom - rect->top, 0);
#endif
        if (debugLevel > 0)
            cerr << "dssi-vst-server[1]: sized window" << endl;

        winm->width = rect->right - rect->left;
        winm->height = rect->bottom - rect->top;
        handlewin = GetPropA(hWnd, "__wine_x11_whole_window");
        winm->handle = (long int) handlewin;
        tryWrite(&m_shm[FIXED_SHM_SIZE], winm, sizeof(winmessage));
#else
    if (debugLevel > 0)
        cerr << "RemoteVSTServer::showGUI(" << "): guiVisible is " << guiVisible << endl;

    if (haveGui == false)
    {
#ifdef WCLASS     
        UnregisterClassA(APPLICATION_CLASS_NAME, GetModuleHandle(0));  
#endif       
        guiVisible = false;
        return;
    }
 
    if (guiVisible)
    {
#ifdef WCLASS     
        UnregisterClassA(APPLICATION_CLASS_NAME, GetModuleHandle(0));
#endif          
        return;
    }
	
	#ifdef DRAG
        hWnd = CreateWindowEx(WS_EX_ACCEPTFILES, APPLICATION_CLASS_NAME, "LinVst", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, GetModuleHandle(0), 0);	    
	#else    
        hWnd = CreateWindow(APPLICATION_CLASS_NAME, "LinVst", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, GetModuleHandle(0), 0);
	#endif    
    if (!hWnd)
    {
#ifdef WCLASS     
    UnregisterClassA(APPLICATION_CLASS_NAME, GetModuleHandle(0)); 
#endif                
    cerr << "dssi-vst-server: ERROR: Failed to create window!\n" << endl;
    guiVisible = false;
    return;
    }

    if(hWnd)
    SetWindowText(hWnd, m_name.c_str());
	
    rect = 0;
    m_plugin->dispatcher(m_plugin, effEditGetRect, 0, 0, &rect, 0);
    m_plugin->dispatcher(m_plugin, effEditOpen, 0, 0, hWnd, 0);
    m_plugin->dispatcher(m_plugin, effEditGetRect, 0, 0, &rect, 0);
    if (!rect)
    {
#ifdef WCLASS     
    UnregisterClassA(APPLICATION_CLASS_NAME, GetModuleHandle(0)); 
#endif        
    cerr << "dssi-vst-server: ERROR: Plugin failed to report window size\n" << endl;
    guiVisible = false;
    return;
    }
    else
    {	    
#ifdef WINONTOP
        SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, rect->right - rect->left + 6, rect->bottom - rect->top + 25, SWP_NOMOVE);
#else	
        SetWindowPos(hWnd, HWND_TOP, 0, 0, rect->right - rect->left + 6, rect->bottom - rect->top + 25, SWP_NOMOVE);        
#endif
        if (debugLevel > 0)
            cerr << "dssi-vst-server[1]: sized window" << endl;

	guiVisible = true;
        ShowWindow(hWnd, SW_SHOWNORMAL);
        UpdateWindow(hWnd);
	    
        timerval = 678;
        timerval = SetTimer(hWnd, timerval, 80, 0);	    
    }
#endif	
}
	
void RemoteVSTServer::hideGUI()
{
    // if (!hWnd)
        // return;
/*
    if ((haveGui == false) || (guiVisible == false))
    {
    hideguival = 0;
    return;
    }
    
*/    

#ifndef EMBED
  ShowWindow(hWnd, SW_HIDE);
  UpdateWindow(hWnd);
#endif

  if(melda == 0)
  {
  m_plugin->dispatcher(m_plugin, effEditClose, 0, 0, 0, 0);	
  }
	
#ifdef EMBED  
#ifndef WCLASS  
    if(remoteVSTServerInstance->haveGui == true)
    {
    if(hWnd)
    {
    HWND child = GetWindow(hWnd, GW_CHILD);  
    if(child)
    DestroyWindow(child);
    }
    }
#endif    	    	        
#endif	
		
  if(hWnd)
  {	
  KillTimer(hWnd, timerval);
  #ifdef EMBED
  #ifdef XEMBED
  DestroyWindow(hWnd);
  #else
  #ifdef XECLOSE	  
  DestroyWindow(hWnd); 
  #endif	  
  #endif	  
  #else
  DestroyWindow(hWnd);
  #endif 
  #ifdef WCLASS  
  UnregisterClassA(APPLICATION_CLASS_NAME, GetModuleHandle(0));
  #ifdef EMBED        
  #ifdef TRACKTIONWM   
  UnregisterClassA(APPLICATION_CLASS_NAME2, GetModuleHandle(0));
  #endif  
  #endif 
  #endif         	  
  }

  if(melda == 1)
  {
  m_plugin->dispatcher(m_plugin, effEditClose, 0, 0, 0, 0);	
  }	  
	
  guiVisible = false;
	
  hidegui = 0;	

   // if (!exiting)
    //    usleep(50000);
}

#ifdef EMBED
void RemoteVSTServer::openGUI()
{
    guiVisible = true;
    ShowWindow(hWnd, SW_SHOWNORMAL);
    // ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);	
	
    timerval = 678;
    timerval = SetTimer(hWnd, timerval, 80, 0);	
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
    int bnk_prg = readIntring(&m_shmControl3->ringBuffer);
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
    int bnk_prg = readIntring(&m_shmControl3->ringBuffer);
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
    int sz = readIntring(&m_shmControl3->ringBuffer);
    if(sz >= CHUNKSIZEMAX)
{
    int bnk_prg = readIntring(&m_shmControl3->ringBuffer);
    void *ptr = chunkptr2;
    int r = m_plugin->dispatcher(m_plugin, effSetChunk, bnk_prg, sz, ptr, 0);
    free(chunkptr2);
    writeInt(&m_shm[FIXED_SHM_SIZE], r);
    return;
}
else
{
    int bnk_prg = readIntring(&m_shmControl3->ringBuffer);
    void *ptr = &m_shm[FIXED_SHM_SIZECHUNKSTART];
    int r = m_plugin->dispatcher(m_plugin, effSetChunk, bnk_prg, sz, ptr, 0);
    writeInt(&m_shm[FIXED_SHM_SIZE], r);
    return;
}
#else
    int sz = readIntring(&m_shmControl3->ringBuffer);
    int bnk_prg = readIntring(&m_shmControl3->ringBuffer);
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
    VstIntPtr rv = 0;
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
#ifdef WAVES
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
#endif
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
    if (!remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun && remoteVSTServerInstance->m_shm3)
    {
#ifndef NEWTIME    
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)opcode);
    remoteVSTServerInstance->writeIntring(&remoteVSTServerInstance->m_shmControl->ringBuffer, value);   
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();
/*    
  if(remoteVSTServerInstance->timeinfo)
    {
    memcpy(remoteVSTServerInstance->timeinfo, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3 - sizeof(VstTimeInfo)], sizeof(VstTimeInfo));
    // printf("%f\n", remoteVSTServerInstance->timeinfo->sampleRate);
    rv = (VstIntPtr)remoteVSTServerInstance->timeinfo;
    }    
*/ 
   memcpy(remoteVSTServerInstance->timeinfo, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3 - sizeof(VstTimeInfo)], sizeof(VstTimeInfo));

  //   printf("%f\n", remoteVSTServerInstance->timeinfo->sampleRate);
 
  rv = (VstIntPtr)remoteVSTServerInstance->timeinfo;
#else
/*    
  if(remoteVSTServerInstance->timeinfo)
    {
    memcpy(remoteVSTServerInstance->timeinfo, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3 - sizeof(VstTimeInfo)], sizeof(VstTimeInfo));
    // printf("%f\n", remoteVSTServerInstance->timeinfo->sampleRate);
    rv = (VstIntPtr)remoteVSTServerInstance->timeinfo;
    }    
*/ 
    memcpy(remoteVSTServerInstance->timeinfo, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3 - sizeof(VstTimeInfo)], sizeof(VstTimeInfo));

  //   printf("%f\n", remoteVSTServerInstance->timeinfo->sampleRate);
  
    rv = (VstIntPtr)remoteVSTServerInstance->timeinfo;    
#endif           
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
		    
	        if (!evnts)
                break;
                    
                if (evnts->numEvents <= 0) 
                break;   	    

                eventnum = evnts->numEvents;

                ptr2 = (int *)remoteVSTServerInstance->m_shm3;

                sizeidx = sizeof(int);

                if (eventnum > VSTSIZE)
                    eventnum = VSTSIZE;            

                for (int i = 0; i < eventnum; i++)
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
        am.pcount = plugin->numPrograms;
        am.parcount = plugin->numParams;
        am.incount = plugin->numInputs;
        am.outcount = plugin->numOutputs;
        am.delay = plugin->initialDelay;
#ifndef DOUBLEP
        am.flags = plugin->flags;	
        am.flags &= ~effFlagsCanDoubleReplacing;
#else
        am.flags = plugin->flags;	
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
{
#ifdef EMBED
#ifndef TRACKTIONWM
#ifdef EMBEDRESIZE
   int opcodegui = 123456789;
	
    if(remoteVSTServerInstance)
    {	
    if (remoteVSTServerInstance->hWnd && remoteVSTServerInstance->guiVisible && !remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun && (remoteVSTServerInstance->guiupdate == 0))
    {	
    remoteVSTServerInstance->guiresizewidth = index;
    remoteVSTServerInstance->guiresizeheight = value;

    // ShowWindow(remoteVSTServerInstance->hWnd, SW_HIDE);
    // SetWindowPos(remoteVSTServerInstance->hWnd, HWND_TOP, 0, 0, remoteVSTServerInstance->guiresizewidth, remoteVSTServerInstance->guiresizeheight, 0);
	    
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)opcodegui);
    remoteVSTServerInstance->writeIntring(&remoteVSTServerInstance->m_shmControl->ringBuffer, index);
    remoteVSTServerInstance->writeIntring(&remoteVSTServerInstance->m_shmControl->ringBuffer, value);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();
    remoteVSTServerInstance->guiupdate = 1;
    rv = 1;
    }
    }
#endif
#endif
#else
     if(remoteVSTServerInstance)
     {	
     if (remoteVSTServerInstance->hWnd && !remoteVSTServerInstance->exiting && remoteVSTServerInstance->effectrun && remoteVSTServerInstance->guiVisible)
     {
/*
//    SetWindowPos(remoteVSTServerInstance->hWnd, 0, 0, 0, index + 6, value + 25, SWP_NOMOVE | SWP_HIDEWINDOW);	
    SetWindowPos(remoteVSTServerInstance->hWnd, 0, 0, 0, index + 6, value + 25, SWP_NOMOVE);	
    ShowWindow(remoteVSTServerInstance->hWnd, SW_SHOWNORMAL);
    UpdateWindow(remoteVSTServerInstance->hWnd);
*/
    remoteVSTServerInstance->guiresizewidth = index;
    remoteVSTServerInstance->guiresizeheight = value;
    remoteVSTServerInstance->guiupdate = 1;	
    rv = 1;
        }
	}
#endif
}	    
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
    strcpy(retstr, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3]); 
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
    strcpy(retstr, &remoteVSTServerInstance->m_shm3[FIXED_SHM_SIZE3]); 
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
    strcpy(&m_shm[FIXED_SHM_SIZE3], (char*)ptr);
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
#ifdef WAVES
                    || !strcmp((char*)ptr, "shellCategory")
                    || !strcmp((char*)ptr, "supportShell")
#endif
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
	break;	    
    }
    return rv;
}

VOID CALLBACK TimerProc(HWND hWnd, UINT message, UINT idTimer, DWORD dwTime)
{
   HWND hwnderr = FindWindow(NULL, "LinVst Error");
   SendMessage(hwnderr, WM_COMMAND, IDCANCEL, 0);
}

void RemoteVSTServer::guiUpdate()
{
#ifdef EMBED
#ifdef EMBEDRESIZE
   remoteVSTServerInstance->guiupdatecount += 1;
	
   if(remoteVSTServerInstance->guiupdatecount == 2)
   {
   ShowWindow(remoteVSTServerInstance->hWnd, SW_SHOWNORMAL);
   UpdateWindow(remoteVSTServerInstance->hWnd);
   remoteVSTServerInstance->guiupdate = 0;
   remoteVSTServerInstance->guiupdatecount = 0;
   }
#endif
#endif
#ifndef EMBED
//      SetWindowPos(remoteVSTServerInstance->hWnd, 0, 0, 0, remoteVSTServerInstance->guiresizewidth + 6, remoteVSTServerInstance->guiresizeheight + 25, SWP_NOMOVE | SWP_HIDEWINDOW);	
    SetWindowPos(remoteVSTServerInstance->hWnd, 0, 0, 0, remoteVSTServerInstance->guiresizewidth + 6, remoteVSTServerInstance->guiresizeheight + 25, SWP_NOMOVE);	
    ShowWindow(remoteVSTServerInstance->hWnd, SW_SHOWNORMAL);
    UpdateWindow(remoteVSTServerInstance->hWnd);
    remoteVSTServerInstance->guiupdate = 0;
#endif
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdline, int cmdshow)
{
    HANDLE  ThreadHandle[3] = {0,0,0};
    string pathName;
    string fileName;
    char cdpath[4096];
    char *libname = 0;
    char *libname2 = 0;
    char *fileInfo = 0;

    cout << "DSSI VST plugin server v" << RemotePluginVersion << endl;
    cout << "Copyright (c) 2012-2013 Filipe Coelho" << endl;
    cout << "Copyright (c) 2010-2011 Kristian Amlie" << endl;
    cout << "Copyright (c) 2004-2006 Chris Cannam" << endl;
    cout << "LinVst version 2.7.2" << endl;

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
	    
	exit(0);    
      //  return 1;
    }

    if (!libname || !libname[0] || !fileInfo || !fileInfo[0])
    {
        cerr << "Usage: dssi-vst-server <vstname.dll>,<tmpfilebase>" << endl;
        cerr << "(Command line was: " << cmdline << ")" << endl;
	    
	exit(0);    
        // return 1;
    }
	
    strcpy(cdpath, libname);
    pathName = cdpath;
    fileName = cdpath;
    size_t found = pathName.find_last_of("/");
    pathName = pathName.substr(0, found);
    size_t found2 = fileName.find_last_of("/");
    fileName = fileName.substr(found2 + 1, strlen(libname) - (found2 +1));
    SetCurrentDirectory(pathName.c_str());

    cout << "Loading  " << libname << endl;

    HINSTANCE libHandle = 0;
    libHandle = LoadLibrary(libname);
    if (!libHandle)
    {
#ifdef  VST6432
    /*
    TCHAR wbuf[1024];
    wsprintf(wbuf, "Error loading plugin dll %s", fileName.c_str());
    UINT_PTR errtimer = SetTimer(NULL, 800, 10000, (TIMERPROC) TimerProc);
    MessageBox(NULL, wbuf, "LinVst Error", MB_OK | MB_TOPMOST);
    KillTimer(NULL, errtimer);    
    */
    cerr << "dssi-vst-server: ERROR: Couldn't load VST DLL \"" << libname << "\"" << endl;	    
    usleep(5000000);   
    exit(0);    
    // return 1;
#else
#ifdef VST32  
    /*
    TCHAR wbuf[1024];
    wsprintf(wbuf, "Error loading plugin dll %s", fileName.c_str());
    UINT_PTR errtimer = SetTimer(NULL, 800, 10000, (TIMERPROC) TimerProc); 
    MessageBox(NULL, wbuf, "LinVst Error", MB_OK | MB_TOPMOST);
    KillTimer(NULL, errtimer);  
    */
    cerr << "dssi-vst-server: ERROR: Couldn't load VST DLL \"" << libname << "\"" << endl;
    usleep(5000000);    
    exit(0);    
    // return 1;
#else
    /*
    TCHAR wbuf[1024];
    wsprintf(wbuf, "Error loading plugin dll %s. This LinVst version is for 64 bit vst's only", fileName.c_str());
    UINT_PTR errtimer = SetTimer(NULL, 800, 10000, (TIMERPROC) TimerProc);
    MessageBox(NULL, wbuf, "LinVst Error", MB_OK | MB_TOPMOST);
    KillTimer(NULL, errtimer);
    */
    cerr << "dssi-vst-server: ERROR: Couldn't load VST DLL \"" << fileName << "\" This LinVst version is for 64 bit vsts only." << endl;
    usleep(5000000); 
    exit(0);    
    // return 1;
#endif
#endif    
    }

    VstEntry getinstance = 0;

    getinstance = (VstEntry)GetProcAddress(libHandle, NEW_PLUGIN_ENTRY_POINT);

    if (!getinstance) {
    getinstance = (VstEntry)GetProcAddress(libHandle, OLD_PLUGIN_ENTRY_POINT);
    if (!getinstance) {
    cerr << "dssi-vst-server: ERROR: VST entrypoints \"" << NEW_PLUGIN_ENTRY_POINT << "\" or \""
                << OLD_PLUGIN_ENTRY_POINT << "\" not found in DLL \"" << libname << "\"" << endl;     
    /*
    TCHAR wbuf[1024];
    wsprintf(wbuf, "Error loading plugin dll %s. Not a VST2 dll", fileName.c_str());
    UINT_PTR errtimer = SetTimer(NULL, 800, 10000, (TIMERPROC) TimerProc);
    MessageBox(NULL, wbuf, "LinVst Error", MB_OK | MB_TOPMOST);
    KillTimer(NULL, errtimer);
    */
    if(libHandle)
    FreeLibrary(libHandle);
    usleep(5000000);    
    exit(0);    
    // return 1;
      }
    }
	
	remoteVSTServerInstance = 0;
	
        string deviceName = fileName;
        size_t foundext = deviceName.find_last_of(".");
        deviceName = deviceName.substr(0, foundext);
        remoteVSTServerInstance = new RemoteVSTServer(fileInfo, deviceName);
	
        // remoteVSTServerInstance = new RemoteVSTServer(fileInfo, libname);
    
    if(!remoteVSTServerInstance)
    {
    cerr << "ERROR: Remote VST startup failed" << endl;
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)disconnectserver);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();  
    remoteVSTServerInstance->waitForClient2exit();
    remoteVSTServerInstance->waitForClient3exit();
    remoteVSTServerInstance->waitForClient4exit();
    remoteVSTServerInstance->waitForClient5exit();
    /*
    TCHAR wbuf[1024];
    wsprintf(wbuf, "Error getting instance %s", fileName.c_str());
    UINT_PTR errtimer = SetTimer(NULL, 800, 10000, (TIMERPROC) TimerProc);
    MessageBox(NULL, wbuf, "LinVst Error", MB_OK | MB_TOPMOST);
    KillTimer(NULL, errtimer);    
    */
	usleep(5000000);    
	if(remoteVSTServerInstance)
	delete remoteVSTServerInstance;
	if(libHandle)
	FreeLibrary(libHandle);
	    
        exit(0);
	// return 1;
        }

    if(remoteVSTServerInstance->starterror == 1)
    {
    cerr << "ERROR: Remote VST startup failed and/or mismatched LinVst versions" << endl;
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)disconnectserver);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();  
    remoteVSTServerInstance->waitForClient2exit();
    remoteVSTServerInstance->waitForClient3exit();
    remoteVSTServerInstance->waitForClient4exit();
    remoteVSTServerInstance->waitForClient5exit();
    /*
    TCHAR wbuf[1024];
    wsprintf(wbuf, "Error getting instance %s", fileName.c_str());
    UINT_PTR errtimer = SetTimer(NULL, 800, 10000, (TIMERPROC) TimerProc);
    MessageBox(NULL, wbuf, "LinVst Error", MB_OK | MB_TOPMOST);
    KillTimer(NULL, errtimer);    
    */
	usleep(5000000);    
	if(remoteVSTServerInstance)
	delete remoteVSTServerInstance;
	if(libHandle)
	FreeLibrary(libHandle);
	    
        exit(0);
	// return 1;
    }

    remoteVSTServerInstance->m_plugin = getinstance(hostCallback);
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
    /*
    TCHAR wbuf[1024];
    wsprintf(wbuf, "Error getting instance %s", fileName.c_str());
    UINT_PTR errtimer = SetTimer(NULL, 800, 10000, (TIMERPROC) TimerProc);
    MessageBox(NULL, wbuf, "LinVst Error", MB_OK | MB_TOPMOST);
    KillTimer(NULL, errtimer);    
    */
	usleep(5000000);    
	if(remoteVSTServerInstance)
	delete remoteVSTServerInstance;
	if(libHandle)
	FreeLibrary(libHandle);
	    
        exit(0);
	// return 1;
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
    /*	    
    TCHAR wbuf[1024];
    wsprintf(wbuf, "Not a VST2 plugin DLL %s", fileName.c_str());
    UINT_PTR errtimer = SetTimer(NULL, 800, 10000, (TIMERPROC) TimerProc);
    MessageBox(NULL, wbuf, "LinVst Error", MB_OK | MB_TOPMOST);
    KillTimer(NULL, errtimer);
    */
	usleep(5000000);    
	if(remoteVSTServerInstance)
	delete remoteVSTServerInstance;
	if(libHandle)
	FreeLibrary(libHandle);
	    
	exit(0);    
        // return 1;
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
    /*
    TCHAR wbuf[1024];
    wsprintf(wbuf, "ProcessReplacing not supported %s", fileName.c_str());
    UINT_PTR errtimer = SetTimer(NULL, 800, 10000, (TIMERPROC) TimerProc);
    MessageBox(NULL, wbuf, "LinVst Error", MB_OK | MB_TOPMOST);
    KillTimer(NULL, errtimer); 
    */
	usleep(5000000);    
	if(remoteVSTServerInstance)
	delete remoteVSTServerInstance;
	if(libHandle)
	FreeLibrary(libHandle);
	    
	exit(0);    
        // return 1;
    }

    if(remoteVSTServerInstance->m_plugin->flags & effFlagsHasEditor)
    remoteVSTServerInstance->haveGui = true;
    else 
    remoteVSTServerInstance->haveGui = false;
	
    DWORD threadIdp = 0;
    ThreadHandle[0] = CreateThread(0, 0, AudioThreadMain, 0, 0, &threadIdp);
    if (!ThreadHandle[0])
    {
    cerr << "Failed to create audio thread!" << endl;
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)disconnectserver);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();  
    remoteVSTServerInstance->waitForClient2exit();
    remoteVSTServerInstance->waitForClient3exit();
    remoteVSTServerInstance->waitForClient4exit();
    remoteVSTServerInstance->waitForClient5exit();
    /*    
    TCHAR wbuf[1024];
    wsprintf(wbuf, "Thread Error audio %s", fileName.c_str());
    UINT_PTR errtimer = SetTimer(NULL, 800, 10000, (TIMERPROC) TimerProc);
    MessageBox(NULL, wbuf, "LinVst Error", MB_OK | MB_TOPMOST);
    KillTimer(NULL, errtimer); 
    */	    
	usleep(5000000);
	if(remoteVSTServerInstance)
	delete remoteVSTServerInstance;
	if(libHandle)
	FreeLibrary(libHandle);
	    
	exit(0);    
        // return 1;
    }

    DWORD threadIdp2 = 0;
    ThreadHandle[1] = CreateThread(0, 0, GetSetThreadMain, 0, 0, &threadIdp2);
    if (!ThreadHandle[1])
    {
    cerr << "Failed to create getset thread!" << endl;
    
    remoteVSTServerInstance->exiting = 1;    
    sleep(1);  
    WaitForMultipleObjects(1, ThreadHandle, TRUE, 5000);    
    if (ThreadHandle[0])
    {
        // TerminateThread(ThreadHandle[0], 0);
        CloseHandle(ThreadHandle[0]);
    }    
//    TerminateThread(ThreadHandle[0], 0);

    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)disconnectserver);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();  
    remoteVSTServerInstance->waitForClient2exit();
    remoteVSTServerInstance->waitForClient3exit();
    remoteVSTServerInstance->waitForClient4exit();
    remoteVSTServerInstance->waitForClient5exit();
    /*
    TCHAR wbuf[1024];
    wsprintf(wbuf, "Thread Error getset %s", fileName.c_str());
    UINT_PTR errtimer = SetTimer(NULL, 800, 10000, (TIMERPROC) TimerProc);
    MessageBox(NULL, wbuf, "LinVst Error", MB_OK | MB_TOPMOST);
    KillTimer(NULL, errtimer);  
    */
	usleep(5000000);   
	if(remoteVSTServerInstance)
	delete remoteVSTServerInstance;
	if(libHandle)
	FreeLibrary(libHandle);
	    
	exit(0);    
        // return 1;
    }

    DWORD threadIdp3 = 0;
    ThreadHandle[2] = CreateThread(0, 0, ParThreadMain, 0, 0, &threadIdp3);
    if (!ThreadHandle[2])
    {
    cerr << "Failed to create par thread!" << endl;
    
    remoteVSTServerInstance->exiting = 1;    
    sleep(1);  
    WaitForMultipleObjects(2, ThreadHandle, TRUE, 5000);    
    if (ThreadHandle[0])
    {
        // TerminateThread(ThreadHandle[0], 0);
        CloseHandle(ThreadHandle[0]);
    }
    if (ThreadHandle[1])
    {
        // TerminateThread(ThreadHandle[1], 0);
        CloseHandle(ThreadHandle[1]);
    }    
//    TerminateThread(ThreadHandle[0], 0);
//    TerminateThread(ThreadHandle[1], 0);
    
    remoteVSTServerInstance->writeOpcodering(&remoteVSTServerInstance->m_shmControl->ringBuffer, (RemotePluginOpcode)disconnectserver);
    remoteVSTServerInstance->commitWrite(&remoteVSTServerInstance->m_shmControl->ringBuffer);
    remoteVSTServerInstance->waitForServer();  
    remoteVSTServerInstance->waitForClient2exit();
    remoteVSTServerInstance->waitForClient3exit();
    remoteVSTServerInstance->waitForClient4exit();
    remoteVSTServerInstance->waitForClient5exit();
    /*
    TCHAR wbuf[1024];
    wsprintf(wbuf, "Thread Error par %s", fileName.c_str());
    UINT_PTR errtimer = SetTimer(NULL, 800, 10000, (TIMERPROC) TimerProc);
    MessageBox(NULL, wbuf, "LinVst Error", MB_OK | MB_TOPMOST);
    KillTimer(NULL, errtimer);    
    */
	usleep(5000000);    
	if(remoteVSTServerInstance)
	delete remoteVSTServerInstance;
	if(libHandle)
	FreeLibrary(libHandle);
	    
	exit(0);    
        // return 1;
    }
	
    remoteVSTServerInstance->deviceName2 = deviceName;	

    MSG msg;
    int tcount = 0;

    while (!remoteVSTServerInstance->exiting)
    {		         
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {       
    if(remoteVSTServerInstance->exiting)
    break;
	    
    if(msg.message == 15 && !remoteVSTServerInstance->guiVisible)
    break;    
    
    TranslateMessage(&msg);
    DispatchMessage(&msg);
       
    if(remoteVSTServerInstance->hidegui == 1)
    break;  
    
    if((msg.message == WM_TIMER) && (msg.wParam == 678)) 
    {
    remoteVSTServerInstance->m_plugin->dispatcher (remoteVSTServerInstance->m_plugin, effEditIdle, 0, 0, NULL, 0);
    if(remoteVSTServerInstance->guiupdate)
    remoteVSTServerInstance->guiUpdate();  
    }      
    } 
    
    if(remoteVSTServerInstance->hidegui == 1)
    {
    remoteVSTServerInstance->hideGUI();
    }
    
    if(remoteVSTServerInstance->exiting)
    break;	                   
    remoteVSTServerInstance->dispatchControl(50);               
    }
	
/*
    for (int i=0;i<10000;i++)
    {
        if (remoteVSTServerInstance->parfin && remoteVSTServerInstance->audfin && remoteVSTServerInstance->getfin)
            break;
	usleep(1000);    
    }
*/
	
    remoteVSTServerInstance->waitForServerexit();
    remoteVSTServerInstance->waitForClient2exit();
    remoteVSTServerInstance->waitForClient3exit();
    remoteVSTServerInstance->waitForClient4exit();
    remoteVSTServerInstance->waitForClient5exit();	
	
    WaitForMultipleObjects(3, ThreadHandle, TRUE, 5000);

    if (debugLevel > 0)
        cerr << "dssi-vst-server[1]: cleaning up" << endl;

    if (ThreadHandle[0])
    {
        // TerminateThread(ThreadHandle[0], 0);
        CloseHandle(ThreadHandle[0]);
    }

    if (ThreadHandle[1])
    {
        // TerminateThread(ThreadHandle[1], 0);
        CloseHandle(ThreadHandle[1]);
    }

    if (ThreadHandle[2])
    {
        // TerminateThread(ThreadHandle[2], 0);
        CloseHandle(ThreadHandle[2]);
    }

    if (debugLevel > 0)
        cerr << "dssi-vst-server[1]: closed threads" << endl;

    if(remoteVSTServerInstance)
    delete remoteVSTServerInstance;
	
    if(libHandle)
    FreeLibrary(libHandle);

    if (debugLevel > 0)
        cerr << "dssi-vst-server[1]: freed dll" << endl;
 //   if (debugLevel > 0)
        cerr << "dssi-vst-server[1]: exiting" << endl;
	
    exit(0);
	
 //   return 0;
}
