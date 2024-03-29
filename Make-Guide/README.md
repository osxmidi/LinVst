# Possible Errors

An error like "remotepluginserver.h:18:10: fatal error: windows.h: No such file or directory" means that the wine development libraries have not been installed.

An error like "relocatable linking with relocations from format elf64-x86-64" means that the 32 bit wine and system libraries have not been installed.

If 64 bit vst's are only required and no 32 bit vst's are required, then use make -f Makefile-64bitonly or copy/overwrite Makefile-64bitonly to Makefile

An error that occurs when trying to make linvstconvert would probably be due to not having the gtk3 development libraries installed.

The linvstconvert makefile uses a -no-pie option which could be removed from the makefile if it causes problems with some compliers.

------

# To make

LinVst consists of 2 parts, the lin-vst-server files and the linvst.so file part and the linvstconvert part.

The lin-vst-server files are installed to /usr/bin and the linvst.so file is installed to ./vst using sudo make install.

linvstconvert is then used to prepare vst2 dll files for use in a daw using the linvst.so file.

Install the wine development files and the 32 bit wine/system development files for 32 bit vst's.

After compiling, the 64 bit and 32 bit lin-vst-server files and the linvst.so file are placed in the current code folder. 

A sudo make install will install the lin-vst-server files to /usr/bin and linvst.so to the current code/vst folder and linvst.so can be used with linvstconvert to prepare vst2 dll files for use in a daw.

To compile linvstconvert use the Makefile-convert makefile, make -f Makefile-convert or cd into the convert folder and then make.

linvstconvert requires the gtk3 development libraries to be installed.

For the deb package see https://github.com/osxmidi/LinVst/tree/master/deb-packages

Remove -DVESTIGE from the makefiles to use the VST2 SDK (by default the VST2 SDK is not required).

If using the VST2 SDK then the plugininterfaces folder needs to be placed inside the LinVst main source folder.

Makefiles in the dragwin folder are for (possible) drag and drop from the plugin to the daw.

------

## For Manjaro/EndeavourOS/Arch
```
sudo pacman -Sy wine-staging libx11 gcc-multilib
```
------

## For Ubuntu/Debian
```
winehq-stable or winehq-staging needs to be installed from WineHQ.

sudo apt-get install libx11-dev

sudo apt-get install wine-stable-dev or sudo apt-get install wine-staging-dev


Additional 32 bit development libraries are needed for 32 bit vst's.

sudo dpkg --add-architecture i386

sudo apt-get install libc6-dev-i386

sudo apt-get install gcc-multilib g++-multilib

sudo apt-get install libx11-dev:i386
```
-------

## For Fedora 
```
sudo yum -y install wine-devel wine-devel.i686 libX11-devel libX11-devel.i686
sudo yum -y install libstdc++.i686 libX11.i686
```
-------
 
## Other distros

```
wine-devel packages for other distros (`sudo apt-get install wine-devel`).

```
--------

## To make linvstconvert

The convert folder is for making linvstconvert that name convert Windows vst dll filenames (.dll) to corresponding Linux shared library filenames (.so).

--------

## To make LinVst
```
sudo make clean

make

sudo make install
```
Installs lin-vst-serverxxxx.exe and lin-vst-serverxxxxx.exe.so to `/usr/bin` and installs linvst.so into the vst folder in the source code folder.






