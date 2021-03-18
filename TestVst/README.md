# TestVst

TestVst can roughly test how a vst dll plugin might run under Wine.

It tries to open the vst dll and display the vst window for roughly 8 seconds and then closes.

Usage is ./testvst.exe "path to vstfile.dll"

paths and vst filenames that contain spaces need to be enclosed in quotes.

for example (cd into the testvst folder using the terminal)

./testvst.exe "/home/your-user-name/.wine/drive_c/Program Files/Steinberg/VstPlugins/delay.dll"

./testvst32.exe "/home/your-user-name/.wine/drive_c/Program Files (x86)/Steinberg/VstPlugins/delay32.dll"

testvst32.exe is for 32bit vst plugins.

Use testvst.exe from a folder that is not in a daw search path.

If testvst.exe.so and/or testvst32.exe.so are in any daw search paths then they can cause problems if the daw tries to load them.

-----

Batch Testing

-------

Gui Batch Testing.

testvstgui can be used to select a vst folder for testing (testvst32gui is used for testing 32 bit vst's).

Unzip the testvst zip file, and then (using the terminal) cd into the unzipped testvst folder.

then enter

chmod +x testvst-batch
(chmod +x testvst32-batch for 32 bit plugins)

Click on (start) testvstgui and then select a folder and then press the Start button and it will cycle through the vst's in the folder.

To cancel the vst cycling before all vst's are tested, open a Terminal and enter 
kill -9 $(pgrep -f testvst-batch)
or for 32 bit vst's
kill -9 $(pgrep -f testvst32-batch)

Starting testvstgui using a Terminal will show what vst is currently loading, so a problematic vst can be identified.

-------

Command Line Batch Testing.

Unzip the testvst zip file, and then (using the terminal) cd into the unzipped testvst folder.

then enter

chmod +x testvst-batch
(chmod +x testvst32-batch for 32 bit plugins)

Usage is ./testvst-batch "path to the vst folder containing the vst dll files"

paths that contain spaces need to be enclosed in quotes.

pathnames must end with a /

Same usage procedure applies to testvst32-batch which tests 32 bit vst dll files.

for example

./testvst-batch "/home/your-user-name/.wine/drive_c/Program Files/Steinberg/VstPlugins/"

./testvst32-batch "/home/your-user-name/.wine/drive_c/Program Files (x86)/Steinberg/VstPlugins/"

After that, testvst.exe (testvst32.exe for 32 bit plugins) will attempt to run the plugins one after another, any plugin dialogs that popup should be dismissed as soon as possible.

If a Wine plugin problem is encountered, then that plugin can be identified by the terminal output from testvst.exe (testvst32.exe for 32 bit plugins).

Use testvst.exe from a folder that is not in a daw search path.

If testvst.exe.so and/or testvst32.exe.so are in any daw search paths then they can cause problems if the daw tries to load them.

To cancel the vst cycling before all vst's are tested, open a Terminal and enter 
kill -9 $(pgrep -f testvst-batch)
or for 32 bit vst's
kill -9 $(pgrep -f testvst32-batch)

-----

TestVst can also be used to test dll overrides (wine dll's that are replaced by windows dll's) and if they are likely to work, without going through dll override trial and error using LinVst.

The terminal output of TestVst can sometimes be used to work out what dll override might be needed (if wine dll unimplemented function errors appear in the terminal output for instance).

Some plugins won't run due to various problems.
