/*
  dssi-vst: a DSSI plugin wrapper for VST effects and instruments
  Copyright 2004-2007 Chris Cannam
*/

#ifndef _RD_WR_OPS_H_

#include "remoteplugin.h"

#include <linux/futex.h>
#include <syscall.h>

#include <atomic>

struct amessage {
  int flags;
  int pcount;
  int parcount;
  int incount;
  int outcount;
  int delay;
};

struct vinfo {
  char a[64 + 8 + (sizeof(int32_t) * 2) + 48];
  // char a[96];
};

struct winmessage {
  int handle;
  int width;
  int height;
  int winerror;
};

#ifdef VST32
struct alignas(32) ShmControl
#else
struct alignas(64) ShmControl
#endif
{
  std::atomic_int runServer;
  std::atomic_int runClient;
  std::atomic_int nwaitersserver;
  std::atomic_int nwaitersclient;
  // int runServer;
  // int runClient;
  RemotePluginOpcode ropcode;
  int retint;
  float retfloat;
  char retstr[512];
  int opcode;
  int value;
  int value2;
  int value3;
  int value4;
  float floatvalue;
  bool retbool;
  char timeget[sizeof(VstTimeInfo)];
  char timeset[sizeof(VstTimeInfo)];
  char amptr[sizeof(amessage)];
  char vret[sizeof(vinfo)];
  char wret[sizeof(winmessage)];
#ifndef VESTIGE
  char vpin[sizeof(VstPinProperties)];
#endif
#ifdef MIDIEFF
  char midikey[sizeof(MidiKeyName)];
  char midiprogram[sizeof(MidiProgramName)];
  char midiprogramcat[sizeof(MidiProgramCategory)];
  char vstspeaker[sizeof(VstSpeakerArrangement)];
  char vstspeaker2[sizeof(VstSpeakerArrangement)];
#endif
#ifdef CANDOEFF
  char sendstr[512];
#endif
  int createthread;
  int parfin;
  int timeinit;
};

#endif
