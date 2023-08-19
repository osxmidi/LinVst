# LinVst

LinVst adds support for Windows vst's to be used in Linux vst capable DAW's.

See LinVst-X (https://github.com/osxmidi/LinVst-X) for running vst plugins in a single Wine process so that plugins can communicate with each other or plugins that can use shared samples between instances will be able to communicate with their other instances.

See LinVst3 https://github.com/osxmidi/LinVst3 for running vst3 plugins.

## Quick Start Guide (see the Detailed-Guide folder for more info) 

(See the Wiki page for a visual guide https://github.com/osxmidi/LinVst/wiki)

Copy all of the lin-vst-serverxxxx files (files with lin-vst-server in their names) to /usr/bin.

Install the vst's.

The vst's will probably be installed by default to a Wine folder, something like ~/.wine/drive_c/Program Files/Steinberg/VSTPlugins (which is similar to where they are installed on Windows).

It's also possible with most plugins to make a folder and install the windows vst's into it.

linvstconvert is used to prepare vst2 dll files for use in a daw using the linvst.so file.

Start linvstconvert (in the convert folder) and then select the linvst.so file.

Point linvstconvert to the folder containing the windows vst's and hit the Start (Convert) button.

Start up the linux DAW and point it to scan the folder containing the windows vst's.

If new vst plugins are added to a folder, then just run linvstconvert again on that folder.

More detailed install info is at https://github.com/osxmidi/LinVst/tree/master/Detailed-Guide which also includes possible performance details.

Install details for vsts are at https://github.com/osxmidi/LinVst/tree/master/Tested-VST-Plugins.

Make info is at https://github.com/osxmidi/LinVst/tree/master/Make-Guide

Binary LinVst releases are available at https://github.com/osxmidi/LinVst/releases

If a LinVst version error pops up then LinVst probably needs to be reinstalled to /usr/bin and the older (renamed) linvst.so files in the vst dll folder need to be overwritten (using linvstconvert).

Scripts are also avaliable as an alternative to linvstconvert in the convert and manage folders and also https://github.com/Goli4thus/linvstmanage

# Daw Notes

Makefiles in the dragwin folder are for (possible) drag and drop from the plugin to the daw.

**Hyperthreading**

For Reaper, in Options/Preferences/Buffering uncheck Auto-detect the number of needed audio processing threads and set 
Audio reading/processing threads to the amount of physical cores of the cpu (not virtual cores such as hyperthreading cores).

This can help with stutters and rough audio response.

Other Daws might have similar settings.

**Waveform**

For Waveform, (maybe) disable sandbox option for plugins.

**Bitwig**

For Bitwig, in Settings->Plug-ins choose "Individually" plugin setting and check all of the LinVst plugins.
For Bitwig 2.4.3, In Settings->Plug-ins choose Independent plug-in host process for "Each plug-in" setting and check all of the LinVst plugins.

**Renoise**

Choose the sandbox option for plugins.

Sometimes a synth vst might not declare itself as a synth and Renoise might not enable it.

A workaround is to install sqlitebrowser

sudo add-apt-repository ppa:linuxgndu/sqlitebrowser-testing

sudo apt-get update && sudo apt-get install sqlitebrowser

Add the synth vst's path to VST_PATH and start Renoise to scan it.

Then exit renoise and edit the database file /home/user/.renoise/V3.1.0/ CachedVSTs_x64.db (enable hidden folders with right click in the file browser).

Go to the "Browse Data" tab in SQLite browser and choose the CachedPlugins table and then locate the entry for the synth vst and enable the "IsSynth" flag from "0" (false) to "1" (true) and save.

## Common Problems/Possible Fixes

Native Access has switched to using Powershell and is therefore not supported by Wine at this time.

An older version of Native Access can be used https://support.native-instruments.com/hc/en-us/articles/360000407909-Native-Access-1-Legacy-Installers-for-Older-Operating-Systems and then install tips (Kontakt etc) can be followed https://github.com/osxmidi/LinVst/tree/master/Tested-VST-Plugins

For recommended Linux kernel and audio setup info see https://github.com/osxmidi/LinVst/blob/master/Detailed-Guide/README.md

To uninstall vst's that have been installed, open a Terminal and enter "wine uninstaller" (without the quotes) and hit Enter.

For performance setup see https://github.com/osxmidi/LinVst/tree/master/Realtime-Audio-Config

If window resizing does not work, then after a resize the UI needs to be closed and then reopened for the new window size to take effect.

One of the problems in trying to intially load multiple vst's all at once is that when the Daw tries to load them, one or more plugins might not scan (and/or run) and the Daw scan might stop/hang.

Some plugins might not load for various reasons, and one of the basic ways to find problem plugins is to run the Daw from the terminal and look at the LinVst output for successful plugin loading as it tries to load each plugin.

It might require multiple Daw plugin rescans (maybe manual plugin rescans for some Daws) and also eliminating problem plugins by deleting their associated (renamed) .so files to get all working plugins scanned.

To be sure about what plugin might run and what plugin might not, it's probably best to proceed by adding one plugin at a time and/or by using the TestVst utility https://github.com/osxmidi/LinVst/releases which can also be run in batch mode to test multiple vsts.

TestVst can also be used to test dll overrides (wine dll's that are replaced by windows dll's) and if they are likely to work, without going through dll override trial and error using LinVst.

The terminal output of TestVst can sometimes be used to work out what dll override might be needed (if wine dll unimplemented function errors appear in the terminal output for instance).

An "X Error of failed request:  GLXBadFBConfig" error might need export MESA_GL_VERSION_OVERRIDE=4.5

Some plugins won't run due to various problems.

Flatpak Daw installations might not work with LinVst.

----

To install a windows vst exe installation file use wine "path to the exe file"

The quotes also handle paths with spaces in their names.

```
For example, if the windows vst exe installation file is located in the Downloads folder and is named delay.exe, then (from the terminal) wine ~/"Downloads/delay.exe" will start the installation (~ stands for the user path ie /home/your-user-name). 

```

msi files can be clicked on to start their installation.

```
Usually by default vst installers install to a path something like ~/.wine/drive_c/Program Files/Steinberg/VSTPlugins and that would usually be the path/folder to use with linvstconvert and then that path would be set in the DAW's plugin search paths.

To change into the plugin directory (using the terminal) cd ~/".wine/drive_c/Program Files/Steinberg/VSTPlugins"

```
-----

If a LinVst version error pops up then LinVst probably needs to be reinstalled to /usr/bin and the older (renamed) linvst.so files in the vst dll folder need to be overwritten (using linvstconvert).

LinVst looks for wine in /usr/bin and if there isn't a /usr/bin/wine then that will probably cause problems.
/usr/bin/wine can be a symbolic link to /opt/wine-staging/bin/wine (for wine staging) for example.

Quite a few plugins need winetricks corefonts installed for fonts.

A large number of vst plugin crashes/problems can be basically narrowed down to the following dll's and then worked around by overriding/disabling them.

Quite a few vst plugins rely on the Visual C++ Redistributable dlls msvcr120.dll msvcr140.dll msvcp120.dll msvcp140.dll etc

Some vst plugins might crash if the Visual C++ Redistributable dlls are not present in /home/username/.wine/drive_c/windows/system32 for 64 bit vst's and /home/username/.wine/drive_c/windows/syswow64 for 32 bit vst's, and then overridden in the winecfg Libraries tab

Some vst plugins might not work due to Wines current capabilities or for some other reason.

Use TestVst for testing how a vst plugin might run under Wine.

Some vst plugins rely on the d2d1 dll which is not totally implemented in current Wine.

If a plugin has trouble with it's display then disabling d2d1 in the winecfg Libraries tab can be tried.

Some plugins need Windows fonts (~/.wine/drive_c/windows/Fonts) ./winetricks corefonts

Setting HKEY_CURRENT_USER Software Wine Direct3D MaxVersionGL to 30002 (MaxVersionGL is a DWORD hex value) might help with some plugins and d2d1 (can also depend on hardware and drivers).

wininet is used by some vst's for net access including registration and online help etc and sometimes wines inbuilt wininet might cause a crash or have unimplemented functions.

wininet can be overridden with wininet.dll and iertutil.dll and nsi.dll

The winbind and libntlm0 and gnutls packages might need to be installed for net access (sudo apt-get install winbind sudo apt-get install gnutls-bin sudo apt-get install libntlm0)
For Arch based distros sudo pacman -Sy gnutls lib32-gnutls samba

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

For details about overriding dll's, see the Wine Config section in the Detailed Guide https://github.com/osxmidi/LinVst/tree/master/Detailed-Guide

To enable 32 bit vst's on a 64 bit system, a distro's multilib needs to be installed (on Ubuntu it would be sudo apt-get install gcc-multilib g++-multilib)

Drag and Drop is enabled for the embedded LinVst version used with Reaper/Tracktion/Waveforn/Bitwig but it's only for items dragged and dropped onto the vst window and not for items dragged and dropped from the vst window to the DAW/host or to the Desktop window.
Usually the dragged item (dragged outside of the vst's window) will be saved as a midi or wav file in a location that is most likely to be located in one of the vst's folders ie a folder in My Documents or a folder that the vst installation has created. The midi or wav file can then be dragged to the DAW.
See MT-PowerDrumKit and EZDrummer2 and Addictive Drums 2 and SSD5 in the Tested VST's folder at https://github.com/osxmidi/LinVst/tree/master/Tested-VST-Plugins for some details.

Also, see the Tested VST's folder at https://github.com/osxmidi/LinVst/tree/master/Tested-VST-Plugins for some vst plugin setups and possible tips.

-------

If plugins were working then suddenly don't, try testing if it's the wineprefix that is causing the problems

Basically to test a plugin using a temporary new wineprefix, open a Terminal and then set it up to use the new wineprefix (only valid for the Terminal that was opened and setup, not other Terminals).

To test if it's the wineprefix that's causing the problem (possible file corruption, installation problems etc), 

open a Terminal

cd into your home directory

then enter into the Terminal

mkdir .my-new-prefix

export WINEPREFIX=/home/yourusername/.my-new-prefix (this sets the wineprefix to use for this Terminal only, system files need to be setup to make it permanent)

winecfg

and that will create a new wineprefix named .my-new-prefix

and then in the same Terminal window install a simple vst "wine delay.exe" or whatever vst

Reinstall LinVst so that possible LinVst file corruption in not an issue, download LinVst from the releases page and copy all of the lin-vst-server files to /usr/bin

sudo cp /home/yourusername/Downloads//LinVst-4.7/lin-vst* /usr/bin

start linvstconvert and select the linvst.so file from the downloaded LinVst 4.7 folder

find the vst dll in the /home/yourusername/.my-new-prefix folder and hit the start button.

Start your daw up (from the same terminal window) and set the search path to the vst folder(s) in /home/yourusername/.my-new-prefix

restart the daw

See if the daw loads and scans the vst ok.

If the plugin runs ok in this new wineprefix (/home/yourusername/.my-new-prefix) and the same plugin doesn't run ok 
in the old wineprefix ~/.wine (default wineprefix) then there is a high probablity that the ~/.wine (default wineprefix) 
is causing some problems.

To test the same plugin using the default wineprefix (~/.wine), delete the temporary wineprefix rm -R .my-new-prefix and then exit the Terminal used for testing and then install the plugin into the default wineprefix and set it up using linvstconvert.

-------

Wine tkg install instructions https://github.com/osxmidi/LinVst/tree/master/Wine-tkg

Optional Symlinks

A symlink can be used to access vst2 plugin folders from another more convenient folder.

Hidden folders such as /home/your-user-name/.wine/drive_c/Program Files/Steinberg/VSTPlugins can be accessed by the Daw by creating a symlink to them using a more convenient folder such as /home/your-user-name/vst2 for instance.

For example

ln -s "/home/your-user-name/.wine/drive_c/Program Files/Steinberg/VSTPlugins" /home/your-user-name/vst2/vst2plugins.so

creates a symbolic link named vst2plugins.so in the /home/your-user-name/vst2 folder that points to the /home/your-user-name/.wine/drive_c/Program Files/Steinberg/VSTPlugins folder containing the vst2 plugins.

The /home/your-user-name/.wine/drive_c/Program Files/Steinberg/VSTPlugins vst2 plugin folder needs to have had the vst2 plugins previously setup by using linvstconvert.

Then the Daw needs to have the /home/your-user-name/vst2 folder included in it's search path.

When the Daw scans the /home/your-user-name/vst2 folder it should also automatically scan the /home/your-user-name/.wine/drive_c/Program Files/Steinberg/VSTPlugins folder that contains the vst2 plugins (that have been previously setup by using linvstconvert).

------

Wine Config

LinVst expects wine to be found in /usr/bin.

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

------
