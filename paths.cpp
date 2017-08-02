/*
vst: a DSSI plugin wrapper for VST effects and instruments
  Copyright 2004-2007 Chris Cannam
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "paths.h"
#include <iostream>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>


std::vector<std::string>
Paths::getPath(std::string envVar, std::string deflt, std::string defltHomeRelPath)
{
    std::vector<std::string> pathList;
    std::string path;

    char *cpath = getenv(envVar.c_str());
    if (cpath) path = cpath;

    if (path == "") {
	path = deflt;
	char *home = getenv("HOME");
	if (home && (defltHomeRelPath != "")) {
	    path = std::string(home) + defltHomeRelPath + ":" + path;
	}
	std::cerr << envVar << " not set, defaulting to " << path << std::endl;
    }

    std::string::size_type index = 0, newindex = 0;

    while ((newindex = path.find(':', index)) < path.size()) {
	pathList.push_back(path.substr(index, newindex - index));
	index = newindex + 1;
    }
    
    pathList.push_back(path.substr(index));

    return pathList;
}

// Behaves like mkstemp, but for shared memory.
int shm_mkstemp(char *fileBase)
{
    const char charSet[] = "abcdefghijklmnopqrstuvwxyz"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "0123456789";
    int size = strlen(fileBase);
    if (size < 6) {
        errno = EINVAL;
        return -1;
    }

    if (strcmp(fileBase + size - 6, "XXXXXX") != 0) {
        errno = EINVAL;
        return -1;
    }

    while (1) {
        for (int c = size - 6; c < size; c++) {
            // Note the -1 to avoid the trailing '\0' in charSet.
            fileBase[c] = charSet[rand() % (sizeof(charSet) - 1)];
        }

        int fd = shm_open(fileBase, O_RDWR | O_CREAT | O_EXCL, 0660);
        if (fd >= 0) {
            return fd;
        } else if (errno != EEXIST) { 
            return -1;
        }
    }
}
