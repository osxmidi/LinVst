#!/usr/bin/make -f
# Makefile for LinVst #



CXX     = g++
WINECXX = wineg++

CXX_FLAGS =

PREFIX  = /usr

BIN_DIR    = $(DESTDIR)$(PREFIX)/bin
VST_DIR = ./vst

BUILD_FLAGS = -fPIC -O2 -DLVRT -DVST6432 -DEMBED -DEMBEDDRAG -DWAVES -DTRACKTIONWM -DVESTIGE -DNEWTIME -DINOUTMEM -DCHUNKBUF -DEMBEDRESIZE -DPCACHE -DBIN_DIR='"$(BIN_DIR)"' $(CXX_FLAGS)
# add -DNOFOCUS to the above line for alternative keyboard/mouse focus operation, add -DEMBEDRESIZE to the above line for window resizing
BUILD_FLAGS_WIN = -m64 -O2 -DVST6432 -DEMBED -DEMBEDDRAG -DWAVES -DTRACKTIONWM -DVESTIGE -DNEWTIME -DINOUTMEM -DCHUNKBUF -DEMBEDRESIZE -DPCACHE -DBIN_DIR='"$(BIN_DIR)"' -I/usr/include/wine-development/windows -I/usr/include/wine-development/wine/windows -I/usr/include/wine/wine/windows
# add -DEMBEDRESIZE to the above line for window resizing
BUILD_FLAGS_WIN32 = -m32 -O2 -DVST6432 -DEMBED -DEMBEDDRAG -DWAVES -DTRACKTIONWM -DVESTIGE -DWCLASS -DNEWTIME -DINOUTMEM -DCHUNKBUF -DEMBEDRESIZE -DPCACHE -DBIN_DIR='"$(BIN_DIR)"' -I/usr/include/wine-development/windows -I/usr/include/wine-development/wine/windows -I/usr/include/wine/wine/windows
# add -DEMBEDRESIZE to the above line for window resizing

LINK_FLAGS   = $(LDFLAGS)

LINK_PLUGIN = -shared -lpthread -ldl -lX11 -lrt $(LINK_FLAGS)
LINK_WINE = -L/opt/wine-stable/lib64/wine -L/opt/wine-devel/lib64/wine -L/opt/wine-staging/lib64/wine -L/opt/wine-stable/lib64/wine/x86_64-unix -L/opt/wine-devel/lib64/wine/x86_64-unix -L/opt/wine-staging/lib64/wine/x86_64-unix -L/usr/lib/x86_64-linux-gnu/wine-development -lpthread -lX11 -lrt -lshell32  -lole32 $(LINK_FLAGS)
LINK_WINE32 = -L/opt/wine-stable/lib/wine -L/opt/wine-devel/lib/wine -L/opt/wine-staging/lib/wine -L/opt/wine-stable/lib/wine/i386-unix -L/opt/wine-devel/lib/wine/i386-unix -L/opt/wine-staging/lib/wine/i386-unix -L/usr/lib/i386-linux-gnu/wine-development -lpthread -lX11 -lrt -lshell32 -lole32 $(LINK_FLAGS)

PATH_TO_FILE = /usr/include/bits

TARGETS     = linvst.so lin-vst-servertrack.exe lin-vst-servertrack32.exe

PATH := $(PATH):/opt/wine-stable/bin:/opt/wine-devel/bin:/opt/wine-staging/bin

# --------------------------------------------------------------

all: $(TARGETS)


linvst.so: linvst.unix.o remotevstclient.unix.o remotepluginclient.unix.o paths.unix.o
	$(CXX) $^ $(LINK_PLUGIN) -o $@
	
lin-vst-servertrack.exe: lin-vst-server.wine.o remotepluginserver.wine.o paths.wine.o
	$(WINECXX) -m64 $^ $(LINK_WINE) -o $@

ifneq ("$(wildcard $(PATH_TO_FILE))","")
lin-vst-servertrack32.exe: lin-vst-server.wine32.o remotepluginserver.wine32.o paths.wine32.o
	$(WINECXX) -m32 $^ $(LINK_WINE32) -o $@
else
lin-vst-servertrack32.exe: 
endif

# --------------------------------------------------------------

linvst.unix.o: linvst.cpp
	$(CXX) $(BUILD_FLAGS) -c $^ -o $@
	
remotevstclient.unix.o: remotevstclient.cpp
	$(CXX) $(BUILD_FLAGS) -c $^ -o $@
	
remotepluginclient.unix.o: remotepluginclient.cpp
	$(CXX) $(BUILD_FLAGS) -c $^ -o $@

paths.unix.o: paths.cpp
	$(CXX) $(BUILD_FLAGS) -c $^ -o $@


# --------------------------------------------------------------

lin-vst-server.wine.o: lin-vst-server.cpp
	$(WINECXX) $(BUILD_FLAGS_WIN) -c $^ -o $@

remotepluginserver.wine.o: remotepluginserver.cpp
	$(WINECXX) $(BUILD_FLAGS_WIN) -c $^ -o $@

paths.wine.o: paths.cpp
	$(WINECXX) $(BUILD_FLAGS_WIN) -c $^ -o $@

lin-vst-server.wine32.o: lin-vst-server.cpp
	$(WINECXX) $(BUILD_FLAGS_WIN32) -c $^ -o $@

remotepluginserver.wine32.o: remotepluginserver.cpp
	$(WINECXX) $(BUILD_FLAGS_WIN32) -c $^ -o $@

paths.wine32.o: paths.cpp
	$(WINECXX) $(BUILD_FLAGS_WIN32) -c $^ -o $@



clean:
	rm -fR *.o *.exe *.so vst $(TARGETS)

install:
	install -d $(BIN_DIR)
	install -d $(VST_DIR)
	install -m 755 linvst.so $(VST_DIR)
	install -m 755 lin-vst-servertrack.exe lin-vst-servertrack.exe.so $(BIN_DIR)
    ifneq ("$(wildcard $(PATH_TO_FILE))","")
	install -m 755 lin-vst-servertrack32.exe lin-vst-servertrack32.exe.so $(BIN_DIR)
        endif
