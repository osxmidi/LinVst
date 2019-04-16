# linvstmanage
Python script to manage Windows VSTs bridged with [LinVst](https://github.com/osxmidi/LinVst) according to a config file.

## Features
- Update/Create *.so files for all VSTs listed in config file
- Enable/Disable VSTs by adding/removing a softlink to the respective *.so file.
  All softlinks are mapped to a *link folder* which can be scanned by your preferred DAW.
- Detection and optional removal of orphaned _*.so-links/files_ in _link folder_

## Dependencies
- python3
- optional: _termcolor_ package for colored status output
    - Can be install with `pip install --user termcolor`

## Usage
### Setup the config file
It's an ini-format config file with basically two parts:
    
    1) General settings
    2) Variable amount of sections that specify one or more VST-dlls using a folder path and their respective names
    
Further documentation can be found within the config file *linvstmanage.ini* itself.

Hint: The config file is parsed once at script startup. Therefore changes to the config file will only be considered after a restart of the script.

### Run the script
Simply run *linvstmanage-cli* from the console.
It will look for the config file at two possible locations:
    
    1) ~/.config/linvst/manage/linvstmanage.ini
    2) current directory

If you'd prefer a different location you could always symlink your config file to one of these locations.

i.e. `ln -s ~/myconfigs/linvstConfig.ini ~/.config/linvst/manage/linvstmanage.ini`
    
    
## Further notes
### VST states

![Status view](https://github.com/Goli4thus/linvstmanage/blob/master/img/uiStatus.png)

| State    | Meaning                                                                   |
| -------- | ------------------------------------------------------------------------- |
| Enabled  | VST is enabled via active softlink                                        |
| Disabled | VST is disabled due to missing softlink                                   |
| Mismatch | Mismatch between linvst.so template and *.so file associated with VST-dll |
| No *.so  | VST-dll has no associated VST-so file                                     |
| Notfound | VST-dll can't be found using the specified config path                    |
| Error    | Specified plugin in config file is not of type 'dll'                      |
    
    
![Help view](https://github.com/Goli4thus/linvstmanage/blob/master/img/uiHelp.png)
    
    
### Typical setup
Different "VST.dll"s are installed into different locations due to making use of different wine prefixes per plugin vendor (i.e. to allow for different dll overrides on a vendor basis).
This gives us a number of different folders containing one or more "VST.dll"s.

For each such folder we create a section within the config file _linvstmanage.ini_ (important: section names must be unique).
Each section is defined by it's path and one or more "VST.dll"s.

After starting *linvstmanage-cli* and assuming the config file is correctly setup, the status view will show an entry for each VST.dll.
Initially the status of each will be _No *.so_ (if this is a fresh setup without prior LinVst converts).

Performing an _update_ will create accompaning VST-so files for each VST.dll.
This results in the status of all VSTs going to _Disabled_.

After the VSTs can be _Enabled_, which creates symlinks within the _link folder_ referencing all previously created VST-so files.

This central _link folder_ can be scanned by whatever DAW you are using.


### Further usecases
#### LinVst version mismatch
In case of updating or chaning versions of LinVst it can happend that one ends up with a version mismatch between the 
installed LinVst server executables and the VST-so files associated with "VST-dll"s.
In order to clean this mess up it's enough to run *linvstmanage-cli* and perform and _update_ (assuming the mismatch was due to a managed VST).

#### Quick enable/disable of VSTs in the DAW
If one wanted to temporarily disable and later on enable a VST managed by LinVst it either meant:
    - deleting the so-files or symlinks manually or
    - having your own script that could do this or 
    - a combination of both

With this script it's centrally managed and _pretty_ fast to handle this usecase.


