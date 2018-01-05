# LinVst

-----

LinVst enables Windows vst's to be used as Linux vst's in Linux vst capable DAW's.

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

linvstconvert (GUI or CLI) and linvstconverttree can automatically batch name convert linvst.so to mutiple windows vst dll names that are located in a folder/directory (the linvstconvert CLI version needs to be run from within the dll's folder/directory).
linvstconverttree can automatically name convert folders and sub folders (directories and sub directories) of vst dll plugins.

sudo permission might be needed for some folders/directories that don't have user permissions.

After the naming conversion, the newly created files (.so files) are ready to be used in Linux vst DAW's from the folder that was used for the naming conversion.

Copying/moving plugins (and in some cases their associated presets etc) to a folder/directory with user permissions (if possible) is generally a good idea unless the vst plugin requires a fixed path.

-------

Using a folder of windows dll files as an example.

Say the folder contains 

Delay.dll

Compressor.dll

Chorus.dll

Synth.dll

then after the renaming (using the renaming utility) the folder will look like

Delay.dll
Delay.so

Compressor.dll
Compressor.so

Chorus.dll
Chorus.so

Synth.dll
Synth.so

The files ending with .so can be used inside Linux Vst DAWS to load and manage the associated dll files (ie Delay.so loads and manages Delay.dll).

Say for instance that a windows vst installation has installed a windows vst named Delay.dll into the VstPlugins directory inside of a WINEPREFIX ie (~/.wine/drive_c/Program Files/VstPlugins), then the renaming utility needs to be run on the VstPlugins directory.

After the renaming there is Delay.dll and Delay.so in the ~/.wine/drive_c/Program Files/VstPlugins directory.

The Linux DAW plugin search path is then appended to include ~/.wine/drive_c/Program Files/VstPlugins and then the plugin(s) can be loaded.

Another way is to make a symbolic link to ~/.wine/drive_c/Program Files/VstPlugins/Delay.so or to the whole ~/.wine/drive_c/Program Files/VstPlugins directory from a more convenient folder such as /home/user/vst and then append /home/user/vst to the Linux DAW's plugin search path.

There can be multiple WINEPREFIXES (by default there is one ~/.wine) containing various vst dll plugins and each WINEPREFIX can have a different wine setup, including dll overrides etc.

Different windows vst plugins that might require different wine setups can be installed into different WINEPREFIXES by creating a new WINEPREFIX (export WINEPREFIX=~/.wine-new winecfg) and then run the windows vst installation program or by setting the WINEPREFIX environmental variable to a particular pre existing WINEPREFIX and then installing the vst into it ie export WINEPREFIX=~/.wine-preexisting and then run the vst install.

A particular WINEPREFIX can be configured by using winecfg with the WINEPREFIX environmental variable set to that particular WINEPREFIX ie export WINEPREFIX=~/.wine-new winecfg.

When a plugin is loaded from within a WINEPREFIX, it picks up on that WINEPREFIXES individual setup (also works for symbolic links as discussed above).

------

linvst.so needs to be renamed to the windows vst name (the .so and .dll extensions are left as they are and are not part of the renaming).

Both the windows vst dll file and the renamed linvst.so file need to be in the same folder/directory.

linvst.so is a Linux vst template that loads and runs any windows vst that linvst.so is renamed to.

linvst.so can be copied and renamed multiple times to load multiple windows vst's.

Basically, linvst.so becomes the windows vst once it's renamed to the windows vst's name and can then be used in Linux vst hosts.

Individual plugins can have their own WINEPREFIX environment.

If a windows vst dll file and it's associated renamed linvst.so file are located within a WINEPREFIX then the plugin will use that WINEPREFIX.

Symlinks can point to renamed linvst.so files located within a WINEPREFIX.

------

Common Problems/Possible Fixes

LinVst looks for wine in /usr/bin and if there isn't a /usr/bin/wine then that will probably cause problems.
/usr/bin/wine can be a symbolic link to /opt/wine-staging/bin/wine (for wine staging) for example.

A large number of vst plugin crashes/problems can be basically narrowed down to the following dll's and then worked around by overriding/disabling them.

Quite a few vst plugins rely on the Visual C++ Redistributable dlls msvcr120.dll msvcr140.dll msvcp120.dll msvcp140.dll etc

Some vst plugins might crash if the Visual C++ Redistributable dlls are not present in /home/username/.wine/drive_c/windows/system32 for 64 bit vst's and /home/username/.wine/drive_c/windows/syswow64 for 32 bit vst's, and then overridden in the winecfg Libraries tab

Quite a few plugins rely on the d2d1 dll 

Some vst's might crash if Wines inbuilt d2d1 is active (which it is by default).

d2d1 can be disabled in the winecfg Libraries tab.

wininet is used by some vst's for net access including registration and online help etc and sometimes wines inbuilt wininet might cause a crash or have unimplemented functions.

wininet can be overridden with wininet.dll and iertutil.dll and nsi.dll

Occasionally other dlls might need to be overridden such as gdiplus.dll etc

Winetricks https://github.com/Winetricks/winetricks can make overriding dll's easier.

For the above dll overrides

winetricks vcrun2013

winetricks vcrun2015

winetricks wininet

winetricks gdiplus

Winetricks also has a force flag --force ie winetricks --force vcrun2013

cabextract needs to be installed (sudo apt-get install cabextract, yum install cabextract etc)

To enable 32 bit vst's on a 64 bit system, a distro's multilib needs to be installed (on Ubuntu it would be sudo apt-get install gcc-multilib g++-multilib)

For details about overriding dll's, see the next section (Wine Config).

------

Wine Config

Sometimes usernames and passwords might need to be copied and pasted into the window because manual entry might not work in all cases.

Keyboard control input etc can be enabled by unchecking the winecfg option "Allow the window manager to control the windows" but some hosts might intercept or not pass on some modifiers.

Sometimes a windows vst needs a Wine dll override.

Finding out what dll's to possibly override can be done by running "strings vstname.dll | grep -i dll", which will display a list of dll's from the plugins dll file.

For instance, if the dll list contains d2d1.dll and there are problems running the plugin, then d2d1 might possibly be a candidate to override or disable.

If the Wine debugger displays "unimplemented function in XXXX.dll" somewhere in it's output, then that dll usually needs to be overriden with a windows dll.

Overriding a dll involves copying the windows dll to a wine windows directory and then running winecfg to configure wine to override the dll.

64 bit .dlls are copied to /home/username/.wine/drive_c/windows/system32 

32 bit .dlls are copied to /home/username/.wine/drive_c/windows/syswow64

Run winecfg and select the Libraries tab and then select the dll to override from the list or type the name.

After adding the dll, check with the edit option that the dll's settings are native first and then builtin.

https://www.winehq.org/docs/wineusr-guide/config-wine-main

Sometimes required dll's might be missing and additional windows redistributable packages might need to be installed.

Some windows vst's use D3D, and Wine uses Linux OpenGL to implement D3D, so a capable Linux OpenGL driver/setup might be required for some windows vst's.

Disabling d2d1 in the Libraries section of winecfg might help with some windows vst's.

Some D3D dll overrides might be needed for some windows vst's.

D3D/OpenGL Wine config advice can be found at gaming forums and other forums.

Additional dll's (dll overrides) might have to be added to Wine for some Windows vst's to work.

Winetricks might help with some plugins https://github.com/Winetricks/winetricks

Some plugins might use wininet for internet connections (online registration, online help, etc) which might cause problems depending on Wines current implementation.

Running winetricks wininet and/or installing winbind for a distro (sudo apt-get install winbind) might help (wininet and it's associated dll's can also be manually installed as dll overrides).

Turning off the vst's multiprocessor support and/or GPU acceleration might help in some cases, due to what features Wine currently supports (Wine version dependent).

On some slower systems Wine can initially take a long time to load properly when Wine is first used, which might cause a LinVst crash.
The solution is to initialise Wine first by running winecfg or any other Wine based program, so that Wine has been initialised before LinVst is used.

Upgrading to the latest wine-stable version is recommended.

------

Latency

Some distros/wine can result in varying latency results.

LinVst has produced reasonable latency results on Ubuntu Studio with a low latency kernel and a real time kernel and on Debian 9 with a realtime kernel (AV Linux should be similar), but not on some other distros without a low latency/real time kernel, so some distros (and their setups, wine etc) might be better than some others for latency.

There is also rtirq https://github.com/rncbc/rtirq (rtirq-init for Ubuntu/Debian)

Ubuntu Studio and it's low latency kernel combined with rtirq-init, can produce reasonable latency results.

Setting wineserver to a higher priority may have an effect on cpu load and system response on some systems/setups/plugins.

wineserver can have it's priority level changed from normal to high or very high (root password needed), by right clicking on wineserver in System Monitor (start winecfg first to activate wineserver in System Monitor).

The wineserver priority can be set with wine-staging by setting the STAGING_RT_PRIORITY_SERVER environmental variable between 1 and 99, for example STAGING_RT_PRIORITY_SERVER=60

------

LinVst tested with Wine 2 and Linux Tracktion 7, Linux Ardour 5.6, Linux Bitwig Studio 2, Linux Reaper 5.5, Linux Renoise 3.1

Tested vst's

Kontakt Player 5.6.8 (turn multiprocessing off). Requires Wine 2.0 and above

Some additional dll overrides (below) might be needed for Kontakt and Wine 2.0.
Kontakt and Wine 2.8 staging only need an additional msvcp140.dll override. 
To override dll's, copy windows dlls to drive_c/windows/system32 and then override the dlls to be native using the winecfg Libraries option.

(Kontakt Wine 2.0 additional dll's, msvcp140.dll concrt140.dll api-ms-win-crt-time-l1-1-0.dll api-ms-win-crt-runtime-l1-1-0.dll ucrtbase.dll)

Guitar Rig 5 (same dll overrides as Kontakt)

Reaktor 6 (msvcp140.dll concrt140.dll dll overrides for Wine 2.0)

FM8

Line 6 Helix Native (msvcr120.dll and gdiplus.dll overrides) (copy and paste username and password into the registration window)

S-Gear Amp Sim

TH3 Amp Sim

IK Amplitube 4

IK SampleTank

Addictive Drums 2 (Addictive Drums 2 requires that the dll (and therefore the renamed linvst.so) needs to be loaded from the installation directory, ie a fixed path).

EZDrummer2

Mercuriall Spark Amp Sim

Melda MXXX Multi Effects (turn GPU acceleration off)

T-RackS

Nebula4

VUMT

Sforzando (sfz drag and drop ok with the LinVst embedded window and standalone window drag and drop enabled versions).

Groove Machine (drag and drop ok with the LinVst embedded window and standalone window drag and drop enabled versions).

Zampler RX

Stillwell plugins

Cobalt (Sanford) Synth

Spire Synth (Disable d2d1 in the Libraries section of winecfg) (32 bit version seems to work ok with a d2d1 version 6.1.7601.17514 32 bit dll override)

OP-X PRO-II (Disable d2d1 in the Libraries section of winecfg)

MT-PowerDrumKit (Disable d2d1 in the Libraries section of winecfg) (drag and drop ok with the LinVst embedded window and standalone window drag and drop enabled versions).

Ignite Amps TPA-1 Amp Sim

LePou Amp Sims

Nick Crow Lab Amp Sims

Voxengo Boogex Guitar Effects

Klanghelm MJUC Compressor

TDR SlickEQ mixing/mastering equalizer 

Toneboosters TrackEssentials (disable d2d1 for Ferox)

Serum Synth (can have some issues with Wines current d2d1, disable d2d1 or try a d2d1 override) (32 bit version seems to work better than the 64 bit version with a d2d1 version 6.1.7601.17514 32 bit dll override)

--------

To make

The plugininterfaces folder contained within the VST2_SDK folder, needs to be placed in the LinVst files folder. https://www.steinberg.net/en/company/developers.html

Wine libwine development files.

For Ubuntu/Debian, sudo apt-get install libwine-development-dev (for Ubuntu 14.04 it's sudo apt-get install wine1.8 and sudo apt-get install wine1.8-dev)

wine-devel packages for other distros (sudo apt-get install wine-devel).

libX11 development needed for embedded version (sudo apt-get install libx11-dev)
 
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

sudo make clean

make

sudo make install

Installs lin-vst-server.exe and lin-vst-server.exe.so to /usr/bin and installs linvst.so to /vst in the source code folder (lin-vst-serverst for standalone window version)

(also installs lin-vst-server32.exe and lin-vst-server32.exe.so to /usr/bin for 32 bit vst support) (lin-vst-serverst32 for standalone window version)

Makefile-embed-6432 and Makefile-standalone-6432 build a LinVst version that autodetects and automatically runs both 64 bit vst's and 32 bit vst's.

Makefile-embed-6432 (64 and 32 bit vsts) and Makefile-embed-64 (64 bit vsts only) are for the daw embedded window option.

Makefile-standalone-6432 (64 and 32 bit vsts) and Makefile-standalone-64 (64 bit vsts only) are for a standalone window version that can be useful for some daw's.

LinVst-Linux32bit-only.zip contains makefiles for Linux 32bit only systems and makes and installs lin-vst-serverlx32 (lin-vst-serverstlx32 for standalone window version).

Undefining WINONTOP for the standalone window versions will make a standalone window version that has standard window behaviour (not an on top window).

For slower systems not displaying the embedded window correctly there is a EMBEDTHREAD define that can be enabled (see the embedded makefiles for details).

LinVst-Linux32bit-only.zip contains makefiles for 32 bit systems and 32 bit vst's only.

linvstconvertsrc.zip is for making the linvstconvert and linvstconverttree utilities.

A -no-pie option might be needed on some systems for the linvstconvert and linvstconverttree utilities icons to appear.



