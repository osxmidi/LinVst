Wine-tkg Releases are at https://github.com/Kron4ek/Wine-Builds

Wine-tkg has certain advantages over the usual WineHQ or distro Wine.

Possibly, the easiest way to install Wine-tkg is to leave the Wine that is already installed (Wine from WineHQ or distro) where it is and just use 
symbolic links in the /usr/bin directory to point to the unarchived Wine tkg location.

If say, wine-8.0-rc2-staging-tkg-amd64.tar.xz is unarchived in the Downloads folder to produce a wine-8.0-rc2-staging-tkg-amd64 folder in the Downloads folder, then running the Install-tkg script (after making it executable) will place symbolic links in /usr/bin that point to the wine-8.0-rc2-staging-tkg-amd64 folder in the Dowmloads folder.

Then run winecfg.

If Wine tkg updates to a new version in the future then the Install-tkg script will need to be updated to the new unarchived Wine-tkg folder name by replacing the "wine-8.0-rc2-staging-tkg-amd64" text with the new unarchived Wine-tkg folder name (simple text editor search and replace) and then running the Install-tkg script and then running winecfg.

For compiling (say, compiling the LinVst code) Wine 8 will have been needed to be installed before the tkg version of Wine8 has been installed (and then running the Install-tkg script).

If Wine gets automatically updated by the distro, then the Install-tkg script will need to be run again (winecfg might need to be run again as well).

If someone wants to stop using Wine tkg and go back to the original (WineHQ, distro) Wine then Wine will need to be installed again 
from WineHQ or the distro.


