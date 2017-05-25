/*
  dssi-vst: a DSSI plugin wrapper for VST effects and instruments
  Copyright 2004-2007 Chris Cannam
*/

#ifndef _PATHS_H_
#define _PATHS_H_

#include <string>
#include <vector>

class Paths
{
public:
    std::vector<std::string> getPath(std::string envVar,
					    std::string deflt,
					    std::string defltHomeRelPath);
};

#endif
