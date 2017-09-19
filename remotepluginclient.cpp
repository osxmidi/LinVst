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

#include "remotepluginclient.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <time.h>
#include <iostream>
#include <errno.h>

#include "paths.h"

void* RemotePluginClient::AMThread()
{
    int         opcode;
    int         val;
    int         ok = 1;

   int timeout = 50;

    VstTimeInfo *timeInfo;

    int         els;
    int         *ptr2;
    int         sizeidx = 0;
    int         size;
    VstEvents   *evptr;

    struct amessage
    {
        int flags;
        int pcount;
        int parcount;
        int incount;
        int outcount;
        int delay;
    } am;

    while (!m_threadbreak)
    {

#ifdef SEM

    if(m_386run == 0)
    {

    timespec ts_timeout;

    clock_gettime(CLOCK_REALTIME, &ts_timeout);
    time_t seconds = timeout / 1000;
    ts_timeout.tv_sec += seconds;
    ts_timeout.tv_nsec += (timeout - seconds * 1000) * 1000000;
    if (ts_timeout.tv_nsec >= 1000000000) {
        ts_timeout.tv_nsec -= 1000000000;
        ts_timeout.tv_sec++;
    }

    if (sem_timedwait(&m_shmControl->runServer, &ts_timeout)) {
        if (errno == ETIMEDOUT) {
          continue;
        } else {
        if(m_inexcept == 0)
            RemotePluginClosedException();
        }
    }
}
else
{
    if (fwait(&m_shmControl->runServer386, timeout)) {
        if (errno == ETIMEDOUT) {
            continue;
        } else {
            RemotePluginClosedException();
        }
    }
}

#else

    if (fwait(&m_shmControl->runServer, timeout)) {
        if (errno == ETIMEDOUT) {
            continue;
        } else {
        if(m_inexcept == 0)
            RemotePluginClosedException();
        }
    }

#endif

    while (dataAvailable(&m_shmControl->ringBuffer)) {

    if(m_threadbreak)
    break;
   
                opcode = -1;

   tryReadring(&m_shmControl->ringBuffer, &opcode, sizeof(int));

                switch(opcode)
                {
                    case audioMasterGetTime:       
                    val = readIntring(&m_shmControl->ringBuffer);
                    timeInfo = (VstTimeInfo *) m_audioMaster(theEffect, audioMasterGetTime, 0, val, 0, 0);
                    memcpy((VstTimeInfo*)&m_shm3[FIXED_SHM_SIZE3 - 8192], timeInfo, sizeof(VstTimeInfo));
                    break;

                case audioMasterIOChanged:
                    memcpy(&am, &m_shm3[FIXED_SHM_SIZE3], sizeof(am));
                    theEffect->flags = am.flags;
                    theEffect->numPrograms = am.pcount;
                    theEffect->numParams = am.parcount;
                    theEffect->numInputs = am.incount;
                    theEffect->numOutputs = am.outcount;
                    theEffect->initialDelay = am.delay;
                    // m_updateio = 1;
                    m_audioMaster(theEffect, audioMasterIOChanged, 0, 0, 0, 0);

                    break;

                case audioMasterProcessEvents:
                    val = readIntring(&m_shmControl->ringBuffer);

                    ptr2 = (int *)m_shm3;
                    els = *ptr2;
                    sizeidx = sizeof(int);

                    if (els > VSTSIZE)
                        els = VSTSIZE;

                    evptr = &vstev[0];
                    evptr->numEvents = els;
                    evptr->reserved = 0;

                    for (int i = 0; i < els; i++)
                    {
                        VstEvent* bsize = (VstEvent*) &m_shm3[sizeidx];
                        size = bsize->byteSize + (2*sizeof(VstInt32));
                        evptr->events[i] = bsize;
                        sizeidx += size;
                    }

                    m_audioMaster(theEffect, audioMasterProcessEvents, 0, val, evptr, 0);
               
                    break;

                default:
                    break;
                }
              }

#ifdef SEM

     if(m_386run == 0)
     {
     if (sem_post(&m_shmControl->runClient)) {
        std::cerr << "Could not post to semaphore\n";
     }
     }
     else
     { 
     if (fpost(&m_shmControl->runClient386)) {
        std::cerr << "Could not post to semaphore\n";
     }
     }

#else

     if (fpost(&m_shmControl->runClient)) {
        std::cerr << "Could not post to semaphore\n";
     }

#endif

    }
     // m_threadbreakexit = 1;
    // pthread_exit(0);
    return 0;
}

RemotePluginClient::RemotePluginClient(audioMasterCallback theMaster) :
    m_audioMaster(theMaster),
    m_shmFd(-1),
    m_shmFd2(-1),
    m_shmFd3(-1),
    m_AMThread(0),
    m_threadinit(0),
    m_threadbreak(0),
    m_threadbreakexit(0),
    m_updateio(0),
    m_shmFileName(0),
    m_shm(0),
    m_shmSize(0),
    m_shmFileName2(0),
    m_shm2(0),
    m_shmSize2(0),
    m_shmFileName3(0),
    m_shm3(0),
    m_shmSize3(0),
    m_shmControlFd(-1),
    m_shmControl(0),
    m_shmControlFileName(0),
    m_shmControl2Fd(-1),
    m_shmControl2(0),
    m_shmControl2FileName(0),
    m_shmControl3Fd(-1),
    m_shmControl3(0),
    m_shmControl3FileName(0),
    m_shmControl4Fd(-1),
    m_shmControl4(0),
    m_shmControl4FileName(0),
    m_shmControl5Fd(-1),
    m_shmControl5(0),
    m_shmControl5FileName(0),
    m_bufferSize(-1),
    m_numInputs(-1),
    m_numOutputs(-1),
    m_inexcept(0),
    m_finishaudio(0),
    m_runok(0),
    m_syncok(0),
    m_386run(0),
    theEffect(0)
{
    char tmpFileBase[60];

    srand(time(NULL));
    
    sprintf(tmpFileBase, "/vstrplugin_shm_XXXXXX");
    m_shmFd = shm_mkstemp(tmpFileBase);
    if (m_shmFd < 0)
    {
        cleanup();
        throw((std::string)"Failed to open or create shared memory file");
    }
    m_shmFileName = strdup(tmpFileBase);

    sprintf(tmpFileBase, "/vstrplugin_shn_XXXXXX");
    m_shmFd2 = shm_mkstemp(tmpFileBase);
    if (m_shmFd2 < 0)
    {
        cleanup();
        throw((std::string)"Failed to open or create shared memory file");
    }
    m_shmFileName2 = strdup(tmpFileBase);

    sprintf(tmpFileBase, "/vstrplugin_sho_XXXXXX");
    m_shmFd3 = shm_mkstemp(tmpFileBase);
    if (m_shmFd3 < 0)
    {
        cleanup();
        throw((std::string)"Failed to open or create shared memory file");
    }
    m_shmFileName3 = strdup(tmpFileBase);

    sprintf(tmpFileBase, "/vstrplugin_sha_XXXXXX");
    m_shmControlFd = shm_mkstemp(tmpFileBase);
    if (m_shmControlFd < 0) {
	cleanup();
	throw((std::string)"Failed to open or create shared memory file");
    }

    m_shmControlFileName = strdup(tmpFileBase);
    ftruncate(m_shmControlFd, sizeof(ShmControl));
    m_shmControl = static_cast<ShmControl *>(mmap(0, sizeof(ShmControl), PROT_READ | PROT_WRITE, MAP_SHARED, m_shmControlFd, 0));
    if (!m_shmControl) {
        cleanup();
        throw((std::string)"Failed to mmap shared memory file");
    }

    memset(m_shmControl, 0, sizeof(ShmControl));

    if(mlock(m_shmControl, sizeof(ShmControl)) != 0)
    perror("mlock fail4");

#ifdef SEM
    if(m_386run == 0)
    {
    if (sem_init(&m_shmControl->runServer, 1, 0)) {
        throw((std::string)"Failed to initialize shared memory semaphore");
    }
    if (sem_init(&m_shmControl->runClient, 1, 0)) {
        throw((std::string)"Failed to initialize shared memory semaphore");
    }
    }
#endif

    sprintf(tmpFileBase, "/vstrplugin_shb_XXXXXX");
    m_shmControl2Fd = shm_mkstemp(tmpFileBase);
    if (m_shmControl2Fd < 0) {
	cleanup();
	throw((std::string)"Failed to open or create shared memory file");
    }

    m_shmControl2FileName = strdup(tmpFileBase);
    ftruncate(m_shmControl2Fd, sizeof(ShmControl));
    m_shmControl2 = static_cast<ShmControl *>(mmap(0, sizeof(ShmControl), PROT_READ | PROT_WRITE, MAP_SHARED, m_shmControl2Fd, 0));
    if (!m_shmControl2) {
        cleanup();
        throw((std::string)"Failed to mmap shared memory file");
    }

    memset(m_shmControl2, 0, sizeof(ShmControl));

    if(mlock(m_shmControl2, sizeof(ShmControl)) != 0)
    perror("mlock fail4");

#ifdef SEM
    if(m_386run == 0)
    {
    if (sem_init(&m_shmControl2->runServer, 1, 0)) {
        throw((std::string)"Failed to initialize shared memory semaphore");
    }
    if (sem_init(&m_shmControl2->runClient, 1, 0)) {
        throw((std::string)"Failed to initialize shared memory semaphore");
    }
    }
#endif

    sprintf(tmpFileBase, "/vstrplugin_shc_XXXXXX");
    m_shmControl3Fd = shm_mkstemp(tmpFileBase);
    if (m_shmControl3Fd < 0) {
	cleanup();
	throw((std::string)"Failed to open or create shared memory file");
    }

    m_shmControl3FileName = strdup(tmpFileBase);
    ftruncate(m_shmControl3Fd, sizeof(ShmControl));
    m_shmControl3 = static_cast<ShmControl *>(mmap(0, sizeof(ShmControl), PROT_READ | PROT_WRITE, MAP_SHARED, m_shmControl3Fd, 0));
    if (!m_shmControl3) {
        cleanup();
        throw((std::string)"Failed to mmap shared memory file");
    }

    memset(m_shmControl3, 0, sizeof(ShmControl));

    if(mlock(m_shmControl3, sizeof(ShmControl)) != 0)
    perror("mlock fail4");

#ifdef SEM
    if(m_386run == 0)
    {
    if (sem_init(&m_shmControl3->runServer, 1, 0)) {
        throw((std::string)"Failed to initialize shared memory semaphore");
    }
    if (sem_init(&m_shmControl3->runClient, 1, 0)) {
        throw((std::string)"Failed to initialize shared memory semaphore");
    }
    }
#endif

    sprintf(tmpFileBase, "/vstrplugin_shd_XXXXXX");
    m_shmControl4Fd = shm_mkstemp(tmpFileBase);
    if (m_shmControl4Fd < 0) {
	cleanup();
	throw((std::string)"Failed to open or create shared memory file");
    }

    m_shmControl4FileName = strdup(tmpFileBase);
    ftruncate(m_shmControl4Fd, sizeof(ShmControl));
    m_shmControl4 = static_cast<ShmControl *>(mmap(0, sizeof(ShmControl), PROT_READ | PROT_WRITE, MAP_SHARED, m_shmControl4Fd, 0));
    if (!m_shmControl4) {
        cleanup();
        throw((std::string)"Failed to mmap shared memory file");
    }

    memset(m_shmControl4, 0, sizeof(ShmControl));

    if(mlock(m_shmControl4, sizeof(ShmControl)) != 0)
    perror("mlock fail4");

#ifdef SEM
    if(m_386run == 0)
    {
    if (sem_init(&m_shmControl4->runServer, 1, 0)) {
        throw((std::string)"Failed to initialize shared memory semaphore");
    }
    if (sem_init(&m_shmControl4->runClient, 1, 0)) {
        throw((std::string)"Failed to initialize shared memory semaphore");
    }
    }
#endif

    sprintf(tmpFileBase, "/vstrplugin_she_XXXXXX");
    m_shmControl5Fd = shm_mkstemp(tmpFileBase);
    if (m_shmControl5Fd < 0) {
	cleanup();
	throw((std::string)"Failed to open or create shared memory file");
    }

    m_shmControl5FileName = strdup(tmpFileBase);
    ftruncate(m_shmControl5Fd, sizeof(ShmControl));
    m_shmControl5 = static_cast<ShmControl *>(mmap(0, sizeof(ShmControl), PROT_READ | PROT_WRITE, MAP_SHARED, m_shmControl5Fd, 0));
    if (!m_shmControl5) {
        cleanup();
        throw((std::string)"Failed to mmap shared memory file");
    }

    memset(m_shmControl5, 0, sizeof(ShmControl));

    if(mlock(m_shmControl5, sizeof(ShmControl)) != 0)
    perror("mlock fail4");

#ifdef SEM
    if(m_386run == 0)
    {
    if (sem_init(&m_shmControl5->runServer, 1, 0)) {
        throw((std::string)"Failed to initialize shared memory semaphore");
    }
    if (sem_init(&m_shmControl5->runClient, 1, 0)) {
        throw((std::string)"Failed to initialize shared memory semaphore");
    }
    }
#endif

    sizeShm();
}

RemotePluginClient::~RemotePluginClient()
{
/*
    m_threadbreak = 1;
    m_threadbreakexit = 1;
 */
    if (theEffect)
    delete theEffect;
    cleanup();
}

void RemotePluginClient::syncStartup()
{
int startok;
int *ptr;

startok = 0;

ptr = (int *)m_shm;

    for (int i=0;i<2000;i++)
    {
        usleep(10000);
        if (*ptr == 1)
         {
            startok = 1;
            break;
         }
    }  

   if(startok == 0)
   {
   cleanup();
   throw((std::string)"Remote plugin did not start correctly");
   }

    if(m_386run == 1)
    {
    *ptr = 3;
    }
    else
    {
    *ptr = 2;
    }

    theEffect = new AEffect;

    m_syncok = 1;
}

void RemotePluginClient::cleanup()
{

/*
    if (m_shm)
        for (int i=0;i<1000;i++)
        {
            usleep(10000);
            if (m_threadbreakexit)
            break;
        }

*/
    if (m_AMThread)
        pthread_join(m_AMThread, NULL);
        
    if (m_shm)
    {
        munmap(m_shm, m_shmSize);
        m_shm = 0;
    }
    if (m_shm2)
    {
        munmap(m_shm2, m_shmSize2);
        m_shm2 = 0;
    }
    if (m_shm3)
    {
        munmap(m_shm3, m_shmSize3);
        m_shm3 = 0;
    }

    if (m_shmFd >= 0)
    {
        close(m_shmFd);
        m_shmFd = -1;
    }
    if (m_shmFd2 >= 0)
    {
        close(m_shmFd2);
        m_shmFd2 = -1;
    }
    if (m_shmFd3 >= 0)
    {
        close(m_shmFd3);
        m_shmFd3 = -1;
    }

    if (m_shmFileName)
    {
	shm_unlink(m_shmFileName);
        free(m_shmFileName);
        m_shmFileName = 0;
    }
    if (m_shmFileName2)
    {
 	shm_unlink(m_shmFileName2);
        free(m_shmFileName2);
        m_shmFileName2 = 0;
    }
    if (m_shmFileName3)
    {
	shm_unlink(m_shmFileName3);
        free(m_shmFileName3);
        m_shmFileName3 = 0;
    }

    if (m_shmControl) {
        munmap(m_shmControl, sizeof(ShmControl));
        m_shmControl = 0;
    }
    if (m_shmControlFd >= 0) {
        close(m_shmControlFd);
        m_shmControlFd = -1;
    }
    if (m_shmControlFileName) {
        shm_unlink(m_shmControlFileName);
        free(m_shmControlFileName);
        m_shmControlFileName = 0;
    }

    if (m_shmControl2) {
        munmap(m_shmControl2, sizeof(ShmControl));
        m_shmControl2 = 0;
    }
    if (m_shmControl2Fd >= 0) {
        close(m_shmControl2Fd);
        m_shmControl2Fd = -1;
    }
    if (m_shmControl2FileName) {
        shm_unlink(m_shmControl2FileName);
        free(m_shmControl2FileName);
        m_shmControl2FileName = 0;
    }

    if (m_shmControl3) {
        munmap(m_shmControl3, sizeof(ShmControl));
        m_shmControl3 = 0;
    }
    if (m_shmControl3Fd >= 0) {
        close(m_shmControl3Fd);
        m_shmControl3Fd = -1;
    }
    if (m_shmControl3FileName) {
        shm_unlink(m_shmControl3FileName);
        free(m_shmControl3FileName);
        m_shmControl3FileName = 0;
    }

    if (m_shmControl4) {
        munmap(m_shmControl4, sizeof(ShmControl));
        m_shmControl4 = 0;
    }
    if (m_shmControl4Fd >= 0) {
        close(m_shmControl4Fd);
        m_shmControl4Fd = -1;
    }
    if (m_shmControl4FileName) {
        shm_unlink(m_shmControl4FileName);
        free(m_shmControl4FileName);
        m_shmControl4FileName = 0;
    }

    if (m_shmControl5) {
        munmap(m_shmControl5, sizeof(ShmControl));
        m_shmControl5 = 0;
    }
    if (m_shmControl5Fd >= 0) {
        close(m_shmControl5Fd);
        m_shmControl5Fd = -1;
    }
    if (m_shmControl5FileName) {
        shm_unlink(m_shmControl5FileName);
        free(m_shmControl5FileName);
        m_shmControl5FileName = 0;
    }
}

std::string RemotePluginClient::getFileIdentifiers()
{
    std::string id;

    id += m_shmFileName + strlen(m_shmFileName) - 6;
    id += m_shmFileName2 + strlen(m_shmFileName2) - 6;
    id += m_shmFileName3 + strlen(m_shmFileName3) - 6;
    id += m_shmControlFileName + strlen(m_shmControlFileName) - 6;
    id += m_shmControl2FileName + strlen(m_shmControl2FileName) - 6;
    id += m_shmControl3FileName + strlen(m_shmControl3FileName) - 6;
    id += m_shmControl4FileName + strlen(m_shmControl4FileName) - 6;
    id += m_shmControl5FileName + strlen(m_shmControl5FileName) - 6;

    //  std::cerr << "Returning file identifiers: " << id << std::endl;
    return id;
}

void RemotePluginClient::sizeShm()
{
    if (m_shm)
        return;

    size_t sz = FIXED_SHM_SIZE + 1024;
    size_t sz2 = FIXED_SHM_SIZE2 + 1024;
    size_t sz3 = FIXED_SHM_SIZE3 + 1024;

    ftruncate(m_shmFd, sz);
    m_shm = (char *)mmap(0, sz, PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd, 0);
    if (!m_shm)
    {
        std::cerr << "RemotePluginClient::sizeShm: ERROR: mmap or mremap failed for " << sz
                    << " bytes from fd " << m_shmFd << "!" << std::endl;
        m_shmSize = 0;
    }
    else
    {
        memset(m_shm, 0, sz);
        m_shmSize = sz;
        
        if(mlock(m_shm, sz) != 0)
        perror("mlock fail1");
    }

    ftruncate(m_shmFd2, sz2);
    m_shm2 = (char *)mmap(0, sz2, PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd2, 0);
    if (!m_shm2)
    {
        std::cerr << "RemotePluginClient::sizeShm: ERROR: mmap or mremap failed for " << sz2
                    << " bytes from fd " << m_shmFd2 << "!" << std::endl;
        m_shmSize2 = 0;
    }
    else
    {
        memset(m_shm2, 0, sz2);
        m_shmSize2 = sz2;

        if(mlock(m_shm2, sz2) != 0)
        perror("mlock fail2");

    }

    ftruncate(m_shmFd3, sz3);
    m_shm3 = (char *)mmap(0, sz3, PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd3, 0);
    if (!m_shm3)
    {
        std::cerr << "RemotePluginClient::sizeShm: ERROR: mmap or mremap failed for " << sz3
                    << " bytes from fd " << m_shmFd3 << "!" << std::endl;
        m_shmSize3 = 0;
    }
    else
    {
        memset(m_shm3, 0, sz3);
        m_shmSize3 = sz3;

        if(mlock(m_shm3, sz3) != 0)
        perror("mlock fail3");

    }

    m_threadbreak = 0;
    // m_threadbreakexit = 0;

}

float   RemotePluginClient::getVersion()
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetVersion);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    return readFloat(&m_shm[FIXED_SHM_SIZE]);
}

int RemotePluginClient::getUID()
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginUniqueID);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    return readInt(&m_shm[FIXED_SHM_SIZE]);
}

std::string RemotePluginClient::getName()
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetName);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    return readString(&m_shm[FIXED_SHM_SIZE]);
}

std::string RemotePluginClient::getMaker()
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetMaker);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    return readString(&m_shm[FIXED_SHM_SIZE]);
}

void RemotePluginClient::setBufferSize(int s)
{
    if (s <= 0)
        return;

    if (s == m_bufferSize)
        return;

    if(m_threadinit == 0)
    {
    pthread_create(&m_AMThread, NULL, RemotePluginClient::callAMThread, this);
    m_threadinit = 1;
    }
       
    m_bufferSize = s;
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginSetBufferSize);
    writeIntring(&m_shmControl5->ringBuffer, s);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
}

void RemotePluginClient::setSampleRate(int s)
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginSetSampleRate);
    writeIntring(&m_shmControl5->ringBuffer, s);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
}

void RemotePluginClient::reset()
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginReset);
    if (m_shmSize > 0)
    {
        memset(m_shm, 0, m_shmSize);
        memset(m_shm2, 0, m_shmSize2);
        memset(m_shm3, 0, m_shmSize3);
    }
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
}

void RemotePluginClient::terminate()
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginTerminate);
}

int RemotePluginClient::getEffInt(int opcode)
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetEffInt);
    writeIntring(&m_shmControl5->ringBuffer, opcode);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  

    return readInt(&m_shm[FIXED_SHM_SIZE]);
}

void RemotePluginClient::getEffString(int opcode, int index, char *ptr, int len)
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetEffString);
    writeIntring(&m_shmControl5->ringBuffer, opcode);
    writeIntring(&m_shmControl5->ringBuffer, index);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    strncpy(ptr, readString(&m_shm[FIXED_SHM_SIZE]).c_str(), len);
    ptr[len-1] = 0;
}

int RemotePluginClient::getFlags()
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetFlags);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    return readInt(&m_shm[FIXED_SHM_SIZE]);
}

int RemotePluginClient::getinitialDelay()
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetinitialDelay);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    return readInt(&m_shm[FIXED_SHM_SIZE]);
}

int RemotePluginClient::getInputCount()
{
    // writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetInputCount);
    // m_numInputs = readInt(m_processResponseFd);

    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetInputCount);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    m_numInputs = readInt(&m_shm[FIXED_SHM_SIZE]);

    return m_numInputs;
}

int RemotePluginClient::getOutputCount()
{
    // writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetOutputCount);
    // m_numOutputs = readInt(m_processResponseFd);

    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetOutputCount);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    m_numOutputs = readInt(&m_shm[FIXED_SHM_SIZE]);

    return m_numOutputs;
}

int RemotePluginClient::getParameterCount()
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetParameterCount);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    return readInt(&m_shm[FIXED_SHM_SIZE]);
}

std::string RemotePluginClient::getParameterName(int p)
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetParameterName);
    writeIntring(&m_shmControl5->ringBuffer, p);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    return readString(&m_shm[FIXED_SHM_SIZE]);
}

void
RemotePluginClient::setParameter(int p, float v)
{

    if (m_finishaudio == 1)
        return;

    writeOpcodering(&m_shmControl4->ringBuffer, RemotePluginSetParameter);
    writeIntring(&m_shmControl4->ringBuffer, p);
    writeFloatring(&m_shmControl4->ringBuffer, v);
    commitWrite(&m_shmControl4->ringBuffer);
    waitForServer4();
}

float
RemotePluginClient::getParameter(int p)
{

    if (m_finishaudio == 1)
        return 0;

    writeOpcodering(&m_shmControl4->ringBuffer, RemotePluginGetParameter);
    writeIntring(&m_shmControl4->ringBuffer, p);
    commitWrite(&m_shmControl4->ringBuffer);
    waitForServer4();
    return readFloat(&m_shm[FIXED_SHM_SIZE - 512]);
}

float RemotePluginClient::getParameterDefault(int p)
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetParameterDefault);
    writeIntring(&m_shmControl5->ringBuffer, p);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    return readFloat(&m_shm[FIXED_SHM_SIZE]);
}

void RemotePluginClient::getParameters(int p0, int pn, float *v)
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetParameters);
    writeIntring(&m_shmControl5->ringBuffer, p0);
    writeIntring(&m_shmControl5->ringBuffer, pn);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    tryRead(&m_shm[FIXED_SHM_SIZE], v, (pn - p0 + 1) * sizeof(float));
}

int RemotePluginClient::getProgramCount()
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetProgramCount);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    return readInt(&m_shm[FIXED_SHM_SIZE]);
}

std::string RemotePluginClient::getProgramNameIndexed(int n)
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetProgramNameIndexed);
    writeIntring(&m_shmControl5->ringBuffer, n);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    return readString(&m_shm[FIXED_SHM_SIZE]);
}

std::string RemotePluginClient::getProgramName()
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetProgramName);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    return readString(&m_shm[FIXED_SHM_SIZE]);
}

void RemotePluginClient::setCurrentProgram(int n)
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginSetCurrentProgram);
    writeIntring(&m_shmControl5->ringBuffer, n);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
}

#ifdef SEM

void
RemotePluginClient::waitForServer2()
{
    if(m_386run == 0)
    {
    sem_post(&m_shmControl2->runServer);

    timespec ts_timeout;
    clock_gettime(CLOCK_REALTIME, &ts_timeout);
    ts_timeout.tv_sec += 5;
    if (sem_timedwait(&m_shmControl2->runClient, &ts_timeout) != 0) {
        if(m_inexcept == 0)
	RemotePluginClosedException();
    }
    }
    else
    {
    fpost(&m_shmControl2->runServer386);

    if (fwait(&m_shmControl2->runClient386, 5000)) {
         if(m_inexcept == 0)
	 RemotePluginClosedException();
    }
    }
}

void
RemotePluginClient::waitForServer3()
{
    if(m_386run == 0)
    {
    sem_post(&m_shmControl3->runServer);

    timespec ts_timeout;
    clock_gettime(CLOCK_REALTIME, &ts_timeout);
    ts_timeout.tv_sec += 5;
    if (sem_timedwait(&m_shmControl3->runClient, &ts_timeout) != 0) {
        if(m_inexcept == 0)
	RemotePluginClosedException();
    }
    }
    else
    {
    fpost(&m_shmControl3->runServer386);

    if (fwait(&m_shmControl3->runClient386, 5000)) {
         if(m_inexcept == 0)
	 RemotePluginClosedException();
    }
    }
}

void
RemotePluginClient::waitForServer4()
{
    if(m_386run == 0)
    {
    sem_post(&m_shmControl4->runServer);

    timespec ts_timeout;
    clock_gettime(CLOCK_REALTIME, &ts_timeout);
    ts_timeout.tv_sec += 5;
    if (sem_timedwait(&m_shmControl4->runClient, &ts_timeout) != 0) {
        if(m_inexcept == 0)
	RemotePluginClosedException();
    }
    }
    else
    {
    fpost(&m_shmControl4->runServer386);

    if (fwait(&m_shmControl4->runClient386, 5000)) {
         if(m_inexcept == 0)
	 RemotePluginClosedException();
    }
    }
}

void
RemotePluginClient::waitForServer5()
{
    if(m_386run == 0)
    {
    sem_post(&m_shmControl5->runServer);

    timespec ts_timeout;
    clock_gettime(CLOCK_REALTIME, &ts_timeout);
    ts_timeout.tv_sec += 5;
    if (sem_timedwait(&m_shmControl5->runClient, &ts_timeout) != 0) {
        if(m_inexcept == 0)
	RemotePluginClosedException();
    }
    }
    else
    {
    fpost(&m_shmControl5->runServer386);

    if (fwait(&m_shmControl5->runClient386, 5000)) {
         if(m_inexcept == 0)
	 RemotePluginClosedException();
    }
    }
}

void
RemotePluginClient::waitForServer2exit()
{
    if(m_386run == 0)
    {
    sem_post(&m_shmControl2->runServer);
    }
    else
    {
    fpost(&m_shmControl2->runServer386);
    }
}

void
RemotePluginClient::waitForServer3exit()
{
    if(m_386run == 0)
    {
    sem_post(&m_shmControl3->runServer);
    }
    else
    {
    fpost(&m_shmControl3->runServer386);
    }
}

void
RemotePluginClient::waitForServer4exit()
{
    if(m_386run == 0)
    {
    sem_post(&m_shmControl4->runServer);
    }
    else
    {
    fpost(&m_shmControl4->runServer386);
    }
}

void
RemotePluginClient::waitForServer5exit()
{
    if(m_386run == 0)
    {
    sem_post(&m_shmControl5->runServer);
    }
    else
    {
    fpost(&m_shmControl5->runServer386);
    }
}

void
RemotePluginClient::waitForClientexit()
{
    if(m_386run == 0)
    {
    sem_post(&m_shmControl->runClient);
    }
    else
    {
    fpost(&m_shmControl->runClient386);
    }
}

#else

void
RemotePluginClient::waitForServer2()
{
    fpost(&m_shmControl2->runServer);

    if (fwait(&m_shmControl2->runClient, 5000)) {
         if(m_inexcept == 0)
	 RemotePluginClosedException();
    }
}

void
RemotePluginClient::waitForServer3()
{
    fpost(&m_shmControl3->runServer);

    if (fwait(&m_shmControl3->runClient, 5000)) {
         if(m_inexcept == 0)
	 RemotePluginClosedException();
    }
}

void
RemotePluginClient::waitForServer4()
{
    fpost(&m_shmControl4->runServer);

    if (fwait(&m_shmControl4->runClient, 5000)) {
         if(m_inexcept == 0)
	 RemotePluginClosedException();
    }
}

void
RemotePluginClient::waitForServer5()
{
    fpost(&m_shmControl5->runServer);

    if (fwait(&m_shmControl5->runClient, 5000)) {
         if(m_inexcept == 0)
	 RemotePluginClosedException();
    }
}

void
RemotePluginClient::waitForServer2exit()
{
    fpost(&m_shmControl2->runServer);
}

void
RemotePluginClient::waitForServer3exit()
{
    fpost(&m_shmControl3->runServer);
}

void
RemotePluginClient::waitForServer4exit()
{
    fpost(&m_shmControl4->runServer);
}

void
RemotePluginClient::waitForServer5exit()
{
    fpost(&m_shmControl5->runServer);
}

void
RemotePluginClient::waitForClientexit()
{
    fpost(&m_shmControl->runClient);
}

#endif

void RemotePluginClient::process(float **inputs, float **outputs, int sampleFrames)
{
    if (m_finishaudio == 1)
        return;
    if ((m_bufferSize <= 0) || (sampleFrames <= 0))
    {
        return;
    }
    if (m_numInputs < 0)
    {
        return;
    }
    if (m_numOutputs < 0)
    {
        return;
    }

    if ((m_numInputs + m_numOutputs) * m_bufferSize * sizeof(float) > (FIXED_SHM_SIZE / 2))
        return;

    size_t blocksz = sampleFrames * sizeof(float);

    if(m_numInputs > 0)
    {
    for (int i = 0; i < m_numInputs; ++i)
    memcpy(m_shm + i * blocksz, inputs[i], blocksz);
    }

    writeOpcodering(&m_shmControl2->ringBuffer, RemotePluginProcess);
    writeIntring(&m_shmControl2->ringBuffer, sampleFrames);

    commitWrite(&m_shmControl2->ringBuffer);

    waitForServer2();  

    if(m_numOutputs > 0)
    {
    for (int i = 0; i < m_numOutputs; ++i)
    memcpy(outputs[i], m_shm + i * blocksz, blocksz);
    }
    return;
}

int RemotePluginClient::processVstEvents(VstEvents *evnts)
{
    int ret;
    int eventnum;
    int *ptr;
    int sizeidx = 0;

    if ((evnts->numEvents <= 0) || (!evnts) || (m_finishaudio == 1) || (!m_shm))
        return 0;    

    ptr = (int *)m_shm2;
    eventnum = evnts->numEvents;
    sizeidx = sizeof(int);

    if (eventnum > VSTSIZE)
    eventnum = VSTSIZE;            

    for (int i = 0; i < evnts->numEvents; i++)
    {
        VstEvent* pEvent = evnts->events[i];

        if (pEvent->type == kVstSysExType)
        {
            eventnum--;
        }
        else
        {
            unsigned int size = (2*sizeof(VstInt32)) + evnts->events[i]->byteSize;
            memcpy(&m_shm2[sizeidx], evnts->events[i] , size);
            sizeidx += size;
        }
    }

    *ptr = eventnum;
    ret = evnts->numEvents;

    writeOpcodering(&m_shmControl2->ringBuffer, RemotePluginProcessEvents);
    commitWrite(&m_shmControl2->ringBuffer);

    waitForServer2();  

    return ret;
}

void RemotePluginClient::setDebugLevel(RemotePluginDebugLevel level)
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginSetDebugLevel);
    tryWritering(&m_shmControl5->ringBuffer, &level, sizeof(RemotePluginDebugLevel));
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  

}

bool RemotePluginClient::warn(std::string str)
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginWarn);
    writeStringring(&m_shmControl5->ringBuffer, str);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    bool b;
    tryRead(&m_shm[FIXED_SHM_SIZE], &b, sizeof(bool));
    return b;
}

void RemotePluginClient::showGUI()
{
    writeOpcodering(&m_shmControl3->ringBuffer, RemotePluginShowGUI);
    commitWrite(&m_shmControl3->ringBuffer);
    waitForServer3();  

#ifdef EMBED
    tryRead(&m_shm[FIXED_SHM_SIZE], &winm, sizeof(winm));
#endif
}

void RemotePluginClient::hideGUI()
{
    writeOpcodering(&m_shmControl3->ringBuffer, RemotePluginHideGUI);
    commitWrite(&m_shmControl3->ringBuffer);
    waitForServer3();  
}

#ifdef EMBED
void RemotePluginClient::openGUI()
{
    writeOpcodering(&m_shmControl3->ringBuffer, RemotePluginOpenGUI);    
    commitWrite(&m_shmControl3->ringBuffer);
    waitForServer3();  

}
#endif

void RemotePluginClient::effVoidOp(int opcode)
{
    if (opcode == effClose)
    {
	waitForClientexit();    
        m_threadbreak = 1;
        m_finishaudio = 1;
        writeOpcodering(&m_shmControl3->ringBuffer, RemotePluginDoVoid);
        writeIntring(&m_shmControl3->ringBuffer, opcode);
        commitWrite(&m_shmControl3->ringBuffer);
        waitForServer2exit(); 
        waitForServer3exit(); 
        waitForServer4exit(); 
        waitForServer5exit();  
    }
    else
    {
        writeOpcodering(&m_shmControl3->ringBuffer, RemotePluginDoVoid);
        writeIntring(&m_shmControl3->ringBuffer, opcode);
        commitWrite(&m_shmControl3->ringBuffer);
        waitForServer3();  
    }
}

int RemotePluginClient::getChunk(void **ptr, int bank_prg)
{
    static void *chunk_ptr = 0;
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetChunk);
    writeIntring(&m_shmControl5->ringBuffer, bank_prg);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  

    int sz = readInt(&m_shm[FIXED_SHM_SIZE]);

    if(sz > (FIXED_SHM_SIZE / 2))
    sz = FIXED_SHM_SIZE / 2;

    if (chunk_ptr != 0)
        free(chunk_ptr);
    chunk_ptr = malloc(sz);

    tryRead(&m_shm[FIXED_SHM_SIZE / 2], chunk_ptr, sz);
    *ptr = chunk_ptr;
    return sz;
}

int RemotePluginClient::setChunk(void *ptr, int sz, int bank_prg)
{
    if(sz > (FIXED_SHM_SIZE / 2))
    sz = FIXED_SHM_SIZE / 2;

    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginSetChunk);
    writeIntring(&m_shmControl5->ringBuffer, sz);
    writeIntring(&m_shmControl5->ringBuffer, bank_prg);
    tryWrite(&m_shm[FIXED_SHM_SIZE / 2], ptr, sz);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    return readInt(&m_shm[FIXED_SHM_SIZE]);
}

/*
int RemotePluginClient::canBeAutomated(int param)
{
    writeOpcodering(&m_shmControl5->ringBufferd, RemotePluginCanBeAutomated);
    writeIntring(&m_shmControl5->ringBuffer, param);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    return readIntring(&m_shm[FIXED_SHM_SIZE]);
}
*/

int RemotePluginClient::getProgram()
{
    writeOpcodering(&m_shmControl5->ringBuffer, RemotePluginGetProgram);
    commitWrite(&m_shmControl5->ringBuffer);
    waitForServer5();  
    return readInt(&m_shm[FIXED_SHM_SIZE]);
}

int RemotePluginClient::EffectOpen()
{
    writeOpcodering(&m_shmControl3->ringBuffer, RemotePluginEffectOpen);
    commitWrite(&m_shmControl3->ringBuffer);
    waitForServer3();  
    return 1;
}

void RemotePluginClient::rdwr_tryReadring(RingBuffer *ringbuf, void *buf, size_t count, const char *file, int line)
{
    char *charbuf = static_cast<char *>(buf);
    size_t tail = ringbuf->tail;
    size_t head = ringbuf->head;
    size_t wrap = 0;

   // if(m_runok == 1)
   // return;

    if (head <= tail) {
        wrap = SHM_RING_BUFFER_SIZE;
    }
    if (head - tail + wrap < count) {
        if(m_inexcept == 0)     
        RemotePluginClosedException();
    }
    size_t readto = tail + count;
    if (readto >= SHM_RING_BUFFER_SIZE) {
        readto -= SHM_RING_BUFFER_SIZE;
        size_t firstpart = SHM_RING_BUFFER_SIZE - tail;
        memcpy(charbuf, ringbuf->buf + tail, firstpart);
        memcpy(charbuf + firstpart, ringbuf->buf, readto);
    } else {
        memcpy(charbuf, ringbuf->buf + tail, count);
    }
    ringbuf->tail = readto;
}

void RemotePluginClient::rdwr_tryWritering(RingBuffer *ringbuf, const void *buf, size_t count, const char *file, int line)
{
    const char *charbuf = static_cast<const char *>(buf);
    size_t written = ringbuf->written;
    size_t tail = ringbuf->tail;
    size_t wrap = 0;

   // if(m_runok == 1)
   // return;

    if (tail <= written) {
        wrap = SHM_RING_BUFFER_SIZE;
    }
    if (tail - written + wrap < count) {
        std::cerr << "Operation ring buffer full! Dropping events." << std::endl;
        ringbuf->invalidateCommit = true;
        return;
    }

    size_t writeto = written + count;
    if (writeto >= SHM_RING_BUFFER_SIZE) {
        writeto -= SHM_RING_BUFFER_SIZE;
        size_t firstpart = SHM_RING_BUFFER_SIZE - written;
        memcpy(ringbuf->buf + written, charbuf, firstpart);
        memcpy(ringbuf->buf, charbuf + firstpart, writeto);
    } else {
        memcpy(ringbuf->buf + written, charbuf, count);
    }
    ringbuf->written = writeto;
   }
   
void RemotePluginClient::rdwr_writeOpcodering(RingBuffer *ringbuf, RemotePluginOpcode opcode, const char *file, int line)
{
rdwr_tryWritering(ringbuf, &opcode, sizeof(RemotePluginOpcode), file, line);
}

int RemotePluginClient::rdwr_readIntring(RingBuffer *ringbuf, const char *file, int line)
{
    int i = 0;
    rdwr_tryReadring(ringbuf, &i, sizeof(int), file, line);
    return i;
}

void RemotePluginClient::rdwr_writeIntring(RingBuffer *ringbuf, int i, const char *file, int line)
{
   rdwr_tryWritering(ringbuf, &i, sizeof(int), file, line);
}

void RemotePluginClient::rdwr_writeFloatring(RingBuffer *ringbuf, float f, const char *file, int line)
{
   rdwr_tryWritering(ringbuf, &f, sizeof(float), file, line);
}

float RemotePluginClient::rdwr_readFloatring(RingBuffer *ringbuf, const char *file, int line)
{
    float f = 0;
    rdwr_tryReadring(ringbuf, &f, sizeof(float), file, line);
    return f;
}

void RemotePluginClient::rdwr_writeStringring(RingBuffer *ringbuf, const std::string &str, const char *file, int line)
{
    int len = str.length();
    rdwr_tryWritering(ringbuf, &len, sizeof(int), file, line);
    rdwr_tryWritering(ringbuf, str.c_str(), len, file, line);
}

std::string RemotePluginClient::rdwr_readStringring(RingBuffer *ringbuf, const char *file, int line)
{
    int len;
    static char *buf = 0;
    static int bufLen = 0;
    rdwr_tryReadring(ringbuf, &len, sizeof(int), file, line);
    if (len + 1 > bufLen) {
	delete buf;
	buf = new char[len + 1];
	bufLen = len + 1;
    }
    rdwr_tryReadring(ringbuf, buf, len, file, line);
    buf[len] = '\0';
    return std::string(buf);
}

void RemotePluginClient::rdwr_commitWrite(RingBuffer *ringbuf, const char *file, int line)
{
    if (ringbuf->invalidateCommit) {
        ringbuf->written = ringbuf->head;
        ringbuf->invalidateCommit = false;
    } else {
        ringbuf->head = ringbuf->written;
    }
}

bool RemotePluginClient::dataAvailable(RingBuffer *ringbuf)
{
    return ringbuf->tail != ringbuf->head;
}

void RemotePluginClient::rdwr_tryRead(char *ptr, void *buf, size_t count, const char *file, int line)
{
  memcpy(buf, ptr, count);
}

void RemotePluginClient::rdwr_tryWrite(char *ptr, const void *buf, size_t count, const char *file, int line)
{
  memcpy(ptr, buf, count);
}

void RemotePluginClient::rdwr_writeOpcode(char *ptr, RemotePluginOpcode opcode, const char *file, int line)
{
memcpy(ptr, &opcode, sizeof(RemotePluginOpcode));
}    

void RemotePluginClient::rdwr_writeString(char *ptr, const std::string &str, const char *file, int line)
{
strcpy(ptr, str.c_str());
}

std::string RemotePluginClient::rdwr_readString(char *ptr, const char *file, int line)
{
static char buf[534];
strcpy(buf, ptr);   
return std::string(buf);
}
   
void RemotePluginClient::rdwr_writeInt(char *ptr, int i, const char *file, int line)
{
int *ptr2;
ptr2 = (int *)ptr;
*ptr2  = i;

// memcpy(ptr, &i, sizeof(int));
}

int RemotePluginClient::rdwr_readInt(char *ptr, const char *file, int line)
{
static int i = 0;
int *ptr2;
ptr2 = (int *)ptr;
i = *ptr2;

//    memcpy(&i, ptr, sizeof(int));

    return i;
}

void RemotePluginClient::rdwr_writeFloat(char *ptr, float f, const char *file, int line)
{
float *ptr2;
ptr2 = (float *)ptr;
*ptr2 = f;

// memcpy(ptr, &f, sizeof(float));
}

float RemotePluginClient::rdwr_readFloat(char *ptr, const char *file, int line)
{
static float f = 0;
float *ptr2;
ptr2 = (float *)ptr;
f = *ptr2;

//    memcpy(&f, ptr, sizeof(float));

    return f;
}

void RemotePluginClient::RemotePluginClosedException()
{
m_inexcept = 1;

 //   m_runok = 1;
#ifdef AMT
    m_threadbreak = 1;
 //   m_threadbreakexit = 1;
#endif
    m_finishaudio = 1;

    effVoidOp(effClose);
}

bool RemotePluginClient::fwait(int *futexp, int ms)
       {
       timespec timeval;
       int retval;

       if(ms > 0) {
		timeval.tv_sec  = ms / 1000;
		timeval.tv_nsec = (ms %= 1000) * 1000000;
	}

       for (;;) {                  
          retval = syscall(SYS_futex, futexp, FUTEX_WAIT, 0, &timeval, NULL, 0);
          if (retval == -1 && errno != EAGAIN)
          return true;

          if((*futexp != 0) && (__sync_val_compare_and_swap(futexp, *futexp, *futexp - 1) > 0))
          break;
          }
                               
          return false;          
       }

bool RemotePluginClient::fpost(int *futexp)
       {
       int retval;

	__sync_fetch_and_add(futexp, 1);

        retval = syscall(SYS_futex, futexp, FUTEX_WAKE, 1, NULL, NULL, 0);
/*
        if (retval  == -1)
        return true;
*/  
        return false;          
       }
