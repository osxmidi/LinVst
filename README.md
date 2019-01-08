# LinVst

LinVst adds support for Windows vst's to be used in Linux vst capable DAW's.

## Quick Start Guide

(See the Wiki page for a visual guide https://github.com/osxmidi/LinVst/wiki)

Decide on what version to run, either the embedded window version or the standalone window version (the embedded version would probably be the default choice)

Copy all of the lin-vst-server files (files with lin-vst-server in their names) from the version that was chosen to /usr/bin (either the embedded or standalone version folder)

Make a folder and place the windows vst's in it.

Start linvstconvert (in the convert folder) and then select the linvst.so from the chosen embedded or standalone window version folder.

Point linvstconvert to the folder containing the windows vst's and hit the Start (Convert) button.

Start up the linux DAW and point it to scan the folder containing the windows vst's.

Binary LinVst releases are available at https://github.com/osxmidi/LinVst/releases

## Symlinks

Symlinks can be used for convenience if wanted.

If the dll plugin files are in a sudo permission folder (or any permission folder) such as /usr/lib/vst, then make a user permission folder such as /home/user/vst and then make symbolic links to /usr/lib/vst in the /home/user/vst folder by changing into /home/user/vst and running&nbsp;&nbsp;ln -s /usr/lib/vst/&lowast;&nbsp;&nbsp;.&nbsp;&nbsp;and then run linvstconvert on the /home/user/vst folder and then set the DAW to search the /home/user/vst folder.

linvstconvert can also be run with sudo permission for folders/directories that need sudo permission.

Another way to use symlinks is, if the vst dll files and correspondingly named linvst .so files (made by using linvstconvert) are in say for example /home/user/.wine/drive_c/"Program Files"/VSTPlugins

then setting up links in say for example /home/user/vst

by creating the /home/user/vst directory and changing into the /home/user/vst directory

and then running

```
ln -s /home/user/.wine/drive_c/"Program Files"/VSTPlugins/*.so /home/user/vst
```

will create symbolic links in /home/user/vst to the linvst .so files in /home/user/.wine/drive_c/"Program Files"/VSTPlugins and then the DAW can be pointed to scan /home/user/vst

## More Detailed Guide

To use LinVst, the linvst.so file simply needs to be renamed to match the windows vst dll's filename.

(the .so and .dll extensions are left as they are and are not part of the renaming)

Both the windows vst dll file and the renamed linvst.so file are then a matched pair and need to be kept together in the same directory/folder.

**For Example**

copy lin-vst-server.exe and lin-vst-server.so to /usr/bin

copy linvst.so to wherever the windows vst dll file is that someone wants to run in a Linux DAW, 
and then rename linvst.so to the windows vst dll filename 

linvst.so gets renamed to test.so for test.dll

Use the renamed file (test.so) in a Linux VST capable DAW 

Load test.so within the Linux DAW and then test.so will load and run the (name associated) test.dll windows vst 

linvstconvert (GUI or CLI) and linvstconverttree can automatically batch name convert linvst.so to mutiple windows vst dll names that are located in a folder/directory (the linvstconvert CLI version needs to be run from within the dll's folder/directory).
linvstconverttree can automatically name convert folders and sub folders (directories and sub directories) of vst dll plugins.

linvstconvert needs to be run with sudo permission for folders/directories that need sudo permission.

After the naming conversion, the newly created files (.so files) are ready to be used in Linux vst DAW's from the folder that was used for the naming conversion.

Copying/moving plugins (and in some cases their associated presets etc) to a folder/directory with user permissions (if possible) is generally a good idea unless the vst plugin requires a fixed path.

### Using a folder of windows dll files as an example.

Say the folder contains 

- Delay.dll
- Compressor.dll
- Chorus.dll
- Synth.dll

then after the renaming (using the renaming utility) the folder will look like

- Delay.dll
- Delay.so



- Compressor.dll
- Compressor.so



- Chorus.dll
- Chorus.so



- Synth.dll
- Synth.so

The files ending with .so can be used inside Linux Vst DAWS to load and manage the associated dll files (ie Delay.so loads and manages Delay.dll).

Say for instance that a windows vst installation has installed a windows vst named Delay.dll into the VstPlugins directory inside of a WINEPREFIX ie (~/.wine/drive_c/Program Files/VstPlugins), then the renaming utility needs to be run on the VstPlugins directory.

After the renaming there is Delay.dll and Delay.so in the ~/.wine/drive_c/Program Files/VstPlugins directory.

The Linux DAW plugin search path is then appended to include ~/.wine/drive_c/Program Files/VstPlugins and then the plugin(s) can be loaded.

Another way is to make a symbolic link to ~/.wine/drive_c/Program Files/VstPlugins/Delay.so or to the whole ~/.wine/drive_c/Program Files/VstPlugins directory from a more convenient folder such as /home/user/vst and then append /home/user/vst to the Linux DAW's plugin search path.

There can be multiple WINEPREFIXES (by default there is one ~/.wine) containing various vst dll plugins and each WINEPREFIX can have a different wine setup, including dll overrides etc.

Different windows vst plugins that might require different wine setups can be installed into different WINEPREFIXES by creating a new WINEPREFIX (export WINEPREFIX=&sim;/.wine-new winecfg) and then run the windows vst installation program.
Or by setting the WINEPREFIX environmental variable to a particular pre existing WINEPREFIX and then installing the vst into it ie export WINEPREFIX=&sim;/.wine-preexisting and then run the vst install.

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

## Common Problems/Possible Fixes

LinVst looks for wine in /usr/bin and if there isn't a /usr/bin/wine then that will probably cause problems.
/usr/bin/wine can be a symbolic link to /opt/wine-staging/bin/wine (for wine staging) for example.

Quite a few plugins need winetricks corefonts installed for fonts.

A large number of vst plugin crashes/problems can be basically narrowed down to the following dll's and then worked around by overriding/disabling them.

Quite a few vst plugins rely on the Visual C++ Redistributable dlls msvcr120.dll msvcr140.dll msvcp120.dll msvcp140.dll etc

Some vst plugins might crash if the Visual C++ Redistributable dlls are not present in /home/username/.wine/drive_c/windows/system32 for 64 bit vst's and /home/username/.wine/drive_c/windows/syswow64 for 32 bit vst's, and then overridden in the winecfg Libraries tab

Quite a few plugins rely on the d2d1 dll 

Some vst's might crash if Wines inbuilt d2d1 is active (which it is by default).

d2d1 can be disabled in the winecfg Libraries tab.

Setting HKEY_CURRENT_USER Software Wine Direct3D MaxVersionGL to 30002 (MaxVersionGL is a DWORD hex value) might help with some plugins and d2d1 (can also depend on hardware and drivers).

wininet is used by some vst's for net access including registration and online help etc and sometimes wines inbuilt wininet might cause a crash or have unimplemented functions.

wininet can be overridden with wininet.dll and iertutil.dll and nsi.dll

The winbind and libntlm0 and gnutls packages might need to be installed for net access (sudo apt-get install winbind sudo apt-get install gnutls-bin sudo apt-get install libntlm0)

Occasionally other dlls might need to be overridden such as gdiplus.dll etc

Winetricks https://github.com/Winetricks/winetricks can make overriding dll's easier.

**For the above dll overrides**

```
winetricks vcrun2013
winetricks vcrun2015
winetricks wininet
winetricks gdiplus
```

Winetricks also has a force flag --force ie winetricks --force vcrun2013

cabextract needs to be installed (sudo apt-get install cabextract, yum install cabextract etc)

To enable 32 bit vst's on a 64 bit system, a distro's multilib needs to be installed (on Ubuntu it would be sudo apt-get install gcc-multilib g++-multilib)

For details about overriding dll's, see the next section (Wine Config).

Drag and Drop is enabled for the embedded LinVst version used with Reaper/Tracktion/Waveforn/Bitwig but it's only for items dragged and dropped into the vst window and not for items dragged and dropped from the vst window to the DAW/host window.

**Renoise**

Sometimes a synth vst doesn't declare itself as a synth and Renoise might not enable it.

A workaround is to install sqlitebrowser

sudo add-apt-repository ppa:linuxgndu/sqlitebrowser-testing

sudo apt-get update && sudo apt-get install sqlitebrowser

Add the synth vst's path to VST_PATH and start Renoise to scan it.

Then exit renoise and edit the database file /home/user/.renoise/V3.1.0/ CachedVSTs_x64.db (enable hidden folders with right click in the file browser).

Go to the "Browse Data" tab in SQLite browser and choose the CachedPlugins table and then locate the entry for the synth vst and enable the "IsSynth" flag from "0" (false) to "1" (true) and save.

## Wine Config

LinVst expects wine to be found in /usr/bin.

Setting WINELOADER etc to a new wine path might possibly be used for different wine paths.

Keyboard input etc can be enabled for the standalone window LinVst version only, by creating a UseTakeFocus string and setting it to a value of N, in HKEY_CURRENT_USER/Software/Wine/X11 Driver (regedit).

More keyboard control (not recommended) can be enabled for the standalone window LinVst version only, by unchecking the winecfg option "Allow the window manager to control the windows".

Keyboard input should be ok for the embedded window version.

The embedded window version needs to be run with the winecfg option "Allow the window manager to control the windows" checked (which is usually the default).

Sometimes usernames and passwords might need to be copied and pasted into the window because manual entry might not work in all cases.

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

Running winetricks wininet and/or installing winbind and libntlm0 for a distro (sudo apt-get install winbind, sudo apt-get install libntlm0) might help (wininet and it's associated dll's can also be manually installed as dll overrides).

Turning off the vst's multiprocessor support and/or GPU acceleration might help in some cases, due to what features Wine currently supports (Wine version dependent).

On some slower systems Wine can initially take a long time to load properly when Wine is first used, which might cause a LinVst crash.
The solution is to initialise Wine first by running winecfg or any other Wine based program, so that Wine has been initialised before LinVst is used.

Upgrading to the latest wine-stable version is recommended.

## Latency/Performance

Some distros/wine can result in varying latency results.

LinVst has produced reasonable latency results on Ubuntu Studio with a low latency kernel and a real time kernel and on Debian 9 with a realtime kernel (AV Linux should be similar), but not so much on some other distros without a low latency/real time kernel but results can vary from system to system, so some distros (and their setups, wine etc) might be better than some others in terms of latency and latency would also depend on the audio hardware/drivers as well.

rtirq https://github.com/rncbc/rtirq (rtirq-init for Ubuntu/Debian) may have some effect.

Ubuntu Studio and it's low latency kernel combined with rtirq-init, can produce reasonable latency results.

Linux Reaper has anticipative fx features which may help with latency and probably even more so if a rt or low latency kernel is also used, latencies < 5ms may be possible depending on hardware/drivers etc.

LinVst is memory access intensive and having memory in 2 (or more) different motherboard memory banks may result in better performance then if the memory was just in one bank (interleaved memory).

Wineserver can be set to a higher priority may have an effect on cpu load and system response on some systems/setups/plugins.

wineserver can have it's priority level changed from normal to high or very high (root password needed), by right clicking on wineserver in System Monitor (start winecfg first to activate wineserver in System Monitor).

The wineserver priority can be set with wine-staging by setting the STAGING_RT_PRIORITY_SERVER environmental variable between 1 and 99, for example STAGING_RT_PRIORITY_SERVER=60

## Tested vst's

>  LinVst tested with Wine2/Wine3/Wine4 and Linux Tracktion 7/Waveform, Linux Ardour 5.6, Linux Bitwig Studio 2.x, Linux Reaper 5.x, Linux Renoise 3.1

**Kontakt Player 5.6.8** (turn multiprocessing off). Requires Wine 2.0 and above

Some additional dll overrides (below) might be needed for Kontakt and Wine 2.0.
Kontakt and Wine 2.8 staging or later only need an additional msvcp140.dll override. 
To override dll's, copy windows dlls to drive_c/windows/system32 and then override the dlls to be native using the winecfg Libraries option.

(Kontakt Wine 2.0 additional dll's, msvcp140.dll concrt140.dll api-ms-win-crt-time-l1-1-0.dll api-ms-win-crt-runtime-l1-1-0.dll ucrtbase.dll)

Native Access requires Wine Devel/Wine Staging 3.5 or later and a msvcp140.dll override.

Native Access aborts during a download, so a manual mounting and install of the downloaded iso file or a manual unzipping and install of the downloaded zip file in ~/.wine/drive_c/users/user/Downloads is needed.

For all NI iso files they need to be mounted using udf and the unhide option.

sudo mount -t udf file.iso -o unhide /mnt

run winecfg and check the Drives tab for a windows drive letter associated with /mnt

cd /mnt and run the installer (wine setup.exe)

To unmount the iso change to a drirectory away from /mnt and then
sudo umount /mnt

For cd installs

sudo mount -t udf -o unhide /dev/sr0 /mnt

The winbind and libntlm0 and gnutls packages might need to be installed for net access.

**DDMF** Metaplugin VST3 to VST2 wrapper (d2d1 might need to be disabled for some video cards) (Draging and Dropping VST3 files to the DDMF Metaplugin window is ok).
DDMF Metaplugin also needs the Microsoft Visual C++ 2010 Redistributable Package which is usually installed by default but if there is a problem then it is possible to also install it by using winetricks vcrun2010.

**Guitar Rig 5** (same dll overrides as Kontakt)

**Reaktor 6** (msvcp140.dll concrt140.dll dll overrides for Wine 2.0)

**FM8** (might need the standalone FM8 to be run first so that the plugin's file browser files appear)

**Line 6 Helix Native** (msvcr120.dll and gdiplus.dll overrides) (copy and paste username and password into the registration window)

**S-Gear Amp Sim**

**TH3 Amp Sim**

**IK Amplitube 4**

**IK SampleTank**

**Addictive Drums 2** (Addictive Drums 2 requires that the dll (and therefore the renamed linvst.so) needs to be loaded from the installation directory, ie a fixed path).

**EZDrummer2** (choose Mixer window before quiting if drumkit is playing to avoid possible hang when quiting) 

**Mercuriall Spark Amp Sim**

**Melda MXXX Multi Effects** (turn GPU acceleration off)

**Izotope Ozone** has multiple dlls and only the main dll (which is a vst dll) needs to have a linvst .so file associated with it. For instance, the Ozone 8.dll would have an associated Ozone 8.so file and none of the other dlls would have associated .so files (otherwise the DAW will try and load the other dlls which are not vst dlls and then produce errors).

**T-RackS**

**Nebula4**

**VUMT**

**Sforzando** (sfz drag and drop ok with the LinVst embedded window and standalone window drag and drop enabled versions).

**BassMidi** sf2 and sfz.

**Groove Machine** (drag and drop ok with the LinVst embedded window and standalone window drag and drop enabled versions).

**Zampler RX**

**Stillwell plugins**

**Cobalt (Sanford) Synth**

**Synth1** needs it's presets path setup (browse and locate using the opt button).

**Spire Synth** (Disable d2d1 in the Libraries section of winecfg) (32 bit version seems to work ok with a d2d1 version 6.1.7601.17514 32 bit dll override)

**OP-X PRO-II** (Disable d2d1 in the Libraries section of winecfg)

**MT-PowerDrumKit** (Disable d2d1 in the Libraries section of winecfg) (drag and drop ok with the LinVst embedded window and standalone window drag and drop enabled versions).
Setting HKEY_CURRENT_USER Software Wine Direct3D MaxVersionGL 30002 might help with d2d1 (can also depend on hardware and drivers).

**FL Sytrus** needs winetricks corefonts to be installed for fonts. The UI might become blank after closing and reopening (minimizing).

**Ignite Amps TPA-1 Amp Sim** 

**LePou Amp Sims**

**Nick Crow Lab Amp Sims**

**Voxengo Boogex Guitar Effects**

**Klanghelm MJUC Compressor**

**TDR SlickEQ mixing/mastering equalizer** 

**Toneboosters TrackEssentials** (disable d2d1 for Ferox)

**Serum Synth** (can have some issues with Wines current d2d1, disable d2d1 or try a d2d1 override) (32 bit version seems to work better than the 64 bit version with a d2d1 version 6.1.7601.17514 32 bit dll override)
Setting HKEY_CURRENT_USER Software Wine Direct3D MaxVersionGL to 30002 might help but there might still be some d2d1 errors (can also depend on hardware and drivers).
Serum needs Wine Staging to register.

**Waves plugins**.

Tested Waves Central and Waves C1 and C4 and C360 and API-560 and Abbey Road Plates plugins with Wine Staging version 3.x and 4.x and Debian Stretch/Sid (other non Debian based systems not tested).

Waves VST3 API-560 and Abbey Road Plates plugins tested with DDMF Metaplugin VST3 to VST2 wrapper and Wine Staging 4.x and Debian Stretch/Sid.

Bitwig seems to need the Waveshell to be unpacked into individual dll's using shell2vst.

The Waves plugins don't seem to work with Tracktion.

Reaper and Ardour seem to work with the Waveshell dll and the individual unpacked dll's.

Waves Central requires Wine Staging 3.x/4.x and also requires robocopy.exe to be installed (Windows Server 2003 Resource Kit Tools) and robocopy also needs a mfc42u.dll 32 bit override and a mfc42.dll 32 bit override to be placed in /windows/syswow64 and added to the winecfg Libraries tab.

Waves Central seems to require a usb stick to be inserted so that the install/license functions operate and the Windows Server 2003 Resource Kit Tools install (robocopy) seems to make a list of available drives at install time, so inserting a usb stick before installing Windows Server 2003 Resource Kit Tools would be a good idea as Waves Central uses robocopy for file transfers and Waves Central checks for usb drives and seems to require a usb stick to be present.

Waves Central also requires some simple registry additions

wine regedit

Add the following environment string variables under HKEY_CURRENT_USER\Environment (New String Value)

COMMONPROGRAMFILES(X86) C:\Program Files (x86)\Common Files

PROGRAMFILES(X86) C:\Program Files (x86)

PUBLIC C:\users\Public

for 32 bit systems it's

COMMONPROGRAMFILES C:\Program Files\Common Files

PROGRAMFILES C:\Program Files

PUBLIC C:\users\Public

Wine Staging is only needed for Waves Central and the Waves plugins themselves can run with any of the Wine versions, Wine-Stable, Wine-Devel or Wine Staging.

The winbind and libntlm0 and gnutls packages might need to be installed for net access as well.

The file to wrap with linvst.so is WaveShellxxxxxxxx.dll which is probably in Program Files/VstPlugIns for 64 bit systems.

The Waves plugins probably require a mfc140.dll override (for 64 bit plugins copy mfc140.dll to windows/system32 and add mfc140 into the Libraries section of winecfg).

For V10

linvst.so needs to go into ~/.wine/drive_c/Program Files/VSTPlugIns and then moved to the waveshell name 
mv linvst.so "WaveShell1-VST 10.0_x64.so"

(If a dialog doesn't appear then WaveShell1-VST 10.0_x64.so needs to be deleted from ~/.wine/drive_c/Program Files/VSTPlugIns and text files need to be deleted rm *.txt in ~/.wine/drive_c/users/"LinuxUserName"/Application Data/Waves Audio/Preferences and then do the above).

Then set the DAW to scan ~/.wine/drive_c/Program Files/VSTPlugIns and then scan, and a select folder dialog will appear.

Descend into the path by double clicking on (C:)
and then double clicking on Program Files (x86)
and then double clicking on Waves
and then just highlight Plug-Ins V10 with a single click (don't descend into it)
and then finish by clicking on open.

The DAW scan should then pick up the Waves plugins.

## To make

Remove -DVESTIGE from the makefiles to use the VST2 SDK (by default the VST2 SDK is not required).

If using the VST2 SDK then the plugininterfaces folder contained within the VST2_SDK folder, needs to be placed in the LinVst files folder. https://www.steinberg.net/en/company/developers.html

Wine libwine development files.

For Ubuntu/Debian, sudo apt-get install libwine-development-dev (for Ubuntu 14.04 it's sudo apt-get install wine1.8 and sudo apt-get install wine1.8-dev)

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

(also installs lin-vst-server32.exe and lin-vst-server32.exe.so to /usr/bin for 32 bit vst support) (lin-vst-serverst32 for standalone window version)

Makefile-embed-6432 and Makefile-standalone-6432 build a LinVst version that autodetects and automatically runs both 64 bit vst's and 32 bit vst's.

Makefile-embed-6432 (64 and 32 bit vsts) and Makefile-embed-64 (64 bit vsts only) are for the daw embedded window option.

Makefile-standalone-6432 (64 and 32 bit vsts) and Makefile-standalone-64 (64 bit vsts only) are for a standalone window version that can be useful for some daw's.

LinVst-Linux32bit-only.zip contains makefiles for Linux 32bit only systems and makes and installs lin-vst-serverlx32 (lin-vst-serverstlx32 for standalone window version).

Undefining WINONTOP for the standalone window versions will make a standalone window version that has standard window behaviour (not an on top window).

Defining EMBEDRESIZE enables vst window resizing for the embedded window version (currently only working with Linux Reaper).

Defining FOCUS enables an alternative keyboard focus operation where the plugin keyboard focus is given up when the mouse pointer leaves the plugin window.

See Makefile comments for define options.

Tracktion/Waveform might have an embedded window offset problem where the plugin's gui is misaligned with the Tracktion/Waveform plugin window and requires overwriting the LinVst files with the files in the Tracktion folder.

The 32bitonly folder contains makefiles for 32 bit systems and 32 bit vst's only.

