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

int shm_mkstemp(char *fileBase);


/**
 * This function provided the full path to a binary provided the basename of
 * the file and the extensions. It may consider enviromental variables and
 * if some enabled macros.
 *
 * \param basename base name of the file
 * \param suffix file suffix before extension
 * \param ext extension of the file
 * \return string containing a complete path to the file.
 */
std::string getBinaryPath(std::string basename, std::string suffix,
                          std::string ext);

/**
 * Generate a list of full paths of the binaries combining all the elements in
 * the input lists.
 *
 * \param basenames list of base name of the file
 * \param suffixes list of file suffix before extension
 * \param extensions list of extensions of the file
 * \return string containing a complete path to the file.
 */
std::vector<std::string>
getCombinedBinaryPaths(std::vector<std::string> basenames,
                       std::vector<std::string> suffixes,
                       std::vector<std::string> extensions);

#endif
