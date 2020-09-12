# To make

For the deb package see https://github.com/osxmidi/LinVst/tree/master/deb-packages

Remove -DVESTIGE from the makefiles to use the VST2 SDK (by default the VST2 SDK is not required).

If using the VST2 SDK then the plugininterfaces folder needs to be placed inside the LinVst main source folder.

Wine libwine development files.

------

## For Manjaro/Arch

```
sudo pacman -Sy wine-staging libx11 gcc-multilib
```

------

## For Ubuntu/Debian

sudo apt-get install libx11-dev`

sudo apt-get install wine-stable-dev or sudo apt-get install wine-staging-dev

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

--

## For LinVst
```
sudo make clean

make

sudo make install
```
Installs lin-vst-serverxxxx.exe and lin-vst-serverxxxxx.exe.so to `/usr/bin` and installs linvst.so into the vst folder in the source code folder.

The default Makefile is for 64 bit vst's only.

Makefile-64-32bit builds a LinVst version that runs both 64 bit vst's and 32 bit vst's.

Defining EMBEDRESIZE enables vst window resizing (may not work for all DAW's).

Defining NOFOCUS enables an alternative keyboard/mouse focus operation

See Makefile comments for define options.

The Standalone folder contains makefiles for a standalone window version.

Undefining WINONTOP for the standalone window versions will make a standalone window version that has standard window behaviour (not an on top window).

The 32bitonly folder contains makefiles for Linux 32 bit systems and 32 bit vst's only.



