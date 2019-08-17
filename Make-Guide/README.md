## To make

For the deb package see https://github.com/osxmidi/LinVst/tree/master/deb-packages

Remove -DVESTIGE from the makefiles to use the VST2 SDK (by default the VST2 SDK is not required).

If using the VST2 SDK then the plugininterfaces folder contained within the VST2_SDK folder, needs to be placed in the LinVst files folder.

Wine libwine development files.

For Ubuntu/Debian, sudo apt-get install libwine-development-dev (For Debian, Wine might need to be reinstalled after installing libwine-development-dev) (for Ubuntu 14.04 it's sudo apt-get install wine1.8 and sudo apt-get install wine1.8-dev)

wine-devel packages for other distros (sudo apt-get install wine-devel).

libX11 development needed for embedded version (sudo apt-get install libx11-dev)

For Fedora 
sudo yum -y install wine-devel wine-devel.i686 libX11-devel libX11-devel.i686
sudo yum -y install libstdc++.i686 libX11.i686
 
Include and Library paths might need to be changed in the Makefile for various 64 bit and 32 bit Wine development path locations (otherwise 32 bit compiles might try to link with 64 bit libraries etc).

Additional 32 bit development libraries are probably needed.

On Ubuntu/Debian 64 bits 

sudo dpkg --add-architecture i386

sudo apt-get install libc6-dev-i386

sudo apt-get install gcc-multilib g++-multilib

sudo apt-get install libwine-development-dev:i386

For Debian add deb http://ftp.de.debian.org/debian distro main non-free to /etc/apt/sources.list (distro gets replaced with distro ie stretch etc)

sudo apt-get update

--------

The convert folder is for making the linvstconvert and linvstconverttree (navigates subfolders) utilities that name convert Windows vst dll filenames (.dll) to corresponding Linux shared library filenames (.so).

The makegtk etc files in the convert folder contain simple commands for making the convert utilities.

A -no-pie option might be needed on some systems for the linvstconvert and linvstconverttree utilities icons to appear.

--

For LinVst

sudo make clean

make

sudo make install

Installs lin-vst-server.exe and lin-vst-server.exe.so to /usr/bin and installs linvst.so to /vst in the source code folder (lin-vst-serverst for standalone window version)

The default Makefile is for 64 bit vst's only (embedded window).

Makefile-embed-6432 and Makefile-standalone-6432 build a LinVst version that autodetects and automatically runs both 64 bit vst's and 32 bit vst's (also installs lin-vst-server32.exe and lin-vst-server32.exe.so to /usr/bin for 32 bit vst support) (lin-vst-server32st for standalone window version)

Makefile-standalone-64 (64 bit vsts only) is for a standalone window version.

Defining TRACKTIONWM makes a Tracktion embedded window compatible version (lin-vst-servertrack.exe lin-vst-servertrack.exe.so (lin-vst-servertrack32.exe lin-vst-servertrack32.exe.so) installed to /usr/bin).

Undefining WINONTOP for the standalone window versions will make a standalone window version that has standard window behaviour (not an on top window).

Defining EMBEDRESIZE enables vst window resizing for the embedded window version (currently only working with Linux Reaper).

Defining FOCUS enables an alternative keyboard focus operation where the plugin keyboard focus is given up when the mouse pointer leaves the plugin window.

See Makefile comments for define options.

The 32bitonly folder contains makefiles for 32 bit systems and 32 bit vst's only and contains makefiles for Linux 32bit only systems and makes and installs lin-vst-server32lx (lin-vst-server32lxst for standalone window version) to /usr/bin.

