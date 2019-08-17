# LinVst

LinVst adds support for Windows vst's to be used in Linux vst capable DAW's.

## Quick Start Guide (see the Detailed-Guide folder for more info) 

(See the Wiki page for a visual guide https://github.com/osxmidi/LinVst/wiki)

Decide on what version to run, either the embedded window version or the standalone window version (the embedded version would probably be the default choice)

Copy all of the lin-vst-serverxxxx files (files with lin-vst-server in their names) from the version that was chosen to /usr/bin (either the embedded or standalone version folder)

Make a folder and place the windows vst's in it.

Start linvstconvert (in the convert folder) and then select the linvst.so from the chosen embedded or standalone window version folder.

Point linvstconvert to the folder containing the windows vst's and hit the Start (Convert) button.

Start up the linux DAW and point it to scan the folder containing the windows vst's.

More detailed info is at https://github.com/osxmidi/LinVst/tree/master/Detailed-Guide

Make info is at https://github.com/osxmidi/LinVst/tree/master/Make-Guide

Binary LinVst releases are available at https://github.com/osxmidi/LinVst/releases

If a LinVst version error pops up then LinVst probably needs to be reinstalled to /usr/bin and the older (renamed) linvst.so files in the vst dll folder need to be overwritten (using linvstconvert or linvstconvertree).

Scripts are also avaliable as an alternative to linvstconvert in the convert and manage folders and also https://github.com/Goli4thus/linvstmanage

## Common Problems/Possible Fixes

If a LinVst version error pops up then LinVst probably needs to be reinstalled to /usr/bin and the older (renamed) linvst.so files in the vst dll folder need to be overwritten (using linvstconvert or linvstconvertree).

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

For details about overriding dll's, see the next section (Wine Config)./home/user/.wine/drive_c/ProgramData//home/user/.wine/drive_c/ProgramData/

Drag and Drop is enabled for the embedded LinVst version used with Reaper/Tracktion/Waveforn/Bitwig but it's only for items dragged and dropped onto the vst window and not for items dragged and dropped from the vst window to the DAW/host or to the Desktop window.
Usually the dragged item (dragged outside of the vst's window) will be saved as a midi or wav file in a location that is most likely to be located in one of the vst's folders ie a folder in My Documents or a folder that the vst installation has created. The midi or wav file can then be dragged to the DAW.
See MT-PowerDrumKit and EZDrummer2 and Addictive Drums 2 and SSD5 below for some details.

Also, see the tested vst's section for some possible tips.

**Renoise**

Sometimes a synth vst doesn't declare itself as a synth and Renoise might not enable it.

A workaround is to install sqlitebrowser

sudo add-apt-repository ppa:linuxgndu/sqlitebrowser-testing

sudo apt-get update && sudo apt-get install sqlitebrowser

Add the synth vst's path to VST_PATH and start Renoise to scan it.

Then exit renoise and edit the database file /home/user/.renoise/V3.1.0/ CachedVSTs_x64.db (enable hidden folders with right click in the file browser).

Go to the "Browse Data" tab in SQLite browser and choose the CachedPlugins table and then locate the entry for the synth vst and enable the "IsSynth" flag from "0" (false) to "1" (true) and save.

