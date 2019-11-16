# TestVst

TestVst can roughly test how a vst dll plugin might run under Wine.

It tries to open the vst and display the vst window for roughly 8 seconds and then closes.

Usage is ./testvst.exe "vstfile.dll"

paths and vst filenames that contain spaces need to be enclosed in quotes.

Makefile32bits is for 32bit vst plugins.

