/*  linvst is based on wacvst Copyright 2009 retroware. All rights reserved. and dssi-vst Copyright 2004-2007 Chris Cannam

    linvst Mark White 2017

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
    along with this program.  If not, see <http://www.gnu.org/licenses/>. *
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include "remotevstclient.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

extern "C" {

#define VST_EXPORT   __attribute__ ((visibility ("default")))

    extern VST_EXPORT AEffect * VSTPluginMain(audioMasterCallback audioMaster);

    AEffect * main_plugin (audioMasterCallback audioMaster) asm ("main");

#define main main_plugin

    VST_EXPORT AEffect * main(audioMasterCallback audioMaster)
    {
        return VSTPluginMain(audioMaster);
    }
}

VstIntPtr dispatcher(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
    RemotePluginClient  *plugin = (RemotePluginClient *) effect->object;
    VstIntPtr           v = 0;
    static ERect        *rp;
    size_t len;
    std::string newname;
    static char newname2[1024];

    if(!plugin)
    return 0;
	
    if(plugin->m_inexcept == 1)
    {
    return 0;
    }

    switch (opcode)
    {
    case effEditGetRect:
    {
#ifdef STANDALONE
        rp = &plugin->retRect;
        *((struct ERect **)ptr) = rp;
#else
        plugin->GetRect();

        rp = &plugin->retRect;

        rp->bottom = plugin->winm.height;
        rp->top = 0;
        rp->right = plugin->winm.width;
        rp->left = 0;

        *((struct ERect **)ptr) = rp;
#endif
    }
        break;

    case effEditIdle:
        v = plugin->effVoidOp2(effEditIdle, index, value, opt);
        break;

    case effStartProcess:
        plugin->effVoidOp(effStartProcess);
        break;

    case effStopProcess:
        plugin->effVoidOp(effStopProcess);
        break;
		    
    case effMainsChanged:
        plugin->effVoidOp2(effMainsChanged, index, value, opt);
        break;

    case effGetVendorString:
        strncpy((char *) ptr, plugin->getMaker().c_str(), kVstMaxVendorStrLen);
        break;

    case effGetEffectName:
        memset(newname2, 0, 1024);
        newname = plugin->getName();
        strcpy(newname2, newname.c_str());
        len = strlen(newname2);
        newname2[len] = '*';
        newname2[len + 1] = '\0';
        strncpy((char *) ptr, newname2, kVstMaxEffectNameLen);
        break;

    case effGetParamName:
      //  strncpy((char *) ptr, plugin->getParameterName(index).c_str(), kVstMaxParamStrLen);
	strncpy((char *) ptr, plugin->getParameterName(index).c_str(), kVstMaxVendorStrLen);
        break;

    case effGetParamLabel:
    //    plugin->getEffString(effGetParamLabel, index, (char *) ptr, kVstMaxParamStrLen);
        plugin->getEffString(effGetParamLabel, index, (char *) ptr, kVstMaxVendorStrLen);
        break;

    case effGetParamDisplay:
     //   plugin->getEffString(effGetParamDisplay, index, (char *) ptr, kVstMaxParamStrLen);
        plugin->getEffString(effGetParamDisplay, index, (char *) ptr, kVstMaxVendorStrLen);
        break;

    case effGetProgramNameIndexed:
        strncpy((char *) ptr, plugin->getProgramNameIndexed(index).c_str(), kVstMaxProgNameLen);
        break;

    case effGetProgramName:
        strncpy((char *) ptr, plugin->getProgramName().c_str(), kVstMaxProgNameLen);
        break;

    case effSetSampleRate:
        plugin->setSampleRate(opt);
        break;

    case effSetBlockSize:
        plugin->setBufferSize ((VstInt32)value);
        break;

#ifdef DOUBLEP
    case effSetProcessPrecision:
        v = plugin->setPrecision(value);
      break;  
#endif

#ifdef MIDIEFF   
    case effGetInputProperties:    
        v = plugin->getEffInProp(index, (char *)ptr); 
        break;  
        
    case effGetOutputProperties:    
        v = plugin->getEffOutProp(index, (char *)ptr);
        break; 
        
    case effGetMidiKeyName:    
        v = plugin->getEffMidiKey(index, (char *) ptr);
        break;   
        
    case effGetMidiProgramName:    
        v = plugin->getEffMidiProgName(index, (char *) ptr);
        break;   
        
    case effGetCurrentMidiProgram:    
        v = plugin->getEffMidiCurProg(index, (char *) ptr);
        break; 
        
    case effGetMidiProgramCategory:    
        v = plugin->getEffMidiProgCat(index, (char *) ptr);
        break;   
        
    case effHasMidiProgramsChanged:    
        v = plugin->getEffMidiProgCh(index);
        break;   
                 
    case effSetSpeakerArrangement:
        v = plugin->setEffSpeaker(value, (char *)ptr);
      break;
      
    case effGetSpeakerArrangement:
        v = plugin->getEffSpeaker(value, (char *)ptr);
      break;  
#endif
		    
    case effGetVstVersion:
        v = kVstVersion;
        break;

    case effGetPlugCategory:
        v = plugin->getEffInt(effGetPlugCategory);
        break;

    case effSetProgram:
        plugin->setCurrentProgram((VstInt32)value);
        break;

    case effEditOpen:               
#ifdef STANDALONE
        plugin->showGUI();
#else
        plugin->winm.width = 0;
        plugin->winm.height = 0;
        plugin->winm.handle = (long int)ptr;

        plugin->showGUI();

        rp = &plugin->retRect;

        rp->bottom = plugin->winm.height;
        rp->top = 0;
        rp->right = plugin->winm.width;
        rp->left = 0;
#endif
        break;

    case effEditClose:

//        if(plugin->displayerr == 1)
//        break;

        plugin->hideGUI();  
        break;

    case effCanDo:
        if (ptr && !strcmp((char *)ptr,"hasCockosExtensions"))
	plugin->reaperid = 1;
      //       plugin->effVoidOp(effCanDo);
#ifdef CANDOEFF
        v = plugin->getEffCanDo((char *)ptr);     
#else		    
        v = 1;
#endif		    
        break;

    case effProcessEvents:
        v = plugin->processVstEvents((VstEvents *) ptr);
        break;

    case effGetChunk:
        v = plugin->getChunk((void **) ptr, index);
        break;

    case effSetChunk:
        v = plugin->setChunk(ptr, value, index);
        break;

    case effGetProgram:
        v = plugin->getProgram();
        break;
		    
    case effCanBeAutomated:
        v = plugin->canBeAutomated(index);
        break;

        case effOpen:
        plugin->EffectOpen();
        break;
	    
    case effClose:
        plugin->effVoidOp(effClose);

        delete plugin;
        break;

    default:
        break;
    }
    return v;
}

void processDouble(AEffect* effect, double** inputs, double** outputs, VstInt32 sampleFrames)
{
#ifdef DOUBLEP
    RemotePluginClient *plugin = (RemotePluginClient *) effect->object;

    if(!plugin)
    return;
    
    if((plugin->m_bufferSize > 0) && (plugin->m_numInputs >= 0) && (plugin->m_numOutputs >= 0))
        plugin->processdouble(inputs, outputs, sampleFrames);
#endif	
    return;
}

void process(AEffect* effect, float** inputs, float** outputs, VstInt32 sampleFrames)
{
    RemotePluginClient *plugin = (RemotePluginClient *) effect->object;

    if(!plugin)
    return;
    
    if((plugin->m_bufferSize > 0) && (plugin->m_numInputs >= 0) && (plugin->m_numOutputs >= 0))
        plugin->process(inputs, outputs, sampleFrames);
    return;
}

void setParameter(AEffect* effect, VstInt32 index, float parameter)
{
    RemotePluginClient *plugin = (RemotePluginClient *) effect->object;

    if(!plugin)
    return;
    
    if((plugin->m_bufferSize > 0) && (plugin->m_numInputs >= 0) && (plugin->m_numOutputs >= 0))
        plugin->setParameter(index, parameter);
    return;
}

float getParameter(AEffect* effect, VstInt32 index)
{
    RemotePluginClient  *plugin = (RemotePluginClient *) effect->object;
    float               retval = -1;

    if(!plugin)
    return retval;
    
    if((plugin->m_bufferSize > 0) && (plugin->m_numInputs >= 0) && (plugin->m_numOutputs >= 0))
        retval = plugin->getParameter(index);
    return retval;
}

void initEffect(AEffect *eff, RemotePluginClient *plugin)
{
    memset(eff, 0x0, sizeof(AEffect));
    eff->magic = kEffectMagic;
    eff->dispatcher = dispatcher;
    eff->setParameter = setParameter;
    eff->getParameter = getParameter;
    eff->numInputs = plugin->getInputCount();
    eff->numOutputs = plugin->getOutputCount();
    eff->numPrograms = plugin->getProgramCount();
    eff->numParams = plugin->getParameterCount();
    eff->flags = plugin->getFlags();
#ifndef DOUBLEP
    eff->flags &= ~effFlagsCanDoubleReplacing;
    eff->flags |= effFlagsCanReplacing;
#endif
    eff->resvd1 = 0;
    eff->resvd2 = 0;
    eff->initialDelay = plugin->getinitialDelay();
    eff->object = (void *) plugin;
    eff->user = 0;
    eff->uniqueID = plugin->getUID();
    eff->version = 100;
    eff->processReplacing = process;
    eff->processDoubleReplacing = processDouble;
}

void errwin2()
{
static Window window = 0;
static Window ignored = 0;
static Display* display = 0;
static int screen = 0;
static Atom winstate;
static Atom winmodal;
    
std::string filename2;

  filename2 = "lin-vst-server/vst not found or LinVst version mismatch";
      
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

VST_EXPORT AEffect* VSTPluginMain (audioMasterCallback audioMaster)
{
    RemotePluginClient *plugin;

    if (!audioMaster (0, audioMasterVersion, 0, 0, 0, 0))
        return 0;

    try
    {
        plugin = new RemoteVSTClient(audioMaster);
    }
    catch (std::string e)
    {
        std::cerr << "Could not connect to Server" << std::endl;
	errwin2();    
        if(plugin)
        {
        plugin->m_runok = 1;
        delete plugin;
        }
        return 0;
    }
		
    if(plugin->m_runok == 1)
    {
        std::cerr << "LinVst Error: lin-vst-server not found or vst dll load timeout or LinVst version mismatch" << std::endl;
        errwin2();
	if(plugin)
        delete plugin;
        return 0;
    }

    initEffect(plugin->theEffect, plugin);
	
#ifdef EMBED
        XInitThreads();
#endif

    return plugin->theEffect;
}
