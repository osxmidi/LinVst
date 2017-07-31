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
    m_controlRequestFd(-1),
    m_controlResponseFd(-1),
    m_parRequestFd(-1),
    m_parResponseFd(-1),
#ifndef RINGB
    m_processFd(-1),
    m_processResponseFd(-1),
#endif
    m_shmFd(-1),
    m_shmFd2(-1),
    m_shmFd3(-1),
#ifdef RINGB
    m_shmControlFd(-1),
    m_shmControlFileName(0),
    m_shmControl(0), 
    m_shmControl2Fd(-1),
    m_shmControl2FileName(0),
    m_shmControl2(0), 
#endif
    m_controlRequestFileName(0),
    m_controlResponseFileName(0),
    m_parRequestFileName(0),
    m_parResponseFileName(0),
#ifndef RINGB
    m_processFileName(0),
    m_processResponseFileName(0),
#endif
#ifdef AMT
    m_AMRequestFd(-1),
    m_AMResponseFd(-1),
    m_AMRequestFileName(0),
    m_AMResponseFileName(0),
#endif

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
    m_threadsfinish(0),
    m_runok(0),
    starterror(0)
    {
    char tmpFileBase[60];

    bool b = false;

    sprintf(tmpFileBase, "/tmp/rplugin_crq_%s", fileIdentifiers.substr(0, 6).c_str());
    m_controlRequestFileName = strdup(tmpFileBase);
    if ((m_controlRequestFd = open(m_controlRequestFileName, O_RDWR)) < 0)
    {
        starterror = 1;
        cleanup();
        return;
    }

    sprintf(tmpFileBase, "/tmp/rplugin_crs_%s", fileIdentifiers.substr(6, 6).c_str());
    m_controlResponseFileName = strdup(tmpFileBase);
    if ((m_controlResponseFd = open(m_controlResponseFileName, O_WRONLY)) < 0)
    {
        starterror = 1;
        cleanup();
        return;
    }

    sprintf(tmpFileBase, "/tmp/rplugin_gpa_%s", fileIdentifiers.substr(12, 6).c_str());
    m_parRequestFileName = strdup(tmpFileBase);
    if ((m_parRequestFd = open(m_parRequestFileName, O_RDWR)) < 0)
    {
        tryWrite(m_controlResponseFd, &b, sizeof(bool));
        starterror = 1;
        cleanup();
        return;
    }

    sprintf(tmpFileBase, "/tmp/rplugin_spa_%s", fileIdentifiers.substr(18, 6).c_str());
    m_parResponseFileName = strdup(tmpFileBase);
    if ((m_parResponseFd = open(m_parResponseFileName, O_WRONLY)) < 0)
    {
        tryWrite(m_controlResponseFd, &b, sizeof(bool));
        starterror = 1;
        cleanup();
        return;
    }

#ifndef RINGB

    sprintf(tmpFileBase, "/tmp/rplugin_prc_%s", fileIdentifiers.substr(24, 6).c_str());
    m_processFileName = strdup(tmpFileBase);
    if ((m_processFd = open(m_processFileName, O_RDONLY)) < 0)
    {
        tryWrite(m_controlResponseFd, &b, sizeof(bool));
        starterror = 1;
        cleanup();
        return;
    }

    sprintf(tmpFileBase, "/tmp/rplugin_prr_%s", fileIdentifiers.substr(30, 6).c_str());
    m_processResponseFileName = strdup(tmpFileBase);
    if ((m_processResponseFd = open(m_processResponseFileName, O_WRONLY)) < 0)
    {
        tryWrite(m_controlResponseFd, &b, sizeof(bool));
        starterror = 1;
        cleanup();
        return;
    }

#ifdef AMT
    sprintf(tmpFileBase, "/tmp/rplugin_cal_%s", fileIdentifiers.substr(36, 6).c_str());
    m_AMRequestFileName = strdup(tmpFileBase);
    if ((m_AMRequestFd = open(m_AMRequestFileName, O_RDWR)) < 0)
    {
        tryWrite(m_controlResponseFd, &b, sizeof(bool));
        starterror = 1;
        cleanup();
        return;
    }

    sprintf(tmpFileBase, "/tmp/rplugin_cap_%s", fileIdentifiers.substr(42, 6).c_str());
    m_AMResponseFileName = strdup(tmpFileBase);
    if ((m_AMResponseFd = open(m_AMResponseFileName, O_WRONLY)) < 0)
    {
        tryWrite(m_controlResponseFd, &b, sizeof(bool));
        starterror = 1;
        cleanup();
        return;
    }
#endif

#ifdef AMT
    sprintf(tmpFileBase, "/vstrplugin_shm_%s", fileIdentifiers.substr(48, 6).c_str());
#else
    sprintf(tmpFileBase, "/vstrplugin_shm_%s", fileIdentifiers.substr(36, 6).c_str());
#endif
    m_shmFileName = strdup(tmpFileBase);

    if ((m_shmFd = shm_open(m_shmFileName, O_RDWR, 0)) < 0) 
    {
        tryWrite(m_controlResponseFd, &b, sizeof(bool));
        starterror = 1;
        cleanup();
        return;
    }

#ifdef AMT
    sprintf(tmpFileBase, "/vstrplugin_shn_%s", fileIdentifiers.substr(54, 6).c_str());
#else
    sprintf(tmpFileBase, "/vstrplugin_shn_%s", fileIdentifiers.substr(42, 6).c_str());
#endif
    m_shmFileName2 = strdup(tmpFileBase);

    if ((m_shmFd2 = shm_open(m_shmFileName2, O_RDWR, 0)) < 0) 
    {
        tryWrite(m_controlResponseFd, &b, sizeof(bool));
        starterror = 1;
        cleanup();
        return;
    }

#ifdef AMT
    sprintf(tmpFileBase, "/vstrplugin_sho_%s", fileIdentifiers.substr(60, 6).c_str());
#else
    sprintf(tmpFileBase, "/vstrplugin_sho_%s", fileIdentifiers.substr(48, 6).c_str());
#endif
    m_shmFileName3 = strdup(tmpFileBase);

    if ((m_shmFd3 = shm_open(m_shmFileName3, O_RDWR, 0)) < 0) 
    {
        tryWrite(m_controlResponseFd, &b, sizeof(bool));
        starterror = 1;
        cleanup();
        return;
    }

#else

#ifdef AMT
    sprintf(tmpFileBase, "/tmp/rplugin_cal_%s", fileIdentifiers.substr(24, 6).c_str());
    m_AMRequestFileName = strdup(tmpFileBase);
    if ((m_AMRequestFd = open(m_AMRequestFileName, O_RDWR)) < 0)
    {
        tryWrite(m_controlResponseFd, &b, sizeof(bool));
        starterror = 1;
        cleanup();
        return;
    }

    sprintf(tmpFileBase, "/tmp/rplugin_cap_%s", fileIdentifiers.substr(30, 6).c_str());
    m_AMResponseFileName = strdup(tmpFileBase);
    if ((m_AMResponseFd = open(m_AMResponseFileName, O_WRONLY)) < 0)
    {
        tryWrite(m_controlResponseFd, &b, sizeof(bool));
        starterror = 1;
        cleanup();
        return;
    }
#endif

#ifdef AMT
    sprintf(tmpFileBase, "/vstrplugin_shm_%s", fileIdentifiers.substr(36, 6).c_str());
#else
    sprintf(tmpFileBase, "/vstrplugin_shm_%s", fileIdentifiers.substr(24, 6).c_str());
#endif
    m_shmFileName = strdup(tmpFileBase);

    if ((m_shmFd = shm_open(m_shmFileName, O_RDWR, 0)) < 0) 
    {
        tryWrite(m_controlResponseFd, &b, sizeof(bool));
        starterror = 1;
        cleanup();
        return;
    }

#ifdef AMT
    sprintf(tmpFileBase, "/vstrplugin_shn_%s", fileIdentifiers.substr(42, 6).c_str());
#else
    sprintf(tmpFileBase, "/vstrplugin_shn_%s", fileIdentifiers.substr(30, 6).c_str());
#endif
    m_shmFileName2 = strdup(tmpFileBase);

    if ((m_shmFd2 = shm_open(m_shmFileName2, O_RDWR, 0)) < 0) 
    {
        tryWrite(m_controlResponseFd, &b, sizeof(bool));
        starterror = 1;
        cleanup();
        return;
    }

#ifdef AMT
    sprintf(tmpFileBase, "/vstrplugin_sho_%s", fileIdentifiers.substr(48, 6).c_str());
#else
    sprintf(tmpFileBase, "/vstrplugin_sho_%s", fileIdentifiers.substr(36, 6).c_str());
#endif
    m_shmFileName3 = strdup(tmpFileBase);

    if ((m_shmFd3 = shm_open(m_shmFileName3, O_RDWR, 0)) < 0) 
    {
        tryWrite(m_controlResponseFd, &b, sizeof(bool));
        starterror = 1;
        cleanup();
        return;
    }

#ifdef AMT
    sprintf(tmpFileBase, "/vstrplugin_shc_%s", fileIdentifiers.substr(54, 6).c_str());
#else
    sprintf(tmpFileBase, "/vstrplugin_shc_%s", fileIdentifiers.substr(42, 6).c_str());
#endif

    m_shmControlFileName = strdup(tmpFileBase);

    m_shmControlFd = shm_open(m_shmControlFileName, O_RDWR, 0);
    if (m_shmControlFd < 0) {
        tryWrite(m_controlResponseFd, &b, sizeof(bool));
        starterror = 1;
        cleanup();
        return;
    }

#ifdef AMT
    sprintf(tmpFileBase, "/vstrplugin_shz_%s", fileIdentifiers.substr(60, 6).c_str());
#else
    sprintf(tmpFileBase, "/vstrplugin_shz_%s", fileIdentifiers.substr(48, 6).c_str());
#endif

    m_shmControl2FileName = strdup(tmpFileBase);

    m_shmControl2Fd = shm_open(m_shmControl2FileName, O_RDWR, 0);
    if (m_shmControl2Fd < 0) {
        tryWrite(m_controlResponseFd, &b, sizeof(bool));
        starterror = 1;
        cleanup();
        return;
    }

#endif

    b = true;
    tryWrite(m_controlResponseFd, &b, sizeof(bool));
}

RemotePluginServer::~RemotePluginServer()
{
    cleanup();
}

void RemotePluginServer::cleanup()
{
    if (m_controlRequestFd >= 0)
    {
        close(m_controlRequestFd);
        m_controlRequestFd = -1;
    }
    if (m_controlResponseFd >= 0)
    {
        close(m_controlResponseFd);
        m_controlResponseFd = -1;
    }
    if (m_parRequestFd >= 0)
    {
        close(m_parRequestFd);
        m_parRequestFd = -1;
    }
    if (m_parResponseFd >= 0)
    {
        close(m_parResponseFd);
        m_parResponseFd = -1;
    }
#ifndef RINGB
    if (m_processFd >= 0)
    {
        close(m_processFd);
        m_processFd = -1;
    }
    if (m_processResponseFd >= 0)
    {
        close(m_processResponseFd);
        m_processResponseFd = -1;
    }
#endif
#ifdef AMT
    if (m_AMRequestFd >= 0)
    {
        close(m_AMRequestFd);
        m_AMRequestFd = -1;
    }
    if (m_AMResponseFd >= 0)
    {
        close(m_AMResponseFd);
        m_AMResponseFd = -1;
    }
#endif

    if (m_controlRequestFileName)
    {
        free(m_controlRequestFileName);
        m_controlRequestFileName = 0;
    }
    if (m_controlResponseFileName)
    {
        free(m_controlResponseFileName);
        m_controlResponseFileName = 0;
    }
    if (m_parRequestFileName)
    {
        free(m_parRequestFileName);
        m_parRequestFileName = 0;
    }
    if (m_parResponseFileName)
    {
        free(m_parResponseFileName);
        m_parResponseFileName = 0;
    }
#ifndef RINGB
    if (m_processFileName)
    {
        free(m_processFileName);
        m_processFileName = 0;
    }
    if (m_processResponseFileName)
    {
        free(m_processResponseFileName);
        m_processResponseFileName = 0;
    }
#endif
#ifdef AMT
    if (m_AMRequestFileName)
    {
        free(m_AMRequestFileName);
        m_AMRequestFileName = 0;
    }
    if (m_AMResponseFileName)
    {
        free(m_AMResponseFileName);
        m_AMResponseFileName = 0;
    }
#endif

    delete m_inputs;
    m_inputs = 0;

    delete m_outputs;
    m_outputs = 0;

    if (m_shm)
    {
        munlock(m_shm, m_shmSize);
        munmap(m_shm, m_shmSize);
        m_shm = 0;
    }
    if (m_shm2)
    {
        munlock(m_shm2, m_shmSize2);
        munmap(m_shm2, m_shmSize2);
        m_shm2 = 0;
    }
    if (m_shm3)
    {
        munlock(m_shm3, m_shmSize3);
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

#ifdef RINGB
    if (m_shmControl) {
        munlock(m_shmControl, sizeof(ShmControl));
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
        munlock(m_shmControl2, sizeof(ShmControl));
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
#endif
}

void RemotePluginServer::sizeShm()
{
    if (m_shm)
        return;

    size_t sz = FIXED_SHM_SIZE;
    size_t sz2 = 66048;
#ifdef AMT
    size_t sz3 = 66048;
#else
    size_t sz3 = 512;
#endif

    m_shm = (char *)mmap(0, sz, PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd, 0);
    if (!m_shm)
    {
        std::cerr << "RemotePluginServer::sizeShm: ERROR: mmap or mremap for failed for " << sz
                    << " bytes from fd " << m_shmFd << "!" << std::endl;
        m_shmSize = 0;
	RemotePluginClosedException();
    }
    else
    {
        memset(m_shm, 0, sz);
        m_shmSize = sz;

        if(mlock(m_shm, sz) != 0)
        perror("mlock fail1");
    }

    m_shm2 = (char *)mmap(0, sz2, PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd2, 0);
    if (!m_shm2)
    {
        std::cerr << "RemotePluginServer::sizeShm: ERROR: mmap or mremap for failed for " << sz2
                    << " bytes from fd " << m_shmFd2 << "!" << std::endl;
        m_shmSize2 = 0;
	RemotePluginClosedException();
    }
    else
    {
        memset(m_shm2, 0, sz2);
        m_shmSize2 = sz2;

        if(mlock(m_shm2, sz2) != 0)
        perror("mlock fail1");
    }

    m_shm3 = (char *)mmap(0, sz3, PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd3, 0);
    if (!m_shm3)
    {
        std::cerr << "RemotePluginServer::sizeShm: ERROR: mmap or mremap for failed for " << sz3
                    << " bytes from fd " << m_shmFd3 << "!" << std::endl;
        m_shmSize3 = 0;
	RemotePluginClosedException();
    }
    else
    {
        memset(m_shm3, 0, sz3);
        m_shmSize3 = sz3;

        if(mlock(m_shm3, sz3) != 0)
        perror("mlock fail1");
    }

#ifdef RINGB

    m_shmControl = static_cast<ShmControl *>(mmap(0, sizeof(ShmControl), PROT_READ | PROT_WRITE, MAP_SHARED, m_shmControlFd, 0));

    if (!m_shmControl) {
	RemotePluginClosedException();
    }

    if(mlock(m_shmControl, sizeof(ShmControl)) != 0)
    perror("mlock fail4");

    m_shmControl2 = static_cast<ShmControl *>(mmap(0, sizeof(ShmControl), PROT_READ | PROT_WRITE, MAP_SHARED, m_shmControl2Fd, 0));

    if (!m_shmControl2) {
	RemotePluginClosedException();
    }

    if(mlock(m_shmControl2, sizeof(ShmControl)) != 0)
    perror("mlock fail5");

#endif

}

void RemotePluginServer::dispatch(int timeout)
{
/*
    struct pollfd pfd[2];

    pfd[0].fd = m_controlRequestFd;
    pfd[1].fd = m_processFd;
    pfd[0].events = pfd[1].events = POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;

    if (poll(pfd, 2, timeout) < 0)
    {
       RemotePluginClosedException();
    }

    if ((pfd[0].revents & POLLIN) || (pfd[0].revents & POLLPRI))
    {
        dispatchControl();
    }
    else if (pfd[1].revents)
    {
        RemotePluginClosedException();
    }

    if ((pfd[1].revents & POLLIN) || (pfd[1].revents & POLLPRI))
    {
        dispatchProcess();
    }
    else if (pfd[1].revents)
    {
        RemotePluginClosedException();
    }
*/
}

void RemotePluginServer::dispatchControl(int timeout)
{
//#if 0
    struct pollfd pfd;
    // printf("in dispatchcontrol event\n");
    pfd.fd = m_controlRequestFd;
    pfd.events = POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;

    if (poll(&pfd, 1, timeout) < 0)
    {
        RemotePluginClosedException();
    }
    // printf("dispatchin control event\n");
    if ((pfd.revents & POLLIN) || (pfd.revents & POLLPRI))
    {
        dispatchControlEvents();
    }
    else if (pfd.revents)
    {
        if (m_threadsfinish == 1)
        return;
        RemotePluginClosedException();
    }
/*
#else
    int n;
    fd_set rfds, ofds;
    timeval timeo = {0,timeout * 1000};
    FD_ZERO(&rfds);
    FD_SET(m_controlRequestFd, &rfds);
    FD_ZERO(&ofds);
    if ((n = select(m_controlRequestFd+1, &rfds, &ofds, &ofds, &timeo)) == -1)
    {
        if (m_threadsfinish == 1)
        return;
        RemotePluginClosedException();
    }
    if (n == 1)
    {
        //printf("got a control select\n");
        dispatchControlEvents();
    }
#endif
*/
}


void RemotePluginServer::dispatchPar(int timeout)
{
//#if 0
    struct pollfd pfd;
    // printf("in dispatchcontrol event\n");
    pfd.fd = m_parRequestFd;
    pfd.events = POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;

    if (poll(&pfd, 1, timeout) < 0)
    {
        RemotePluginClosedException();
    }
    // printf("dispatchin control event\n");
    if ((pfd.revents & POLLIN) || (pfd.revents & POLLPRI))
    {
        dispatchParEvents();
    }
    else if (pfd.revents)
    {
        if (m_threadsfinish == 1)
        return;
        RemotePluginClosedException();
    }
/*
#else
    int n;
    fd_set rfds, ofds;
    timeval timeo = {0,timeout * 1000};
    FD_ZERO(&rfds);
    FD_SET(m_parRequestFd, &rfds);
    FD_ZERO(&ofds);
    if ((n = select(m_parRequestFd+1, &rfds, &ofds, &ofds, &timeo)) == -1)
    {
        if (m_threadsfinish == 1)
        return;
        RemotePluginClosedException();
    }
    if (n == 1)
    {
        // printf("got a control select\n");
        dispatchParEvents();
    }
#endif
*/
}

#ifdef RINGB

void RemotePluginServer::dispatchProcess(int timeout)
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

    if (sem_timedwait(&m_shmControl2->runServer, &ts_timeout)) {
        if (errno == ETIMEDOUT) {
            return;
        } else {
            RemotePluginClosedException();
        }
    }

    while (dataAvailable(&m_shmControl2->ringBuffer)) {
        dispatchProcessEvents();
    }

    if (sem_post(&m_shmControl2->runClient)) {
        std::cerr << "Could not post to semaphore\n";
    }
}

void RemotePluginServer::dispatchProcessEvents()
{
    RemotePluginOpcode opcode = RemotePluginNoOpcode;

    tryReadring(&m_shmControl2->ringBuffer, &opcode, sizeof(RemotePluginOpcode));

/*
    if (read(m_processFd, &opcode, sizeof(RemotePluginOpcode)) != sizeof(RemotePluginOpcode))
    {
        std::cerr << "ERROR: RemotePluginServer: couldn't read opcode" << std::endl;
        RemotePluginClosedException();
        return;
    }
*/
    // std::cerr << "read process opcode: " << opcode << std::endl;

    switch (opcode)
    {
    case RemotePluginProcess:
    {
         int sampleFrames(readIntring(&m_shmControl2->ringBuffer));

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

        for (int i = 0; i < m_numInputs; ++i)
        {
            m_inputs[i] = (float *)(m_shm + i * blocksz);
        }
        for (int i = 0; i < m_numOutputs; ++i)
        {
            m_outputs[i] = (float *)(m_shm + i * blocksz);
        }

        process(m_inputs, m_outputs, sampleFrames);
    }
        break;

    case RemotePluginProcessEvents:
        processVstEvents();
        break;

    default:
        std::cerr << "WARNING: RemotePluginServer::dispatchProcessEvents: unexpected opcode " << opcode << std::endl;
    }
    // std::cerr << "dispatched process event\n";
}

void
RemotePluginServer::dispatchGetSet(int timeout)
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
            return;
        } else {
            RemotePluginClosedException();
        }
    }

    while (dataAvailable(&m_shmControl->ringBuffer)) {
        dispatchGetSetEvents();
    }

    if (sem_post(&m_shmControl->runClient)) {
        std::cerr << "Could not post to semaphore\n";
    }
}

void RemotePluginServer::dispatchGetSetEvents()
{
    RemotePluginOpcode opcode = RemotePluginNoOpcode;

    tryReadring(&m_shmControl->ringBuffer, &opcode, sizeof(RemotePluginOpcode));
/*
    if (read(m_processFd, &opcode, sizeof(RemotePluginOpcode)) != sizeof(RemotePluginOpcode))
    {
        std::cerr << "ERROR: RemotePluginServer: couldn't read opcode" << std::endl;
        throw RemotePluginClosedException();
        return;
    }
*/
    // std::cerr << "read process opcode: " << opcode << std::endl;

    switch (opcode)
    {

    case RemotePluginSetParameter:
    {
        int pn(readIntring(&m_shmControl->ringBuffer));
        setParameter(pn, readFloatring(&m_shmControl->ringBuffer));
	break;
    }

    case RemotePluginGetParameter:
    {
        float *ptr2;
        float retval;
        ptr2 = (float *)&m_shm3[65536];
        retval = getParameter(readIntring(&m_shmControl->ringBuffer));
        *ptr2 = retval;
        break;
    }

    default:
           int i = 5;
       // std::cerr << "WARNING: RemotePluginServer::dispatchGetSetEvents: unexpected opcode " << opcode << std::endl;
    }
    // std::cerr << "dispatched process event\n";
}

#else

void RemotePluginServer::dispatchProcess(int timeout)
{
//#if 0
    struct pollfd pfd;
    // printf("in dispatchcontrol event\n");
    pfd.fd = m_processFd;
    pfd.events = POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;

    if (poll(&pfd, 1, timeout) < 0)
    {
        RemotePluginClosedException();
    }
    // printf("dispatchin control event\n");
    if ((pfd.revents & POLLIN) || (pfd.revents & POLLPRI))
    {
        dispatchProcessEvents();
    }
    else if (pfd.revents)
    {
        if (m_threadsfinish == 1)
        return;
        RemotePluginClosedException();
    }
/*
#else
    int n;
    fd_set rfds, ofds;
    timeval timeo = {0,timeout * 1000};
    FD_ZERO(&rfds);
    FD_SET(m_processFd, &rfds);
    FD_ZERO(&ofds);
    if ((n = select(m_processFd+1, &rfds, &ofds, &ofds, &timeo)) == -1)
    {
        if (m_threadsfinish == 1)
        return;
        RemotePluginClosedException();
    }
    if (n == 1)
    {
        // printf("got a control select\n");
        dispatchProcessEvents();
    }
#endif
*/
    // just block in dispatchProcessEvents
    // dispatchProcessEvents();
}

void RemotePluginServer::dispatchProcessEvents()
{
    RemotePluginOpcode opcode = RemotePluginNoOpcode;

    tryRead(m_processFd, &opcode, sizeof(RemotePluginOpcode));

/*
    if (read(m_processFd, &opcode, sizeof(RemotePluginOpcode)) != sizeof(RemotePluginOpcode))
    {
        std::cerr << "ERROR: RemotePluginServer: couldn't read opcode" << std::endl;
        RemotePluginClosedException();
        return;
    }
*/
    // std::cerr << "read process opcode: " << opcode << std::endl;

    switch (opcode)
    {
    case RemotePluginProcess:
    {
        int sampleFrames = readInt(m_processFd);

        if (m_bufferSize < 0)
        {
            writeInt(m_processResponseFd, 100);
            break;
        }
        if (m_numInputs < 0)
        {
            writeInt(m_processResponseFd, 100);
            break;
        }
        if (m_numOutputs < 0)
        {
            writeInt(m_processResponseFd, 100);
            break;
        }

        size_t blocksz = sampleFrames * sizeof(float);

        for (int i = 0; i < m_numInputs; ++i)
        {
            m_inputs[i] = (float *)(m_shm + i * blocksz);
        }
        for (int i = 0; i < m_numOutputs; ++i)
        {
            m_outputs[i] = (float *)(m_shm + i * blocksz);
        }

        process(m_inputs, m_outputs, sampleFrames);
        writeInt(m_processResponseFd, 100);
    }
        break;

    case RemotePluginProcessEvents:
        processVstEvents();
        break;

/*
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
        }
        m_numInputs = numin;
        writeInt(m_processResponseFd, m_numInputs);
    }
        break;

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
        }
        m_numOutputs = numout;
        writeInt(m_processResponseFd, m_numOutputs);
    }
        break;
*/

    default:
        std::cerr << "WARNING: RemotePluginServer::dispatchProcessEvents: unexpected opcode " << opcode << std::endl;
    }
    // std::cerr << "dispatched process event\n";
}

#endif

void RemotePluginServer::dispatchParEvents()
{
    RemotePluginOpcode  opcode = RemotePluginNoOpcode;
    static float        *parameterBuffer = 0;

    tryRead(m_parRequestFd, &opcode, sizeof(RemotePluginOpcode));
    // std::cerr << "control opcoded " << opcode << std::endl;

    switch (opcode)
    {

#ifndef RINGB

    case RemotePluginSetParameter:
    {
        int pn(readInt(m_parRequestFd));
        setParameter(pn, readFloat(m_parRequestFd));
        writeInt(m_parResponseFd, 1);
        break;
    }

    case RemotePluginGetParameter:
        writeFloat(m_parResponseFd, getParameter(readInt(m_parRequestFd)));
        break;

#endif

    case RemotePluginSetCurrentProgram:
        setCurrentProgram(readInt(m_parRequestFd));
        break;

    case RemotePluginSetBufferSize:
    {
        if (!m_shm)
            sizeShm();

        int newSize = readInt(m_parRequestFd);
        setBufferSize(newSize);
        m_bufferSize = newSize;
        break;
    }

    case RemotePluginSetSampleRate:
        setSampleRate(readInt(m_parRequestFd));
        break;

    case RemotePluginReset:
        reset();
        break;

    case RemotePluginGetVersion:
        writeFloat(m_parResponseFd, getVersion());
        break;

    case RemotePluginUniqueID:
        writeInt(m_parResponseFd, getUID());
        break;

    case RemotePluginGetName:
        writeString(m_parResponseFd, getName());
        break;

    case RemotePluginGetMaker:
        writeString(m_parResponseFd, getMaker());
        break;

    case RemotePluginTerminate:
        terminate();
        break;

    case RemotePluginGetFlags:
        m_flags = getFlags();
        writeInt(m_parResponseFd, m_flags);
        break;

    case RemotePluginGetinitialDelay:
        m_delay = getinitialDelay();
        writeInt(m_parResponseFd, m_delay);
        break;

    case RemotePluginGetParameterCount:
        writeInt(m_parResponseFd, getParameterCount());
        break;

    case RemotePluginGetParameterName:
        writeString(m_parResponseFd, getParameterName(readInt(m_parRequestFd)));
        break;

    case RemotePluginGetParameterDefault:
        writeFloat(m_parResponseFd, getParameterDefault(readInt(m_parRequestFd)));
        break;

    case RemotePluginGetParameters:
    {
        if (!parameterBuffer)
            parameterBuffer = new float[getParameterCount()];
        int p0 = readInt(m_parRequestFd);
        int pn = readInt(m_parRequestFd);
        getParameters(p0, pn, parameterBuffer);
        tryWrite(m_parResponseFd, parameterBuffer, (pn - p0 + 1) * sizeof(float));
        break;
    }

    case RemotePluginGetProgramCount:
        writeInt(m_parResponseFd, getProgramCount());
        break;

    case RemotePluginGetProgramNameIndexed:
        writeString(m_parResponseFd, getProgramNameIndexed(readInt(m_parRequestFd)));
        break;

    case RemotePluginGetProgramName:
        writeString(m_parResponseFd, getProgramName());
        break;

    case RemotePluginSetDebugLevel:
    {
        RemotePluginDebugLevel newLevel = m_debugLevel;
        tryRead(m_parRequestFd, &newLevel, sizeof(RemotePluginDebugLevel));
        setDebugLevel(newLevel);
        m_debugLevel = newLevel;
        break;
    }

    case RemotePluginWarn:
    {
        bool b = warn(readString(m_parRequestFd));
        tryWrite(m_parResponseFd, &b, sizeof(bool));
        break;
    }

    case RemotePluginNoOpcode:
        break;

    case RemotePluginGetEffInt:
    {
        int opcode = readInt(m_parRequestFd);
        writeInt(m_parResponseFd, getEffInt(opcode));
        break;
    }

    case RemotePluginGetEffString:
    {
        int opcode = readInt(m_parRequestFd);
        int idx = readInt(m_parRequestFd);
        writeString(m_parResponseFd, getEffString(opcode, idx));
        break;
    }

/*
    case RemoteMainsChanged:
    {
        int v = readInt(m_parRequestFd);
        // std::cerr << "Mains changing " << v << std::endl;
        eff_mainsChanged(v);
        writeInt(m_parResponseFd, 1);
        break;
    }
*/

    case RemotePluginGetChunk:
        getChunk();
        break;

    case RemotePluginSetChunk:
        setChunk();
        break;

/*
    case RemotePluginCanBeAutomated:
    {
        canBeAutomated();
        break;
    }
*/

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
        }
        m_numInputs = numin;
        writeInt(m_parResponseFd, m_numInputs);
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
        }
        m_numOutputs = numout;
        writeInt(m_parResponseFd, m_numOutputs);
        break;
    }

    default:
        std::cerr << "WARNING: RemotePluginServer::dispatchParEvents: unexpected opcode " << opcode << std::endl;
    }
}


void RemotePluginServer::dispatchControlEvents()
{
    RemotePluginOpcode opcode = RemotePluginNoOpcode;

    tryRead(m_controlRequestFd, &opcode, sizeof(RemotePluginOpcode));
    // std::cerr << "control opcoded " << opcode << std::endl;

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

    case RemotePluginDoVoid:
    {
        int opcode = readInt(m_controlRequestFd);
        if (opcode == effClose)
            m_threadsfinish = 1;
        effDoVoid(opcode);
        break;
    }

    case RemotePluginEffectOpen:
        EffectOpen();
        break;

    default:
        std::cerr << "WARNING: RemotePluginServer::dispatchControlEvents: unexpected opcode " << opcode << std::endl;
    }
    // std::cerr << "done dispatching control\n";
}

#ifdef RINGB

void RemotePluginServer::rdwr_tryWrite2(int fd, const void *buf, size_t count, const char *file, int line)
{
     ssize_t w = 0;

     if(m_runok == 1)
     return;
    
     while ((w = write(fd, buf, count)) < (ssize_t)count) {

     if (w < 0) {

        if (errno == EPIPE) 
        {
        if(m_threadsfinish == 1)
        return;
        }
    
    	if (errno != EAGAIN) {
		char message[100];
		sprintf(message, "Write failed on fd %d at %s:%d", fd, file, line);
		perror(message);
		RemotePluginClosedException(); 
                break;
	    }
	    w = 0;
    }
    
    buf = (void *)(((char *)buf) + w);
	count -= w;
    }
}

void RemotePluginServer::rdwr_tryRead(int fd, void *buf, size_t count, const char *file, int line)
{
    ssize_t r = 0;

    if(m_runok == 1)
    return;

    while ((r = read(fd, buf, count)) < (ssize_t)count) {

	if (r == 0) {
	    // end of file

            if(m_threadsfinish == 1)
            return;
            else 
            {
	    RemotePluginClosedException();
            break;
            }
	} else if (r < 0) {
	    if (errno != EAGAIN) {
		char message[100];
		sprintf(message, "Read failed on fd %d at %s:%d", fd, file, line);
		perror(message);
		RemotePluginClosedException();
                break;
	    }
	    r = 0;
	}

	buf = (void *)(((char *)buf) + r);
	count -= r;
    }
}

void RemotePluginServer::rdwr_tryWrite(int fd, const void *buf, size_t count, const char *file, int line)
{
        if(m_runok == 1)
        return;

        ssize_t w = write(fd, buf, count);

        if (w < 0) {
	char message[100];

        if (errno == EPIPE) 
        {
        if(m_threadsfinish == 1)
        return;
        }
       
	sprintf(message, "Write failed on fd %d at %s:%d", fd, file, line);
	perror(message);
	RemotePluginClosedException();
        }

        if (w < (ssize_t)count) {
	fprintf(stderr, "Failed to complete write on fd %d (have %d, put %d) at %s:%d\n",
		fd, count, w, file, line);
	RemotePluginClosedException();
        }
}

void RemotePluginServer::rdwr_tryReadring(RingBuffer *ringbuf, void *buf, size_t count, const char *file, int line)
{
    char *charbuf = static_cast<char *>(buf);
    size_t tail = ringbuf->tail;
    size_t head = ringbuf->head;
    size_t wrap = 0;

    if(m_runok == 1)
    return;

    if (head <= tail) {
        wrap = SHM_RING_BUFFER_SIZE;
    }
    if (head - tail + wrap < count) {
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
    const char *charbuf = static_cast<const char *>(buf);
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

void RemotePluginServer::rdwr_writeOpcode(int fd, RemotePluginOpcode opcode, const char *file, int line)
{
    rdwr_tryWrite(fd, &opcode, sizeof(RemotePluginOpcode), file, line);
}    

void RemotePluginServer::rdwr_writeString(int fd, const std::string &str, const char *file, int line)
{
    int len = str.length();
    rdwr_tryWrite(fd, &len, sizeof(int), file, line);
    rdwr_tryWrite(fd, str.c_str(), len, file, line);
}

std::string RemotePluginServer::rdwr_readString(int fd, const char *file, int line)
{
    int len;
    static char *buf = 0;
    static int bufLen = 0;
    rdwr_tryRead(fd, &len, sizeof(int), file, line);
    if (len + 1 > bufLen) {
	delete buf;
	buf = new char[len + 1];
	bufLen = len + 1;
    }
    rdwr_tryRead(fd, buf, len, file, line);
    buf[len] = '\0';
    return std::string(buf);
}

void RemotePluginServer::rdwr_writeInt(int fd, int i, const char *file, int line)
{
    rdwr_tryWrite(fd, &i, sizeof(int), file, line);
}

int RemotePluginServer::rdwr_readInt(int fd, const char *file, int line)
{
    int i = 0;
    rdwr_tryRead(fd, &i, sizeof(int), file, line);
    return i;
}

void RemotePluginServer::rdwr_writeFloat(int fd, float f, const char *file, int line)
{
    rdwr_tryWrite(fd, &f, sizeof(float), file, line);
}

float RemotePluginServer::rdwr_readFloat(int fd, const char *file, int line)
{
    float f = 0;
    rdwr_tryRead(fd, &f, sizeof(float), file, line);
    return f;
}

#else

void RemotePluginServer::rdwr_tryWrite2(int fd, const void *buf, size_t count, const char *file, int line)
{
     ssize_t w = 0;

     if(m_runok == 1)
     return;
    
     while ((w = write(fd, buf, count)) < (ssize_t)count) {

     if (w < 0) {
        if (errno == EPIPE) 
        {
        if(m_threadsfinish == 1)
        return;
        }
    
    	if (errno != EAGAIN) {
		char message[100];
		sprintf(message, "Write failed on fd %d at %s:%d", fd, file, line);
		perror(message);
		RemotePluginClosedException();
                break;
	    }
	    w = 0;
        }
        buf = (void *)(((char *)buf) + w);
	count -= w;
    }
}

void RemotePluginServer::rdwr_tryRead(int fd, void *buf, size_t count, const char *file, int line)
{
    ssize_t r = 0;

    if(m_runok == 1)
    return;

    while ((r = read(fd, buf, count)) < (ssize_t)count) {

	if (r == 0) {
	    // end of file

            if(m_threadsfinish == 1)
            return;
            else
            {
	    RemotePluginClosedException();
            break;
            }
            
	} else if (r < 0) {
	    if (errno != EAGAIN) {
		char message[100];
		sprintf(message, "Read failed on fd %d at %s:%d", fd, file, line);
		perror(message);
		RemotePluginClosedException();
                break;
	    }
	    r = 0;
	}

	buf = (void *)(((char *)buf) + r);
	count -= r;
    }
}

void RemotePluginServer::rdwr_tryWrite(int fd, const void *buf, size_t count, const char *file, int line)
{
        if(m_runok == 1)
        return;

        ssize_t w = write(fd, buf, count);

        if (w < 0) {
	char message[100];

        if (errno == EPIPE) 
        {
        if(m_threadsfinish == 1)
        return;
        }
       
	sprintf(message, "Write failed on fd %d at %s:%d", fd, file, line);
	perror(message);
	RemotePluginClosedException();
        }

        if (w < (ssize_t)count) {
	fprintf(stderr, "Failed to complete write on fd %d (have %d, put %d) at %s:%d\n",
		fd, count, w, file, line);
	RemotePluginClosedException();
    }
}

void RemotePluginServer::rdwr_writeOpcode(int fd, RemotePluginOpcode opcode, const char *file, int line)
{
    rdwr_tryWrite(fd, &opcode, sizeof(RemotePluginOpcode), file, line);
}    

void RemotePluginServer::rdwr_writeString(int fd, const std::string &str, const char *file, int line)
{
    int len = str.length();
    rdwr_tryWrite(fd, &len, sizeof(int), file, line);
    rdwr_tryWrite(fd, str.c_str(), len, file, line);
}

std::string RemotePluginServer::rdwr_readString(int fd, const char *file, int line)
{
    int len;
    static char *buf = 0;
    static int bufLen = 0;
    rdwr_tryRead(fd, &len, sizeof(int), file, line);
    if (len + 1 > bufLen) {
	delete buf;
	buf = new char[len + 1];
	bufLen = len + 1;
    }
    rdwr_tryRead(fd, buf, len, file, line);
    buf[len] = '\0';
    return std::string(buf);
}

void RemotePluginServer::rdwr_writeInt(int fd, int i, const char *file, int line)
{
    rdwr_tryWrite(fd, &i, sizeof(int), file, line);
}

int RemotePluginServer::rdwr_readInt(int fd, const char *file, int line)
{
    int i = 0;
    rdwr_tryRead(fd, &i, sizeof(int), file, line);
    return i;
}

void RemotePluginServer::rdwr_writeFloat(int fd, float f, const char *file, int line)
{
    rdwr_tryWrite(fd, &f, sizeof(float), file, line);
}

float RemotePluginServer::rdwr_readFloat(int fd, const char *file, int line)
{
    float f = 0;
    rdwr_tryRead(fd, &f, sizeof(float), file, line);
    return f;
}

#endif

void RemotePluginServer::RemotePluginClosedException()
{
m_runok = 1;
terminate();
}
