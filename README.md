# LinVst

LinVst is a Linux vst plugin that loads Windows vst's.

To make

The plugininterfaces folderfrom the VST2.4 SDK needs to be in the LinVst files folder.

make clean

make

sudo make install

Installs lin-vst-server.exe and lin-vst-server.exe.so to /usr/bin and installs linvst.so to /usr/lib/vst

linvst.so needs to be renamed to the windows vst name (without the .dll extension) and the windows vst and the renamed linvst.so need to be in the same folder/directory.

So, for example if linvst.so is renamed to test.so for a windows vst named test.dll then it will load test.dll.

linvst.so is a Linux vst that loads any Windows vst that linvts.so is renamed to.

linvst.so can be copied and renamed multiple times to load multiple windows vst's.

Basically, linvst.so becomes the windows vst once it's renamed to the windows vst's name and can then be used in Linux vst hosts.

Tested with Wine 2.1 devel




