/*  dssi-vst: a DSSI plugin wrapper for VST effects and instruments
    Copyright 2004-2007 Chris Cannam

    This file is part of linvst.

    linvst is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A P386ARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "remotepluginserver.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <iostream>
#include <time.h>

RemotePluginServer::RemotePluginServer(std::string fileIdentifiers)
    : m_bufferSize(-1), m_numInputs(-1), m_numOutputs(-1), m_updateio(0),
      m_updatein(0), m_updateout(0),
#ifdef CHUNKBUF
      chunkptr(0), chunkptr2(0),
#endif
      m_flags(0), m_delay(0), timeinfo(0), bufferSize(1024), sampleRate(44100),
      m_inexcept(0), m_shmFd(-1), m_shmControl(0), m_shmControl2(0),
      m_shmControl3(0), m_shmControl4(0), m_shmControl5(0), m_shmControl6(0), m_shmFileName(0),
      m_shm(0), m_shmSize(0), m_shm2(0), m_shm3(0), m_shm4(0), m_shm5(0),
#ifndef INOUTMEM
      m_inputs(0), m_outputs(0),
#ifdef DOUBLEP
      m_inputsdouble(0), m_outputsdouble(0),
#endif
#endif
#ifdef EMBED
#ifdef TRACKTIONWM
      hosttracktion(0),
#endif
#endif
#ifdef PCACHE
     m_shm6(0),
#endif    
      m_threadsfinish(0), m_386run(0), starterror(0) {
  char tmpFileBase[60];
  int startok;

  sprintf(tmpFileBase, "/tmp/rplugin_shm_%s", fileIdentifiers.substr(0, 6).c_str());

  m_shmFileName = strdup(tmpFileBase);

  if ((m_shmFd = open(m_shmFileName, O_RDWR)) < 0) {
    starterror = 1;
    cleanup();
    return;
  }

  if (sizeShm()) {
    starterror = 1;
    cleanup();
    return;
  }

  m_shmControl->ropcode = RemotePluginNoOpcode;
  m_shmControl2->ropcode = RemotePluginNoOpcode;
  m_shmControl3->ropcode = RemotePluginNoOpcode;
  m_shmControl4->ropcode = RemotePluginNoOpcode;
  m_shmControl5->ropcode = RemotePluginNoOpcode;
  m_shmControl6->ropcode = RemotePluginNoOpcode;  

  atomic_init(&m_shmControl->runServer, 0);
  atomic_init(&m_shmControl->runClient, 0);

  atomic_init(&m_shmControl2->runServer, 0);
  atomic_init(&m_shmControl2->runClient, 0);

  atomic_init(&m_shmControl3->runServer, 0);
  atomic_init(&m_shmControl3->runClient, 0);

  atomic_init(&m_shmControl4->runServer, 0);
  atomic_init(&m_shmControl4->runClient, 0);

  atomic_init(&m_shmControl5->runServer, 0);
  atomic_init(&m_shmControl5->runClient, 0);
  
  atomic_init(&m_shmControl6->runServer, 0);
  atomic_init(&m_shmControl6->runClient, 0);  

  atomic_init(&m_shmControl->nwaitersserver, 0);
  atomic_init(&m_shmControl2->nwaitersserver, 0);
  atomic_init(&m_shmControl3->nwaitersserver, 0);
  atomic_init(&m_shmControl4->nwaitersserver, 0);
  atomic_init(&m_shmControl5->nwaitersserver, 0);
  atomic_init(&m_shmControl6->nwaitersserver, 0);  

  atomic_init(&m_shmControl->nwaitersclient, 0);
  atomic_init(&m_shmControl2->nwaitersclient, 0);
  atomic_init(&m_shmControl3->nwaitersclient, 0);
  atomic_init(&m_shmControl4->nwaitersclient, 0);
  atomic_init(&m_shmControl5->nwaitersclient, 0);
  atomic_init(&m_shmControl6->nwaitersclient, 0);  

  //   timeinfo = new VstTimeInfo;

  timeinfo = &timeinfo2;
}

RemotePluginServer::~RemotePluginServer() {
  if (starterror == 0) {
#ifndef INOUTMEM
    if (m_inputs) {
      delete m_inputs;
      m_inputs = 0;
    }

    if (m_outputs) {
      delete m_outputs;
      m_outputs = 0;
    }

#ifdef DOUBLEP
    if (m_inputsdouble) {
      delete m_inputsdouble;
      m_inputsdouble = 0;
    }

    if (m_outputsdouble) {
      delete m_outputsdouble;
      m_outputsdouble = 0;
    }
#endif
#endif

    //   if(timeinfo)
    //   delete timeinfo;

    cleanup();
  }
}

void RemotePluginServer::cleanup() {
  if (m_shm) {
    munmap(m_shm, m_shmSize);
    m_shm = 0;
  }

  if (m_shmFd >= 0) {
    close(m_shmFd);
    m_shmFd = -1;
  }

  if (m_shmFileName) {
    shm_unlink(m_shmFileName);
    free(m_shmFileName);
    m_shmFileName = 0;
  }
}

int RemotePluginServer::sizeShm() {
  if (m_shm)
    return 0;

int *ptr;

int pagesize = sysconf(_SC_PAGESIZE);
int chunksize;
int chunks;
int chunkrem;

    chunksize = PROCESSSIZE;
    chunks = chunksize / pagesize;
    chunkrem = chunksize % pagesize;

    if(chunkrem > 0)
    chunks += 1;

    int processsize = chunks * pagesize;

    chunksize = VSTEVENTS_PROCESS;
    chunks = chunksize / pagesize;
    chunkrem = chunksize % pagesize;

    if(chunkrem > 0)
    chunks += 1;

    int vsteventsprocess = chunks * pagesize;

    chunksize = CHUNKSIZEMAX;
    chunks = chunksize / pagesize;
    chunkrem = chunksize % pagesize;

    if(chunkrem > 0)
    chunks += 1;

    int chunksizemax = chunks * pagesize;

    chunksize = VSTEVENTS_SEND;
    chunks = chunksize / pagesize;
    chunkrem = chunksize % pagesize;

    if(chunkrem > 0)
    chunks += 1;

    int vsteventssend = chunks * pagesize;

    chunksize = sizeof(ShmControl);
    chunks = chunksize / pagesize;
    chunkrem = chunksize % pagesize;

    if(chunkrem > 0)
    chunks += 1;

    int chunksizecontrol = chunks * pagesize;

#ifdef PCACHE
    chunksize = PARCACHE;
    chunks = chunksize / pagesize;
    chunkrem = chunksize % pagesize;

    if(chunkrem > 0)
    chunks += 1;

    int parcachesize = chunks * pagesize;
  size_t sz = processsize + vsteventsprocess + chunksizemax + vsteventssend + (chunksizecontrol * 6) + parcachesize;
#else
  size_t sz = processsize + vsteventsprocess + chunksizemax + vsteventssend + (chunksizecontrol * 6);
#endif  

  m_shm = (char *)mmap(0, sz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,
                       m_shmFd, 0);
  if (!m_shm) {
    std::cerr
        << "RemotePluginServer::sizeShm: ERROR: mmap or mremap for failed for "
        << sz << " bytes from fd " << m_shmFd << "!" << std::endl;
    m_shmSize = 0;
    return 1;
  } else {
    madvise(m_shm, sz, MADV_SEQUENTIAL | MADV_WILLNEED | MADV_DONTFORK);
    memset(m_shm, 0, sz);
    m_shmSize = sz;

    if (mlock(m_shm, sz) != 0)
      perror("mlock fail1");
  }

  m_shm2 = &m_shm[processsize];
  m_shm3 = &m_shm[processsize + vsteventsprocess];
  m_shm4 = &m_shm[processsize + vsteventsprocess + chunksizemax];

  m_shm5 = &m_shm[processsize + vsteventsprocess + chunksizemax + vsteventssend];

#ifdef PCACHE
  m_shm6 = &m_shm[processsize + vsteventsprocess + chunksizemax + vsteventssend + (chunksizecontrol * 6)];
#endif  

  m_shmControl = (ShmControl *)m_shm5;
//  memset(m_shmControl, 0, sizeof(ShmControl));
  m_shmControl2 = (ShmControl *)&m_shm5[chunksizecontrol];
//  memset(m_shmControl2, 0, sizeof(ShmControl));
  m_shmControl3 = (ShmControl *)&m_shm5[chunksizecontrol * 2];
//  memset(m_shmControl3, 0, sizeof(ShmControl));
  m_shmControl4 = (ShmControl *)&m_shm5[chunksizecontrol * 3];
//  memset(m_shmControl4, 0, sizeof(ShmControl));
  m_shmControl5 = (ShmControl *)&m_shm5[chunksizecontrol * 4];
//  memset(m_shmControl5, 0, sizeof(ShmControl));
  m_shmControl6 = (ShmControl *)&m_shm5[chunksizecontrol * 5];
//  memset(m_shmControl6, 0, sizeof(ShmControl));  

  int startok;

  startok = 0;

  ptr = (int *)m_shm;

  *ptr = 478;

  for (int i = 0; i < 400000; i++) {
    if ((*ptr == 2) || (*ptr == 3)) {
      startok = 1;
      break;
    }

    if (*ptr == 4) {
      startok = 0;
      break;
    }
    usleep(100);
  }

  if (startok == 0)
    return 1;

  if (*ptr == 3)
    m_386run = 1;

  return 0;
}

void RemotePluginServer::waitForClient2exit() {
  fpost(m_shmControl2, &m_shmControl2->runClient);
  fpost(m_shmControl2, &m_shmControl2->runServer);
}

void RemotePluginServer::waitForClient3exit() {
  fpost(m_shmControl3, &m_shmControl3->runClient);
  fpost(m_shmControl3, &m_shmControl3->runServer);
}

void RemotePluginServer::waitForClient4exit() {
  fpost(m_shmControl4, &m_shmControl4->runClient);
  fpost(m_shmControl4, &m_shmControl4->runServer);
}

void RemotePluginServer::waitForClient5exit() {
  fpost(m_shmControl5, &m_shmControl5->runClient);
  fpost(m_shmControl5, &m_shmControl5->runServer);
}

void RemotePluginServer::waitForClient6exit() {
  fpost(m_shmControl6, &m_shmControl6->runClient);
  fpost(m_shmControl6, &m_shmControl6->runServer);
}

void RemotePluginServer::dispatch(int timeout) {}

void RemotePluginServer::dispatchProcess(int timeout) {
  ShmControl *m_shmControlptr2;

  m_shmControlptr2 = m_shmControl2;

  if (fwait2(m_shmControlptr2, &m_shmControlptr2->runServer, timeout)) {
    if (errno == ETIMEDOUT) {
      return;
    } else {
      if (m_inexcept == 0)
        RemotePluginClosedException();
    }
  }

  if (m_shmControlptr2->ropcode != RemotePluginNoOpcode)
    dispatchProcessEvents();

  if (fpost2(m_shmControlptr2, &m_shmControlptr2->runClient)) {
    std::cerr << "Could not post to semaphore\n";
  }
}

void RemotePluginServer::dispatchProcessEvents() {
  ShmControl *m_shmControlptr2;

  m_shmControlptr2 = m_shmControl2;

  RemotePluginOpcode opcode = RemotePluginNoOpcode;

  opcode = m_shmControlptr2->ropcode;

  if (opcode == RemotePluginNoOpcode)
    return;

  switch (opcode) {
  case RemotePluginProcess: {
#ifndef OLDMIDI
    int els;
    int *ptr;

    ptr = (int *)m_shm2;
    els = *ptr;

    if (els > 0) {
      processVstEvents();      
      *ptr = 0;    
    }
#endif

    int sampleFrames = m_shmControlptr2->value2;
    //	if(m_updateio == 1)

#ifndef INOUTMEM
    if (sampleFrames == -1) {
      if (m_inputs) {
        delete m_inputs;
        m_inputs = 0;
      }
      if (m_updatein > 0)
        m_inputs = new float *[m_updatein];

      if (m_outputs) {
        delete m_outputs;
        m_outputs = 0;
      }
      if (m_updateout > 0)
        m_outputs = new float *[m_updateout];

      m_numInputs = m_updatein;
      m_numOutputs = m_updateout;

      m_updateio = 0;
      break;
    }
#else
    if (sampleFrames == -1) {
      m_numInputs = m_updatein;
      m_numOutputs = m_updateout;

      m_updateio = 0;
      break;
    }
#endif

    if (m_bufferSize < 0) {
      break;
    }
    if (m_numInputs < 0) {
      break;
    }
    if (m_numOutputs < 0) {
      break;
    }

    size_t blocksz = sampleFrames * sizeof(float);

#ifndef INOUTMEM
    if (m_inputs) {
      for (int i = 0; i < m_numInputs; ++i) {
        m_inputs[i] = (float *)(m_shm + i * blocksz);
      }
    }

    if (m_outputs) {
      for (int i = 0; i < m_numOutputs; ++i) {
        m_outputs[i] = (float *)(m_shm + i * blocksz);
      }
    }

    process(m_inputs, m_outputs, sampleFrames);
#else
    if ((m_numInputs < 1024) && (m_numOutputs < 1024)) {
      if (m_inputs) {
        for (int i = 0; i < m_numInputs; ++i) {
          m_inputs[i] = (float *)(m_shm + i * blocksz);
        }
      }

      if (m_outputs) {
        for (int i = 0; i < m_numOutputs; ++i) {
        m_outputs[i] = (float *)(m_shm + i * blocksz);
        }
      }

      process(m_inputs, m_outputs, sampleFrames);
    }
#endif
  } break;

#ifdef DOUBLEP
  case RemotePluginProcessDouble: {
#ifndef OLDMIDI
    int els;
    int *ptr;

    ptr = (int *)m_shm2;
    els = *ptr;

    if (els > 0) {
      processVstEvents();
      *ptr = 0;
    }
#endif

    int sampleFrames = m_shmControlptr2->value2;
    //	if(m_updateio == 1)

#ifndef INOUTMEM
    if (sampleFrames == -1) {
      if (m_inputsdouble) {
        delete m_inputsdouble;
        m_inputsdouble = 0;
      }
      if (m_updatein > 0)
        m_inputsdouble = new double *[m_updatein];

      if (m_outputsdouble) {
        delete m_outputsdouble;
        m_outputsdouble = 0;
      }
      if (m_updateout > 0)
        m_outputsdouble = new double *[m_updateout];

      m_numInputs = m_updatein;
      m_numOutputs = m_updateout;

      m_updateio = 0;
      break;
    }
#else
    if (sampleFrames == -1) {
      m_numInputs = m_updatein;
      m_numOutputs = m_updateout;

      m_updateio = 0;
      break;
    }
#endif

    if (m_bufferSize < 0) {
      break;
    }
    if (m_numInputs < 0) {
      break;
    }
    if (m_numOutputs < 0) {
      break;
    }

    size_t blocksz = sampleFrames * sizeof(double);

#ifndef INOUTMEM
    if (m_inputsdouble) {
      for (int i = 0; i < m_numInputs; ++i) {
        m_inputsdouble[i] = (double *)(m_shm + i * blocksz);
      }
    }

    if (m_outputsdouble) {
      for (int i = 0; i < m_numOutputs; ++i) {
        m_outputsdouble[i] = (double *)(m_shm + i * blocksz);
      }
    }

    processdouble(m_inputsdouble, m_outputsdouble, sampleFrames);
#else
    if ((m_numInputs < 1024) && (m_numOutputs < 1024)) {
      if (m_inputsdouble) {
        for (int i = 0; i < m_numInputs; ++i) {
          m_inputsdouble[i] = (double *)(m_shm + i * blocksz);
        }
      }

      if (m_outputsdouble) {
        for (int i = 0; i < m_numOutputs; ++i) {
          m_outputsdouble[i] = (double *)(m_shm + i * blocksz);
        }
      }

      processdouble(m_inputsdouble, m_outputsdouble, sampleFrames);
    }
#endif
  } break;
#endif

  case RemotePluginProcessEvents:
    processVstEvents();
    break;

  default:
    std::cerr << "WARNING: RemotePluginServer::dispatchProcessEvents: "
                 "unexpected opcode "
              << opcode << std::endl;
  }
  m_shmControlptr2->ropcode = RemotePluginNoOpcode;
}

void RemotePluginServer::dispatchGetSet(int timeout) {
  ShmControl *m_shmControlptr4;

  m_shmControlptr4 = m_shmControl4;

  if (fwait2(m_shmControlptr4, &m_shmControlptr4->runServer, timeout)) {
    if (errno == ETIMEDOUT) {
      return;
    } else {
      if (m_inexcept == 0)
        RemotePluginClosedException();
    }
  }

  if (m_shmControlptr4->ropcode != RemotePluginNoOpcode)
    dispatchGetSetEvents();

  if (fpost2(m_shmControlptr4, &m_shmControlptr4->runClient)) {
    std::cerr << "Could not post to semaphore\n";
  }
}

void RemotePluginServer::dispatchGetSetEvents() {
  ShmControl *m_shmControlptr4;
  m_shmControlptr4 = m_shmControl4;
  RemotePluginOpcode opcode = RemotePluginNoOpcode;
  opcode = m_shmControlptr4->ropcode;
  if (opcode == RemotePluginNoOpcode)
    return;

  switch (opcode) {

  case RemotePluginSetParameter: {
    int pn = m_shmControlptr4->value;
    float floatval = m_shmControlptr4->floatvalue;
    setParameter(pn, floatval);
    break;
  }

  default:
    std::cerr << "WARNING: RemotePluginServer::dispatchGetSetEvents: "
                 "unexpected opcode "
              << opcode << std::endl;
  }
  m_shmControlptr4->ropcode = RemotePluginNoOpcode;
}

void RemotePluginServer::dispatchPar(int timeout) {
  ShmControl *m_shmControlptr;
  m_shmControlptr = m_shmControl5;

  if (fwait2(m_shmControlptr, &m_shmControlptr->runServer, timeout)) {
    if (errno == ETIMEDOUT) {
      return;
    } else {
      if (m_inexcept == 0)
        RemotePluginClosedException();
    }
  }

  if (m_shmControlptr->ropcode != RemotePluginNoOpcode)
    dispatchParEvents();

  if (fpost2(m_shmControlptr, &m_shmControlptr->runClient)) {
    std::cerr << "Could not post to semaphore\n";
  }
}

void RemotePluginServer::dispatchParEvents() {
  ShmControl *m_shmControlptr5;
  m_shmControlptr5 = m_shmControl5;
  RemotePluginOpcode opcode = RemotePluginNoOpcode;
  opcode = m_shmControlptr5->ropcode;
  if (opcode == RemotePluginNoOpcode)
    return;

  switch (opcode) {

  case RemotePluginGetParameter: {
    int intval = m_shmControlptr5->value;
    float floatval = getParameter(intval);
    m_shmControlptr5->retfloat = floatval;
    break;
  }

  default:
    std::cerr
        << "WARNING: RemotePluginServer::dispatchParEvents: unexpected opcode "
        << opcode << std::endl;
  }
  m_shmControlptr5->ropcode = RemotePluginNoOpcode;
}

void RemotePluginServer::dispatchControl2(int timeout) {
  int startok;
  ShmControl *m_shmControlptr3;
  m_shmControlptr3 = m_shmControl6;

  if (fwait2(m_shmControlptr3, &m_shmControlptr3->runServer, timeout)) {
    if (errno == ETIMEDOUT) {
      return;
    } else {
      if (m_inexcept == 0)
        RemotePluginClosedException();
    }
  }

  if (m_shmControlptr3->ropcode != RemotePluginNoOpcode)
    dispatchControlEvents(m_shmControlptr3);

  if (fpost2(m_shmControlptr3, &m_shmControlptr3->runClient)) {
    std::cerr << "Could not post to semaphore\n";
  }
}

void RemotePluginServer::dispatchControl(int timeout) {
  int startok;
  ShmControl *m_shmControlptr3;
  m_shmControlptr3 = m_shmControl3;

  if (fwait2(m_shmControlptr3, &m_shmControlptr3->runServer, timeout)) {
    if (errno == ETIMEDOUT) {
      return;
    } else {
      if (m_inexcept == 0)
        RemotePluginClosedException();
    }
  }

  if (m_shmControlptr3->ropcode != RemotePluginNoOpcode)
    dispatchControlEvents(m_shmControlptr3);

  if (fpost2(m_shmControlptr3, &m_shmControlptr3->runClient)) {
    std::cerr << "Could not post to semaphore\n";
  }
}

void RemotePluginServer::dispatchControlEvents(ShmControl *m_shmControlptr) {
  RemotePluginOpcode opcode = RemotePluginNoOpcode;
  opcode = m_shmControlptr->ropcode;
  if (opcode == RemotePluginNoOpcode)
    return;

  switch (opcode) {
  case RemotePluginShowGUI:
    memcpy(winm, m_shmControl3->wret, sizeof(winmessage));
    showGUI(m_shmControl3);
    break;

  case RemotePluginHideGUI:
    hideGUI();
    break;
    /*
       case RemotePluginHideGUI:
           hideGUI2();
           break;
    */
#ifdef EMBED
  case RemotePluginOpenGUI:
    openGUI();
    break;
#endif

  case RemotePluginEffectOpen:
    EffectOpen(m_shmControl3);
    break;

  case RemotePluginGetEffInt: {
    int retint;
    int opcode = m_shmControlptr->opcode;
    int value = m_shmControlptr->value;
    retint = getEffInt(opcode, value);
    m_shmControlptr->retint = retint;
    break;
  }

  case RemotePluginGetProgramNameIndexed: {
    char name[512];
    int val;
    val = m_shmControlptr->value;
    int retvalprogramname = getProgramNameIndexed(val, name);
    m_shmControlptr->retint = retvalprogramname;
    strcpy(m_shmControlptr->retstr, name);
    break;
  }

  case RemotePluginGetProgramName:
    strcpy(m_shmControlptr->retstr, getProgramName().c_str());
    break;

  case RemotePluginGetProgram:
    getProgram(m_shmControlptr);
    break;

  case RemotePluginSetCurrentProgram:
    setCurrentProgram(m_shmControlptr->value);
    break;

  case RemotePluginUniqueID: {
    int retval;
    retval = getUID();
    m_shmControlptr->retint = retval;
  } break;

  case RemotePluginGetFlags: {
    int retval;
    retval = getFlags();
    m_flags = retval;
    m_shmControlptr->retint = retval;
  } break;

  case RemotePluginGetinitialDelay: {
    int retval;
    retval = getinitialDelay();
    m_delay = retval;
    m_shmControlptr->retint = retval;
  } break;

  case RemotePluginGetProgramCount: {
    int retval;
    retval = getProgramCount();
    m_shmControlptr->retint = retval;
  } break;

  case RemotePluginGetInputCount: {
    int numin = getInputCount();
    // m_numInputs = getInputCount();

#ifndef INOUTMEM
    if (numin != m_numInputs) {
      if (m_inputs) {
        delete m_inputs;
        m_inputs = 0;
      }
      if (numin > 0)
        m_inputs = new float *[numin];

#ifdef DOUBLEP
      if (m_inputsdouble) {
        delete m_inputsdouble;
        m_inputsdouble = 0;
      }
      if (numin > 0)
        m_inputsdouble = new double *[numin];
#endif
    }
#endif
    m_numInputs = numin;
    m_shmControlptr->retint = numin;
    break;
  }

  case RemotePluginGetOutputCount: {
    int numout = getOutputCount();

    // m_numOutputs = getOutputCount();

#ifndef INOUTMEM
    if (numout != m_numOutputs) {
      if (m_outputs) {
        delete m_outputs;
        m_outputs = 0;
      }
      if (numout > 0)
        m_outputs = new float *[numout];

#ifdef DOUBLEP
      if (m_outputsdouble) {
        delete m_outputsdouble;
        m_outputsdouble = 0;
      }
      if (numout > 0)
        m_outputsdouble = new double *[numout];
#endif
    }
#endif
    m_numOutputs = numout;
    m_shmControlptr->retint = numout;
    break;
  }

  case RemotePluginGetParameterName:
    strcpy(m_shmControlptr->retstr,
           getParameterName(m_shmControlptr->value).c_str());
    break;

  case RemotePluginGetParameterLabel:
    strcpy(m_shmControlptr->retstr,
           getParameterLabel(m_shmControlptr->value).c_str());
    break;

  case RemotePluginGetParameterDisplay:
    strcpy(m_shmControlptr->retstr,
           getParameterDisplay(m_shmControlptr->value).c_str());
    break;

  case RemotePluginGetParameterCount:
    m_shmControlptr->retint = getParameterCount();
    break;

  case RemotePluginDoVoid: {
    int opcode2 = m_shmControlptr->opcode;
    if (opcode2 == effClose) {
      m_threadsfinish = 1;
      waitForClient2exit();
      waitForClient3exit();
      waitForClient4exit();
      waitForClient5exit();
      waitForClient6exit();      

      effDoVoid(opcode2);
      m_shmControlptr->ropcode = RemotePluginNoOpcode;
    } else
      effDoVoid(opcode2);
    break;
  }

  case RemoteInProp: {
    int index = m_shmControlptr->value;
    bool b = getInProp(index, m_shmControlptr);
    m_shmControlptr->retbool = b;
    break;
  }

  case RemoteOutProp: {
    int index = m_shmControlptr->value;
    bool b = getOutProp(index, m_shmControlptr);
    m_shmControlptr->retbool = b;
    break;
  }

  case RemotePluginDoVoid2: {
    int opcode2 = m_shmControlptr->opcode;
    int index = m_shmControlptr->value;
    int value = m_shmControlptr->value2;
    float opt = m_shmControlptr->value3;
    int b = effDoVoid2(opcode2, index, value, opt);
    m_shmControlptr->retint = b;
    break;
  }

  case RemotePluginSetBufferSize: {
    int newSize = m_shmControlptr->value;
    setBufferSize(newSize);
    m_bufferSize = newSize;
    break;
  }

  case RemotePluginSetSampleRate:
    setSampleRate(m_shmControlptr->value);
    break;

  case RemotePluginReset:
    reset();
    break;

  case RemotePluginGetVersion:
    m_shmControlptr->value = getVersion();
    break;

  case RemotePluginGetName:
    strcpy(m_shmControlptr->retstr, getName().c_str());
    break;

  case RemotePluginGetMaker:
    strcpy(m_shmControlptr->retstr, getMaker().c_str());
    break;

  case RemotePluginTerminate:
    terminate();
    break;

#ifdef WAVES
  case RemotePluginGetShellName: {
    char name[512];
    int retvalshell = getShellName(name);
    m_shmControlptr->retint = retvalshell;
    strcpy(m_shmControlptr->retstr, name);
  } break;
#endif

  case RemotePluginGetParameterDefault:
    break;

  case RemotePluginGetParameters:
    break;

  case RemotePluginSetDebugLevel:
    break;

  case RemotePluginWarn:
    break;

  case RemotePluginGetEffString: {
    int opcode = m_shmControlptr->opcode;
    int idx = m_shmControlptr->value;
    strcpy(m_shmControlptr->retstr, getEffString(opcode, idx).c_str());
    break;
  }

  case RemotePluginCanBeAutomated:
    canBeAutomated(m_shmControlptr);
    break;

#ifdef DOUBLEP
  case RemoteSetPrecision: {
    int value = m_shmControlptr->value;
    bool b = setPrecision(value);
    m_shmControlptr->retbool = b;
    break;
  }
#endif

#ifdef MIDIEFF
  case RemoteMidiKey: {
    int index = m_shmControlptr->value;
    bool b = getMidiKey(index, m_shmControlptr);
    m_shmControlptr->retbool = b;
    break;
  }

  case RemoteMidiProgName: {
    int index = m_shmControlptr->value;
    bool b = getMidiProgName(index, m_shmControlptr);
    m_shmControlptr->retbool = b;
    break;
  }

  case RemoteMidiCurProg: {
    int index = m_shmControlptr->value;
    bool b = getMidiCurProg(index, m_shmControlptr);
    m_shmControlptr->retbool = b;
    break;
  }

  case RemoteMidiProgCat: {
    int index = m_shmControlptr->value;
    bool b = getMidiProgCat(index, m_shmControlptr);
    m_shmControlptr->retbool = b;
    break;
  }

  case RemoteMidiProgCh: {
    int index = m_shmControlptr->value;
    bool b = getMidiProgCh(index, m_shmControlptr);
    m_shmControlptr->retbool = b;
    break;
  }

  case RemoteSetSpeaker: {
    bool b = setSpeaker(m_shmControlptr);
    m_shmControlptr->retbool = b;
    break;
  }

  case RemoteGetSpeaker: {
    bool b = getSpeaker(m_shmControlptr);
    m_shmControlptr->retbool = b;
    break;
  }
#endif

#ifdef CANDOEFF
  case RemotePluginEffCanDo: {
        bool b =  getEffCanDo(m_shmControlptr->sendstr);
        m_shmControlptr->retbool = b;
        break;
  }
#endif

  case RemotePluginGetChunk:
    getChunk(m_shmControlptr);
    break;

  case RemotePluginSetChunk:
    setChunk(m_shmControlptr);
    break;

#ifdef CHUNKBUF
  case RemotePluginGetBuf: {
    char *chunkbuf = (char *)chunkptr;
    int curchunk = m_shmControlptr->value;
    int idx = m_shmControlptr->value2;
    memcpy(m_shm3, &chunkbuf[idx], curchunk);
    break;
  }

  case RemotePluginSetBuf: {
    int curchunk = m_shmControlptr->value;
    int idx = m_shmControlptr->value2;
    int sz2 = m_shmControlptr->value3;

    if (sz2 > 0)
      chunkptr2 = (char *)malloc(sz2);

    if (!chunkptr2)
      break;

    memcpy(&chunkptr2[idx], m_shm3, curchunk);
    break;
  }
#endif

  default:
    std::cerr << "WARNING: RemotePluginServer::dispatchControlEvents: "
                 "unexpected opcode "
              << opcode << std::endl;
  }
  m_shmControlptr->ropcode = RemotePluginNoOpcode;
}

#define disconnectserver 32143215

void RemotePluginServer::waitForServerexcept() {
  fpost2(m_shmControl, &m_shmControl->runServer);

  if (fwait2(m_shmControl, &m_shmControl->runClient, 60000)) {
    if (m_inexcept == 0)
      RemotePluginClosedException();
  }
}

void RemotePluginServer::RemotePluginClosedException() {
  m_inexcept = 1;

  m_shmControl->ropcode = (RemotePluginOpcode)disconnectserver;
  waitForServerexcept();
  waitForClient2exit();
  waitForClient3exit();
  waitForClient4exit();
  waitForClient5exit();
  waitForClient6exit();  

  sleep(5);

  terminate();
}

using namespace std;

bool RemotePluginServer::fwait2(ShmControl *m_shmControlptr,
                                std::atomic_int *futexp, int ms) {
  timespec timeval;
  int retval;

  if (ms > 0) {
    timeval.tv_sec = ms / 1000;
    timeval.tv_nsec = (ms %= 1000) * 1000000;
  }

  for (;;) {
    int value = atomic_load_explicit(futexp, std::memory_order_seq_cst);
    if ((*futexp != 0) &&
        (std::atomic_compare_exchange_strong(futexp, &value, value - 1) > 0))
      break;
     std::atomic_fetch_add_explicit((std::atomic_int *)&m_shmControlptr->nwaitersserver, 1, std::memory_order_seq_cst);
    retval = syscall(SYS_futex, futexp, FUTEX_WAIT, 0, &timeval, NULL, 0);
    std::atomic_fetch_sub_explicit((std::atomic_int *)&m_shmControlptr->nwaitersserver, 1, std::memory_order_seq_cst);
    if (retval == -1 && errno != EAGAIN)
      return true;
  }
  return false;
}

bool RemotePluginServer::fpost2(ShmControl *m_shmControlptr,
                                std::atomic_int *futexp) {
  int retval;
  int nval =
      std::atomic_fetch_add_explicit(futexp, 1, std::memory_order_seq_cst);
   int value = atomic_load_explicit((std::atomic_int *)&m_shmControlptr->nwaitersclient, std::memory_order_seq_cst); 
   if(value > 0)
  //{
  retval = syscall(SYS_futex, futexp, FUTEX_WAKE, 1, NULL, NULL, 0);
  //}
  /*
          if (retval  == -1)
          return true;
  */
  return false;
}

bool RemotePluginServer::fwait(ShmControl *m_shmControlptr,
                               std::atomic_int *futexp, int ms) {
  timespec timeval;
  int retval;

  if (ms > 0) {
    timeval.tv_sec = ms / 1000;
    timeval.tv_nsec = (ms %= 1000) * 1000000;
  }

  for (;;) {
    int value = atomic_load_explicit(futexp, std::memory_order_seq_cst);
    if ((*futexp != 0) &&
        (std::atomic_compare_exchange_strong(futexp, &value, value - 1) > 0))
      break;
    retval = syscall(SYS_futex, futexp, FUTEX_WAIT, 0, &timeval, NULL, 0);
    if (retval == -1 && errno != EAGAIN)
      return true;
  }
  return false;
}

bool RemotePluginServer::fpost(ShmControl *m_shmControlptr,
                               std::atomic_int *futexp) {
  int retval;

  std::atomic_fetch_add_explicit(futexp, 1, std::memory_order_seq_cst);
  retval = syscall(SYS_futex, futexp, FUTEX_WAKE, 1, NULL, NULL, 0);
  /*
          if (retval  == -1)
          return true;
  */
  return false;
}
