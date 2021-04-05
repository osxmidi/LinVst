## To use LinVst

The vst dll files need to have associated .so files (that have the same name as the vst dll files), so that a Linux DAW can load them.

linvstconvert can produce the associated .so files automatically.

Install the vst's.

The vst's will probably be installed by default to a Wine folder, something like ~/.wine/drive_c/Program Files/Steinberg/VSTPlugins (which is similar to where they are installed on Windows).

It's also possible with most plugins to make a folder and install the windows vst's into it.

Start linvstconvert (in the convert folder) and then select the linvst.so file.

Point linvstconvert to the folder containing the windows vst's and hit the Start (Convert) button.

Start up the linux DAW and point it to scan the folder containing the windows vst's.

If new vst plugins are added to a folder, then just run linvstconvert again on that folder.

linvstconvert is installed into /usr/bin

-------------

The LinVst source folder should be located in a folder/directory with user permissions otherwise cmake might produce permission errors.

## Building the deb package.

The Deb package files are in the deb-packages folder.

There are 3 different versions.

A 64 bit version only, a 64 bit and 32 bit version and a 32 bit version only (for 32 bit only Linux systems/distros)

The 64 bit only version can run 64 bit vst's on a 64 bit Linux system.

The 64 bit and 32 bit version can run 64 bit vst's and also run 32 bit vst's on a 64 bit Linux system (needs multilib and wine 32 bit development libraries and 32 bit development libraries, see make instructions).

The 32 bit only version can run 32 bit vst's on a 32 bit Linux system.

------------------------------------

## building the 64 bits only version.

In the LinVst/deb-packages directory:

```bash
mkdir build64
cd build64
cmake ../64bitsonly
make
cpack
```

This should create a LinVst-64bit-x.y.z.deb file ready to install.

## How to use the 64 bits only package

The 64 bits package installs the following files:

```bash
/usr/bin/lin-vst-servertrack.exe
/usr/bin/lin-vst-servertrack.exe.so
/usr/bin/linvstconvert
/usr/share/LinVst/64bit/linvst.so
/usr/share/LinVst/Readme
```

-------------------------------

## building the 64 bit and 32 bit version.

In the LinVst/deb-packages directory:

```bash
mkdir build64-32
cd build64-32
cmake ../64bits-32bits
make
cpack
```

This should create a LinVst-64bit-32bit-x.y.z.deb file ready to install.

## How to use the 64 bit and 32 bit package

The 64 bit and 32 bit package installs the following files:

```bash
/usr/bin/lin-vst-servertrack.exe
/usr/bin/lin-vst-servertrack.exe.so
/usr/bin/lin-vst-servertrack32.exe
/usr/bin/lin-vst-servertrack32.exe.so
/usr/bin/linvstconvert
/usr/share/LinVst/64bit-32bit/linvst.so
/usr/share/LinVst/Readme
```

---------------------

## building the 32 bits only version.

In the LinVst/deb-packages directory:

```bash
mkdir build32
cd build32
cmake ../32bitonly
make
cpack
```

This should create a LinVst-32bit-x.y.z.deb file ready to install.

## How to use the 32 bits only package

The 64 bits package installs the following files:

```bash
/usr/bin/lin-vst-server32lx.exe
/usr/bin/lin-vst-server32lx.exe.so
/usr/bin/linvstconvert
/usr/share/LinVst/32bit/linvst.so
/usr/share/LinVst/Readme
```

------------------


