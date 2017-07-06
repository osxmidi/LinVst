/*
vst: a DSSI plugin wrapper for VST effects and instruments
  Copyright 2004-2007 Chris Cannam
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "paths.h"
#include <iostream>

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
