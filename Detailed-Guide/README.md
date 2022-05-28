## Latency/Performance

**Hyperthreading**

For Reaper, in Options/Preferences/Buffering uncheck Auto-detect the number of needed audio processing threads and set 
Audio reading/processing threads to the amount of physical cores of the cpu (not virtual cores such as hyperthreading cores).

This can help with stutters and rough audio response.

Other Daws might have similar settings

---

**Kernel/Realtime Audio Setup**

Some distros/hardware can result in varying latency results.

LinVst has produced reasonable latency results with a low latency kernel along with realtime audio priorities also being setup/configured, but results can vary from system to system.

Some Distros have realtime audio priorities setup/configured and ready to go but some don't (see the next section on how to setup realtime audio priorities).

Realtime audio priority setup/configuration can affect audio performance.

A low latency kernel would be the preference along with realtime audio priorities being setup/configured.

Some problems might appear when realtime kernels (Debian real time kernel) are used with Wine, such as vst plugin windows freezing and/or frozen dialog boxes, possible poor audio performance (due to possible real time kernel incompatibilities with some windows plugins run with wine) etc.

For Debian based distros (Debian/MX Linux/Ubuntu etc) there is the Liquorix kernel.

rtirq https://github.com/rncbc/rtirq (rtirq-init for Ubuntu/Debian) and irqbalance may also have some effect.

A fast video card helps with Daw automation.

LinVst is memory access intensive.

Systems with faster memory are most likely to perform better ie (ddr4).

Having memory in 2 (or more) different motherboard memory banks may result in better performance then if the memory was just in one bank (interleaved memory).

Wineserver can be set to a higher priority which may have an effect on cpu load and system response on some systems/setups/plugins.

Wineserver can have it's priority level changed from normal to high or very high (root password needed), by right clicking on wineserver in System Monitor (start winecfg first to activate wineserver in System Monitor).

The Wineserver priority can be set with wine-staging by setting the STAGING_RT_PRIORITY_SERVER environmental variable between 1 and 99, for example STAGING_RT_PRIORITY_SERVER=60

---

**Set Realtime Audio Priorities**

If they are not set then cpu spiking etc can occur with Kontakt and other plugins.

```
sudo edit /etc/security/limits.conf

add

@audio - rtprio 95
@audio - memlock unlimited

------

run from Terminal

sudo usermod -a -G audio your-user-name

------------

sudo edit /etc/security/limits.d/audio.conf

add

@audio   -  rtprio     95
@audio   -  memlock    unlimited
#@audio   -  nice      -19

```
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

linvstconvert will not convert vst dll's in sub folders and so some plugins might not appear after a daw scan, so if that happens then use linvstconverttree.

linvstconverttree will convert vst dll's in all sub folders.

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

Optional Symlinks

A symlink can be used to access vst2 plugin folders from another more convenient folder.

Hidden folders such as /home/your-user-name/.wine/drive_c/Program Files/Steinberg/VSTPlugins can be accessed by the Daw by creating a symlink to them using a more convenient folder such as /home/your-user-name/vst2 for instance.

For example

ln -s "/home/your-user-name/.wine/drive_c/Program Files/Steinberg/VSTPlugins" /home/your-user-name/vst2/vst2plugins.so

creates a symbolic link named vst2plugins.so in the /home/your-user-name/vst2 folder that points to the /home/your-user-name/.wine/drive_c/Program Files/Steinberg/VSTPlugins folder containing the vst2 plugins.

The /home/your-user-name/.wine/drive_c/Program Files/Steinberg/VSTPlugins vst2 plugin folder needs to have had the vst2 plugins previously setup by using linvstconvert.

Then the Daw needs to have the /home/your-user-name/vst2 folder included in it's search path.

When the Daw scans the /home/your-user-name/vst2 folder it should also automatically scan the /home/your-user-name/.wine/drive_c/Program Files/Steinberg/VSTPlugins folder that contains the vst2 plugins (that have been previously setup by using linvstconvert).
