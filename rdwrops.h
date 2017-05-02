/*
  dssi-vst: a DSSI plugin wrapper for VST effects and instruments
  Copyright 2004-2007 Chris Cannam
*/

#ifndef _RD_WR_OPS_H_

#include <string>
#include "remoteplugin.h"

extern void rdwr_tryWrite2(int fd, const void *buf, size_t count, const char *file, int line);
extern void rdwr_tryRead(int fd, void *buf, size_t count, const char *file, int line);
extern void rdwr_tryWrite(int fd, const void *buf, size_t count, const char *file, int line);
extern void rdwr_writeOpcode(int fd, RemotePluginOpcode opcode, const char *file, int line);
extern void rdwr_writeString(int fd, const std::string &str, const char *file, int line);
extern std::string rdwr_readString(int fd, const char *file, int line);
extern void rdwr_writeInt(int fd, int i, const char *file, int line);
extern int rdwr_readInt(int fd, const char *file, int line);
extern void rdwr_writeFloat(int fd, float f, const char *file, int line);
extern float rdwr_readFloat(int fd, const char *file, int line);

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
