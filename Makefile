#!/usr/bin/make -f
# Makefile for LinVst #

CXX     = g++
WINECXX = wineg++

CXX_FLAGS =

PREFIX  = /usr

BIN_DIR    = $(DESTDIR)$(PREFIX)/bin
VST_DIR = $(DESTDIR)$(PREFIX)/lib/vst

BUILD_FLAGS  = -fPIC -O2 -DAMT $(CXX_FLAGS)
BUILD_FLAGS_WIN = -m64 -O2 -DAMT -I/usr/include/wine-development/windows

LINK_FLAGS   = $(LDFLAGS)

LINK_PLUGIN = -shared -lpthread -ldl $(LINK_FLAGS)
LINK_WINE   = -lpthread  $(LINK_FLAGS)

TARGETS     = linvst.so lin-vst-server.exe

# --------------------------------------------------------------

all: $(TARGETS)

linvst.so: linvst.unix.o remotevstclient.unix.o remotepluginclient.unix.o rdwrops.unix.o paths.unix.o
	$(CXX) $^ $(LINK_PLUGIN) -o $@
	
lin-vst-server.exe: lin-vst-server.wine.o remotepluginserver.wine.o rdwrops.wine.o paths.wine.o
	$(WINECXX) $^ $(LINK_WINE) -o $@

# --------------------------------------------------------------

linvst.unix.o: linvst.cpp
	$(CXX) $(BUILD_FLAGS) -c $^ -o $@
	
remotevstclient.unix.o: remotevstclient.cpp
	$(CXX) $(BUILD_FLAGS) -c $^ -o $@
	
remotepluginclient.unix.o: remotepluginclient.cpp
	$(CXX) $(BUILD_FLAGS) -c $^ -o $@

rdwrops.unix.o: rdwrops.cpp
	$(CXX) $(BUILD_FLAGS) -c $^ -o $@

paths.unix.o: paths.cpp
	$(CXX) $(BUILD_FLAGS) -c $^ -o $@


# --------------------------------------------------------------

lin-vst-server.wine.o: lin-vst-server.cpp
	$(WINECXX) $(BUILD_FLAGS_WIN) -c $^ -o $@

remotepluginserver.wine.o: remotepluginserver.cpp
	$(WINECXX) $(BUILD_FLAGS_WIN) -c $^ -o $@

rdwrops.wine.o: rdwrops.cpp
	$(WINECXX) $(BUILD_FLAGS_WIN) -c $^ -o $@

paths.wine.o: paths.cpp
	$(WINECXX) $(BUILD_FLAGS_WIN) -c $^ -o $@


clean:
	rm -f *.o *.exe *.so $(TARGETS)

install:
	install -d $(BIN_DIR)
	install -d $(VST_DIR)
	install -m 755 linvst.so $(VST_DIR)
	install -m 755 lin-vst-server.exe lin-vst-server.exe.so $(BIN_DIR)
