# LinVst

LinVst enables Windows VSTs to be used in Linux DAWs which support the VST standard.


## What is LinVst?

LinVst is a bridge between Linux DAWs and Windows VSTs. Windows VSTs are ran with Wine.

It looks like this:

|  |  |  |  |
|--|--|--|--|
| Linux DAW | **LinVst** | Wine | Windows VST (.dll) |

The included `linvst.so` is a native VST. When loaded with a Linux DAW, it looks for a Windows `.dll` with the same name, and runs it with Wine.

## How to use LinVst

Make sure you have Wine installed.


### Manually

1. Copy `lin-vst-server.exe` and `lin-vst-server.so` to `/usr/bin`
2. Copy `linvst.so` to the location of the Windows VST `.dll` file that you want to run in a Linux DAW. This will be something like `~/.wine/drive_c/Program Files/VstPlugins`
3. Rename `linvst.so` to the name of the Windows VST .dll. For example, for `example.dll`, rename `linvst.so` to `example.so`
4. Set your Linux DAW to look in the folder that the Windows VST is in
5. Load `example.so` in the Linux DAW. `example.so` will load and run the Windows VST with the matching name, `example.dll`

The renamed `.so` file acts as a bridge to the `.dll` file, so the `.dll` and the `.so` files need to be kept together in the same folder.


### Using the included GUI tool

1. Copy `lin-vst-server.exe` and `lin-vst-server.so` to `/usr/bin`
2. Run the linvstconvert GUI tool.


### Using the CLI tool

1. Copy `lin-vst-server.exe` and `lin-vst-server.so` to `/usr/bin`
2. Run `linvstconvert [path]` or `linvstconverttree [path]`. The path will be something like `~/.wine/drive_c/Program Files/VstPlugins`. So for example, `linvstconvert "~/.wine/drive_c/Program Files/VstPlugins"`.
  - This will convert all files in the specified path. `linvstconverttree` is recursive.
3. Set your Linux DAW to look in the folder that the Windows VST is in
4. Load `example.so` in the Linux DAW. `example.so` will load and run the Windows VST with the matching name, `example.dll`


## Included batch renaming tools

The included tools `linvstconvert` and `linvstconverttree` will create renamed `linvst.so` files for each VST `.dll` in a specified folder, essentially doing the above steps for you. `linvstconvert` only processes one folder, while `linvstconverttree` recursively explores folders (i.e. explores each child of a specified parent folder). There are GUI and CLI versions.

You may need to use sudo (root privileges) for some folders.

Copying or moving plugins (and in some cases associated files like presets) to a folder with user permissions is a good idea, unless the VST plugin requires a fixed path.

To illustrate, this is a folder that contains some Windows VSTs:
```
Delay.dll
Compressor.dll
Chorus.dll
Synth.dll
```

After using the renaming tools, the folder will look like:
```
Delay.dll
Delay.so
Compressor.dll
Compressor.so
Chorus.dll
Chorus.so
Synth.dll
Synth.so
```

Another way is to make a symbolic link to `~/.wine/drive_c/Program Files/VstPlugins/Delay.so` or to the whole `~/.wine/drive_c/Program Files/VstPlugins` directory from a more convenient folder, such as `/home/user/vst`. Then you can add `/home/user/vst` to the Linux DAW's plugin search paths.


### Setting up custom Wine prefixes (`WINEPREFIX`)

There can be multiple Wine prefix folders (the default is `~/.wine`) containing various VST `.dll` plugins. Each Wine prefix can have a different Wine configuration, including native Windows DLLs and DLL override settings. Individual plugins can have their own Wine prefix environment.

Windows VST plugins that require a different Wine environment can be installed into different Wine prefixes by creating a new Wine prefix or using an already existing Wine prefix and then running the Windows VST installation program inside it. See [WineHQ's section on Wineprefixes](https://wiki.winehq.org/FAQ#Wineprefixes) for how to set up Wine prefixes and run programs inside them.

When LinVst is loaded from within a Wine prefix, it uses the Wine prefix it's inside. So `~/.wine-hello/drive_c/Program Files/VstPlugins/Synth.so` (which uses `Synth.dll` inside the same folder) will use `~/.wine-hello` as the Wine prefix. This also works for symbolic links.

Make sure your Linux DAW is set to look for any paths inside newly created Wine prefixes.


## Common Problems and Possible Fixes

LinVst looks for wine in `/usr/bin`. If there isn't a `/usr/bin/wine`, it will likely cause problems. If the VST bridged with LinVST is not loading at all and you've followed the above steps properly, check if `/usr/bin/wine` exists. If it doesn't, you can make a symbolic link at `/usr/bin/wine` which links to `/opt/wine-staging/bin/wine` for example (for Wine staging).

A large number of VST plugin crashes/problems are due to VSTs that require the native Windows DLLs. Common DLLs include:
- Visual C++ Redistributable (`msvcr120.dll`, `msvcr140.dll`, `msvcp120.dll`, `msvcp140.dll`, the DLLs depend on the version of Visual C++ required).
- `d2d1.dll` (Direct2D).
- `wininet.dll`, `iertutil.dll` and `nsi.dll` (used for internet access, e.g. registration and online help)
- `gdiplus.dll`

This can be worked around by using the native Windows DLL versions of the required DLLs and overriding it in the `winecfg` library secton.

However it's recommended to use (Winetricks)[https://github.com/Winetricks/winetricks] instead which does this for you. Make sure to install `cabextract` with your package manager (apt or rpm) as Winetricks often requires this to extract files from installers.

For the above DLLs, you can run `winetricks vcrun2013 vcrun2015 wininet gdiplus` in the terminal. Winetricks also has a force flag (`winetricks --force vcrun2013`) if required.

To enable 32-bit VSTs on a 64-bit system, a distro's multilib needs to be installed (on Ubuntu it would be `sudo apt install gcc-multilib g++-multilib`)

For details about overriding DLLs, see the next section, Wine Config.

Setting `HKEY_CURRENT_USER\Software\Wine\Direct3D\MaxVersionGL` to 30002 (MaxVersionGL is a DWORD hex value) in Wine's regedit might help with some plugins and `d2d1.dll` (can also depend on hardware and drivers).


## Wine Config

<!-- ???: Setting WINELOADER to a new wine path might possibly be used for different wine paths. -->
### Keyboard Input

Keyboard input etc can be enabled for the standalone window LinVst version only, by creating a UseTakeFocus string and setting it to a value of N, in `HKEY_CURRENT_USER/Software/Wine/X11 Driver` (via Wine's regedit). More keyboard control (not recommended) can be enabled for the standalone window LinVst version only, by unchecking the `winecfg` option "Allow the window manager to control the Windows". 

Keyboard input should be OK for the embedded window version. The embedded window version needs to be run with the `winecfg` option "Allow the window manager to control the Windows" checked (which is usually the default).

Sometimes usernames and passwords might need to be copied and pasted into the window because manual entry might not work in all cases.


### DLL Overrides

Sometimes a Windows VST needs the native Windows version of a DLL. Sometimes required DLLs might be missing and additional Windows redistributable packages might need to be installed.

Finding out what DLLs to possibly override can be done by running `strings vstname.dll | grep -i dll`, which will display a list of DLLs from the plugins DLL file. For instance, if the dll list contains `d2d1.dll` and there are problems running the plugin, then `d2d1.dll` (Direct2D) might possibly be why the plugin isn't working.

If the Wine debugger displays "unimplemented function in XXXX.dll" somewhere in it's output, then that DLL usually needs to be overridden with a Windows DLL.

For instructions on overriding DLLs in Wine's winecfg, see [the Libraries section on the Wine User's Guide](https://wiki.winehq.org/Wine_User%27s_Guide#Libraries_Settings). On 64-bit Wine prefixes, 64-bit `.dll` files are located at `/home/username/.wine/drive_c/Windows/system32`, while 32-bit `.dll` files are located at `/home/username/.wine/drive_c/Windows/syswow64`. On 32-bit Wine prefixes, 32-bit `.dll` files are located at `/home/username/.wine/drive_c/Windows/system32`.

Common DLL issues:
- Some Windows VSTs use D3D, and Wine uses Linux OpenGL to implement D3D, so a capable Linux OpenGL driver/setup might be required for some Windows VSTs
- Some D3D `.dll` overrides might be needed for some Windows VSTs (D3D/OpenGL Wine advice can be found at gaming forums and other forums)
- Disabling `d2d1.dll` in the Libraries section of winecfg might help with some Windows VSTs
- For wininet (often used for online registration and online helps), running `winetricks wininet` and/or installing `winbind` for your distro (e.g. `sudo apt install winbind`) might help.

Wine related issues:
- Turning off the VST's multiprocessor support and/or GPU acceleration might help in some cases, due to what features Wine currently supports (depends on Wine version).
- In certain cases Wine can take time to load properly if it's the first time you've used it since boot, which might cause LinVst to crash. Try initializing Wine with `wine wineboot` or running any Wine program beforehand.
- Try upgrading to the latest Wine version (wine-devel 3.3 as of writing).


## Latency/Performance

Some Linux distributions and Wine versions can result in varying latency results.

LinVst has produced reasonable latency results on Ubuntu Studio with a low latency kernel and a real time kernel, and on Debian 9 with a real time kernel (AV Linux should be similar), but not so much on some other distros without a low latency/real time kernel.

Results can vary from system to system, so some distros, setups and Wine versions might be better than some others in terms of latency. Latency also depends on the audio hardware/drivers as well.

For more information on latency on Linux, see [this page](https://wiki.linuxaudio.org/wiki/system_configuration).

Various notes:
- rtirq https://github.com/rncbc/rtirq (rtirq-init for Ubuntu/Debian) may have some effect. Ubuntu Studio and it's low latency kernel combined with rtirq-init, can produce reasonable latency results.
- LinVst is memory access intensive and having memory in 2 (or more) different motherboard memory banks may result in better performance then if the memory was just in one bank (interleaved memory).
- Wineserver can be set to a higher priority, which may have an effect on cpu load and system response on some systems, setups or plugins.
- wineserver can have it's priority level changed from normal to high or very high (requires root access), by right clicking on wineserver in System Monitor (start winecfg first to activate wineserver in System Monitor).
- The wineserver priority can be set with wine-staging by setting the STAGING_RT_PRIORITY_SERVER environmental variable between 1 and 99, for example STAGING_RT_PRIORITY_SERVER=60


## Tested setups

#### Tested Linux DAWs (with Wine 2)
- Tracktion 7
- Ardour 5.6
- Bitwig Studio 2
- Reaper 5.5
- Renoise 3.1

#### Tested VSTs (with Wine 2.8, notes and requirements may not apply to newer versions of Wine)
- Kontakt Player 5.6.8
  - May require multiprocessing to be turned off
  - May need msvcp140.dll (provided with a Visual C++ version)
  - May also require msvcp140.dll concrt140.dll api-ms-win-crt-time-l1-1-0.dll api-ms-win-crt-runtime-l1-1-0.dll ucrtbase.dll
- Guitar Rig 5 (same DLLs as Kontakt)
- Reaktor 6 (msvcp140.dll concrt140.dll dll overrides for Wine 2.0)
- FM8
- Line 6 Helix Native (msvcr120.dll and gdiplus.dll overrides) (copy and paste username and password into the registration window)
- S-Gear Amp Sim
- TH3 Amp Sim
- IK Amplitube 4
- IK SampleTank
- Addictive Drums 2:
  - Requires the DLL and matching `.so` (the renamed `linvst.so`) to be loaded from the installation directory (i.e. a fixed path).
- EZDrummer2:
  - Choose Mixer window before quiting if drum kit is playing to avoid possible hang when quitting
- Mercuriall Spark Amp Sim
- Melda MXXX Multi Effects:
  - May require GPU acceleration off
- T-RackS
- Nebula4
- VUMT
- Sforzando:
  - Drag and drop is OK with the LinVst embedded window and standalone window drag-and-drop enabled versions
- Groove Machine:
  - Drag and drop is OK with the LinVst embedded window and standalone window drag-and-drop enabled versions
- Zampler RX
- Stillwell plugins
- Cobalt (Sanford) Synth
- Spire Synth:
  - May require disabling d2d1 in the Libraries section of winecfg
  - 32-bit version seems to work OK with the 32-bit version of `d2d1.dll` (version 6.1.7601.17514)
- OP-X PRO-II:
  - May require disabling d2d1 in the Libraries section of winecfg
- MT-PowerDrumKit:
  - May require disabling d2d1 in the Libraries section of winecfg
  - Drag and drop is OK with the LinVst embedded window and standalone window drag-and-drop enabled versions
  - Setting `HKEY_CURRENT_USER\Software\Wine\Direct3D\MaxVersionGL` to 30002 might help with `d2d1.dll` (can also depend on hardware and drivers)
- Ignite Amps TPA-1 Amp Sim 
- LePou Amp Sims
- Nick Crow Lab Amp Sims
- Voxengo Boogex Guitar Effects
- Klanghelm MJUC Compressor
- TDR SlickEQ mixing/mastering equalizer 
- Toneboosters TrackEssentials (disable d2d1 for Ferox)
- Serum:
  - Can have some issues with Wine 2.8's current `d2d1.dll`, disable d2d1 or try a d2d1 override. 32-bit version seems to work better than the 64-bit version with an override of the 32-bit version of d2d1 (version 6.1.7601.17514).
  - Setting `HKEY_CURRENT_USER\Software\Wine\Direct3D\MaxVersionGL` to 30002 might help, but there might still be some `d2d1.dll` errors (can also depend on hardware and drivers)


## Building

### Requirements
- The `plugininterfaces` folder contained within the [VST2_SDK](https://www.steinberg.net/en/company/developers.html) folder needs to be placed in the LinVst files folder.
- Wine libwine development files. For Ubuntu/Debian, `sudo apt install libwine-development-dev` (for Ubuntu 14.04 it's `sudo apt-get install wine1.8` and `sudo apt-get install wine1.8-dev`). wine-devel packages for other distros (sudo apt-get install wine-devel).
- libX11 development needed for embedded version (sudo apt-get install libx11-dev)
 - Include and Library paths might need to be changed in the Makefile for various 64-bit and 32-bit Wine development paths (otherwise 32-bit compiles might try to link with 64-bit libraries, etc.).
- Additional 32-bit development libraries are probably needed.

On Ubuntu/Debian, for 64-bit: 
```
sudo dpkg --add-architecture i386
sudo apt-get install libc6-dev-i386
sudo apt-get install gcc-multilib g++-multilib
sudo apt-get install libwine-development-dev:i386
```

For Debian, you'll need to enable the non-free repositories (run sudo apt update after).

The convert folder is for making the linvstconvert and linvstconverttree utilities that name convert Windows vst dll filenames (.dll) to Linux shared library filenames (.so).

The makegtk etc files in the convert folder contain simple commands for making the convert utilities.

A -no-pie option might be needed on some systems for the linvstconvert and linvstconverttree utilities icons to appear.


### Building

```
sudo make clean
make
sudo make install
```

This installs lin-vst-server.exe and lin-vst-server.exe.so to /usr/bin and installs linvst.so to /vst in the source code folder (`lin-vst-serverst` for standalone window version). It also installs `lin-vst-server32.exe` and `lin-vst-server32.exe.so` to `/usr/bin` for 32-bit VST support (`lin-vst-serverst32` for standalone window version).

Repo contents:
- `Makefile-embed-6432` and `Makefile-standalone-6432` build a LinVst version that autodetects and automatically runs both 64-bit VSTs and 32-bit VSTs.
- `Makefile-embed-6432` (64-bit and 32-bit VSTs) and `Makefile-embed-64` (64-bit VSTs only) are for the DAW embedded window option.
- `Makefile-standalone-6432` (64-bit and 32-bit VSTs) and `Makefile-standalone-64` (64-bit VSTs only) are for a standalone window version that can be useful for some DAWs.
- `LinVst-Linux32bit-only.zip` contains makefiles for Linux 32-bit only systems and makes and installs `lin-vst-serverlx32` (`lin-vst-serverstlx32` for standalone window version).
- Undefining `WINONTOP` for the standalone window versions will make a standalone window version that has standard window behaviour (not an on-top window).
- Defining `EMBEDRESIZE` enables VST window resizing for the embedded window version (currently only works with Reaper).
- Defining `EMBEDTHREAD` enables an embedded version display delay that may be needed for slower systems.

See Makefile comments for define options.

Tracktion/Waveform might have an embedded window offset problem where the plugin's GUI is misaligned with the Tracktion/Waveform plugin window and adding `-DTRACKTIONWM` to the embedded Makefile's `BUILD_FLAGS`, `BUILD_FLAGS_WIN` and `BUILD_FLAGS_WIN32` lines might help (for Tracktion/Waveform use only).

The `32bitonly` folder contains makefiles for 32-bit systems and 32-bit VSTs only.

