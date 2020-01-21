@echo off
cd /D %~dp0
call build_hdf5_debug.bat
cd /D %~dp0
md build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 16 2019" -A x64 ..\src
cmake --build . --config Debug --target install
