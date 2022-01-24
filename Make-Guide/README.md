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

To compile linvstconvert use the Makefile-convert makefile, make -f Makefile-convert
linvstconvert requires the gtk3 development libraries to be installed.


For the deb package see https://github.com/osxmidi/LinVst/tree/master/deb-packages

Remove -DVESTIGE from the makefiles to use the VST2 SDK (by default the VST2 SDK is not required).

If using the VST2 SDK then the plugininterfaces folder needs to be placed inside the LinVst main source folder.

------

## For Manjaro/Arch

```
sudo pacman -Sy wine-staging libx11 gcc-multilib
```

------

## For Ubuntu/Debian

sudo apt-get install libx11-dev

sudo apt-get install wine-stable-dev or sudo apt-get install wine-staging-dev

or sudo apt-get install libwine-dev

-------

## For Fedora 
```
sudo yum -y install wine-devel wine-devel.i686 libX11-devel libX11-devel.i686
sudo yum -y install libstdc++.i686 libX11.i686
```

-------
 
Include and Library paths might need to be changed in the Makefile for various 64 bit and 32 bit Wine development path locations (otherwise 32 bit compiles might try to link with 64 bit libraries etc).

Additional 32 bit development libraries are probably needed.

## On Ubuntu/Debian 64 bits 
```
sudo dpkg --add-architecture i386

sudo apt-get install libc6-dev-i386

sudo apt-get install gcc-multilib g++-multilib

```
-----

## Other distros

```
wine-devel packages for other distros (`sudo apt-get install wine-devel`).

```
--------

The convert folder is for making the linvstconvert and linvstconverttree (navigates subfolders) utilities that name convert Windows vst dll filenames (.dll) to corresponding Linux shared library filenames (.so).

The makegtk etc files in the convert folder contain simple commands for making the convert utilities.

A -no-pie option might be needed on some systems for the linvstconvert and linvstconverttree utilities icons to appear.

use make -f Makefile-convert to make linvstconvert in main directory

--

## For LinVst
```
sudo make clean

make

sudo make install
```
Installs lin-vst-serverxxxx.exe and lin-vst-serverxxxxx.exe.so to `/usr/bin` and installs linvst.so into the vst folder in the source code folder.

To make the drag and drop from vst plugin to daw version, use the Makefile in the dragwin folder.

See Makefile comments for define options.

The 32bitonly folder contains makefiles for Linux 32 bit systems and 32 bit vst's only.



