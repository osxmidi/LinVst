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

#ifdef EMBED

#include <X11/Xlib.h>
// #include <X11/Xatom.h>
#endif


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
    static ERect        retRect = {0,0,200,500};

    switch (opcode)
    {
    case effEditGetRect:
    {
        ERect *rp = &retRect;
        *((struct ERect **)ptr) = rp;
    }

        break;

    case effEditIdle:
        // plugin->effVoidOp(effEditIdle);
        break;

    case effStartProcess:
        plugin->effVoidOp(effStartProcess);
        break;

    case effStopProcess:
        plugin->effVoidOp(effStopProcess);
        break;

    case effGetVendorString:
        strncpy((char *) ptr, plugin->getMaker().c_str(), kVstMaxVendorStrLen);
        break;

    case effGetEffectName:
        strncpy((char *) ptr, plugin->getName().c_str(), kVstMaxEffectNameLen);
        break;

    case effGetParamName:
        strncpy((char *) ptr, plugin->getParameterName(index).c_str(), kVstMaxParamStrLen);
        break;

    case effGetParamLabel:
        plugin->getEffString(effGetParamLabel, index, (char *) ptr, kVstMaxParamStrLen);
        break;

    case effGetParamDisplay:
        plugin->getEffString(effGetParamDisplay, index, (char *) ptr, kVstMaxParamStrLen);
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
#ifdef EMBED
    {
        Window  child;
        Window  parent;
        Display *display;
        int     handle = 0;
        int     width = 0;
        int     height = 0;

        plugin->showGUI();
        // usleep(500000);

        handle = plugin->winm.handle;
        width = plugin->winm.width;
        height = plugin->winm.height;
        parent = (Window) ptr;
        child = (Window) handle;

        display = XOpenDisplay(0);
        // XResizeWindow(display, child, width, height);
        XReparentWindow(display, child, parent, 0, 0);
        XSync(display, false);
        XFlush(display);
        plugin->openGUI();
        XCloseDisplay(display);

        ERect *rp = &retRect;
        rp->bottom = height;
        rp->top = 0;
        rp->right = width;
        rp->left = 0;
    }
#else
        plugin->showGUI();
#endif
        break;

    case effEditClose:
        plugin->hideGUI();
        break;

    case effCanDo:
        if (ptr && !strcmp((char *)ptr,"hasCockosExtensions"))
             plugin->effVoidOp(effCanDo);
        v = 1;
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

    case effClose:
        plugin->effVoidOp(effClose);

/*
#ifdef AMT
        if(plugin->m_shm3)
        {
            for(int i=0;i<500;i++)
            {
                usleep(10000);
                if(plugin->m_threadbreakexit)
                break;
            }
        }
        else
        {
            usleep(500000);
        }
#else
        usleep(500000);
#endif
*/
        delete plugin;
        break;

    default:
        break;
    }
    return v;
}

void processDouble(AEffect* effect, double** inputs, double** outputs, VstInt32 sampleFrames)
{
    return;
}

void process(AEffect* effect, float** inputs, float** outputs, VstInt32 sampleFrames)
{
    RemotePluginClient *plugin = (RemotePluginClient *) effect->object;

    if((plugin->m_bufferSize > 0) && (plugin->m_numInputs >= 0) && (plugin->m_numOutputs >= 0))
        plugin->process(inputs, outputs, sampleFrames);
    return;
}

void setParameter(AEffect* effect, VstInt32 index, float parameter)
{
    RemotePluginClient *plugin = (RemotePluginClient *) effect->object;

    if((plugin->m_bufferSize > 0) && (plugin->m_numInputs >= 0) && (plugin->m_numOutputs >= 0))
        plugin->setParameter(index, parameter);
    return;
}

float getParameter(AEffect* effect, VstInt32 index)
{
    RemotePluginClient  *plugin = (RemotePluginClient *) effect->object;
    float               retval = -1;

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
    eff->flags &= ~effFlagsCanDoubleReplacing;
    eff->flags |= effFlagsCanReplacing;
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
        delete plugin;
        return 0;
    }

    if(plugin->m_runok == 1)
    {
        delete plugin;
        return 0;
    }

    if(plugin->EffectOpen() == 1)
        initEffect(plugin->theEffect, plugin);

    return plugin->theEffect;
}
