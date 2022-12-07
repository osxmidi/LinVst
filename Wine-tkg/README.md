Wine-tkg Releases are at https://github.com/Kron4ek/Wine-Builds

Wine-tkg has certain advantages over the usual WineHQ or distro Wine.

Possibly, the easiest way to install Wine-tkg is to leave the Wine that is already installed (Wine from WineHQ or distro) where it is and just use 
symbolic links in the /usr/bin directory to point to the unarchived Wine tkg location.

If say, wine-7.22-staging-tkg-amd64.tar.xz is unarchived in the Downloads folder to produce a wine-7.22-staging-tkg-amd64 folder in the Downloads folder, then running the Install-tkg script (after making it executable) will place symbolic links in /usr/bin that point to the wine-7.22-staging-tkg-amd64 folder in the Dowmloads folder.

Then run winecfg.

If Wine tkg updates to a new version (say, wine-7.23-staging-tkg-amd64) in the future then the Install-tkg script will need to be updated to the new name by replacing the "wine-7.22-staging-tkg-amd64" text with "wine-7.23-staging-tkg-amd64" and then running the Install-tkg script and then running winecfg.

For compiling (say, compiling the LinVst code) the original Wine winegcc needs to be used (left where it is) and that is also done by the Install-tkg script.

If someone wants to stop using Wine tkg and go back to the original (WineHQ, distro) Wine then Wine will need to be installed again 
from WineHQ or the distro.


