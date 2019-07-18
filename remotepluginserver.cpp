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
#include <unistd.h>
#include <string.h>

#include "remotepluginserver.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <fcntl.h>

#include <time.h>
#include <iostream>

RemotePluginServer::RemotePluginServer(std::string fileIdentifiers) :
    m_bufferSize(-1),
    m_numInputs(-1),
    m_numOutputs(-1),
    m_updateio(0),
    m_updatein(0),
    m_updateout(0),
#ifdef CHUNKBUF
    chunkptr(0),
    chunkptr2(0),
#endif
    m_flags(0),
    m_delay(0),
    timeinfo(0),
    m_inexcept(0),
    m_shmFd(-1),
    m_shmFd2(-1),
    m_shmFd3(-1),
    m_shmControlFd(-1),
    m_shmControlFileName(0),
    m_shmControl(0), 
    m_shmControl2Fd(-1),
    m_shmControl2FileName(0),
    m_shmControl2(0), 
    m_shmControl3Fd(-1),
    m_shmControl3FileName(0),
    m_shmControl3(0), 
    m_shmControl4Fd(-1),
    m_shmControl4FileName(0),
    m_shmControl4(0), 
    m_shmControl5Fd(-1),
    m_shmControl5FileName(0),
    m_shmControl5(0), 
    m_shmFileName(0),
    m_shm(0),
    m_shmSize(0),
    m_shmFileName2(0),
    m_shm2(0),
    m_shmSize2(0),
    m_shmFileName3(0),
    m_shm3(0),
    m_shmSize3(0),
    m_inputs(0),
    m_outputs(0),
#ifdef DOUBLEP
    m_inputsdouble(0),
    m_outputsdouble(0),   
#endif
    m_threadsfinish(0),
    m_runok(0),
    m_386run(0),
    starterror(0)
    {
    char tmpFileBase[60];
    int startok;

    sprintf(tmpFileBase, "/vstrplugin_shm_%s", fileIdentifiers.substr(0, 6).c_str());

    m_shmFileName = strdup(tmpFileBase);

    if ((m_shmFd = shm_open(m_shmFileName, O_RDWR, 0)) < 0) 
    {
        starterror = 1;
        cleanup();
        return;
    }

    sprintf(tmpFileBase, "/vstrplugin_shn_%s", fileIdentifiers.substr(6, 6).c_str());

    m_shmFileName2 = strdup(tmpFileBase);

    if ((m_shmFd2 = shm_open(m_shmFileName2, O_RDWR, 0)) < 0) 
    {
        starterror = 1;
        cleanup();
        return;
    }

    sprintf(tmpFileBase, "/vstrplugin_sho_%s", fileIdentifiers.substr(12, 6).c_str());

    m_shmFileName3 = strdup(tmpFileBase);

    if ((m_shmFd3 = shm_open(m_shmFileName3, O_RDWR, 0)) < 0) 
    {
        starterror = 1;
        cleanup();
        return;
    }

    sprintf(tmpFileBase, "/vstrplugin_sha_%s", fileIdentifiers.substr(18, 6).c_str());

    m_shmControlFileName = strdup(tmpFileBase);

    m_shmControlFd = shm_open(m_shmControlFileName, O_RDWR, 0);
    if (m_shmControlFd < 0) {
        starterror = 1;
        cleanup();
        return;
    }

    m_shmControl = (ShmControl *)mmap(0, sizeof(ShmControl), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, m_shmControlFd, 0);
    if (!m_shmControl) {
        starterror = 1;
        cleanup();
        return;
    }

    if(mlock(m_shmControl, sizeof(ShmControl)) != 0)
    perror("mlock fail4");

    sprintf(tmpFileBase, "/vstrplugin_shb_%s", fileIdentifiers.substr(24, 6).c_str());

    m_shmControl2FileName = strdup(tmpFileBase);

    m_shmControl2Fd = shm_open(m_shmControl2FileName, O_RDWR, 0);
    if (m_shmControl2Fd < 0) {
        starterror = 1;
        cleanup();
        return;
    }

    m_shmControl2 = (ShmControl *)mmap(0, sizeof(ShmControl), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, m_shmControl2Fd, 0);
    if (!m_shmControl2) {
        starterror = 1;
        cleanup();
        return;
    }

    if(mlock(m_shmControl2, sizeof(ShmControl)) != 0)
    perror("mlock fail5");

    sprintf(tmpFileBase, "/vstrplugin_shc_%s", fileIdentifiers.substr(30, 6).c_str());

    m_shmControl3FileName = strdup(tmpFileBase);

    m_shmControl3Fd = shm_open(m_shmControl3FileName, O_RDWR, 0);
    if (m_shmControl3Fd < 0) {
        starterror = 1;
        cleanup();
        return;
    }

    m_shmControl3 = (ShmControl *)mmap(0, sizeof(ShmControl), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, m_shmControl3Fd, 0);
    if (!m_shmControl3) {
        starterror = 1;
        cleanup();
        return;
    }

    if(mlock(m_shmControl3, sizeof(ShmControl)) != 0)
    perror("mlock fail5");

    sprintf(tmpFileBase, "/vstrplugin_shd_%s", fileIdentifiers.substr(36, 6).c_str());

    m_shmControl4FileName = strdup(tmpFileBase);

    m_shmControl4Fd = shm_open(m_shmControl4FileName, O_RDWR, 0);
    if (m_shmControl4Fd < 0) {
        starterror = 1;
        cleanup();
        return;
    }

    m_shmControl4 = (ShmControl *)mmap(0, sizeof(ShmControl), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, m_shmControl4Fd, 0);
    if (!m_shmControl4) {
        starterror = 1;
        cleanup();
        return;
    }

    if(mlock(m_shmControl4, sizeof(ShmControl)) != 0)
    perror("mlock fail5");

    sprintf(tmpFileBase, "/vstrplugin_she_%s", fileIdentifiers.substr(42, 6).c_str());

    m_shmControl5FileName = strdup(tmpFileBase);

    m_shmControl5Fd = shm_open(m_shmControl5FileName, O_RDWR, 0);
    if (m_shmControl5Fd < 0) {
        starterror = 1;
        cleanup();
        return;
    }

    m_shmControl5 = (ShmControl *)mmap(0, sizeof(ShmControl), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, m_shmControl5Fd, 0);
    if (!m_shmControl5) {
        starterror = 1;
        cleanup();
        return;
    }

    if(mlock(m_shmControl5, sizeof(ShmControl)) != 0)
    perror("mlock fail5");

    if(sizeShm())
    {
        starterror = 1;
        cleanup();
        return;        
    }

    startok = 0;

    int *ptr;

    ptr = (int *)m_shm;

    for (int i=0;i<4000;i++)
    {
        usleep(10000);
        if ((*ptr == 2) || (*ptr == 3))
         {      
         startok = 1;
            break;
         }
	    
         if (*ptr == 4)
         {      
         startok = 0;
            break;
         }
    }  

    if(startok == 0)
    {
        starterror = 1;
        cleanup();
        return;
    }

   if(*ptr == 3)
   m_386run = 1;
	    
   timeinfo = new VstTimeInfo;
}

RemotePluginServer::~RemotePluginServer()
{
    if(starterror == 0)
    {
    if(m_inputs)
    {
    delete m_inputs;
    m_inputs = 0;
    }

    if(m_outputs)
    {
    delete m_outputs;
    m_outputs = 0;
    }

#ifdef DOUBLEP
    if(m_inputsdouble)
    {
    delete m_inputsdouble;
    m_inputsdouble = 0; 
    }
    
    if(m_outputsdouble)
    {
    delete m_outputsdouble;
    m_outputsdouble = 0;	    
    }
#endif    
	    
    if(timeinfo)
    delete timeinfo;	
	    
    cleanup();	    
    }
}

void RemotePluginServer::cleanup()
{
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
        free(m_shmFileName);
        m_shmFileName = 0;
    }
    if (m_shmFileName2)
    {
        free(m_shmFileName2);
        m_shmFileName2 = 0;
    }
    if (m_shmFileName3)
    {
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
        free(m_shmControl5FileName);
        m_shmControl5FileName = 0;
    }
}

int RemotePluginServer::sizeShm()
{
    if (m_shm)
        return 0;

    int *ptr;

    size_t sz = FIXED_SHM_SIZE + 1024;
    size_t sz2 = FIXED_SHM_SIZE2 + 1024 + (2 * sizeof(float));
    size_t sz3 = FIXED_SHM_SIZE3 + 1024;

    m_shm = (char *)mmap(0, sz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, m_shmFd, 0);
    if (!m_shm)
    {
        std::cerr << "RemotePluginServer::sizeShm: ERROR: mmap or mremap for failed for " << sz
                    << " bytes from fd " << m_shmFd << "!" << std::endl;
        m_shmSize = 0;
        return 1;	    
    }
    else
    {
	    
        madvise(m_shm, sz, MADV_SEQUENTIAL | MADV_WILLNEED | MADV_DONTFORK);memset(m_shm, 0, sz);
        m_shmSize = sz;

        if(mlock(m_shm, sz) != 0)
        perror("mlock fail1");
    }

    m_shm2 = (char *)mmap(0, sz2, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, m_shmFd2, 0);
    if (!m_shm2)
    {
        std::cerr << "RemotePluginServer::sizeShm: ERROR: mmap or mremap for failed for " << sz2
                    << " bytes from fd " << m_shmFd2 << "!" << std::endl;
        m_shmSize2 = 0;
        return 1;	    
    }
    else
    {
	madvise(m_shm2, sz2, MADV_SEQUENTIAL | MADV_WILLNEED | MADV_DONTFORK);    
        memset(m_shm2, 0, sz2);
        m_shmSize2 = sz2;

        if(mlock(m_shm2, sz2) != 0)
        perror("mlock fail1");
    }

    m_shm3 = (char *)mmap(0, sz3, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, m_shmFd3, 0);
    if (!m_shm3)
    {
        std::cerr << "RemotePluginServer::sizeShm: ERROR: mmap or mremap for failed for " << sz3
                    << " bytes from fd " << m_shmFd3 << "!" << std::endl;
        m_shmSize3 = 0;
        return 1;	    
    }
    else
    {
	madvise(m_shm3, sz3, MADV_SEQUENTIAL | MADV_WILLNEED | MADV_DONTFORK);    
        memset(m_shm3, 0, sz3);
        m_shmSize3 = sz3;

        if(mlock(m_shm3, sz3) != 0)
        perror("mlock fail1");
    }

    ptr = (int *)m_shm;

    *ptr = 254;
	
     return 0;	
}

#ifdef SEM

void
RemotePluginServer::waitForClient2exit()
{
    if(m_386run == 0)
    {
    sem_post(&m_shmControl2->runClient);
    }
    else
    {
    fpost(&m_shmControl2->runClient386);
    }
}

void
RemotePluginServer::waitForClient3exit()
{
    if(m_386run == 0)
    {
    sem_post(&m_shmControl3->runClient);
    }
    else
    {
    fpost(&m_shmControl3->runClient386);
    }
}

void
RemotePluginServer::waitForClient4exit()
{
    if(m_386run == 0)
    {
    sem_post(&m_shmControl4->runClient);
    }
    else
    {
    fpost(&m_shmControl4->runClient386);
    }
}

void
RemotePluginServer::waitForClient5exit()
{
    if(m_386run == 0)
    {
    sem_post(&m_shmControl5->runClient);
    }
    else
    {
    fpost(&m_shmControl5->runClient386);
    }
}

#else

void
RemotePluginServer::waitForClient2exit()
{
    fpost(&m_shmControl2->runClient);
}

void
RemotePluginServer::waitForClient3exit()
{
    fpost(&m_shmControl3->runClient);
}

void
RemotePluginServer::waitForClient4exit()
{
    fpost(&m_shmControl4->runClient);
}

void
RemotePluginServer::waitForClient5exit()
{
    fpost(&m_shmControl5->runClient);
}

#endif

void RemotePluginServer::dispatch(int timeout)
{
}

void RemotePluginServer::dispatchProcess(int timeout)
{

#ifdef SEM

    timespec ts_timeout;

    if(m_386run == 0)
    {
    clock_gettime(CLOCK_REALTIME, &ts_timeout);
    time_t seconds = timeout / 1000;
    ts_timeout.tv_sec += seconds;
    ts_timeout.tv_nsec += (timeout - seconds * 1000) * 1000000;
    if (ts_timeout.tv_nsec >= 1000000000) {
        ts_timeout.tv_nsec -= 1000000000;
        ts_timeout.tv_sec++;
    }

    if (sem_timedwait(&m_shmControl2->runServer, &ts_timeout)) {
        if (errno == ETIMEDOUT) {
            return;
        } else {
            if(m_inexcept == 0)
            RemotePluginClosedException();
        }
    }
    }
    else
    {
    if (fwait(&m_shmControl2->runServer386, 50)) {
        if (errno == ETIMEDOUT) {
           return;
        } else {
            if(m_inexcept == 0)
            RemotePluginClosedException();
        }
     }
    }

#else

    if (fwait(&m_shmControl2->runServer, 50)) {
        if (errno == ETIMEDOUT) {
           return;
        } else {
            if(m_inexcept == 0)
            RemotePluginClosedException();
        }
     }

#endif

    while (dataAvailable(&m_shmControl2->ringBuffer)) {
        dispatchProcessEvents();
    }

#ifdef SEM

    if(m_386run == 0)
    {
    if (sem_post(&m_shmControl2->runClient)) {
        std::cerr << "Could not post to semaphore\n";
    }
    }
    else
    {
    if (fpost(&m_shmControl2->runClient386)) {
        std::cerr << "Could not post to semaphore\n";
    }
    }

#else

    if (fpost(&m_shmControl2->runClient)) {
        std::cerr << "Could not post to semaphore\n";
    }

#endif
}

void RemotePluginServer::dispatchProcessEvents()
{
    RemotePluginOpcode opcode = RemotePluginNoOpcode;

    tryReadring(&m_shmControl2->ringBuffer, &opcode, sizeof(RemotePluginOpcode));

    switch (opcode)
    {
    case RemotePluginProcess:
    { 
#ifndef OLDMIDI
    int         els;
    int         *ptr;

    ptr = (int *) m_shm2;
    els = *ptr;
    
    if(els > 0)
    {
    processVstEvents();
    *ptr = 0;
    }
#endif
	    
	int sampleFrames = readIntring(&m_shmControl2->ringBuffer);    
//	if(m_updateio == 1)
	if(sampleFrames == -1) 
        {
            if (m_inputs)
            {
                delete m_inputs;
                m_inputs = 0;
            }
            if (m_updatein > 0)
                m_inputs = new float*[m_updatein];

#ifdef DOUBLEP
            if (m_inputsdouble)
            {
                delete m_inputsdouble;
                m_inputsdouble = 0;
            }
            if (m_updatein > 0)
                m_inputsdouble = new double*[m_updatein];
#endif

            if (m_outputs)
            {
                delete m_outputs;
                m_outputs = 0;
            }
            if (m_updateout > 0)
                m_outputs = new float*[m_updateout];

#ifdef DOUBLEP
            if (m_outputsdouble)
            {
                delete m_outputsdouble;
                m_outputsdouble = 0;
            }
            if (m_updateout > 0)
                m_outputsdouble = new double*[m_updateout];
#endif

        m_numInputs = m_updatein;
        m_numOutputs = m_updateout;

        m_updateio = 0;
	break;
        }

        if (m_bufferSize < 0)
        {
            break;
        }
        if (m_numInputs < 0)
        {
            break;
        }
        if (m_numOutputs < 0)
        {
             break;
        }

        size_t blocksz = sampleFrames * sizeof(float);

	if(m_inputs)
	{
        for (int i = 0; i < m_numInputs; ++i)
        {
            m_inputs[i] = (float *)(m_shm + i * blocksz);
        }
	}
	    
	if(m_outputs)
	{
        for (int i = 0; i < m_numOutputs; ++i)
        {
            m_outputs[i] = (float *)(m_shm + i * blocksz);
        }
	}

        process(m_inputs, m_outputs, sampleFrames);
    }
        break;

#ifdef DOUBLEP
    case RemotePluginProcessDouble:
    {  
#ifndef OLDMIDI
    int         els;
    int         *ptr;

    ptr = (int *) m_shm2;
    els = *ptr;
    
    if(els > 0)
    {
    processVstEvents();
    *ptr = 0;
    }
#endif
	    
	int sampleFrames = readIntring(&m_shmControl2->ringBuffer);    
//	if(m_updateio == 1)
	if(sampleFrames == -1) 
        {
            if (m_inputs)
            {
                delete m_inputs;
                m_inputs = 0;
            }
            if (m_updatein > 0)
                m_inputs = new float*[m_updatein];

#ifdef DOUBLEP
            if (m_inputsdouble)
            {
                delete m_inputsdouble;
                m_inputsdouble = 0;
            }
            if (m_updatein > 0)
                m_inputsdouble = new double*[m_updatein];
#endif

            if (m_outputs)
            {
                delete m_outputs;
                m_outputs = 0;
            }
            if (m_updateout > 0)
                m_outputs = new float*[m_updateout];

#ifdef DOUBLEP
            if (m_outputsdouble)
            {
                delete m_outputsdouble;
                m_outputsdouble = 0;
            }
            if (m_updateout > 0)
                m_outputsdouble = new double*[m_updateout];
#endif

        m_numInputs = m_updatein;
        m_numOutputs = m_updateout;

        m_updateio = 0;
	break;
        }

        if (m_bufferSize < 0)
        {
            break;
        }
        if (m_numInputs < 0)
        {
            break;
        }
        if (m_numOutputs < 0)
        {
             break;
        }

        size_t blocksz = sampleFrames * sizeof(double);

	if(m_inputsdouble) 
	{
        for (int i = 0; i < m_numInputs; ++i)
        {
            m_inputsdouble[i] = (double *)(m_shm + i * blocksz);
        }
	}
	    
	if(m_outputsdouble)
	{
        for (int i = 0; i < m_numOutputs; ++i)
        {
            m_outputsdouble[i] = (double *)(m_shm + i * blocksz);
        }
	}

        processdouble(m_inputsdouble, m_outputsdouble, sampleFrames);
    }
        break;
#endif

    case RemotePluginProcessEvents:
        processVstEvents();
        break;
		    
    case RemotePluginDoVoid:
    {
        int opcode = readIntring(&m_shmControl2->ringBuffer);
        if (opcode == effClose)
	{	
        m_threadsfinish = 1;
	    waitForClient2exit();
        waitForClient3exit();
        waitForClient4exit();
        waitForClient5exit();
	}	
        effDoVoid(opcode);
        break;
    }

    default:
        std::cerr << "WARNING: RemotePluginServer::dispatchProcessEvents: unexpected opcode " << opcode << std::endl;
    }
}

void RemotePluginServer::dispatchGetSet(int timeout)
{

#ifdef SEM

    timespec ts_timeout;

    if(m_386run == 0)
    {
    clock_gettime(CLOCK_REALTIME, &ts_timeout);
    time_t seconds = timeout / 1000;
    ts_timeout.tv_sec += seconds;
    ts_timeout.tv_nsec += (timeout - seconds * 1000) * 1000000;
    if (ts_timeout.tv_nsec >= 1000000000) {
        ts_timeout.tv_nsec -= 1000000000;
        ts_timeout.tv_sec++;
    }

    if (sem_timedwait(&m_shmControl4->runServer, &ts_timeout)) {
        if (errno == ETIMEDOUT) {
            return;
        } else {
            if(m_inexcept == 0)
            RemotePluginClosedException();
        }
    }
    }
    else
    {
    if (fwait(&m_shmControl4->runServer386, 50)) {
        if (errno == ETIMEDOUT) {
           return;
        } else {
            if(m_inexcept == 0)
            RemotePluginClosedException();
        }
     }
    }

#else

    if (fwait(&m_shmControl4->runServer, 50)) {
        if (errno == ETIMEDOUT) {
           return;
        } else {
            if(m_inexcept == 0)
            RemotePluginClosedException();
        }
     }

#endif

    while (dataAvailable(&m_shmControl4->ringBuffer)) {
        dispatchGetSetEvents();
    }

#ifdef SEM

    if(m_386run == 0)
    {
    if (sem_post(&m_shmControl4->runClient)) {
        std::cerr << "Could not post to semaphore\n";
    }
    }
    else
    {
    if (fpost(&m_shmControl4->runClient386)) {
        std::cerr << "Could not post to semaphore\n";
    }
    }

#else

    if (fpost(&m_shmControl4->runClient)) {
        std::cerr << "Could not post to semaphore\n";
    }

#endif
}

void RemotePluginServer::dispatchGetSetEvents()
{
    RemotePluginOpcode opcode = RemotePluginNoOpcode;

    tryReadring(&m_shmControl4->ringBuffer, &opcode, sizeof(RemotePluginOpcode));

    switch (opcode)
    {

    case RemotePluginSetParameter:
    {
        int pn = readIntring(&m_shmControl4->ringBuffer);
        float floatval = readFloatring(&m_shmControl4->ringBuffer);
        setParameter(pn, floatval);
	break;
    }

    case RemotePluginGetParameter:
    {
        int intval = readIntring(&m_shmControl4->ringBuffer);
        float floatval = getParameter(intval);
        writeFloat(&m_shm2[FIXED_SHM_SIZE2 + 1024], floatval);
        break;
    }
		    		    
    default:
        std::cerr << "WARNING: RemotePluginServer::dispatchGetSetEvents: unexpected opcode " << opcode << std::endl;
    }
}

void RemotePluginServer::dispatchControl(int timeout)
{

#ifdef SEM

    timespec ts_timeout;

    if(m_386run == 0)
    {
    clock_gettime(CLOCK_REALTIME, &ts_timeout);
    time_t seconds = timeout / 1000;
    ts_timeout.tv_sec += seconds;
    ts_timeout.tv_nsec += (timeout - seconds * 1000) * 1000000;
    if (ts_timeout.tv_nsec >= 1000000000) {
        ts_timeout.tv_nsec -= 1000000000;
        ts_timeout.tv_sec++;
    }

    if (sem_timedwait(&m_shmControl3->runServer, &ts_timeout)) {
        if (errno == ETIMEDOUT) {
            return;
        } else {
            if(m_inexcept == 0)
            RemotePluginClosedException();
        }
    }
    }
    else
    {
    if (fwait(&m_shmControl3->runServer386, 50)) {
        if (errno == ETIMEDOUT) {
           return;
        } else {
            if(m_inexcept == 0)
            RemotePluginClosedException();
        }
     }  
    }

#else

    if (fwait(&m_shmControl3->runServer, 50)) {
        if (errno == ETIMEDOUT) {
           return;
        } else {
            if(m_inexcept == 0)
            RemotePluginClosedException();
        }
     }

#endif

    while (dataAvailable(&m_shmControl3->ringBuffer)) {
        dispatchControlEvents();
    }

#ifdef SEM

    if(m_386run == 0)
    {
    if (sem_post(&m_shmControl3->runClient)) {
        std::cerr << "Could not post to semaphore\n";
    }
    }
    else
    {
    if (fpost(&m_shmControl3->runClient386)) {
        std::cerr << "Could not post to semaphore\n";
    }
    }

#else

    if (fpost(&m_shmControl3->runClient)) {
        std::cerr << "Could not post to semaphore\n";
    }

#endif
}

void RemotePluginServer::dispatchPar(int timeout)
{

#ifdef SEM

    timespec ts_timeout;

    if(m_386run == 0)
    {
    clock_gettime(CLOCK_REALTIME, &ts_timeout);
    time_t seconds = timeout / 1000;
    ts_timeout.tv_sec += seconds;
    ts_timeout.tv_nsec += (timeout - seconds * 1000) * 1000000;
    if (ts_timeout.tv_nsec >= 1000000000) {
        ts_timeout.tv_nsec -= 1000000000;
        ts_timeout.tv_sec++;
    }

    if (sem_timedwait(&m_shmControl5->runServer, &ts_timeout)) {
        if (errno == ETIMEDOUT) {
            return;
        } else {
            if(m_inexcept == 0)
            RemotePluginClosedException();
        }
    }
    }
    else
    {
    if (fwait(&m_shmControl5->runServer386, 50)) {
        if (errno == ETIMEDOUT) {
           return;
        } else {
            if(m_inexcept == 0)
            RemotePluginClosedException();
        }
     }
    }

#else

    if (fwait(&m_shmControl5->runServer, 50)) {
        if (errno == ETIMEDOUT) {
           return;
        } else {
            if(m_inexcept == 0)
            RemotePluginClosedException();
        }
     }

#endif

    while (dataAvailable(&m_shmControl5->ringBuffer)) {
        dispatchParEvents();
    }

#ifdef SEM

    if(m_386run == 0)
    {
    if (sem_post(&m_shmControl5->runClient)) {
        std::cerr << "Could not post to semaphore\n";
    }
    }
    else
    {
    if (fpost(&m_shmControl5->runClient386)) {
        std::cerr << "Could not post to semaphore\n";
    }
    }

#else

    if (fpost(&m_shmControl5->runClient)) {
        std::cerr << "Could not post to semaphore\n";
    }

#endif
}

void RemotePluginServer::dispatchParEvents()
{
    RemotePluginOpcode opcode = RemotePluginNoOpcode;
    float        *parameterBuffer = 0;

    tryReadring(&m_shmControl5->ringBuffer, &opcode, sizeof(RemotePluginOpcode));

    switch (opcode)
    {
		    
    case RemotePluginDoVoid2:
    {
        int opcode = readIntring(&m_shmControl5->ringBuffer);
        int index = readIntring(&m_shmControl5->ringBuffer);
        int value = readIntring(&m_shmControl5->ringBuffer);
        float opt = readFloatring(&m_shmControl5->ringBuffer);
        int b = effDoVoid2(opcode, index, value, opt);
        tryWrite(&m_shm[FIXED_SHM_SIZE], &b, sizeof(int));
        break;
    }		          		    

    case RemotePluginSetCurrentProgram:
        setCurrentProgram(readIntring(&m_shmControl5->ringBuffer));
        break;

    case RemotePluginSetBufferSize:
    {
        int newSize = readIntring(&m_shmControl5->ringBuffer);
        setBufferSize(newSize);
        m_bufferSize = newSize;
        break;
    }

    case RemotePluginSetSampleRate:
        setSampleRate(readIntring(&m_shmControl5->ringBuffer));
        break;

    case RemotePluginReset:
        reset();
        break;

    case RemotePluginGetVersion:
        writeFloat(&m_shm[FIXED_SHM_SIZE], getVersion());
        break;

    case RemotePluginUniqueID:
        writeInt(&m_shm[FIXED_SHM_SIZE], getUID());
        break;

    case RemotePluginGetName:
        writeString(&m_shm[FIXED_SHM_SIZE], getName());
        break;

    case RemotePluginGetMaker:
        writeString(&m_shm[FIXED_SHM_SIZE], getMaker());
        break;

    case RemotePluginTerminate:
        terminate();
        break;

    case RemotePluginGetFlags:
        m_flags = getFlags();
        writeInt(&m_shm[FIXED_SHM_SIZE], m_flags);
        break;

    case RemotePluginGetinitialDelay:
        m_delay = getinitialDelay();
        writeInt(&m_shm[FIXED_SHM_SIZE], m_delay);
        break;

    case RemotePluginGetParameterCount:
        writeInt(&m_shm[FIXED_SHM_SIZE], getParameterCount());
        break;

    case RemotePluginGetParameterName:
        writeString(&m_shm[FIXED_SHM_SIZE], getParameterName(readIntring(&m_shmControl5->ringBuffer)));
        break;
		    
#ifdef WAVES
    case RemotePluginGetShellName:
     {
       char name[512];
       int retvalshell = getShellName(name);
       writeInt(&m_shm[FIXED_SHM_SIZE + 512], retvalshell);
       writeString(&m_shm[FIXED_SHM_SIZE], name);
     }
        break;
#endif

    case RemotePluginGetParameterDefault:
        writeFloat(&m_shm[FIXED_SHM_SIZE], getParameterDefault(readIntring(&m_shmControl5->ringBuffer)));
        break;

    case RemotePluginGetParameters:
    {
        if (!parameterBuffer)
            parameterBuffer = new float[getParameterCount()];
        int p0 = readIntring(&m_shmControl5->ringBuffer);
        int pn = readIntring(&m_shmControl5->ringBuffer);
        getParameters(p0, pn, parameterBuffer);
        tryWrite(&m_shm[FIXED_SHM_SIZE], parameterBuffer, (pn - p0 + 1) * sizeof(float));
        break;
    }

    case RemotePluginGetProgramCount:
        writeInt(&m_shm[FIXED_SHM_SIZE], getProgramCount());
        break;

    case RemotePluginGetProgramNameIndexed:
    {
        char name[512];
        int retvalprogramname = getProgramNameIndexed(readIntring(&m_shmControl5->ringBuffer), name);
        writeInt(&m_shm[FIXED_SHM_SIZE + 512], retvalprogramname);
        writeString(&m_shm[FIXED_SHM_SIZE], name);
        break;
    }

    case RemotePluginGetProgramName:
        writeString(&m_shm[FIXED_SHM_SIZE], getProgramName());
        break;

    case RemotePluginSetDebugLevel:
    {
        RemotePluginDebugLevel newLevel = m_debugLevel;
        tryReadring(&m_shmControl5->ringBuffer, &newLevel, sizeof(RemotePluginDebugLevel));
        setDebugLevel(newLevel);
        m_debugLevel = newLevel;
        break;
    }

    case RemotePluginWarn:
    {
        bool b = warn(readStringring(&m_shmControl5->ringBuffer));
        tryWrite(&m_shm[FIXED_SHM_SIZE], &b, sizeof(bool));
        break;
    }

    case RemotePluginGetEffInt:
    {
        int opcode = readIntring(&m_shmControl5->ringBuffer);
        writeInt(&m_shm[FIXED_SHM_SIZE], getEffInt(opcode));
        break;
    }

    case RemotePluginGetEffString:
    {
        int opcode = readIntring(&m_shmControl5->ringBuffer);
        int idx = readIntring(&m_shmControl5->ringBuffer);
        writeString(&m_shm[FIXED_SHM_SIZE], getEffString(opcode, idx));
        break;
    }

/*
    case RemoteMainsChanged:
    {
        int v = readIntring(&m_shmControl5->ringBuffer);
        // std::cerr << "Mains changing " << v << std::endl;
        eff_mainsChanged(v);
        break;
    }
*/

    case RemotePluginGetChunk:
        getChunk();
        break;

    case RemotePluginSetChunk:
        setChunk();
        break;

    case RemotePluginCanBeAutomated:
        canBeAutomated();
        break;

    case RemotePluginGetProgram:
        getProgram();
        break;

    case RemotePluginGetInputCount:
    {
        int numin = getInputCount();
        // m_numInputs = getInputCount();

        if (numin != m_numInputs)
        {
            if (m_inputs)
            {
                delete m_inputs;
                m_inputs = 0;
            }
            if (numin > 0)
                m_inputs = new float*[numin];

#ifdef DOUBLEP
            if (m_inputsdouble)
            {
                delete m_inputsdouble;
                m_inputsdouble = 0;
            }
            if (numin > 0)
                m_inputsdouble = new double*[numin];
#endif
        }
        m_numInputs = numin;
        writeInt(&m_shm[FIXED_SHM_SIZE], m_numInputs);
        break;
    }

    case RemotePluginGetOutputCount:
    {
        int numout = getOutputCount();
        // m_numOutputs = getOutputCount();

        if (numout != m_numOutputs)
        {
            if (m_outputs)
            {
                delete m_outputs;
                m_outputs = 0;
            }
            if (numout > 0)
                m_outputs = new float*[numout];

#ifdef DOUBLEP
            if (m_outputsdouble)
            {
                delete m_outputsdouble;
                m_outputsdouble = 0;
            }
            if (numout > 0)
                m_outputsdouble = new double*[numout];
#endif
        }
        m_numOutputs = numout;
        writeInt(&m_shm[FIXED_SHM_SIZE], m_numOutputs);
        break;
    }
		    		    		   		    
#ifdef DOUBLEP
     case RemoteSetPrecision:
    {   
        int value = readIntring(&m_shmControl5->ringBuffer);
        bool b = setPrecision(value);
        tryWrite(&m_shm[FIXED_SHM_SIZE], &b, sizeof(bool));
        break;
    }  
#endif

#ifdef MIDIEFF
     case RemoteInProp:
    {   
        int index = readIntring(&m_shmControl5->ringBuffer);
        bool b = getInProp(index);
        tryWrite(&m_shm2[FIXED_SHM_SIZE2], &b, sizeof(bool));
        break;
    }
    
     case RemoteOutProp:
    {   
        int index = readIntring(&m_shmControl5->ringBuffer);
        bool b = getOutProp(index);
        tryWrite(&m_shm2[FIXED_SHM_SIZE2], &b, sizeof(bool));
        break;
    }
    
     case RemoteMidiKey:
    {   
        int index = readIntring(&m_shmControl5->ringBuffer);
        bool b = getMidiKey(index);
        tryWrite(&m_shm2[FIXED_SHM_SIZE2], &b, sizeof(bool));
        break;
    }  
    
      case RemoteMidiProgName:
    {   
        int index = readIntring(&m_shmControl5->ringBuffer);
        bool b = getMidiProgName(index);
        tryWrite(&m_shm2[FIXED_SHM_SIZE2], &b, sizeof(bool));
        break;
    }  
   
     case RemoteMidiCurProg:
    {   
        int index = readIntring(&m_shmControl5->ringBuffer);
        bool b = getMidiCurProg(index);
        tryWrite(&m_shm2[FIXED_SHM_SIZE2], &b, sizeof(bool));
        break;
    }  

     case RemoteMidiProgCat:
    {   
        int index = readIntring(&m_shmControl5->ringBuffer);
        bool b = getMidiProgCat(index);
        tryWrite(&m_shm2[FIXED_SHM_SIZE2], &b, sizeof(bool));
        break;
    }  

     case RemoteMidiProgCh:
    {   
        int index = readIntring(&m_shmControl5->ringBuffer);
        bool b = getMidiProgCh(index);
        tryWrite(&m_shm2[FIXED_SHM_SIZE2], &b, sizeof(bool));
        break;
    }  

     case RemoteSetSpeaker:
    {   
        bool b = setSpeaker();
        tryWrite(&m_shm2[FIXED_SHM_SIZE2], &b, sizeof(bool));
        break;
    }  
    
     case RemoteGetSpeaker:
    {   
        bool b = getSpeaker();
        tryWrite(&m_shm2[FIXED_SHM_SIZE2], &b, sizeof(bool));
        break;
    }  
#endif
		    
#ifdef CANDOEFF        
     case RemotePluginEffCanDo:
    {
        bool b =  getEffCanDo(readStringring(&m_shmControl5->ringBuffer));
        tryWrite(&m_shm[FIXED_SHM_SIZE], &b, sizeof(bool));
        break;
    }
#endif  
		    
#ifdef CHUNKBUF
     case RemotePluginGetBuf:
    {      
        char *chunkbuf = (char *)chunkptr;
        int curchunk = readIntring(&m_shmControl5->ringBuffer);
        int idx = readIntring(&m_shmControl5->ringBuffer);
        tryWrite(&m_shm[FIXED_SHM_SIZECHUNKSTART], &chunkbuf[idx], curchunk);
        break;
    }  
		    
     case RemotePluginSetBuf:
    {     
        int curchunk = readIntring(&m_shmControl5->ringBuffer);
        int idx = readIntring(&m_shmControl5->ringBuffer);
        int sz2 = readIntring(&m_shmControl5->ringBuffer);

        if(sz2 > 0)
        chunkptr2 = (char *)malloc(sz2);

        if(!chunkptr2)
        break;

        tryRead(&m_shm[FIXED_SHM_SIZECHUNKSTART], &chunkptr2[idx], curchunk);
        break;
    }  		    
#endif		    
		    
    default:
        std::cerr << "WARNING: RemotePluginServer::dispatchParEvents: unexpected opcode " << opcode << std::endl;
    }
}

void RemotePluginServer::dispatchControlEvents()
{
    RemotePluginOpcode opcode = RemotePluginNoOpcode;

    tryReadring(&m_shmControl3->ringBuffer, &opcode, sizeof(RemotePluginOpcode));

    switch (opcode)
    {

    case RemotePluginShowGUI:
        showGUI();
        break;
		    
   case RemotePluginHideGUI:
        hideGUI();
        break;		    
		    
#ifdef EMBED
    case RemotePluginOpenGUI:
        openGUI();
        break;
#endif
		    
    case RemotePluginEffectOpen:
        EffectOpen();
        break;
		    		    
    default:
        std::cerr << "WARNING: RemotePluginServer::dispatchControlEvents: unexpected opcode " << opcode << std::endl;
    }
}

void RemotePluginServer::rdwr_tryReadring(RingBuffer *ringbuf, void *buf, size_t count, const char *file, int line)
{
    char *charbuf = (char *)buf;
    size_t tail = ringbuf->tail;
    size_t head = ringbuf->head;
    size_t wrap = 0;

    if(m_runok == 1)
    return;

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

void RemotePluginServer::rdwr_tryWritering(RingBuffer *ringbuf, const void *buf, size_t count, const char *file, int line)
{
    const char *charbuf = (const char *)buf;
    size_t written = ringbuf->written;
    size_t tail = ringbuf->tail;
    size_t wrap = 0;

    if(m_runok == 1)
    return;

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
   
void RemotePluginServer::rdwr_writeOpcodering(RingBuffer *ringbuf, RemotePluginOpcode opcode, const char *file, int line)
{
rdwr_tryWritering(ringbuf, &opcode, sizeof(RemotePluginOpcode), file, line);
}

int RemotePluginServer::rdwr_readIntring(RingBuffer *ringbuf, const char *file, int line)
{
    int i = 0;
    rdwr_tryReadring(ringbuf, &i, sizeof(int), file, line);
    return i;
}

void RemotePluginServer::rdwr_writeIntring(RingBuffer *ringbuf, int i, const char *file, int line)
{
   rdwr_tryWritering(ringbuf, &i, sizeof(int), file, line);
}

void RemotePluginServer::rdwr_writeFloatring(RingBuffer *ringbuf, float f, const char *file, int line)
{
   rdwr_tryWritering(ringbuf, &f, sizeof(float), file, line);
}

float RemotePluginServer::rdwr_readFloatring(RingBuffer *ringbuf, const char *file, int line)
{
    float f = 0;
    rdwr_tryReadring(ringbuf, &f, sizeof(float), file, line);
    return f;
}

void RemotePluginServer::rdwr_writeStringring(RingBuffer *ringbuf, const std::string &str, const char *file, int line)
{
    int len = str.length();
    rdwr_tryWritering(ringbuf, &len, sizeof(int), file, line);
    rdwr_tryWritering(ringbuf, str.c_str(), len, file, line);
}

std::string RemotePluginServer::rdwr_readStringring(RingBuffer *ringbuf, const char *file, int line)
{
    int len;
    char *buf = 0;
    int bufLen = 0;
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

void RemotePluginServer::rdwr_commitWrite(RingBuffer *ringbuf, const char *file, int line)
{
    if (ringbuf->invalidateCommit) {
        ringbuf->written = ringbuf->head;
        ringbuf->invalidateCommit = false;
    } else {
        ringbuf->head = ringbuf->written;
    }
}

bool RemotePluginServer::dataAvailable(RingBuffer *ringbuf)
{
    return ringbuf->tail != ringbuf->head;
}

void RemotePluginServer::rdwr_tryRead(char *ptr, void *buf, size_t count, const char *file, int line)
{
  memcpy(buf, ptr, count);
}

void RemotePluginServer::rdwr_tryWrite(char *ptr, const void *buf, size_t count, const char *file, int line)
{
  memcpy(ptr, buf, count);
}

void RemotePluginServer::rdwr_writeOpcode(char *ptr, RemotePluginOpcode opcode, const char *file, int line)
{
memcpy(ptr, &opcode, sizeof(RemotePluginOpcode));
}    

void RemotePluginServer::rdwr_writeString(char *ptr, const std::string &str, const char *file, int line)
{
strcpy(ptr, str.c_str());
}

std::string RemotePluginServer::rdwr_readString(char *ptr, const char *file, int line)
{
char buf[534];
strcpy(buf, ptr);   
return std::string(buf);
}
   
void RemotePluginServer::rdwr_writeInt(char *ptr, int i, const char *file, int line)
{
int *ptr2;
ptr2 = (int *)ptr;
*ptr2  = i;

// memcpy(ptr, &i, sizeof(int));
}

int RemotePluginServer::rdwr_readInt(char *ptr, const char *file, int line)
{
int i = 0;
int *ptr2;
ptr2 = (int *)ptr;
i = *ptr2;

//    memcpy(&i, ptr, sizeof(int));

    return i;
}

void RemotePluginServer::rdwr_writeFloat(char *ptr, float f, const char *file, int line)
{
float *ptr2;
ptr2 = (float *)ptr;
*ptr2 = f;

// memcpy(ptr, &f, sizeof(float));
}

float RemotePluginServer::rdwr_readFloat(char *ptr, const char *file, int line)
{
float f = 0;
float *ptr2;
ptr2 = (float *)ptr;
f = *ptr2;

//    memcpy(&f, ptr, sizeof(float));

    return f;
}

#define disconnectserver 32143215

#ifdef SEM

void
RemotePluginServer::waitForServerexcept()
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

#else

void RemotePluginServer::waitForServerexcept()
{
    fpost(&m_shmControl->runServer);

    if (fwait(&m_shmControl->runClient, 60000)) {
         if(m_inexcept == 0)
	 RemotePluginClosedException();
    }
}
#endif

void RemotePluginServer::RemotePluginClosedException()
{
m_inexcept = 1;

m_runok = 1;

    writeOpcodering(&m_shmControl->ringBuffer, (RemotePluginOpcode)disconnectserver);
    commitWrite(&m_shmControl->ringBuffer);
    waitForServerexcept();  
    waitForClient2exit();
    waitForClient3exit();
    waitForClient4exit();
    waitForClient5exit();
    
    sleep(5);
    
terminate();
}

bool RemotePluginServer::fwait(int *futexp, int ms)
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

bool RemotePluginServer::fpost(int *futexp)
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
