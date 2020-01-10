# TestVst

TestVst can roughly test how a vst dll plugin might run under Wine.

It tries to open the vst and display the vst window for roughly 8 seconds and then closes.

Usage is ./testvst.exe "vstfile.dll"

paths and vst filenames that contain spaces need to be enclosed in quotes.

Makefile32bits is for 32bit vst plugins.

If TestVst is run from a terminal and the output is looked at, then sometimes there can be an unimplemented function error in some dll and that dll can then be overriden.

