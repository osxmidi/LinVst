Wine-tkg Releases are at https://github.com/Kron4ek/Wine-Builds

Wine-tkg has certain advantages over the usual WineHQ or distro Wine.

---------

Arch wine has changed to Wow64 versions which do not run 32 bit vst's.

For arch, it's possible to install Wine 10.10 (and possibly future versions) from tkg which runs 64 and 32 bit vst's (tkg also has a wow64 version)

Download wine-10.10-staging-tkg-amd64.tar.xz and wine-10.10-staging-tkg-x86.tar.xz (or any non wow64 versions) from https://github.com/Kron4ek/Wine-Builds/releases

uninstall wine-gecko and wine-mono and wine or wine-staging

sudo pacman -Rs wine-gecko

sudo pacman -Rs wine-mono

sudo pacman -Rs wine-staging (or wine)

-------------

unpack wine-10.10-staging-tkg-amd64.tar.xz and wine-10.10-staging-tkg-x86.tar.xz in Downloads folder

then copy tkg wine to directories

sudo cp -r ~/Downloads/wine-10.10-staging-tkg-amd64/bin/* /usr/bin

sudo cp -r ~/Downloads/wine-10.10-staging-tkg-amd64/include/* /usr/include

sudo cp -r ~/Downloads/wine-10.10-staging-tkg-amd64/share/* /usr/share

sudo cp -r ~/Downloads/wine-10.10-staging-tkg-amd64/lib/* /lib

sudo cp -r ~/Downloads/wine-10.10-staging-tkg-x86/lib* /lib

---------

if wanting to compile

sudo ln -s /usr/bin/gcc /usr/bin/gcc-11

sudo ln -s /usr/bin/g++ /usr/bin/g++-11

---------

Another way to install Wine-tkg is to leave the Wine that is already installed (Wine from WineHQ or distro) where it is and just use 
symbolic links in the /usr/bin directory to point to the unarchived Wine tkg location.

If say, wine-8.1-staging-tkg-amd64.tar.xz is unarchived in the Downloads folder to produce a wine-8.1-staging-tkg-amd64 folder in the Downloads folder, then running the Install-tkg script (after making it executable) will place symbolic links in /usr/bin that point to the wine-8.1-staging-tkg-amd64 folder in the Dowmloads folder.

Then run winecfg.

If Wine tkg updates to a new version in the future then the Install-tkg script will need to be updated to the new unarchived Wine-tkg folder name by replacing the "wine-8.1-staging-tkg-amd64" text with the new unarchived Wine-tkg folder name (simple text editor search and replace) and then running the Install-tkg script and then running winecfg.

For compiling (say, compiling the LinVst code) some symlinks are needed, sudo ln -s /usr/bin/gcc /usr/bin/gcc-11 and sudo ln -s /usr/bin/g++ /usr/bin/g++-11

If Wine gets automatically updated by the distro, then the Install-tkg script will need to be run again (winecfg might need to be run again as well).
If Wine tkg becomes the Wine that the user mainly uses then the distro or WineHQ Wine could be uninstalled.

If someone wants to stop using Wine tkg and go back to the original (WineHQ, distro) Wine then Wine will need to be installed again from WineHQ or the distro.

---------

Another way is create a new wineprefix and run Wine-tkg releases from there.

Assuming that Wine staging (wine-8.1-staging-tkg-amd64) has been downloaded (and unarchived in the Downloads folder) from the Wine-tkg releases site at https://github.com/Kron4ek/Wine-Builds

In a terminal (which will set WINEPREFIX and PATH just for that terminal)

```
mkdir ~/.wineX
WINEPREFIX=~/.wineX
cp -a ~/Downloads/wine-8.1-staging-tkg-amd64/* ~/.wineX
export PATH=$PATH:~/.wineX/bin

```
then run winecfg in the new wineprefix

WINEPREFIX and PATH can then be permanently set at startup.



