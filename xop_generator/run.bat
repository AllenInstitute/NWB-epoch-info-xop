@echo off
set CURPATH=%cd%
cd /D %~dp0
perl XOP-function-stub-generator\xop-stub-generator.pl interface.h

set IgorPath="%PROGRAMFILES%\WaveMetrics\Igor Pro 8 Folder\IgorBinaries_x64\Igor64.exe"

if exist %IgorPath% goto foundIgor
echo Igor Pro could not be found in %IgorPath%, please adjust the variable IgorPath in the script
goto done

:foundIgor
%IgorPath% /I "Operation-Template-Generator.pxp"
:done

copy/B CustomExceptions.cpp %CURPATH%
copy/B functions.cpp %CURPATH%
copy/B Helpers.cpp %CURPATH%
copy/B CustomExceptions.h %CURPATH%
copy/B functions.h %CURPATH%
copy/B Helpers.h %CURPATH%
copy/B resource.h %CURPATH%
copy/B functions.rc %CURPATH%
copy/B CMakeLists.txt %CURPATH%
md %CURPATH%\cmake
xcopy/Y/E cmake %CURPATH%\cmake
md %CURPATH%\fmt
xcopy/Y/E fmt %CURPATH%\fmt

cd /D %CURPATH%
set CURPATH=
