Install approx guide


create bottle named vst2 (or whatever)


(To check) ls ~/.var/app/com.usebottles.bottles/data/bottles/bottles/vst2


run executable (in bottle) to install a vst2 plugin ie vst2 dll installed into (can be installed to other locations in "Program Files" depending on the plugin ie could be installed into the Steinberg foder instead of VST2 or whatever)


(To check) ~/.var/app/com.usebottles.bottles/data/bottles/bottles/vst2/drive_c/"Program Files"/"Common Files"/VST2


use linvstconvert and choose ~/var/app/com.usebottles.bottles/data/bottles/bottles/vst2/drive_c/"Program Files"/"Common Files"/VST2


add DAW VST search to include ~/var/app/com.usebottles.bottles/data/bottles/bottles/vst2/drive_c/"Program Files"/"Common Files"/VST2


The WINELOADER variable needs to be set as it points to the location where wine is (wine is usually in usr/bin but with bottles it's not).

USE (depending where wine is in a bottle)

export WINELOADER=/home/osxuser/.var/app/com.usebottles.bottles/data/bottles/runners/soda-9.0-1/bin/wine (in a terminal)

to set the WINELOADER

Run Daw from the terminal (or configure WINELOADER to be set on boot)

LinVst automatically sets the WINEPREFIX variable based on the path of the linvst.so file (that has been renamed to the vst2 dll's name so that the Linux Daw can load it).

So, LinVst automatically sets the WINEPREFIX to the wine location in a bottle

check if wine loader is set

env | grep WINELOADER

check wine loader version

$WINELOADER --version
