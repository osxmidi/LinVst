#!/usr/bin/make -f
# Makefile for LinVst #

CXX     = g++
WINECXX = wineg++

CXX_FLAGS =

BUILD_FLAGS_WIN = -fPIC -m64 -DVESTIGE -I/usr/include/wine-development/windows -I/usr/include/wine-development/wine/windows -I/usr/include/wine/wine/windows

LINK_FLAGS   = $(LDFLAGS)

LINK_WINE   = -L/opt/wine-stable/lib64/wine -L/opt/wine-devel/lib64/wine -L/opt/wine-staging/lib64/wine -L/usr/lib/x86_64-linux-gnu/wine-development -lpthread -lrt -lshell32 $(LINK_FLAGS)

TARGETS     = testvst.exe

# --------------------------------------------------------------

all: $(TARGETS)

testvst.exe: testvst.wine.o
	$(WINECXX) -m64 $^ $(LINK_WINE) -o $@

# --------------------------------------------------------------

testvst.wine.o: testvst.cpp
	$(WINECXX) $(BUILD_FLAGS_WIN) -c $^ -o $@

 
