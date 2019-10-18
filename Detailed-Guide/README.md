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

## Symlinks

Symlinks can be used for convenience if wanted.
One reason to use symlinks would be to be able to group plugins together outside of their Wine install paths and reference them from a common folder via symlinks.

For a quick simple example, say that the plugin dll file is in ~/.wine/drive_c/Program Files/Vstplugins and is called plugin.dll.
Then just open that folder using the file manager and drag linvst.so to it and rename linvst.so to whatever the dll name is (plugin.so for plugin.dll).
Then create a symlink to plugin.so using a right click and selecting create symlink from the option menu, the make sure the symlink ends in a .so extension (might need to edit the symlinks name) and then drag that symlink to anywhere the DAW searches (say ~/vst for example) and then plugin.so should load (via the symlink) within the DAW.

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

## Running VST3 plugins.

There are a few ways to try to run a Windows vst3 plugin (this is mainly useful when there might not be vst2 versions of a plugin available).

https://github.com/osxmidi/LinVst3

**vst3shell (from Polac VST Loaders for Jeskola Buzz)**

https://www.xlutop.com/buzz/

Put vst3shell.x64.dll and vsti3shell.x64.dll and vst3shell.x64.core from the Gear/Vst folder into your vst plugins folder and rename a copy of linvst.so to vst3shell.x64.so and also rename a copy of linvst.so to vsti3shell.x64.so, and then do a plugin scan in the daw (Point the daw to scan your vst plugins folder). 

The vst3 plugins should appear in the daw's plugin list (the ~/.wine/drive_c/Program Files/Common Files/VST3 folder is scanned for vst3 plugins).

A new daw plugin scan is required after adding new vst3 plugins.

(For Linux 32 bit systems only (not 64 bit Linux systems) the dll's to use are vst3shell.dll and vsti3shell.dll and vst3shell.core)

**DDMF** Metaplugin VST3 to VST2 wrapper (d2d1 might need to be disabled for some video cards) (Draging and Dropping VST3 files to the DDMF Metaplugin window is ok).

DDMF Metaplugin also needs the Microsoft Visual C++ 2010 Redistributable Package which is usually installed by default but if there is a problem then it is possible to also install it by using winetricks vcrun2010.

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

Some plugins need Windows fonts (~/.wine/drive_c/windows/Fonts) ./winetricks corefonts

Some plugins might use wininet for internet connections (online registration, online help, etc) which might cause problems depending on Wines current implementation.

Running winetricks wininet and/or installing winbind and libntlm0 for a distro (sudo apt-get install winbind, sudo apt-get install libntlm0) might help (wininet and it's associated dll's can also be manually installed as dll overrides).

Turning off the vst's multiprocessor support and/or GPU acceleration might help in some cases, due to what features Wine currently supports (Wine version dependent).

On some slower systems Wine can initially take a long time to load properly when Wine is first used, which might cause a LinVst crash.
The solution is to initialise Wine first by running winecfg or any other Wine based program, so that Wine has been initialised before LinVst is used.

Upgrading to the latest wine-stable version is recommended.

## Latency/Performance

Some distros/hardware can result in varying latency results.

LinVst has produced reasonable latency results with a low latency kernel and with a real time kernel and with the Liquorix kernel but results can vary from system to system.

A kernel that has PREEMPT in it's info would be the preference (obtain kernel info by running uname -a).

rtirq https://github.com/rncbc/rtirq (rtirq-init for Ubuntu/Debian) may have some effect.

LinVst is memory access intensive and having memory in 2 (or more) different motherboard memory banks may result in better performance then if the memory was just in one bank (interleaved memory).

Wineserver can be set to a higher priority may have an effect on cpu load and system response on some systems/setups/plugins.

wineserver can have it's priority level changed from normal to high or very high (root password needed), by right clicking on wineserver in System Monitor (start winecfg first to activate wineserver in System Monitor).

The wineserver priority can be set with wine-staging by setting the STAGING_RT_PRIORITY_SERVER environmental variable between 1 and 99, for example STAGING_RT_PRIORITY_SERVER=60

## Tested vst's

>  LinVst tested with Wine4 and Linux Tracktion 7/Waveform, Linux Ardour 5.x, Linux Reaper 5.x, Linux Renoise 3.1, Linux Bitwig Studio 2.5 (For Bitwig 2.5, In Settings->Plug-ins choose "Individually" plugin setting and check all of the LinVst plugins.
For Bitwig 2.4.3, In Settings->Plug-ins choose Independent plug-in host process for "Each plug-in" setting and check all of the LinVst plugins).

d2d1 based plugins

d2d1.dll can cause errors because Wine's current d2d1 support is not complete and using a d2d1.dll override might produce a black (blank) display.

Some plugins might need d2d1 to be disabled in the winecfg Libraries tab (add a d2d1 entry and then edit it and select disable).

A scan of the plugin dll file can be done to find out if the plugin depends on d2d1 

"strings vstname.dll | grep -i d2d1"

**Kontakt Player/Native Access updated info (Wine 4)

The Native Access install seems to require a Windows 10 setting in winecfg, otherwise the Native Access install might get hungup on the ISO Driver Install.

It's also probably a good idea to install winetricks cmd to avoid possible cmd bugs that might be in Wine.

Dll overrides are probably not needed anymore.

**Kontakt Player 5.6.8 and 6.0** (turn multiprocessing off). Requires Wine 2.0 and above

Some additional dll overrides (below) might be needed for Kontakt and Wine 2.0.
Kontakt and Wine 2.8 staging or later only need an additional msvcp140.dll override. 
To override dll's, copy windows dlls to drive_c/windows/system32 and then override the dlls to be native using the winecfg Libraries option.

(Kontakt Wine 2.0 additional dll's, msvcp140.dll concrt140.dll api-ms-win-crt-time-l1-1-0.dll api-ms-win-crt-runtime-l1-1-0.dll ucrtbase.dll)

Native Access requires Wine Devel/Wine Staging 3.5 or later and a msvcp140.dll override.

Because Wine might have problems mounting the downloaded iso file, Native Access aborts partway through a download but the iso file has been downloaded, so a manual mounting and install of the downloaded iso file or a manual unzipping and install of the downloaded zip file in ~/.wine/drive_c/users/user/Downloads is needed.

For all NI iso files they need to be mounted using udf and the unhide option.

sudo mount -t udf file.iso -o unhide /mnt

run winecfg and check the Drives tab for a windows drive letter associated with /mnt

cd /mnt and run the installer (wine setup.exe)

To unmount the iso change to a drirectory away from /mnt and then
sudo umount /mnt

For cd installs

sudo mount -t udf -o unhide /dev/sr0 /mnt

The winbind and libntlm0 and gnutls (gnutls-bin) packages might need to be installed for net access.

**Waves plugins**.

Because Wine has some missing parts as compared to Windows (ie Robocopy, reg entries, some dll's etc) some things need to be installed and setup.

Basic Install Procedure for Waves Central/64bit Waves plugins would be

1: Install Wine Staging (Ubuntu needs libfaudio0) and install winetricks cmd (to workaround possible cmd problems that can cause hangs).

2: Install the mfc42 and mfc42u 32bit dll overrides into ~/.wine/drive_c/windows/syswow64 (and optionally add the dll names in winecfg's Libraries tab)

3: Install rktools.exe (Robocopy from Windows Server 2003 Resource Kit Tools)

4: Install the mfc140.dll override into ~/.wine/drive_c/windows/system32 (and optionally add the dll names in winecfg's Libraries tab)

5: Import Waves.reg (in LinVst's Waves folder) into regedit

6: Install Waves Central

7: The vst 64 bit dll to wrap is WaveShellxxxxxx.dll in ~/.wine/drive_c/Program Files/VSTPlugIns/ (there might be more than one WaveShellxxxxxx.dll depending on additional Waves plugins installs)

7: A dialog box will popup (asking for a path) the first time a Waves V10 plugin is run, 
descend into the path by double clicking on (C:)
and then double click on Program Files (x86) (or Program Files if Plug-Ins V10 can't be found)
and then double click on Waves
and then just highlight Plug-Ins V10 with a single click (don't descend into it)
and then finish by clicking on open.

Tested Waves Central and Waves C1 and C4 and C360 and API-560 and Abbey Road Plates and Bass Slapper(1GB samples) plugins with Wine Staging version 3.x and 4.x and Debian Stretch/Sid (other non Debian based systems not tested).

Waves VST3 API-560 and Abbey Road Plates plugins tested with LinVst3 and the DDMF Metaplugin VST3 to VST2 wrapper and Wine Staging 4.x and Debian Stretch/Sid.

Bitwig seems to need the Waveshell to be unpacked into individual dll's using shell2vst.

The Waves plugins don't seem to work with Tracktion.

Reaper and Ardour seem to work with the Waveshell dll and the individual unpacked dll's.

Waves Central requires Wine Staging 3.x/4.x and also requires robocopy.exe to be installed (Windows Server 2003 Resource Kit Tools) and robocopy also needs a mfc42u.dll 32 bit override and a mfc42.dll 32 bit override to be placed in /windows/syswow64 and added to the winecfg Libraries tab.

Waves Central seems to require a usb stick to be inserted so that the install/license functions operate and the Windows Server 2003 Resource Kit Tools install (robocopy) seems to make a list of available drives at install time, so inserting a usb stick before installing Windows Server 2003 Resource Kit Tools would be a good idea as Waves Central uses robocopy for file transfers and Waves Central checks for usb drives and seems to require a usb stick to be present.

Waves Central also requires some simple registry additions

Just import the Waves.reg file (in the Waves folder) into the registry using regedit or manually do the below

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

The file to wrap with linvst.so is WaveShellxxxxxxxx.dll which is probably in Program Files/VSTPlugIns for 64 bit systems.

The Waves plugins probably require a mfc140.dll override (for 64 bit plugins copy mfc140.dll to windows/system32 and add mfc140 into the Libraries section of winecfg).

For V10

linvst.so needs to go into ~/.wine/drive_c/Program Files/VSTPlugIns and then moved to the waveshell name 
mv linvst.so "WaveShell1-VST 10.0_x64.so"

(If a dialog doesn't appear then WaveShell1-VST 10.0_x64.so needs to be deleted from ~/.wine/drive_c/Program Files/VSTPlugIns and text files need to be deleted rm *.txt in ~/.wine/drive_c/users/"LinuxUserName"/Application Data/Waves Audio/Preferences and then do the above).

Then set the DAW to scan ~/.wine/drive_c/Program Files/VSTPlugIns and then scan, and a select folder dialog will appear.

Descend into the path by double clicking on (C:)
and then double click on Program Files (x86) (or Program Files if Plug-Ins V10 can't be found)
and then double click on Waves
and then just highlight Plug-Ins V10 with a single click (don't descend into it)
and then finish by clicking on open.

The DAW scan should then pick up the Waves plugins.

**Addictive Drums 2** (Addictive Drums 2 requires that the dll (and therefore the renamed linvst.so) needs to be loaded from the installation directory, ie a fixed path).
Drag the clip from the Beats window to outside of the Addictive Drums 2 window and then a midi file will appear (probably AD2Beat.mid) in /home/user/.wine/drive_c/users/user/My Documents/Addictive Drums 2/XXXXXX/Settings which should contain the clip and which can then be dragged to the DAW. Same thing applies for the recorded wav file (ADDrop.wav).

**BassMidi** sf2 and sfz.

**BFD3 FXpansion**

**Cobalt (Sanford) Synth**

**EZDrummer2** (visual glitches can be removed by choosing the Windows XP version in winecfg). 
Exports EZdrummer Libraries clips to a midi file in the /home/user/.wine/drive_c/ProgramData/Toontrack/EZdrummer folder (the last dragged clip that was dragged outside of the EZDrummer2 window) and then the midi file can be drag and dropped into the DAW.
Choose Select All (from the right side Menu drop down) to be able to drag multiple clips to the midi file in the /home/user/.wine/drive_c/ProgramData/Toontrack/EZdrummer folder

**FL Sytrus** needs winetricks corefonts to be installed for fonts. The UI might become blank after closing and reopening (minimizing).

**FM8** (might need the standalone FM8 to be run first so that the plugin's file browser files appear)

**Groove Machine** (drag and drop ok with the LinVst embedded window and standalone window drag and drop enabled versions).

**Guitar Rig 5** (same dll overrides as Kontakt)

**HoRNet Tape**

**Ignite Amps TPA-1 Amp Sim** 

**IK Amplitube 4**

**IK SampleTank**

**iLOK Software Licence Manager** needs winetricks msxml3 (might need crypt32.dll overrides, winetricks crypt32)

**Izotope Ozone** has multiple dlls and only the main dll (which is a vst dll) needs to have a linvst .so file associated with it. For instance, the Ozone 8.dll would have an associated Ozone 8.so file and none of the other dlls would have associated .so files (otherwise the DAW will try and load the other dlls which are not vst dlls and then produce errors).

**Klanghelm IVGI Saturation**

**Klanghelm MJUC Compressor**

**Klanghelm SDDR Saturation**

**Klanghelm VUMT analog style metering and channel tools**

**LePou Amp Sims**

**Line 6 Helix Native** (msvcr120.dll and gdiplus.dll overrides or winetricks vcrun2013 gdiplus wininet) (copy and paste username and password into the registration window)

**Melda MXXX Multi Effects** (turn GPU acceleration off)

**Mercuriall Spark Amp Sim**

**MinimogueVA**

**MT-PowerDrumKit** (Disable d2d1 in the Libraries section of winecfg) (drag and drop ok with the LinVst embedded window and standalone window drag and drop enabled versions).
Setting HKEY_CURRENT_USER Software Wine Direct3D MaxVersionGL 30002 might help with d2d1 (can also depend on hardware and drivers).

**MT-PowerDrumKit** has a Linux version that is a Windows plugin but it does not seem to need d2d1 and avoids any d2d1 problems.

**MT-PowerDrumKit** exports grooves to mtpdk.mid in the /home/user/Documents folder (the last dragged groove or fill to the composer) and then mtpdk.mid can be drag and dropped into the DAW.
To deal with multiple grooves/fills (compositions), drag the composition from the composer window to outside of the MT-PowerDrumKit window and then mtpdk.mid should contain the whole composition.

**Nebula4**

**Nick Crow Lab Amp Sims**

**OP-X PRO-II** (Disable d2d1 in the Libraries section of winecfg)

**Reaktor 6** (msvcp140.dll concrt140.dll dll overrides for Wine 2.0)

**S-Gear Amp Sim**

**Serum Synth** (can have some issues with Wines current d2d1, disable d2d1 or try a d2d1 override) (32 bit version seems to work better than the 64 bit version with a d2d1 version 6.1.7601.17514 32 bit dll override)
Setting HKEY_CURRENT_USER Software Wine Direct3D MaxVersionGL to 30002 might help but there might still be some d2d1 errors (can also depend on hardware and drivers).
Serum needs Wine Staging to register.

**Sforzando** (sfz drag and drop ok with the LinVst embedded window and standalone window drag and drop enabled versions).

**Spire Synth** (Disable d2d1 in the Libraries section of winecfg) (32 bit version seems to work ok with a d2d1 version 6.1.7601.17514 32 bit dll override)

**Steven Slate Drums SSD5** Drag the Groove/Fill from the Grooves window to outside of the SSD5 window and then a midi file will appear in /home/user/.wine/drive_c/users/user/Temp which should contain the clip and which can then be dragged to the DAW.
Samples might need to be manually extracted (by clicking on the slatepack files) (using Archive Manager).

**Stillwell plugins**

**Synth1** needs it's presets path setup (browse and locate using the opt button).

**T-RackS**

**TDR Nova parallel dynamic equalizer**

**TDR SlickEQ mixing/mastering equalizer** 

**TH3 Amp Sim Overloud**

**Toneboosters TrackEssentials** (disable d2d1 for Ferox)

**ValhallaPlate plate reverb**

**Voxengo Boogex Guitar Effects**

**Voxengo SPAN spectrum analyzer**

**Zampler RX**

