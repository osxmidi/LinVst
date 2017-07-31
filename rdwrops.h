/*
  dssi-vst: a DSSI plugin wrapper for VST effects and instruments
  Copyright 2004-2007 Chris Cannam
*/

#ifndef _RD_WR_OPS_H_

#include <string>
#include "remoteplugin.h"

#ifdef RINGB
#include <vector>
#include <semaphore.h>

#define SHM_RING_BUFFER_SIZE 4096

struct RingBuffer
{
    int head;
    int tail;
    int written;
    bool invalidateCommit;
    char buf[SHM_RING_BUFFER_SIZE];
};

struct ShmControl
{
    // 32 and 64-bit binaries align semaphores differently.
    // Let's make sure there's plenty of room for either one.
    union {
        sem_t runServer;
        char alignServerSem[32];
    };
    union {
        sem_t runClient;
        char alignClientSem[32];
    };
    RingBuffer ringBuffer;
};

#define tryWrite2(a, b, c) rdwr_tryWrite2(a, b, c, __FILE__, __LINE__)
#define tryRead(a, b, c) rdwr_tryRead(a, b, c, __FILE__, __LINE__)
#define tryWrite(a, b, c) rdwr_tryWrite(a, b, c, __FILE__, __LINE__)
#define writeOpcode(a, b) rdwr_writeOpcode(a, b, __FILE__, __LINE__)
#define writeString(a, b) rdwr_writeString(a, b, __FILE__, __LINE__)
#define readString(a) rdwr_readString(a, __FILE__, __LINE__)
#define writeInt(a, b) rdwr_writeInt(a, b, __FILE__, __LINE__)
#define readInt(a) rdwr_readInt(a, __FILE__, __LINE__)
#define writeFloat(a, b) rdwr_writeFloat(a, b, __FILE__, __LINE__)
#define readFloat(a) rdwr_readFloat(a, __FILE__, __LINE__)
#define commitWrite(a) rdwr_commitWrite(a, __FILE__, __LINE__)
#define purgeRead(a) rdwr_purgeRead(a, __FILE__, __LINE__)
#define writeOpcodering(a, b) rdwr_writeOpcodering(a, b, __FILE__, __LINE__)
#define writeIntring(a, b) rdwr_writeIntring(a, b, __FILE__, __LINE__)
#define readIntring(a) rdwr_readIntring(a, __FILE__, __LINE__)
#define tryReadring(a, b, c) rdwr_tryReadring(a, b, c, __FILE__, __LINE__)
#define writeFloatring(a, b) rdwr_writeFloatring(a, b, __FILE__, __LINE__)
#define readFloatring(a) rdwr_readFloatring(a, __FILE__, __LINE__)

#else

#define tryWrite2(a, b, c) rdwr_tryWrite2(a, b, c, __FILE__, __LINE__)
#define tryRead(a, b, c) rdwr_tryRead(a, b, c, __FILE__, __LINE__)
#define tryWrite(a, b, c) rdwr_tryWrite(a, b, c, __FILE__, __LINE__)
#define writeOpcode(a, b) rdwr_writeOpcode(a, b, __FILE__, __LINE__)
#define writeString(a, b) rdwr_writeString(a, b, __FILE__, __LINE__)
#define readString(a) rdwr_readString(a, __FILE__, __LINE__)
#define writeInt(a, b) rdwr_writeInt(a, b, __FILE__, __LINE__)
#define readInt(a) rdwr_readInt(a, __FILE__, __LINE__)
#define writeFloat(a, b) rdwr_writeFloat(a, b, __FILE__, __LINE__)
#define readFloat(a) rdwr_readFloat(a, __FILE__, __LINE__)

#endif

#endif