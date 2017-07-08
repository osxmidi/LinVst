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

#include "rdwrops.h"


#ifdef AMT
#define finishthread 9999

void* RemotePluginClient::AMThread()
{
    int         opcode;
    int         val;
    int         ok = 1;

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
        if (m_shm)
        {
//#if 0
            struct pollfd pfd;
            // printf("in dispatchcontrol event\n");
            pfd.fd = m_AMResponseFd;
            pfd.events = POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;

            if (poll(&pfd, 1, 50) < 0)
            {
                // throw RemotePluginClosedException();
                m_threadbreak = 1;
                continue;
            }
            // printf("dispatchin control event\n");
            if ((pfd.revents & POLLIN) || (pfd.revents & POLLPRI))
            {
                // dispatchControlEvents();
/*
#else
            int n;
            fd_set rfds, ofds;
            timeval timeo = {0,timeout * 1000};
            FD_ZERO(&rfds);
            FD_SET(m_AMResponseFd, &rfds);
            FD_ZERO(&ofds);
            if ((n = select(m_AMResponseFd+1, &rfds, &ofds, &ofds, &timeo)) == -1)
            {
                // throw RemotePluginClosedException();
                m_threadbreak = 1;
                continue;
            }
            if (n == 1)
            {
#endif
*/
                opcode = -1;
                tryRead(m_AMResponseFd, &opcode, sizeof(int));
                readSetSchedInfo(m_AMResponseFd);

                if (opcode == finishthread)
                    break;

                switch(opcode)
                {
                case audioMasterGetTime:
                    tryRead(m_AMResponseFd, &val, sizeof(int));
                    timeInfo = (VstTimeInfo *) m_audioMaster(theEffect, audioMasterGetTime, 0, val, 0, 0);
                    tryWrite(m_AMRequestFd, timeInfo, sizeof(VstTimeInfo));
                    break;

                case audioMasterIOChanged:
                    tryRead(m_AMResponseFd, &am, sizeof(am));
                    theEffect->flags = am.flags;
                    theEffect->numPrograms = am.pcount;
                    theEffect->numParams = am.parcount;
                    theEffect->numInputs = am.incount;
                    theEffect->numOutputs = am.outcount;
                    theEffect->initialDelay = am.delay;
                    // m_updateio = 1;
                    m_audioMaster(theEffect, audioMasterIOChanged, 0, 0, 0, 0);
                    writeInt(m_AMRequestFd, ok);
                    break;

                case audioMasterProcessEvents:
                    tryRead(m_AMResponseFd, &val, sizeof(int));
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
                    writeInt(m_AMRequestFd, ok);
                    break;

                default:
                    break;
                }
            }
            else if (pfd.revents)
            {
                // throw RemotePluginClosedException();
                m_threadbreak = 1;
                continue;
            }
        }
    }
    // m_threadbreakexit = 1;
    // pthread_exit(0);
    return 0;
}

#endif

RemotePluginClient::RemotePluginClient(audioMasterCallback theMaster) :

#ifdef AMT
    m_audioMaster(theMaster),
#endif

    m_controlRequestFd(-1),
    m_controlResponseFd(-1),
    m_parRequestFd(-1),
    m_parResponseFd(-1),
    m_processFd(-1),
    m_processResponseFd(-1),
    m_shmFd(-1),
    m_shmFd2(-1),
    m_shmFd3(-1),
    m_controlRequestFileName(0),
    m_controlResponseFileName(0),
    m_parRequestFileName(0),
    m_parResponseFileName(0),
    m_processFileName(0),
    m_processResponseFileName(0),

#ifdef AMT
    m_AMRequestFd(-1),
    m_AMResponseFd(-1),
    m_AMRequestFileName(0),
    m_AMResponseFileName(0),
    m_AMThread(0),
    m_threadbreak(0),
    m_threadbreakexit(0),
    m_updateio(0),
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
    m_bufferSize(-1),
    m_numInputs(-1),
    m_numOutputs(-1),
    m_finishaudio(0),
    m_runok(0),
    theEffect(0)
{
    char tmpFileBase[60];

    sprintf(tmpFileBase, "/tmp/rplugin_crq_XXXXXX");
    if (mkstemp(tmpFileBase) < 0)
    {
        cleanup();
        throw((std::string)"Failed to obtain temporary filename");
    }
    m_controlRequestFileName = strdup(tmpFileBase);

    unlink(m_controlRequestFileName);
    if (mkfifo(m_controlRequestFileName, 0666)) //FIXME what permission is correct here?
    {
        perror(m_controlRequestFileName);
        cleanup();
        throw((std::string)"Failed to create FIFO");
    }

    sprintf(tmpFileBase, "/tmp/rplugin_crs_XXXXXX");
    if (mkstemp(tmpFileBase) < 0)
    {
        cleanup();
        throw((std::string)"Failed to obtain temporary filename");
    }
    m_controlResponseFileName = strdup(tmpFileBase);

    unlink(m_controlResponseFileName);
    if (mkfifo(m_controlResponseFileName, 0666)) //FIXME what permission is correct here?
    {
        perror(m_controlResponseFileName);
        cleanup();
        throw((std::string)"Failed to create FIFO");
    }

    sprintf(tmpFileBase, "/tmp/rplugin_gpa_XXXXXX");
    if (mkstemp(tmpFileBase) < 0)
    {
        cleanup();
        throw((std::string)"Failed to obtain temporary filename");
    }
    m_parRequestFileName = strdup(tmpFileBase);

    unlink(m_parRequestFileName);
    if (mkfifo(m_parRequestFileName, 0666)) //FIXME what permission is correct here?
    {
        perror(m_parRequestFileName);
        cleanup();
        throw((std::string)"Failed to create FIFO");
    }

    sprintf(tmpFileBase, "/tmp/rplugin_spa_XXXXXX");
    if (mkstemp(tmpFileBase) < 0)
    {
        cleanup();
        throw((std::string)"Failed to obtain temporary filename");
    }
    m_parResponseFileName = strdup(tmpFileBase);

    unlink(m_parResponseFileName);
    if (mkfifo(m_parResponseFileName, 0666)) //FIXME what permission is correct here?
    {
        perror(m_parResponseFileName);
        cleanup();
        throw((std::string)"Failed to create FIFO");
    }

    sprintf(tmpFileBase, "/tmp/rplugin_prc_XXXXXX");
    if (mkstemp(tmpFileBase) < 0)
    {
        cleanup();
        throw((std::string)"Failed to obtain temporary filename");
    }
    m_processFileName = strdup(tmpFileBase);

    unlink(m_processFileName);
    if (mkfifo(m_processFileName, 0666)) //FIXME what permission is correct here?
    {
        perror(m_processFileName);
        cleanup();
        throw((std::string)"Failed to create FIFO");
    }

    sprintf(tmpFileBase, "/tmp/rplugin_prr_XXXXXX");
    if (mkstemp(tmpFileBase) < 0)
    {
        cleanup();
        throw((std::string)"Failed to obtain temporary filename");
    }
    m_processResponseFileName = strdup(tmpFileBase);

    unlink(m_processResponseFileName);
    if (mkfifo(m_processResponseFileName, 0666)) //FIXME what permission is correct here?
    {
        perror(m_processResponseFileName);
        cleanup();
        throw((std::string)"Failed to create FIFO");
    }

#ifdef AMT
    sprintf(tmpFileBase, "/tmp/rplugin_cal_XXXXXX");
    if (mkstemp(tmpFileBase) < 0)
    {
        cleanup();
        throw((std::string)"Failed to obtain temporary filename");
    }
    m_AMRequestFileName = strdup(tmpFileBase);

    unlink(m_AMRequestFileName);
    if (mkfifo(m_AMRequestFileName, 0666)) //FIXME what permission is correct here?
    {
        perror(m_AMRequestFileName);
        cleanup();
        throw((std::string)"Failed to create FIFO");
    }

    sprintf(tmpFileBase, "/tmp/rplugin_cap_XXXXXX");
    if (mkstemp(tmpFileBase) < 0)
    {
        cleanup();
        throw((std::string)"Failed to obtain temporary filename");
    }
    m_AMResponseFileName = strdup(tmpFileBase);

    unlink(m_AMResponseFileName);
    if (mkfifo(m_AMResponseFileName, 0666)) //FIXME what permission is correct here?
    {
        perror(m_AMResponseFileName);
        cleanup();
        throw((std::string)"Failed to create FIFO");
    }
#endif

    sprintf(tmpFileBase, "/tmp/rplugin_shm_XXXXXX");
    if (mkstemp(tmpFileBase) < 0)
    {
        cleanup();
        throw((std::string)"Failed to obtain temporary filename");
    }
    m_shmFileName = strdup(tmpFileBase);

    m_shmFd = open(m_shmFileName, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (m_shmFd < 0)
    {
        cleanup();
        throw((std::string)"Failed to open or create shared memory file");
    }


    sprintf(tmpFileBase, "/tmp/rplugin_shn_XXXXXX");
    if (mkstemp(tmpFileBase) < 0)
    {
        cleanup();
        throw((std::string)"Failed to obtain temporary filename");
    }
    m_shmFileName2 = strdup(tmpFileBase);

    m_shmFd2 = open(m_shmFileName2, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (m_shmFd2 < 0)
    {
        cleanup();
        throw((std::string)"Failed to open or create shared memory file");
    }

    sprintf(tmpFileBase, "/tmp/rplugin_sho_XXXXXX");
    if (mkstemp(tmpFileBase) < 0)
    {
        cleanup();
        throw((std::string)"Failed to obtain temporary filename");
    }
    m_shmFileName3 = strdup(tmpFileBase);

    m_shmFd3 = open(m_shmFileName3, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (m_shmFd3 < 0)
    {
        cleanup();
        throw((std::string)"Failed to open or create shared memory file");
    }
}

RemotePluginClient::~RemotePluginClient()
{
/*
#ifdef AMT
    m_threadbreak = 1;
    m_threadbreakexit = 1;
#endif
 */
    if (theEffect)
        delete theEffect;
    cleanup();
}

void RemotePluginClient::syncStartup()
{
    // The first (write) fd we open in a nonblocking call, with a
    // short retry loop so we can easily give up if the other end
    // doesn't appear to be responding.  We want a nonblocking FIFO
    // for this and the process fd anyway.
    bool    connected = false;
    int     timeout = 60;

    for (int attempt = 0; attempt < timeout; ++attempt)
    {
        if ((m_controlRequestFd = open(m_controlRequestFileName, O_WRONLY | O_NONBLOCK)) >= 0)
        {
            connected = true;
            break;
        }
        else if (errno != ENXIO)
            break; // an actual error occurred
        usleep(250000);
    }

    if (!connected)
    {
        cleanup();
        throw((std::string)"Plugin server timed out on startup");
    }

    if (connected)
    {
        if ((m_controlResponseFd = open(m_controlResponseFileName, O_RDONLY)) < 0)
        {
            cleanup();
            throw((std::string)"Failed to open control FIFO");
        }

        connected = false;

        for (int attempt = 0; attempt < timeout; ++attempt)
        {
            if ((m_parRequestFd = open(m_parRequestFileName, O_WRONLY | O_NONBLOCK)) >= 0)
            {
                connected = true;
                break;
            }
            else if (errno != ENXIO)
                break; // an actual error occurred
            usleep(250000);
        }

        if (!connected)
        {
            cleanup();
            throw((std::string)"Plugin server timed out on startup");
        }

        if ((m_parResponseFd = open(m_parResponseFileName, O_RDONLY)) < 0)
        {
            cleanup();
            throw((std::string)"Failed to open control FIFO");
        }

        connected = false;

        for (int attempt = 0; attempt < timeout; ++attempt)
        {
            if ((m_processFd = open(m_processFileName, O_WRONLY | O_NONBLOCK)) >= 0)
            {
                connected = true;
                break;
            }
            else if (errno != ENXIO)
                break; // an actual error occurred
            usleep(250000);
        }

        if (!connected)
        {
            cleanup();
            throw((std::string)"Failed to open process FIFO");
        }

        if ((m_processResponseFd = open(m_processResponseFileName, O_RDONLY)) < 0)
        {
            cleanup();
            throw((std::string)"Failed to open process FIFO");
        }

#ifdef AMT
        connected = false;

        for (int attempt = 0; attempt < timeout; ++attempt)
        {
            if ((m_AMRequestFd = open(m_AMRequestFileName, O_WRONLY | O_NONBLOCK)) >= 0)
            {
                connected = true;
                break;
            }
            else if (errno != ENXIO)
                break; // an actual error occurred
            usleep(250000);
        }

        if (!connected)
        {
            cleanup();
            throw((std::string)"Plugin server timed out on startup");
        }


        if ((m_AMResponseFd = open(m_AMResponseFileName, O_RDONLY)) < 0)
        {
            cleanup();
            throw((std::string)"Failed to open control FIFO");
        }
#endif
    }

    bool b = false;
    tryRead(m_controlResponseFd, &b, sizeof(bool));
    if (!b)
    {
        cleanup();
        throw((std::string)"Remote plugin did not start correctly");
    }

    theEffect = new AEffect;
}

void RemotePluginClient::cleanup()
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
        unlink(m_controlRequestFileName);
        free(m_controlRequestFileName);
        m_controlRequestFileName = 0;
    }
    if (m_controlResponseFileName)
    {
        unlink(m_controlResponseFileName);
        free(m_controlResponseFileName);
        m_controlResponseFileName = 0;
    }
    if (m_parRequestFileName)
    {
        unlink(m_parRequestFileName);
        free(m_parRequestFileName);
        m_parRequestFileName = 0;
    }
    if (m_parResponseFileName)
    {
        unlink(m_parResponseFileName);
        free(m_parResponseFileName);
        m_parResponseFileName = 0;
    }
    if (m_processFileName)
    {
        unlink(m_processFileName);
        free(m_processFileName);
        m_processFileName = 0;
    }
    if (m_processResponseFileName)
    {
        unlink(m_processResponseFileName);
        free(m_processResponseFileName);
        m_processResponseFileName = 0;
    }

#ifdef AMT
    if (m_AMRequestFileName)
    {
        unlink(m_AMRequestFileName);
        free(m_AMRequestFileName);
        m_AMRequestFileName = 0;
    }
    if (m_AMResponseFileName)
    {
        unlink(m_AMResponseFileName);
        free(m_AMResponseFileName);
        m_AMResponseFileName = 0;
    }
#endif

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
        unlink(m_shmFileName);
        free(m_shmFileName);
        m_shmFileName = 0;
    }
    if (m_shmFileName2)
    {
        unlink(m_shmFileName2);
        free(m_shmFileName2);
        m_shmFileName2 = 0;
    }
    if (m_shmFileName3)
    {
        unlink(m_shmFileName3);
        free(m_shmFileName3);
        m_shmFileName3 = 0;
    }
#ifdef AMT
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
#endif
}

std::string RemotePluginClient::getFileIdentifiers()
{
    std::string id;
    id += m_controlRequestFileName + strlen(m_controlRequestFileName) - 6;
    id += m_controlResponseFileName + strlen(m_controlResponseFileName) - 6;
    id += m_parRequestFileName + strlen(m_parRequestFileName) - 6;
    id += m_parResponseFileName + strlen(m_parResponseFileName) - 6;
    id += m_processFileName + strlen(m_processFileName) - 6;
    id += m_processResponseFileName + strlen(m_processResponseFileName) - 6;

#ifdef AMT
    id += m_AMRequestFileName + strlen(m_AMRequestFileName) - 6;
    id += m_AMResponseFileName + strlen(m_AMResponseFileName) - 6;
#endif

    id += m_shmFileName + strlen(m_shmFileName) - 6;
    id += m_shmFileName2 + strlen(m_shmFileName2) - 6;
    id += m_shmFileName3 + strlen(m_shmFileName3) - 6;

    //  std::cerr << "Returning file identifiers: " << id << std::endl;
    return id;
}

void RemotePluginClient::sizeShm()
{
    if (m_shm)
        return;

    size_t sz = FIXED_SHM_SIZE;
    size_t sz2 = 128000;
#ifdef AMT
    size_t sz3 = 128000;
#else
    size_t sz3 = 512;
#endif

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
    }

#ifdef AMT
    m_threadbreak = 0;
    // m_threadbreakexit = 0;
    pthread_create(&m_AMThread, NULL, RemotePluginClient::callAMThread, this);
 #endif
}

float   RemotePluginClient::getVersion()
{
    //FIXME!!! client code needs to be testing this
    writeOpcode(m_parRequestFd, RemotePluginGetVersion);
    getWriteSchedInfo(m_parRequestFd);
    return readFloat(m_parResponseFd);
}

int RemotePluginClient::getUID()
{
    writeOpcode(m_parRequestFd, RemotePluginUniqueID);
    getWriteSchedInfo(m_parRequestFd);
    return readInt(m_parResponseFd);
}

std::string RemotePluginClient::getName()
{
    writeOpcode(m_parRequestFd, RemotePluginGetName);
    getWriteSchedInfo(m_parRequestFd);
    return readString(m_parResponseFd);
}

std::string RemotePluginClient::getMaker()
{
    writeOpcode(m_parRequestFd, RemotePluginGetMaker);
    getWriteSchedInfo(m_parRequestFd);
    return readString(m_parResponseFd);
}

void RemotePluginClient::setBufferSize(int s)
{
    if (s <= 0)
        return;

    if (!m_shm)
        sizeShm();

    if (s == m_bufferSize)
        return;

    m_bufferSize = s;
    writeOpcode(m_parRequestFd, RemotePluginSetBufferSize);
    getWriteSchedInfo(m_parRequestFd);
    writeInt(m_parRequestFd, s);
    int ok = readInt(m_parResponseFd);
}

void RemotePluginClient::setSampleRate(int s)
{
    writeOpcode(m_parRequestFd, RemotePluginSetSampleRate);
    getWriteSchedInfo(m_parRequestFd);
    writeInt(m_parRequestFd, s);
    int ok = readInt(m_parResponseFd);
}

void RemotePluginClient::reset()
{
    writeOpcode(m_parRequestFd, RemotePluginReset);
    getWriteSchedInfo(m_parRequestFd);
    if (m_shmSize > 0)
    {
        memset(m_shm, 0, m_shmSize);
        memset(m_shm2, 0, m_shmSize2);
        memset(m_shm3, 0, m_shmSize3);
    }
    int ok = readInt(m_parResponseFd);
}

void RemotePluginClient::terminate()
{
    writeOpcode(m_parRequestFd, RemotePluginTerminate);
    getWriteSchedInfo(m_parRequestFd);
}

int RemotePluginClient::getEffInt(int opcode)
{
    writeOpcode(m_parRequestFd, RemotePluginGetEffInt);
    getWriteSchedInfo(m_parRequestFd);
    writeInt(m_parRequestFd, opcode);
    return readInt(m_parResponseFd);
}

void RemotePluginClient::getEffString(int opcode, int index, char *ptr, int len)
{
    writeOpcode(m_parRequestFd, RemotePluginGetEffString);
    getWriteSchedInfo(m_parRequestFd);
    writeInt(m_parRequestFd, opcode);
    writeInt(m_parRequestFd, index);
    strncpy(ptr, readString(m_parResponseFd).c_str(), len);
    ptr[len-1] = 0;
}

int RemotePluginClient::getFlags()
{
    writeOpcode(m_parRequestFd, RemotePluginGetFlags);
    getWriteSchedInfo(m_parRequestFd);
    return readInt(m_parResponseFd);
}

int RemotePluginClient::getinitialDelay()
{
    writeOpcode(m_parRequestFd, RemotePluginGetinitialDelay);
    getWriteSchedInfo(m_parRequestFd);
    return readInt(m_parResponseFd);
}

int RemotePluginClient::getInputCount()
{
    // writeOpcode(m_processFd, RemotePluginGetInputCount);
    // m_numInputs = readInt(m_processResponseFd);

    writeOpcode(m_parRequestFd, RemotePluginGetInputCount);
    getWriteSchedInfo(m_parRequestFd);
    m_numInputs = readInt(m_parResponseFd);
    return m_numInputs;
}

int RemotePluginClient::getOutputCount()
{
    // writeOpcode(m_processFd, RemotePluginGetOutputCount);
    // m_numOutputs = readInt(m_processResponseFd);

    writeOpcode(m_parRequestFd, RemotePluginGetOutputCount);
    getWriteSchedInfo(m_parRequestFd);
    m_numOutputs = readInt(m_parResponseFd);
    return m_numOutputs;
}

int RemotePluginClient::getParameterCount()
{
    writeOpcode(m_parRequestFd, RemotePluginGetParameterCount);
    getWriteSchedInfo(m_parRequestFd);
    return readInt(m_parResponseFd);
}

std::string RemotePluginClient::getParameterName(int p)
{
    writeOpcode(m_parRequestFd, RemotePluginGetParameterName);
    getWriteSchedInfo(m_parRequestFd);
    writeInt(m_parRequestFd, p);
    return readString(m_parResponseFd);
}

void RemotePluginClient::setParameter(int p, float v)
{
    writeOpcode(m_parRequestFd, RemotePluginSetParameter);
    getWriteSchedInfo(m_parRequestFd);
    writeInt(m_parRequestFd, p);
    writeFloat(m_parRequestFd, v);
    // wait for a response
    int ok = readInt(m_parResponseFd);
}

float RemotePluginClient::getParameter(int p)
{
    writeOpcode(m_parRequestFd, RemotePluginGetParameter);
    getWriteSchedInfo(m_parRequestFd);
    writeInt(m_parRequestFd, p);
    return readFloat(m_parResponseFd);
}

float RemotePluginClient::getParameterDefault(int p)
{
    writeOpcode(m_parRequestFd, RemotePluginGetParameterDefault);
    getWriteSchedInfo(m_parRequestFd);
    writeInt(m_parRequestFd, p);
    return readFloat(m_parResponseFd);
}

void RemotePluginClient::getParameters(int p0, int pn, float *v)
{
    writeOpcode(m_parRequestFd, RemotePluginGetParameters);
    getWriteSchedInfo(m_parRequestFd);
    writeInt(m_parRequestFd, p0);
    writeInt(m_parRequestFd, pn);
    tryRead(m_parResponseFd, v, (pn - p0 + 1) * sizeof(float));
}

int RemotePluginClient::getProgramCount()
{
    writeOpcode(m_parRequestFd, RemotePluginGetProgramCount);
    getWriteSchedInfo(m_parRequestFd);
    return readInt(m_parResponseFd);
}

std::string RemotePluginClient::getProgramNameIndexed(int n)
{
    writeOpcode(m_parRequestFd, RemotePluginGetProgramNameIndexed);
    getWriteSchedInfo(m_parRequestFd);
    writeInt(m_parRequestFd, n);
    return readString(m_parResponseFd);
}

std::string RemotePluginClient::getProgramName()
{
    writeOpcode(m_parRequestFd, RemotePluginGetProgramName);
    getWriteSchedInfo(m_parRequestFd);
    return readString(m_parResponseFd);
}

void RemotePluginClient::setCurrentProgram(int n)
{
    writeOpcode(m_parRequestFd, RemotePluginSetCurrentProgram);
    getWriteSchedInfo(m_parRequestFd);
    writeInt(m_parRequestFd, n);
    int ok = readInt(m_parResponseFd);
}

void RemotePluginClient::process(float **inputs, float **outputs, int sampleFrames)
{
    if ((m_finishaudio == 1) || (m_bufferSize < 0) || (m_numInputs < 0) || (m_numOutputs < 0))
        return;
    if ((m_numInputs + m_numOutputs) * m_bufferSize * sizeof(float) > FIXED_SHM_SIZE)
        return;

#ifdef AMT
    if (m_updateio)
    {
        getInputCount();
        getOutputCount();
        m_updateio = 0;
    }
#endif

    size_t blocksz = m_bufferSize * sizeof(float);

    for (int i = 0; i < m_numInputs; ++i)
        memcpy(m_shm + i * blocksz, inputs[i], sampleFrames*sizeof(float));

    writeOpcode(m_processFd, RemotePluginProcess);
    getWriteSchedInfo(m_processFd);
    writeInt(m_processFd, sampleFrames);

    int resp;
    while ((resp = readInt(m_processResponseFd)) != 100)
    {
    }

    for (int i = 0; i < m_numOutputs; ++i)
        memcpy(outputs[i], m_shm + (i + m_numInputs) * blocksz, sampleFrames*sizeof(float));

    return;
}

int RemotePluginClient::processVstEvents(VstEvents *evnts)
{
    int ret;
    int eventnum;
    int *ptr;
    int sizeidx = 0;

    if ((evnts->numEvents <= 0) || (!evnts) || (m_finishaudio == 1))
        return 0;

    ptr = (int *)m_shm2;
    eventnum = evnts->numEvents;
    sizeidx = sizeof(int);

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

    writeOpcode(m_processFd, RemotePluginProcessEvents);
    getWriteSchedInfo(m_processFd);
    int ok = readInt(m_processResponseFd);
    return ret;
}

void RemotePluginClient::setDebugLevel(RemotePluginDebugLevel level)
{
    writeOpcode(m_parRequestFd, RemotePluginSetDebugLevel);
    getWriteSchedInfo(m_parRequestFd);
    tryWrite(m_parRequestFd, &level, sizeof(RemotePluginDebugLevel));
}

bool RemotePluginClient::warn(std::string str)
{
    writeOpcode(m_parRequestFd, RemotePluginWarn);
    getWriteSchedInfo(m_parRequestFd);
    writeString(m_parRequestFd, str);
    bool b;
    tryRead(m_parResponseFd, &b, sizeof(bool));
    return b;
}

void RemotePluginClient::showGUI()
{
    writeOpcode(m_controlRequestFd, RemotePluginShowGUI);

#ifdef EMBED
    tryRead(m_controlResponseFd, &winm, sizeof(winm));
#else
    int ok = readInt(m_controlResponseFd);
#endif
}

void RemotePluginClient::hideGUI()
{
    writeOpcode(m_controlRequestFd, RemotePluginHideGUI);
    int ok = readInt(m_controlResponseFd);
}

#ifdef EMBED
void RemotePluginClient::openGUI()
{
    writeOpcode(m_controlRequestFd, RemotePluginOpenGUI);
}
#endif

void RemotePluginClient::effVoidOp(int opcode)
{
    if (opcode == effClose)
    {
        m_finishaudio = 1;
        writeOpcode(m_controlRequestFd, RemotePluginDoVoid);
        writeInt(m_controlRequestFd, opcode);
        int ok = readInt(m_controlResponseFd);
    }
    else
    {
        writeOpcode(m_controlRequestFd, RemotePluginDoVoid);
        writeInt(m_controlRequestFd, opcode);
        int ok2 = readInt(m_controlResponseFd);
    }
}

int RemotePluginClient::getChunk(void **ptr, int bank_prg)
{
    static void *chunk_ptr = 0;
    writeOpcode(m_parRequestFd, RemotePluginGetChunk);
    getWriteSchedInfo(m_parRequestFd);
    writeInt(m_parRequestFd, bank_prg);
    int sz = readInt(m_parResponseFd);

    if (chunk_ptr != 0)
        free(chunk_ptr);
    chunk_ptr = malloc(sz);

    tryRead(m_parResponseFd, chunk_ptr, sz);
    *ptr = chunk_ptr;
    return sz;
}

int RemotePluginClient::setChunk(void *ptr, int sz, int bank_prg)
{
    writeOpcode(m_parRequestFd, RemotePluginSetChunk);
    getWriteSchedInfo(m_parRequestFd);
    writeInt(m_parRequestFd, sz);
    writeInt(m_parRequestFd, bank_prg);
    tryWrite2(m_parRequestFd, ptr, sz);
    return readInt(m_parResponseFd);
}

/*
int RemotePluginClient::canBeAutomated(int param)
{
    writeOpcode(m_parRequestFd, RemotePluginCanBeAutomated);
    writeInt(m_parRequestFd, param);
    return readInt(m_parResponseFd);
}
*/

int RemotePluginClient::getProgram()
{
    writeOpcode(m_parRequestFd, RemotePluginGetProgram);
    getWriteSchedInfo(m_parRequestFd);
    return readInt(m_parResponseFd);
}

int RemotePluginClient::EffectOpen()
{
    writeOpcode(m_controlRequestFd, RemotePluginEffectOpen);
    return readInt(m_controlResponseFd);
}

