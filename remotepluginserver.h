/*  dssi-vst: a DSSI plugin wrapper for VST effects and instruments
    Copyright 2004-2007 Chris Cannam
*/

#ifndef REMOTE_PLUGIN_SERVER_H
#define REMOTE_PLUGIN_SERVER_H

#ifdef __WINE__
#else
#define __cdecl
#endif




#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#ifdef DRAGWIN 
#include <objidl.h>
#include <shellapi.h>
#endif
#undef min
#undef max

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

#include "rdwrops.h"
#include "remoteplugin.h"
#include <string>

#include <atomic>



class RemotePluginServer {
public:
  virtual ~RemotePluginServer();

  virtual int getVersion() = 0;
  virtual std::string getName() = 0;
  virtual std::string getMaker() = 0;

  virtual void setBufferSize(int) = 0;
  virtual void setSampleRate(int) = 0;

  virtual void reset() = 0;
  virtual void terminate() = 0;

  virtual int getInputCount() = 0;
  virtual int getOutputCount() = 0;
  virtual int getFlags() = 0;
  virtual int getinitialDelay() = 0;

  virtual int processVstEvents() = 0;

  virtual void getChunk(ShmControl *m_shmControlptr) = 0;
  virtual void setChunk(ShmControl *m_shmControlptr) = 0;
  virtual void canBeAutomated(ShmControl *m_shmControlptr) = 0;
  virtual void getProgram(ShmControl *m_shmControlptr) = 0;
  virtual void EffectOpen(ShmControl *m_shmControlptr) = 0;

  // virtual int             getUniqueID() = 0;
  // virtual int             getVersion() = 0;
  // virtual void            eff_mainsChanged(int v) = 0;

  virtual int getUID() { return 0; }
  virtual int getParameterCount() { return 0; }
  virtual std::string getParameterName(int) { return ""; }
  virtual std::string getParameterLabel(int) { return ""; }
  virtual std::string getParameterDisplay(int) { return ""; }
  virtual void setParameter(int, float) { return; }
  virtual float getParameter(int) { return 0.0f; }
  virtual float getParameterDefault(int) { return 0.0f; }
  virtual void getParameters(int p0, int pn, float *v) {
    for (int i = p0; i <= pn; ++i)
      v[i - p0] = 0.0f;
  }
#ifdef WAVES
  virtual int getShellName(char *name) { return 0; }
#endif
  virtual int getProgramCount() { return 0; }
  virtual int getProgramNameIndexed(int, char *name) { return 0; }
  virtual std::string getProgramName() { return ""; }
  virtual void setCurrentProgram(int) { return; }

  virtual int getEffInt(int opcode, int value) { return 0; }
  virtual std::string getEffString(int opcode, int index) { return ""; }
  virtual void effDoVoid(int opcode) { return; }
  virtual int effDoVoid2(int opcode, int index, int value, float opt) {
    return 0;
  }

  virtual void process(float **inputs, float **outputs, int sampleFrames) = 0;
#ifdef DOUBLEP
  virtual void processdouble(double **inputs, double **outputs,
                             int sampleFrames) = 0;
  virtual bool setPrecision(int value) { return false; }
#ifndef INOUTMEM
  double **m_inputsdouble;
  double **m_outputsdouble;
#else
  double *m_inputsdouble[1024];
  double *m_outputsdouble[1024];
#endif
#endif

  virtual bool getInProp(int index, ShmControl *m_shmControlptr) {
    return false;
  }
  virtual bool getOutProp(int index, ShmControl *m_shmControlptr) {
    return false;
  }

#ifdef MIDIEFF
  virtual bool getMidiKey(int index, ShmControl *m_shmControlptr) {
    return false;
  }
  virtual bool getMidiProgName(int index, ShmControl *m_shmControlptr) {
    return false;
  }
  virtual bool getMidiCurProg(int index, ShmControl *m_shmControlptr) {
    return false;
  }
  virtual bool getMidiProgCat(int index, ShmControl *m_shmControlptr) {
    return false;
  }
  virtual bool getMidiProgCh(int index, ShmControl *m_shmControlptr) {
    return false;
  }
  virtual bool setSpeaker(ShmControl *m_shmControlptr) { return false; }
  virtual bool getSpeaker(ShmControl *m_shmControlptr) { return false; }
#endif
#ifdef CANDOEFF
  virtual bool getEffCanDo(std::string) = 0;
#endif

  virtual void setDebugLevel(RemotePluginDebugLevel) { return; }
  virtual bool warn(std::string) = 0;

  virtual void showGUI(ShmControl *m_shmControlptr) {}
  virtual void hideGUI() {}
  virtual void hideGUI2() {}
#ifdef EMBED
  virtual void openGUI() {}
#endif
  virtual void guiUpdate() {}
  virtual void finisherror() {}

  void dispatch(int timeout = -1); // may throw RemotePluginClosedException
  void dispatchControl(int timeout = -1); // may throw RemotePluginClosedException
  void dispatchProcess(int timeout = -1); // may throw RemotePluginClosedException
  void dispatchGetSet(int timeout = -1);   // may throw RemotePluginClosedException
  void dispatchPar(int timeout = -1); // may throw RemotePluginClosedException
  void dispatchControl2(int timeout = -1); // may throw RemotePluginClosedException

  int sizeShm();
  char *m_shm;
  char *m_shm2;
  char *m_shm3;
  char *m_shm4;
  char *m_shm5;
#ifdef PCACHE
  char *m_shm6;

  struct alignas(64) ParamState {
  float value;
  float valueupdate;
  char changed;
  };
#endif
  int m_shmControlFd;

  int m_threadsfinish;

  void waitForClient2exit();
  void waitForClient3exit();
  void waitForClient4exit();
  void waitForClient5exit();
  void waitForClient6exit();

protected:
  RemotePluginServer(std::string fileIdentifiers);
  void cleanup();

private:
  void dispatchControlEvents(ShmControl *m_shmControlptr);
  void dispatchProcessEvents();
  void dispatchGetSetEvents();
  void dispatchParEvents();

  int m_flags;
  int m_shmFd;
  size_t m_shmSize;
  char *m_shmFileName;

#ifndef INOUTMEM
  float **m_inputs;
  float **m_outputs;
#else
  float *m_inputs[1024];
  float *m_outputs[1024];
#endif

  RemotePluginDebugLevel m_debugLevel;

public:
#ifdef CHUNKBUF
  void *chunkptr;
  char *chunkptr2;
#endif
  int m_bufferSize;
  int m_numInputs;
  int m_numOutputs;

  ShmControl *m_shmControl;
  ShmControl *m_shmControl2;
  ShmControl *m_shmControl3;
  ShmControl *m_shmControl4;
  ShmControl *m_shmControl5;
  ShmControl *m_shmControl6;

  void waitForServer(ShmControl *m_shmControlptr);
  void waitForServerexit();
  void waitForServerexcept();

  void RemotePluginClosedException();

  int m_inexcept;

  int m_386run;

#ifdef EMBED
#ifdef TRACKTIONWM
  int hosttracktion;
#endif
#endif

  int starterror;

  int initdone;

  bool fwait(ShmControl *m_shmControlptr, std::atomic_int *fcount, int ms);
  bool fpost(ShmControl *m_shmControlptr, std::atomic_int *fcount);

  bool fwait2(ShmControl *m_shmControlptr, std::atomic_int *fcount, int ms);
  bool fpost2(ShmControl *m_shmControlptr, std::atomic_int *fcount);

  VstTimeInfo *timeinfo;
  VstTimeInfo timeinfo2;

  int bufferSize;
  int sampleRate;

  struct alignas(64) vinfo {
    char a[64 + 8 + (sizeof(int32_t) * 2) + 48];
    // char a[96];
  };
  
  struct alignas(64) winmessage {
    int handle;
    int width;
    int height;
    int winerror;
  } winm2;
  winmessage *winm;
  
  HANDLE ThreadHandle[4];

  ShmControl *m_shmControlptr;

  int m_updateio;
  int m_updatein;
  int m_updateout;
  int m_delay;
};
#endif
