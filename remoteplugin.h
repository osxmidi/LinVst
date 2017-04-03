/*
  dssi-vst: a DSSI plugin wrapper for VST effects and instruments
  Copyright 2004-2007 Chris Cannam
*/

#ifndef REMOTE_PLUGIN_H
#define REMOTE_PLUGIN_H

#define FIXED_SHM_SIZE  (16 * 65536 * sizeof(float))

const float RemotePluginVersion = 0.986;

enum RemotePluginDebugLevel {
    RemotePluginDebugNone,
    RemotePluginDebugSetup,
    RemotePluginDebugEvents,
    RemotePluginDebugData
};

enum RemotePluginOpcode {

    RemotePluginGetVersion = 0,
    RemotePluginGetName,
    RemotePluginGetMaker,
	RemotePluginGetFlags,
	RemotePluginGetinitialDelay,
	RemotePluginProcessEvents,
	RemotePluginGetChunk,
	RemotePluginSetChunk,
	// RemotePluginCanBeAutomated,
	RemotePluginGetProgram,
	
	// RemotePluginGetUniqueID,
	// RemotePluginGetInitialDelay,



    RemotePluginSetBufferSize = 100,
    RemotePluginSetSampleRate,
    RemotePluginReset,
    RemotePluginTerminate,

    RemotePluginGetInputCount = 200,
    RemotePluginGetOutputCount,

    RemotePluginGetParameterCount = 300,
    RemotePluginGetParameterName,
    RemotePluginSetParameter,
    RemotePluginGetParameter,
    RemotePluginGetParameterDefault,
    RemotePluginGetParameters,

    RemotePluginGetProgramCount = 350,
    RemotePluginGetProgramName,
    RemotePluginSetCurrentProgram,

    RemotePluginProcess = 500,
    RemotePluginIsReady,

    RemotePluginSetDebugLevel = 600,
    RemotePluginWarn,

    RemotePluginShowGUI = 700,
    RemotePluginHideGUI,

#ifdef EMBED
    RemotePluginOpenGUI,
#endif

    RemotePluginGetEffInt = 800,
    RemotePluginGetEffString,
    RemotePluginDoVoid,
	
    RemotePluginNoOpcode = 9999
};

class RemotePluginClosedException { };

#endif
