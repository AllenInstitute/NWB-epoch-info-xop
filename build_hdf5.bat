@echo off
cd /D %~dp0
cd hdf5_package
7z x CMake-hdf5-1.10.6.zip *
cd CMake-hdf5-1.10.6
call build-VS2017-64.bat
7z x -o..\..\hdf5 HDF5-1.10.6-win64.zip *
