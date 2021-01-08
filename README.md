# UnInstall

Plugin for Far Manager 3.

This Far Manager plugin is a replacement for Windows Control Panel "Uninstall a program" functionality.
It aims to be fast and handy index of installed programs with ability to view installations details or to remove selected one.

## How to build from source
To build plugin from source, you will need:

* Visual Studio 2019
* cmake 3.15 (included in Visual Studio 2019)

#### Release
From root of source call `vc.build.release.cmd`. The build result will be in ./bin folder.

#### Debug or develop
For example: 
```
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
mkdir cmake-build-debug
cd cmake-build-debug
cmake .. -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 16 2019"
UnInstall.sln
```

# Common info
* Cloned from: https://code.google.com/p/conemu-maximus5 , alternative  https://github.com/Maximus5/FarPlugins/tree/master/Uninstall 
* PlugRing page: https://plugring.farmanager.com/plugin.php?pid=698
* Discuss: https://forum.farmanager.com/viewtopic.php?t=3597