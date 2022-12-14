@echo off
cd /D %~dp0
cd hdf5_package\CMake-hdf5-1.10.6
7z x -y -o..\..\hdf5 hdf5_x86_release.zip *

cd /D %~dp0
md build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 16 2019" -A Win32 ..\src
cmake --build . --config Release --target install
