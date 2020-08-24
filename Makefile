#!/usr/bin/make -f
# Makefile for LinVst #

CXX     = g++
WINECXX = wineg++

CXX_FLAGS =

PREFIX  = /usr

BIN_DIR    = $(DESTDIR)$(PREFIX)/bin
VST_DIR = ./vst

BUILD_FLAGS  = -fPIC -O2 -DLVRT -DEMBED -DEMBEDDRAG -DWAVES -DTRACKTIONWM -DVESTIGE -DXECLOSE -DNEWTIME -DINOUTMEM -DCHUNKBUF $(CXX_FLAGS)
# add -DNOFOCUS to the above line for alternative keyboard/mouse focus operation, add -DEMBEDRESIZE to the above line for window resizing
BUILD_FLAGS_WIN = -m64 -O2 -DEMBED -DEMBEDDRAG -DWAVES -DTRACKTIONWM -DVESTIGE -DXECLOSE -DWCLASS -DNEWTIME -DINOUTMEM -DCHUNKBUF -I/usr/include/wine-development/windows -I/usr/include/wine-development/wine/windows -I/usr/include/wine/wine/windows
# add -DEMBEDRESIZE to the above line for window resizing

LINK_FLAGS   = $(LDFLAGS)

LINK_PLUGIN = -shared -lpthread -ldl -lX11 -lrt $(LINK_FLAGS)
LINK_WINE   = -L/opt/wine-stable/lib64/wine -L/opt/wine-devel/lib64/wine -L/opt/wine-staging/lib64/wine -L/usr/lib/x86_64-linux-gnu/wine-development -lpthread -lrt $(LINK_FLAGS)

TARGETS     = linvst.so lin-vst-servertrack.exe

# --------------------------------------------------------------

all: $(TARGETS)

linvst.so: linvst.unix.o remotevstclient.unix.o remotepluginclient.unix.o paths.unix.o
	$(CXX) $^ $(LINK_PLUGIN) -o $@
	
lin-vst-servertrack.exe: lin-vst-server.wine.o remotepluginserver.wine.o paths.wine.o
	$(WINECXX) $^ $(LINK_WINE) -o $@

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


clean:
	rm -fR *.o *.exe *.so vst $(TARGETS)

install:
	install -d $(BIN_DIR)
	install -d $(VST_DIR)
	install -m 755 linvst.so $(VST_DIR)
	install -m 755 lin-vst-servertrack.exe lin-vst-servertrack.exe.so $(BIN_DIR)
