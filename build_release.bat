@echo off
cd /D %~dp0
call build_hdf5.bat
cd /D %~dp0
md build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 16 2019" -A x64 ..\src
cmake --build . --config Release --target install
