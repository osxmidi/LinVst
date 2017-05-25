# LinVst

-----

LinVst enables Windows vst's to be used as Linux vst's in Linux vst capable daws.

-------

How to use LinVst

To use LinVst, the linvst.so file simply needs to be renamed to match the windows vst dll's filename.

(the .so and .dll extensions are left as they are and are not part of the renaming)

Both the windows vst dll file and the renamed linvst.so file are then a matched pair and need to be kept together in the same directory/folder.

For Example

copy lin-vst-server.exe and lin-vst-server.so to /usr/bin

copy linvst.so to wherever the windows vst dll file is that someone wants to run in a Linux DAW, 
and then rename linvst.so to the windows vst dll filename 

linvst.so gets renamed to test.so for test.dll

Use the renamed file (test.so) in a Linux VST capable DAW 

Load test.so within the Linux DAW and then test.so will load and run the (name associated) test.dll windows vst 

linvstconvert can automatically batch rename linvst.so to mutiple windows vst dll names that are in a folder/directory.

Copy linvstconvert and linvst.so to the folder/directory containing the windows vst dll's and change into that folder/directory and run linvstconvert in that folder/directory (sudo permission might be needed for some folders/directories).

After the naming conversion, the newly created files (.so files) are ready to be used in Linux vst DAW's from the folder that was used for the naming conversion.

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
 
--------

A 32 bit WINEPREFIX is needed to enable 32 bit vst's on Linux 64 bit systems https://wiki.winehq.org/FAQ#How_do_I_create_a_32_bit_wineprefix_on_a_64_bit_system.3F

Include and Library paths might need to be changed in the Makefile for various 64 bit and 32 bit Wine development path locations (otherwise 32 bit compiles might try to link with 64 bit libraries etc).


For LinVst32

MakefileVst32 or Makefile-embedVst32

Additional 32 bit development libraries are probably needed.

On Ubuntu/Debian 64 bits

maybe 

sudo dpkg --add-architecture i386

sudo apt-get install libc6-dev-i386

sudo apt-get install gcc-multilib g++-multilib

sudo apt-get install libwine-development-dev:i386

export WINEPREFIX=/home/user/prefix32

export WINEARCH=win32

--------

sudo make clean

make

sudo make install

Installs lin-vst-server.exe and lin-vst-server.exe.so to /usr/bin and installs linvst.so to /vst in the source code folder

(also installs lin-vst-server32.exe and lin-vst-server32.exe.so to /usr/bin for 32 bit vst support)

Makefile-embed6432 and Makefile-6432 build a LinVst version that autodetects and automatically runs both 64 bit vst's and 32 bit vst's.

Use Makefile-embed6432 for host embedded window option (experimental)

------

LinVst 

Tested with Linux Tracktion 7, Linux Ardour 5.6, Linux Bitwig Studio 2, Linux Reaper 5.4

Tested with Wine 2.1 devel

Turning off the vst's multiprocessor support and/or GPU acceleration might help in some cases, due to what features Wine currently supports (Wine version dependent).

Some plugins might use wininet for internet connections (online registration, online help, etc) which might cause problems depending on Wines implementation.

Winetricks wininet might help.

Additional dll's might have to be added to Wine for some plugins.

On some slower systems Wine can initially take a long time to load properly when Wine is first used, which might cause a LinVst crash.
The solution is to initialise Wine first by running winecfg or any other Wine based program, so that Wine has been initialised before LinVst is used.

Upgrading to the latest wine-stable version is recommended.

Winetricks might help with some plugins.

------

Tested windows vst's

Kontakt Player 5.6.8 (turn multiprocessing off). Requires Wine 2.0 and above

Some additional dll overrides (below) might be needed for Kontakt and Wine 2.0.
Kontakt and Wine 2.8 staging only need an additional msvcp140.dll override. 
To override dll's, copy windows dlls to drive_c/windows/system32 and then override the dlls to be native using the winecfg Libraries option.

msvcp140.dll
concrt140.dll
api-ms-win-crt-time-l1-1-0.dll
api-ms-win-crt-runtime-l1-1-0.dll
ucrtbase.dll

Guitar Rig 5

Reaktor 6

Melda MXXX Multi Effects (turn GPU acceleration off)

u-he Podolski Synth

u-he Protoverb Reverb

Obxd Synth

Ignite Amps TPA-1 Amp Sim

LePou Amp Sims

Nick Crow Lab Amp Sims

Voxengo Boogex Guitar Effects

Klanghelm MJUC Compressor

TDR SlickEQ mixing/mastering equalizer 
