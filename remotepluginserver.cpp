/*
  dssi-vst: a DSSI plugin wrapper for VST effects and instruments
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

#include "rdwrops.h"


RemotePluginServer::RemotePluginServer(std::string fileIdentifiers) :
    m_bufferSize(-1),
    m_numInputs(-1),
    m_numOutputs(-1),
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
    m_outputs(0)
{
   
    char tmpFileBase[60];
    sprintf(tmpFileBase, "/tmp/rplugin_crq_%s",
	    fileIdentifiers.substr(0, 6).c_str());
    m_controlRequestFileName = strdup(tmpFileBase);

    if ((m_controlRequestFd = open(m_controlRequestFileName, O_RDWR)) < 0) {
	cleanup();
	throw((std::string)"Failed to open FIFO");
    }
    
    sprintf(tmpFileBase, "/tmp/rplugin_crs_%s",
	    fileIdentifiers.substr(6, 6).c_str());
    m_controlResponseFileName = strdup(tmpFileBase);

    if ((m_controlResponseFd = open(m_controlResponseFileName, O_WRONLY)) < 0) {
	cleanup();
	throw((std::string)"Failed to open FIFO");
    }
       
    sprintf(tmpFileBase, "/tmp/rplugin_gpa_%s",
	    fileIdentifiers.substr(12, 6).c_str());
    m_parRequestFileName = strdup(tmpFileBase);

    if ((m_parRequestFd = open(m_parRequestFileName, O_RDWR)) < 0) {
	cleanup();
	throw((std::string)"Failed to open FIFO");
    }
    
    sprintf(tmpFileBase, "/tmp/rplugin_spa_%s",
	    fileIdentifiers.substr(18, 6).c_str());
    m_parResponseFileName = strdup(tmpFileBase);

    if ((m_parResponseFd = open(m_parResponseFileName, O_WRONLY)) < 0) {
	cleanup();
	throw((std::string)"Failed to open FIFO");
    }

    sprintf(tmpFileBase, "/tmp/rplugin_prc_%s",
	    fileIdentifiers.substr(24, 6).c_str());
    m_processFileName = strdup(tmpFileBase);

    if ((m_processFd = open(m_processFileName, O_RDONLY)) < 0) {
	cleanup();
	throw((std::string)"Failed to open FIFO");
    }
    
    sprintf(tmpFileBase, "/tmp/rplugin_prr_%s",
	    fileIdentifiers.substr(30, 6).c_str());
    m_processResponseFileName = strdup(tmpFileBase);

    if ((m_processResponseFd = open(m_processResponseFileName, O_WRONLY)) < 0) {
	cleanup();
	throw((std::string)"Failed to open process response FIFO");
    }
    
#ifdef AMT

    sprintf(tmpFileBase, "/tmp/rplugin_cal_%s",
	    fileIdentifiers.substr(36, 6).c_str());
    m_AMRequestFileName = strdup(tmpFileBase);

    if ((m_AMRequestFd = open(m_AMRequestFileName, O_RDWR)) < 0) {
	cleanup();
	throw((std::string)"Failed to open FIFO");
    }
    
    sprintf(tmpFileBase, "/tmp/rplugin_cap_%s",
	    fileIdentifiers.substr(42, 6).c_str());
    m_AMResponseFileName = strdup(tmpFileBase);

    if ((m_AMResponseFd = open(m_AMResponseFileName, O_WRONLY)) < 0) {
	cleanup();
	throw((std::string)"Failed to open FIFO");
    }

#endif

#ifdef AMT
sprintf(tmpFileBase, "/tmp/rplugin_shm_%s", fileIdentifiers.substr(48, 6).c_str());
#else
    sprintf(tmpFileBase, "/tmp/rplugin_shm_%s", fileIdentifiers.substr(36, 6).c_str());
#endif
    m_shmFileName = strdup(tmpFileBase);

    bool b = false;

    if ((m_shmFd = open(m_shmFileName, O_RDWR)) < 0) {
	tryWrite(m_controlResponseFd, &b, sizeof(bool));
	cleanup();
	throw((std::string)"Failed to open shared memory file 1");
    }
#ifdef AMT
        sprintf(tmpFileBase, "/tmp/rplugin_shn_%s", fileIdentifiers.substr(54, 6).c_str());

#else      
        sprintf(tmpFileBase, "/tmp/rplugin_shn_%s", fileIdentifiers.substr(42, 6).c_str());
#endif
    m_shmFileName2 = strdup(tmpFileBase);

    if ((m_shmFd2 = open(m_shmFileName2, O_RDWR)) < 0) {
	tryWrite(m_controlResponseFd, &b, sizeof(bool));
	cleanup();
	throw((std::string)"Failed to open shared memory file 2");
    }
#ifdef AMT
sprintf(tmpFileBase, "/tmp/rplugin_sho_%s", fileIdentifiers.substr(60, 6).c_str());
#else   
        sprintf(tmpFileBase, "/tmp/rplugin_sho_%s", fileIdentifiers.substr(48, 6).c_str());
#endif
    m_shmFileName3 = strdup(tmpFileBase);

    if ((m_shmFd3 = open(m_shmFileName3, O_RDWR)) < 0) {
	tryWrite(m_controlResponseFd, &b, sizeof(bool));
	cleanup();
	throw((std::string)"Failed to open shared memory file 3");
    }

     b = true;
     tryWrite(m_controlResponseFd, &b, sizeof(bool));

}


RemotePluginServer::~RemotePluginServer()
{
    cleanup();
}


void
RemotePluginServer::cleanup()
{
    if (m_shm) {
	munmap(m_shm, m_shmSize);
	m_shm = 0;
    }
   if (m_shm2) {
	munmap(m_shm2, m_shmSize2);
	m_shm2 = 0;
    }
   if (m_shm3) {
	munmap(m_shm3, m_shmSize3);
	m_shm3 = 0;
    }

    if (m_controlRequestFd >= 0) {
	close(m_controlRequestFd);
	m_controlRequestFd = -1;
    }
    if (m_controlResponseFd >= 0) {
	close(m_controlResponseFd);
	m_controlResponseFd = -1;
    }
    if (m_parRequestFd >= 0) {
	close(m_parRequestFd);
	m_parRequestFd = -1;
    }
    if (m_parResponseFd >= 0) {
	close(m_parResponseFd);
	m_parResponseFd = -1;
    }
    if (m_processFd >= 0) {
	close(m_processFd);
	m_processFd = -1;
    }
    if (m_processResponseFd >= 0) {
	close(m_processResponseFd);
	m_processResponseFd = -1;
    }

#ifdef AMT

    if (m_AMRequestFd >= 0) {
	close(m_AMRequestFd);
	m_AMRequestFd = -1;
    }
    if (m_AMResponseFd >= 0) {
	close(m_AMResponseFd);
	m_AMResponseFd = -1;
    }

#endif

    if (m_shmFd >= 0) {
	close(m_shmFd);
	m_shmFd = -1;
    }
    if (m_shmFd2 >= 0) {
	close(m_shmFd2);
	m_shmFd2 = -1;
    }
    if (m_shmFd3 >= 0) {
	close(m_shmFd3);
	m_shmFd3 = -1;
    }
    if (m_controlRequestFileName) {
	free(m_controlRequestFileName);
	m_controlRequestFileName = 0;
    }
    if (m_controlResponseFileName) {
	free(m_controlResponseFileName);
	m_controlResponseFileName = 0;
    }
    if (m_parRequestFileName) {
	free(m_parRequestFileName);
	m_parRequestFileName = 0;
    }
    if (m_parResponseFileName) {
	free(m_parResponseFileName);
	m_parResponseFileName = 0;
    }
    if (m_processFileName) {
	free(m_processFileName);
	m_processFileName = 0;
    }
    if (m_processResponseFileName) {
	free(m_processResponseFileName);
	m_processResponseFileName = 0;
    }

#ifdef AMT

    if (m_AMRequestFileName) {
	free(m_AMRequestFileName);
	m_AMRequestFileName = 0;
    }
    if (m_AMResponseFileName) {
	free(m_AMResponseFileName);
	m_AMResponseFileName = 0;
    }

#endif

    if (m_shmFileName) {
	free(m_shmFileName);
	m_shmFileName = 0;
    }

    if (m_shmFileName2) {
	free(m_shmFileName2);
	m_shmFileName2 = 0;
    }

    if (m_shmFileName3) {
	free(m_shmFileName3);
	m_shmFileName3 = 0;
    }
    
    delete m_inputs;
    m_inputs = 0;

    delete m_outputs;
    m_outputs = 0;
}

void
RemotePluginServer::sizeShm()
{

if(m_shm)
return;

//    size_t sz = (m_numInputs + m_numOutputs) * m_bufferSize * sizeof(float);
	size_t sz = FIXED_SHM_SIZE;
	
	size_t sz2 = 128000;
	
#ifdef AMT

	size_t sz3 = 128000;
	
#else

	size_t sz3 = 512;
	
#endif	
	
	m_shm = (char *)mmap(0, sz, PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd, 0);
    
    if (!m_shm) {
	std::cerr << "RemotePluginServer::sizeShm: ERROR: mmap or mremap for failed for " << sz
		  << " bytes from fd " << m_shmFd << "!" << std::endl;
	m_shmSize = 0;
    } else {
             memset(m_shm, 0, sz);
	     m_shmSize = sz;
	    }
       
	m_shm2 = (char *)mmap(0, sz2, PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd2, 0);
    
    if (!m_shm2) {
	std::cerr << "RemotePluginServer::sizeShm: ERROR: mmap or mremap for failed for " << sz2
		  << " bytes from fd " << m_shmFd2 << "!" << std::endl;
	m_shmSize2 = 0;
    } else {
         memset(m_shm2, 0, sz2);
	m_shmSize2 = sz2;

    }
	m_shm3 = (char *)mmap(0, sz3, PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd3, 0);
    
    if (!m_shm3) {
	std::cerr << "RemotePluginServer::sizeShm: ERROR: mmap or mremap for failed for " << sz3
		  << " bytes from fd " << m_shmFd3 << "!" << std::endl;
	m_shmSize3 = 0;
    } else {
        memset(m_shm3, 0, sz3);
	m_shmSize3 = sz3;
    }   
}    

void
RemotePluginServer::dispatch(int timeout)
{

/*

    struct pollfd pfd[2];
    
    pfd[0].fd = m_controlRequestFd;
    pfd[1].fd = m_processFd;
    pfd[0].events = pfd[1].events = POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;

    if (poll(pfd, 2, timeout) < 0) {
	throw RemotePluginClosedException();
    }
    
    if ((pfd[0].revents & POLLIN) || (pfd[0].revents & POLLPRI)) {
	dispatchControl();
    } else if (pfd[1].revents) {
	throw RemotePluginClosedException();
    }
    
    if ((pfd[1].revents & POLLIN) || (pfd[1].revents & POLLPRI)) {
	dispatchProcess();
    } else if (pfd[1].revents) {
	throw RemotePluginClosedException();
    }

*/

}

void
RemotePluginServer::dispatchControl(int timeout)
{
 //#if 0
    struct pollfd pfd;
  //  printf("in dispatchcontrol event\n");    
    pfd.fd = m_controlRequestFd;
    pfd.events = POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;

    if (poll(&pfd, 1, timeout) < 0) {
	throw RemotePluginClosedException();
    }
  //  printf("dispatchin control event\n");    
    if ((pfd.revents & POLLIN) || (pfd.revents & POLLPRI)) {
	dispatchControlEvents();
    } else if (pfd.revents) {
	throw RemotePluginClosedException();
    }
/*
#else
    int n;
    fd_set rfds, ofds;
    timeval timeo = {0,timeout * 1000};
    FD_ZERO(&rfds);
    FD_SET(m_controlRequestFd, &rfds);
    FD_ZERO(&ofds);
    if ((n = select(m_controlRequestFd+1, &rfds, &ofds, &ofds, &timeo)) == -1) {
	throw RemotePluginClosedException();
    }
    if (n == 1) {
	//	printf("got a control select\n");
	dispatchControlEvents();
    }
#endif
*/
}


void
RemotePluginServer::dispatchPar(int timeout)
{
 //#if 0
    struct pollfd pfd;
  //  printf("in dispatchcontrol event\n");    
    pfd.fd = m_parRequestFd;
    pfd.events = POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;

    if (poll(&pfd, 1, timeout) < 0) {
	throw RemotePluginClosedException();
    }
  //  printf("dispatchin control event\n");    
    if ((pfd.revents & POLLIN) || (pfd.revents & POLLPRI)) {
	dispatchParEvents();
    } else if (pfd.revents) {
	throw RemotePluginClosedException();
    }
/*
#else
    int n;
    fd_set rfds, ofds;
    timeval timeo = {0,timeout * 1000};
    FD_ZERO(&rfds);
    FD_SET(m_parRequestFd, &rfds);
    FD_ZERO(&ofds);
    if ((n = select(m_parRequestFd+1, &rfds, &ofds, &ofds, &timeo)) == -1) {
	throw RemotePluginClosedException();
    }
    if (n == 1) {
	//	printf("got a control select\n");
	dispatchParEvents();
    }
#endif
*/
}

void
RemotePluginServer::dispatchProcess(int timeout)
{
 //#if 0
    struct pollfd pfd;
  //  printf("in dispatchcontrol event\n");    
    pfd.fd = m_processFd;
    pfd.events = POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;

    if (poll(&pfd, 1, timeout) < 0) {
	throw RemotePluginClosedException();
    }
  //  printf("dispatchin control event\n");    
    if ((pfd.revents & POLLIN) || (pfd.revents & POLLPRI)) {
	dispatchProcessEvents();
    } else if (pfd.revents) {
	throw RemotePluginClosedException();
    }
/*
#else
    int n;
    fd_set rfds, ofds;
    timeval timeo = {0,timeout * 1000};
    FD_ZERO(&rfds);
    FD_SET(m_processFd, &rfds);
    FD_ZERO(&ofds);
    if ((n = select(m_processRequestFd+1, &rfds, &ofds, &ofds, &timeo)) == -1) {
	throw RemotePluginClosedException();
    }
    if (n == 1) {
	//	printf("got a control select\n");
	dispatchProcessEvents();
    }
#endif
*/


	// just block in dispatchProcessEvents
	// dispatchProcessEvents();
}

void
RemotePluginServer::dispatchProcessEvents()
{    
    RemotePluginOpcode opcode = RemotePluginNoOpcode;
   
        tryRead(m_processFd, &opcode, sizeof(RemotePluginOpcode));
/*
    if (read(m_processFd, &opcode, sizeof(RemotePluginOpcode)) != 
	sizeof(RemotePluginOpcode)) {
	std::cerr << "ERROR: RemotePluginServer: couldn't read opcode" << std::endl;
	throw RemotePluginClosedException();
	return;
    }
*/
    //    std::cerr << "read process opcode: " << opcode << std::endl;

    switch (opcode) {

    case RemotePluginProcess:
    {

	int sampleFrames = readInt(m_processFd);

	if (m_bufferSize < 0) {
           writeInt(m_processResponseFd, 100);
	    return;
	}
	if (m_numInputs < 0) {
           writeInt(m_processResponseFd, 100);
	    return;
	}
	if (m_numOutputs < 0) {
            writeInt(m_processResponseFd, 100);
	    return;
	}
	if (!m_shm) {
	    sizeShm();
	    if (!m_shm) {
            writeInt(m_processResponseFd, 100);
		return;
	    }
	}

	size_t blocksz = m_bufferSize * sizeof(float);

	for (int i = 0; i < m_numInputs; ++i) {
	    m_inputs[i] = (float *)(m_shm + i * blocksz);
	}
	for (int i = 0; i < m_numOutputs; ++i) {
	    m_outputs[i] = (float *)(m_shm + (i + m_numInputs) * blocksz);
	}

       	process(m_inputs, m_outputs, sampleFrames);

	writeInt(m_processResponseFd, 100);

        }

    	break;

	case RemotePluginProcessEvents:
	{        
        if (!m_shm2) {
        sizeShm();
        if (!m_shm2)
        return;
        }
	processVstEvents();
	}
        break;
	
     case RemotePluginGetInputCount:
       {

	if (!m_shm) 
	sizeShm();

        int numin = getInputCount();
	// m_numInputs = getInputCount();
 
      if(numin != m_numInputs)
       {
       if(m_inputs)
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

       if(numout != m_numOutputs)
       {
       if(m_outputs)
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

    default:
    
	std::cerr << "WARNING: RemotePluginServer::dispatchProcessEvents: unexpected opcode "
		  << opcode << std::endl;
    }
    //    std::cerr << "dispatched process event\n";
}



void
RemotePluginServer::dispatchParEvents()
{    
    RemotePluginOpcode opcode = RemotePluginNoOpcode;

    tryRead(m_parRequestFd, &opcode, sizeof(RemotePluginOpcode));
    //    std::cerr << "control opcoded " << opcode << std::endl;

    switch (opcode) {

   case RemotePluginGetParameter:
	writeFloat(m_parResponseFd, getParameter(readInt(m_parRequestFd)));
	break;

   case RemotePluginSetParameter:
    {
	int pn(readInt(m_parRequestFd));
	setParameter(pn, readFloat(m_parRequestFd));
	writeInt(m_parResponseFd, 1);
	break;
    }

   default:
	std::cerr << "WARNING: RemotePluginServer::dispatchParEvents: unexpected opcode "
		  << opcode << std::endl;

                 }
  }


void
RemotePluginServer::dispatchControlEvents()
{    
    RemotePluginOpcode opcode = RemotePluginNoOpcode;
    static float *parameterBuffer = 0;

    tryRead(m_controlRequestFd, &opcode, sizeof(RemotePluginOpcode));
    //    std::cerr << "control opcoded " << opcode << std::endl;

    switch (opcode) {

    case RemotePluginSetCurrentProgram:
	setCurrentProgram(readInt(m_controlRequestFd));
	break;


    case RemotePluginSetBufferSize:
    {
	int newSize = readInt(m_controlRequestFd);
	setBufferSize(newSize);
	m_bufferSize = newSize;
	break;
    }

    case RemotePluginSetSampleRate:
	setSampleRate(readInt(m_controlRequestFd));
	break;
    
    case RemotePluginReset:
	reset();
	break;

    case RemotePluginGetVersion:
	writeFloat(m_controlResponseFd, getVersion());
	break;

    case RemotePluginGetName:
	writeString(m_controlResponseFd, getName());
	break;

    case RemotePluginGetMaker:
	writeString(m_controlResponseFd, getMaker());
	break;
    
    case RemotePluginTerminate:
	terminate();
	break;
           
    case RemotePluginGetFlags:
        m_flags = getFlags();
	writeInt(m_controlResponseFd, m_flags);
	break;

    case RemotePluginGetinitialDelay:
       m_delay = getinitialDelay();
	writeInt(m_controlResponseFd, m_delay);
        break;

    case RemotePluginGetParameterCount:
	writeInt(m_controlResponseFd, getParameterCount());
	break;
	
    case RemotePluginGetParameterName:
	writeString(m_controlResponseFd, getParameterName(readInt(m_controlRequestFd)));
	break;
        
    case RemotePluginGetParameterDefault:
	writeFloat(m_controlResponseFd, getParameterDefault(readInt(m_controlRequestFd)));
	break;

    case RemotePluginGetParameters:
    {
	if (!parameterBuffer) {
	    parameterBuffer = new float[getParameterCount()];
	}
 	int p0 = readInt(m_controlRequestFd);
	int pn = readInt(m_controlRequestFd);
	getParameters(p0, pn, parameterBuffer);
	tryWrite(m_controlResponseFd, parameterBuffer, (pn - p0 + 1) * sizeof(float));
	break;
    }
      
    case RemotePluginGetProgramCount:
	writeInt(m_controlResponseFd, getProgramCount());
	break;

    case RemotePluginGetProgramName:
	writeString(m_controlResponseFd, getProgramName(readInt(m_controlRequestFd)));
	break;

    case RemotePluginSetDebugLevel:
    {
	RemotePluginDebugLevel newLevel = m_debugLevel;
	tryRead(m_controlRequestFd, &newLevel, sizeof(RemotePluginDebugLevel));
	setDebugLevel(newLevel);
	m_debugLevel = newLevel;
	break;
    }

    case RemotePluginWarn:
    {
	bool b = warn(readString(m_controlRequestFd));
	tryWrite(m_controlResponseFd, &b, sizeof(bool));
	break;
    }

    case RemotePluginShowGUI:
    {
	showGUI();
	break;
    }

    case RemotePluginHideGUI:
    {
	hideGUI();
	break;
    }

#ifdef EMBED

    case RemotePluginOpenGUI:
    {
	openGUI();
	break;
    }

#endif

    case RemotePluginNoOpcode:
	break;

    case RemotePluginGetEffInt:
    {
	int opcode = readInt(m_controlRequestFd);
	writeInt(m_controlResponseFd, getEffInt(opcode));
	break;
    }

    case RemotePluginDoVoid:
    {
	int opcode = readInt(m_controlRequestFd);
	effDoVoid(opcode);
	break;
    }

    case RemotePluginGetEffString:
    {
	int opcode = readInt(m_controlRequestFd);
	int idx = readInt(m_controlRequestFd);
	writeString(m_controlResponseFd, getEffString(opcode, idx));
	break;
    }

  
  /*
  case RemoteMainsChanged:
    {
	int v = readInt(m_controlRequestFd);
	//std::cerr << "Mains changing " << v << std::endl;
	eff_mainsChanged(v);
	writeInt(m_controlResponseFd, 1);
	break;
    }
    
    */

	
	case RemotePluginGetChunk:
	{
		getChunk();
		break;
	}	
 
	case RemotePluginSetChunk:
	{
		setChunk();
		break;
	}	

/*
	case RemotePluginCanBeAutomated:
	{
		canBeAutomated();
		break;
	}	

*/

	case RemotePluginGetProgram:
	{
            getProgram();
		break;
	}	
	
    default:
	std::cerr << "WARNING: RemotePluginServer::dispatchControlEvents: unexpected opcode "
		  << opcode << std::endl;
    }
    //    std::cerr << "done dispatching control\n";
}

