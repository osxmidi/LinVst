**Waves plugins**.

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
