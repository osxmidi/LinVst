/*  dssi-vst: a DSSI plugin wrapper for VST effects and instruments
    Copyright 2004-2007 Chris Cannam
*/


#ifndef REMOTE_PLUGIN_H
#define REMOTE_PLUGIN_H

#ifdef CHUNKBUF
#define CHUNKSIZEMAX 1024 * 1024
#define CHUNKSIZE 1024 * 1024
#else
#define CHUNKSIZEMAX 1024 * 1024 * 2
#endif

#define PROCESSSIZE (1024 * 1024 * 2)

#define FIXED_SHM_SIZE  PROCESSSIZE + CHUNKSIZEMAX
#define FIXED_SHM_SIZE2 (1024 * 128)
#define FIXED_SHM_SIZE3 (1024 * 128)

#define FIXED_SHM_SIZECHUNKSTART PROCESSSIZE

#define VSTSIZE 2048

const float RemotePluginVersion = 0.986;

enum RemotePluginDebugLevel
{
    RemotePluginDebugNone,
    RemotePluginDebugSetup,
    RemotePluginDebugEvents,
    RemotePluginDebugData
};

enum RemotePluginOpcode
{
    RemotePluginGetVersion = 0,
    RemotePluginUniqueID,
    RemotePluginGetName,
    RemotePluginGetMaker,
    RemotePluginGetFlags,
    RemotePluginGetinitialDelay,
    RemotePluginProcessEvents,
    RemotePluginGetChunk,
    RemotePluginSetChunk,
    RemotePluginCanBeAutomated,
    RemotePluginGetProgram,
    RemotePluginEffectOpen,
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
#ifdef WAVES
    RemotePluginGetShellName,
#endif
    RemotePluginSetParameter,
    RemotePluginGetParameter,
    RemotePluginGetParameterDefault,
    RemotePluginGetParameters,

    RemotePluginGetProgramCount = 350,
    RemotePluginGetProgramNameIndexed,
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
    RemotePluginDoVoid2,
#ifdef DOUBLEP
    RemotePluginProcessDouble,
    RemoteSetPrecision,
#endif
#ifndef MIDIEFF 
#ifdef VESTIGE
    RemoteInProp,
    RemoteOutProp,
#endif
#endif
#ifdef MIDIEFF
    RemoteInProp,
    RemoteOutProp,
    RemoteMidiKey,
    RemoteMidiProgName,
    RemoteMidiCurProg,
    RemoteMidiProgCat,
    RemoteMidiProgCh,
    RemoteSetSpeaker,
    RemoteGetSpeaker,
#endif
#ifdef CANDOEFF
    RemotePluginEffCanDo,
#endif	
#ifdef CHUNKBUF
    RemotePluginGetBuf,
    RemotePluginSetBuf,
#endif
    RemotePluginNoOpcode = 9999
};

#endif
