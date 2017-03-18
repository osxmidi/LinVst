# LinVst

LinVst is a Linux vst plugin that loads Windows vst's.

To make

The plugininterfaces folderfrom the VST2.4 SDK needs to be in the LinVst files folder.

make clean

make

sudo make install

Installs lin-vst-server.exe and lin-vst-server.exe.so to /usr/bin and installs linvst.so to /usr/lib/vst

linvst.so needs to be renamed to the windows vst name and the windows vst and the renamed linvst.so need to be in the same folder/directory.

So, for example linvst.so is renamed to test.so for a windows vst named test.dll 

Tested with Wine 2.1 devel




