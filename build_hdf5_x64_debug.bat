@echo off
cd /D %~dp0
cd hdf5_package
7z x -y CMake-hdf5-1.10.6.zip *
cd CMake-hdf5-1.10.6
ctest -S HDF5config.cmake,BUILD_GENERATOR=VS201964 -C Debug -V -O hdf5.log -parallel 16
move /Y HDF5-1.10.6-win64.zip hdf5_x64_debug.zip
7z x -y -o..\..\hdf5 hdf5_x64_debug.zip *
