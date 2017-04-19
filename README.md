# LinVst

-----

LinVst is a Linux vst plugin that runs Windows 64 bit and 32 bit vst's.

-------

How to use LinVst

To use LinVst, the linvst.so file simply needs to be renamed to match the windows vst dll's filename.

(the .so and .dll extensions are left as they are and are not part of the renaming)

Both the windows vst dll file and the renamed linvst.so file need to be in the same directory/folder

For Example

copy lin-vst-server.exe and lin-vst-server.so to /usr/bin

copy linvst.so to wherever the windows vst dll file is that someone wants to run in a Linux DAW, 
and then rename linvst.so to the windows vst dll filename 

linvst.so gets renamed to test.so for test.dll

Use the renamed file (test.so) in a Linux VST capable DAW 

Load test.so within the Linux DAW and then test.so will load and run the (name associated) test.dll windows vst 

--------

linvst.so needs to be renamed to the windows vst name (the .so and .dll extensions are left as they are and are not part of the renaming).

Both the windows vst dll file and the renamed linvst.so file need to be in the same folder/directory.

linvst.so is a Linux vst template that loads and runs any windows vst that linvst.so is renamed to.

linvst.so can be copied and renamed multiple times to load multiple windows vst's.

Basically, linvst.so becomes the windows vst once it's renamed to the windows vst's name and can then be used in Linux vst hosts.

--------

To make

The plugininterfaces folder contained within the VST2_SDK folder, needs to be placed in the LinVst files folder. https://www.steinberg.net/en/company/developers.html

Wine libwine development files.

For Ubuntu/Debian, sudo apt-get install libwine-development-dev

wine-devel packages for other distros.

libX11 development needed for embedded version.

--

For LinVst32

MakefileVst32 or Makefile-embedVst32

Additional 32 bit development libraries are probably needed.

On Ubuntu 64 bits

sudo apt-get install libc6-dev-i386

sudo apt-get install gcc-multilib g++-multilib

sudo apt-get install wine-devel-dev:i386

export WINEPREFIX=/home/user/prefix32

export WINEARCH=win32

--------

sudo make clean

make

sudo make install

Installs lin-vst-server.exe and lin-vst-server.exe.so to /usr/bin and installs linvst.so to /vst in the source code folder

Use Makefile-embed for host embedded window option (experimental)

------

LinVst 

Tested with Linux Tracktion 7, Linux Ardour 5.6, Linux Bitwig Studio 2, Linux Reaper 5.4

Tested with Wine 2.1 devel

Turning off the vst's multiprocessor support and/or GPU acceleration might help in some cases, due to what features Wine currently supports (Wine version dependent).

On some slower systems Wine can initially take a long time to load properly when Wine is first used, which might cause a LinVst crash.
The solution is to initialise Wine first by running winecfg or any other Wine based program, so that Wine has been initialised before LinVst is used.

Tested windows vst's

Kontakt Player 5.6 (turn multiprocessing off). Requires Wine 2.1 and above

(additional dll's might be needed 
msvcp140.dll
concrt140.dll
api-ms-win-crt-time-l1-1-0.dll
api-ms-win-crt-runtime-l1-1-0.dll
ucrtbase.dll)

Melda MXXX (turn GPU acceleration off)

u-he Podolski

Obxd Synth

Ignite Amps TPA-1




