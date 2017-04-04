# LinVst

-----

LinVst is a Linux vst plugin that runs Windows vst's.

To make

The plugininterfaces folder contained within the VST2_SDK folder, needs to be placed in the LinVst files folder. https://www.steinberg.net/en/company/developers.html

Wine libwine development files https://packages.debian.org/sid/libwine-development-dev

-----

make clean

make

sudo make install

Installs lin-vst-server.exe and lin-vst-server.exe.so to /usr/bin and installs linvst.so to /usr/lib/vst

Use Makefile-embed for host embedded window option (experimental)

-------

linvst.so needs to be renamed to the windows vst name (without the .dll extension) and the windows vst and the renamed linvst.so need to be in the same folder/directory.

So, for example if linvst.so is renamed to test.so then it will run test.dll when test.so is loaded within a Linux VST capable DAW.

linvst.so is a Linux vst template that loads and runs any windows vst that linvst.so is renamed to.

linvst.so can be copied and renamed multiple times to load multiple windows vst's.

Basically, linvst.so becomes the windows vst once it's renamed to the windows vst's name and can then be used in Linux vst hosts.

------

Tested with Wine 2.1 devel

Tested windows vst's

Kontakt Player 5.6 (turn multiprocessing off)

(additional dll's might be needed msvcp140.dll
concrt140.dll
api-ms-win-crt-time-l1-1-0.dll
api-ms-win-crt-runtime-l1-1-0.dll
ucrtbase.dll)

Melda MXXX (turn GPU acceleration off)

u-he Podolski

Obxd Synth

Ignite Amps TPA-1




