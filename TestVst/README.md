# TestVst

TestVst can roughly test how a vst dll plugin might run under Wine.

It tries to open the vst and display the vst window for roughly 8 seconds and then closes.

Usage is ./testvst.exe "vstfile.dll"

paths and vst filenames that contain spaces need to be enclosed in quotes.

Makefile32bits is for 32bit vst plugins.

If TestVst is run from a terminal and the output is looked at, then sometimes there can be an unimplemented function error in some dll and that dll can then be overriden.

Some vst plugins might not work due to Wines current capabilities or for some other reason.

Some vst plugins rely on the d2d1 dll which is not totally implemented in current Wine.

If a plugin has trouble with it's display then disabling d2d1 in the winecfg Libraries tab can be tried.

-----

Batch Testing

For testing multiple vst dll files at once, place testvst.exe (and/or testvst32.exe for 32 bit plugins) and testvst-batch (testvst32-batch for 32 bit plugins) into the vst folder containing the vst dll files.

Using the terminal, cd into the vst folder and enter

chmod +x testvst-batch
(chmod +x testvst32-batch for 32 bit plugins)

then enter

./testvst-batch
(./testvst32-batch for 32 bit plugins)

After that, testvst.exe (testvst32.exe for 32 bit plugins) will attempt to run the plugins one after another, any plugin dialogs that popup should be dismissed as soon as possible.

If a Wine plugin problem is encountered, then that plugin can be identified by the terminal output from testvst.exe (testvst32.exe for 32 bit plugins).


