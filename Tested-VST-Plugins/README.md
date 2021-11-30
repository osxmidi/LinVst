## Tested vst's

LinVst tested with Waveform, Ardour, Reaper, Renoise, Bitwig Studio (For Bitwig 2.5 and later, In Settings->Plug-ins choose "Individually" plugin setting and check all of the LinVst plugins.
For Bitwig 2.4.3, In Settings->Plug-ins choose Independent plug-in host process for "Each plug-in" setting and check all of the LinVst plugins).

If a plugin needs a dll override it can be found by running TestVst from the terminal and looking for any unimplemented function in xxxx.dll errors in the output (the xxxx.dll is the dll that needs to be replaced with a dll override).


d2d1 based plugins

d2d1.dll can cause errors because Wine's current d2d1 support is not complete and using a d2d1.dll override might produce a black (blank) display.

Some plugins might need d2d1 to be disabled in the winecfg Libraries tab (add a d2d1 entry and then edit it and select disable).

A scan of the plugin dll file can be done to find out if the plugin depends on d2d1 

"strings vstname.dll | grep -i d2d1"

Wine (current versions) doesn't have the temp directory paths set in the registry, so importing Waves.reg in the Waves folder using regedit is probably a good idea as it sets the temp file path in the registry (for 64 bit Windows) and some plugins might need this info to install properly.

**Kontakt (Wine 4)

The Native Access install might hang when installing the ISO Driver and setting Windows 10 in winecfg might get around it.

Install winetricks cmd (to workaround possible cmd problems that can cause hangs).

**Kontakt Player 5.x and 6.x** (can try turning multiprocessing off for some setups).

Some additional dll overrides (below) might be needed for Kontakt.

Kontakt when used with Wine (Stable) 4.x needs a msvcp140.dll override whereas Kontakt used with Wine Staging 4.x doesn't. 

To override a Wine dll, copy the windows dll (64 bit) to ~/.wine/drive_c/windows/system32 and then override the dll to be native by entering the dll name in the winecfg Libraries option tab.

Native Access might need a msvcp140.dll override.

Temp directory paths might need to be setup in the registry for some Native Access installs (Use regedit to import Waves.reg in the Waves folder).

Native Access can abort or crash partway through a download (because Wine can have problems mounting the downloaded iso file) but the iso file has been downloaded, so a manual mounting and install of the downloaded iso file or a manual unzipping and install of the downloaded zip file in ~/.wine/drive_c/users/user/Downloads is needed.

All NI iso files need to be mounted using udf and the unhide option (because they are dual PC/Mac iso files).

sudo mount -t udf file.iso -o unhide /mnt

run winecfg and check the Drives tab for a windows drive letter associated with /mnt

cd /mnt and run the installer (wine setup.exe)

To unmount the iso change to a drirectory away from /mnt and then
sudo umount /mnt

For cd installs

sudo mount -t udf -o unhide /dev/sr0 /mnt

The winbind and libntlm0 and gnutls packages might need to be installed for net access.
For Arch based distros sudo pacman -Sy gnutls lib32-gnutls samba

Kontakt tries to install the vc redist dlls and they might not actually get installed.

To install the real vc redist dlls (in a dll override way) use winetricks vcrunxxxx.

**Waves plugins**.

It is possible to install Waves Central v12 or Waves Central v13 and then install Waves plugins by installing https://github.com/PietJankbal/powershell-wrapper-for-wine and following their instructions, but it is not all smooth sailing.

Wine Staging is needed (not just Wine).

Ignore any warning messages and manipulate/move the Waves Central window into different positions to get past any black window update problems (if selecting a checkbox, move the window to force a window update) and also maybe manipulate the mouse/improvise when selecting plugins to install.

-------

Waves Central v11 needs windows powershell which does not currently work.

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

Waves VST3 API-560 and Abbey Road Plates plugins tested with DDMF Metaplugin VST3 to VST2 wrapper and Wine Staging 4.x and Debian Stretch/Sid.

Bitwig seems to need the Waveshell to be unpacked into individual dll's using shell2vst.

The Waves plugins don't seem to work with Tracktion.

Reaper and Ardour seem to work with the Waveshell dll and the individual unpacked dll's.

Waves Central requires Wine Staging 3.x/4.x and also requires robocopy.exe to be installed (Windows Server 2003 Resource Kit Tools) and robocopy also needs a mfc42u.dll 32 bit override and a mfc42.dll 32 bit override to be placed in /windows/syswow64 and added to the winecfg Libraries tab.

Waves Central seems to require a usb stick to be inserted so that the install/license functions operate and the Windows Server 2003 Resource Kit Tools install (robocopy) seems to make a list of available drives at install time, so inserting a usb stick before installing Windows Server 2003 Resource Kit Tools would be a good idea as Waves Central uses robocopy for file transfers and Waves Central checks for usb drives and seems to require a usb stick to be present.

Waves Central also requires some simple registry additions

Just import the Waves.reg file (in the Waves folder) into the registry using regedit or manually do the below

wine regedit

Add the following environment string variables under HKEY_CURRENT_USER\Environment (New String Value)

COMMONPROGRAMFILES C:\Program Files\Common Files

PROGRAMFILES C:\Program Files

COMMONPROGRAMFILES(X86) C:\Program Files (x86)\Common Files

PROGRAMFILES(X86) C:\Program Files (x86)

PUBLIC C:\users\Public

-----------------

for 32 bit only systems it's

COMMONPROGRAMFILES C:\Program Files\Common Files

PROGRAMFILES C:\Program Files

PUBLIC C:\users\Public

Wine Staging is only needed for Waves Central and the Waves plugins themselves can run with any of the Wine versions, Wine-Stable, Wine-Devel or Wine Staging.

The winbind and libntlm0 and gnutls packages might need to be installed for net access as well.
For Arch based distros sudo pacman -Sy gnutls lib32-gnutls samba

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

**iLOK Software Licence Manager** seems to be ok with Wine Staging 5.x (no overrides needed) but might need things like kerberos to work depending on the plugin(s).

(for earlier Wine versions it might need winetricks msxml3 (might also need crypt32.dll overrides, winetricks crypt32))

**Audio/Midi clips**
The Audacity Windows version can be used to drag audio/midi clips from the vst window to Audacity (after maybe previewing/arranging them in the vst such as MT-PowerDrumKit and EZdrummer) and then there is the option of editing them in Audacity before saving and dragging them to the Linux Daw.
It works with Ardour as well.

**MT-PowerDrumKit** (Disable d2d1 in the Libraries section of winecfg) (drag and drop ok with the LinVst embedded window and standalone window drag and drop enabled versions).
Setting HKEY_CURRENT_USER Software Wine Direct3D MaxVersionGL 30002 might help with d2d1 (can also depend on hardware and drivers).

**MT-PowerDrumKit** has a Linux version that is a Windows plugin but it does not seem to need d2d1 and avoids any d2d1 problems.

**MT-PowerDrumKit** exports grooves to mtpdk.mid in the /home/user/Documents folder (the last dragged groove or fill to the composer) and then mtpdk.mid can be drag and dropped into the DAW.
To deal with multiple grooves/fills (compositions), drag the composition from the composer window to outside of the MT-PowerDrumKit window and then mtpdk.mid should contain the whole composition.

**EZDrummer2** (visual glitches can be removed by choosing the Windows XP version in winecfg). 
Exports EZdrummer Libraries clips to a midi file in the /home/user/.wine/drive_c/ProgramData/Toontrack/EZdrummer folder (the last dragged clip that was dragged outside of the EZDrummer2 window) and then the midi file can be drag and dropped into the DAW.
Choose Select All (from the right side Menu drop down) to be able to drag multiple clips to the midi file in the /home/user/.wine/drive_c/ProgramData/Toontrack/EZdrummer folder

**Addictive Drums 2** (Addictive Drums 2 requires that the dll (and therefore the renamed linvst.so) needs to be loaded from the installation directory, ie a fixed path).
Drag the clip from the Beats window to outside of the Addictive Drums 2 window and then a midi file will appear (probably AD2Beat.mid) in /home/user/.wine/drive_c/users/user/My Documents/Addictive Drums 2/XXXXXX/Settings which should contain the clip and which can then be dragged to the DAW. Same thing applies for the recorded wav file (ADDrop.wav).

**Steven Slate Drums SSD5** Drag the Groove/Fill from the Grooves window to outside of the SSD5 window and then a midi file will appear in /home/user/.wine/drive_c/users/user/Temp which should contain the clip and which can then be dragged to the DAW.
Samples might need to be manually extracted (by clicking on the slatepack files) (using Archive Manager).

**BFD3 FXpansion**

**Spire Synth** (Disable d2d1 in the Libraries section of winecfg) (32 bit version seems to work ok with a d2d1 version 6.1.7601.17514 32 bit dll override)

**OP-X PRO-II** Use the 32 bit XP version.
(The 64 bit version can run (but with some GUI problems) by disabling d2d1 in the Libraries section of winecfg)
It does run ok with Wine-tkg

**Toneboosters TrackEssentials** (disable d2d1 for Ferox)

**Serum Synth** 
Disable d2d1 in winecfg libraries and install gdiplus dll override (via winetricks, winetricks gdiplus).
Serum can have some issues with Wines current d2d1, disable d2d1 or try a d2d1 override) (32 bit version seems to work better than the 64 bit version with a d2d1 version 6.1.7601.17514 32 bit dll override)
Setting HKEY_CURRENT_USER Software Wine Direct3D MaxVersionGL to 30002 might help but there might still be some d2d1 errors (can also depend on hardware and drivers).
Serum needs Wine Staging to register.

**Guitar Rig 5** (same dll overrides as Kontakt)

**Reaktor 6** (msvcp140.dll concrt140.dll dll overrides for Wine 2.0)

**FM8** (might need the standalone FM8 to be run first so that the plugin's file browser files appear)

**Line 6 Helix Native** (msvcr120.dll and gdiplus.dll overrides or winetricks vcrun2013 gdiplus wininet) (copy and paste username and password into the registration window)

**S-Gear Amp Sim**

**TH3 Amp Sim Overloud**

**IK Amplitube 4**

**IK SampleTank**

**Martinic Plugins**

**Mercuriall Spark Amp Sim**

**Melda 
Installer needs corefonts (winetricks corefonts)
Melda MXXX Multi Effects** (turn GPU acceleration off)

**Izotope Ozone** has multiple dlls and only the main dll (which is a vst dll) needs to have a linvst .so file associated with it. For instance, the Ozone 8.dll would have an associated Ozone 8.so file and none of the other dlls would have associated .so files (otherwise the DAW will try and load the other dlls which are not vst dlls and then produce errors).

**T-RackS**

**Nebula4**

**VUMT**

**Sforzando** (sfz drag and drop ok with the LinVst embedded window and standalone window drag and drop enabled versions).
Sforzando newer versions might not work properly due to Wine d2d1 problems.

**BassMidi** sf2 and sfz.

**Groove Machine** (drag and drop ok with the LinVst embedded window and standalone window drag and drop enabled versions).

**Zampler RX**

**Stillwell plugins**

**Cobalt (Sanford) Synth**

**Synth1** needs it's presets path setup (browse and locate using the opt button).

**FL Sytrus** needs winetricks corefonts to be installed for fonts. The UI might become blank after closing and reopening (minimizing).

**Ignite Amps TPA-1 Amp Sim** 

**LePou Amp Sims**

**Nick Crow Lab Amp Sims**

**Voxengo Boogex Guitar Effects**

**Klanghelm MJUC Compressor**

**TDR SlickEQ mixing/mastering equalizer** 

**Arturia Plugins** install cmd via winetricks "winetricks cmd"

