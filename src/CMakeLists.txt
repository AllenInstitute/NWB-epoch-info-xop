CMAKE_MINIMUM_REQUIRED(VERSION 3.8)

# This file is part of the `XOP` project and licensed under BSD-3-Clause.

PROJECT(mies-nwb2-compound-XOP)

OPTION(COVERAGE "Enable coverage instrumentation" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/CreateLaunchers/cmake")
include(CreateLaunchers)

IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
  SET(bitness "")
  SET(hdf5bitness "32")
  SET(bitnessWithDash "")
  SET(bitnessLibFolder "x86")

  IF(MSVC)
    SET(igorPath "C:/Program Files/WaveMetrics/Igor Pro 8 Folder/IgorBinaries_Win32/Igor.exe")
  ELSEIF(APPLE)
    MESSAGE(FATAL_ERROR "No 32bit support on MacOSX.")
  ENDIF()
ELSEIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET(bitness "64")
  SET(hdf5bitness "64")
  SET(bitnessWithDash "-64")
  SET(bitnessLibFolder "x64")

  IF(MSVC)
    SET(igorPath "C:/Program Files/WaveMetrics/Igor Pro 8 Folder/IgorBinaries_x64/Igor64.exe")
  ELSEIF(APPLE)
    SET(igorPath "/Applications/Igor\ Pro\ 8\ Folder/Igor64.app/Contents/MacOS/Igor64")
  ENDIF()
ENDIF()

SET(libname "${PROJECT_NAME}${bitnessWithDash}")

IF(APPLE)
  # use RPATH
  cmake_policy(SET CMP0042 NEW)
  SET(RESOURCES "${PROJECT_NAME}_xop.r")
  SET(installFolder "${CMAKE_SOURCE_DIR}/../output/mac")
ELSEIF(WIN32)
  SET(RESOURCES "${PROJECT_NAME}_xop.rc")
  SET(installFolder "${CMAKE_SOURCE_DIR}/../output/win/${bitnessLibFolder}")
ENDIF()

# Source files for which we are interested in the coverage
SET(COVERAGE_SOURCES
  ${PROJECT_NAME}_handler.cpp
  ${PROJECT_NAME}_xop.cpp
)

SET(SOURCES
  ${COVERAGE_SOURCES}
  CustomExceptions.cpp
  functions.cpp
  Helpers.cpp
)

SET(HEADERS
  CustomExceptions.h
  functions.h
  Helpers.h
  ${PROJECT_NAME}_handler.h
  ${PROJECT_NAME}_xop.h
  xop_errors.h
  Operations.h
)

SOURCE_GROUP("Headers" FILES ${HEADERS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../XOPSupport ${CMAKE_SOURCE_DIR}/../hdf5/HDF5-1.10.6-win${hdf5bitness}/include)

IF(APPLE)

  # heavily inspired by https://github.com/Kitware/CMake/blob/master/Tests/CFBundleTest/CMakeLists.txt

  SET(MISC
    InfoPlist.strings
    ${CMAKE_BINARY_DIR}/${libname}.rsrc)

  ADD_EXECUTABLE(${libname} MACOSX_BUNDLE ${SOURCES} ${SOURCES_EXT} ${HEADERS} ${RESOURCES} ${MISC})

  SET_TARGET_PROPERTIES(${libname} PROPERTIES PREFIX "")
  SET_TARGET_PROPERTIES(${libname} PROPERTIES BUNDLE_EXTENSION "xop")
  SET_TARGET_PROPERTIES(${libname} PROPERTIES ENABLE_EXPORTS true)
  SET_TARGET_PROPERTIES(${libname} PROPERTIES XCODE_ATTRIBUTE_EXPORTED_SYMBOLS_FILE "${CMAKE_SOURCE_DIR}/Exports.exp")
  SET_TARGET_PROPERTIES(${libname} PROPERTIES XCODE_ATTRIBUTE_MACH_O_TYPE mh_bundle)

  SET(sysroot)
  IF(CMAKE_OSX_SYSROOT)
    SET(sysroot -isysroot ${CMAKE_OSX_SYSROOT})
  ENDIF()

  SET_TARGET_PROPERTIES(${libname} PROPERTIES CXX_STANDARD 14)

  FIND_LIBRARY(CARBON_LIBRARY Carbon)
  FIND_LIBRARY(COCOA_LIBRARY Cocoa)
  FIND_LIBRARY(AUDIO_UNIT AudioUnit)
  FIND_LIBRARY(AUDIO_TOOLBOX AudioToolbox)
  MARK_AS_ADVANCED(CARBON_LIBRARY AUDIO_TOOLBOX AUDIO_UNIT COCOA_LIBRARY)
  SET(EXTRA_LIBS ${CARBON_LIBRARY} ${COCOA_LIBRARY} ${AUDIO_TOOLBOX} ${AUDIO_UNIT})

  FIND_PROGRAM(APPLE_RESOURCE Rez /Developer/Tools)
  IF(NOT APPLE_RESOURCE)
    MESSAGE(FATAL_ERROR "Could not find Rez to build resources from .r file.")
  ENDIF()

  ADD_CUSTOM_COMMAND(TARGET ${libname}
    PRE_BUILD
    COMMAND ${APPLE_RESOURCE}
    ${sysroot}
    -i ${CMAKE_SOURCE_DIR}/../XOPSupport
    -F ${CARBON_LIBRARY}
    -useDF
    ${RESOURCES}
    -o ${CMAKE_BINARY_DIR}/${libname}.rsrc
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

  ADD_CUSTOM_COMMAND(TARGET ${libname}
    POST_BUILD
    COMMAND zip
    -r ${libname}.zip
    ${CMAKE_CFG_INTDIR}/${libname}.xop
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

  SET_SOURCE_FILES_PROPERTIES(
      ${CMAKE_BINARY_DIR}/${libname}.rsrc
      PROPERTIES GENERATED 1)

  SET_SOURCE_FILES_PROPERTIES(
    ${CMAKE_BINARY_DIR}/${libname}.rsrc
    PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

  SET_SOURCE_FILES_PROPERTIES(
    ${CMAKE_SOURCE_DIR}/InfoPlist.strings
    PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/English.lproj")

  TARGET_LINK_LIBRARIES(${libname} ${EXTRA_LIBS}
                         ${CMAKE_SOURCE_DIR}/../XOPSupport/Xcode/libXOPSupport${bitness}.a)

  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D=TARGET_OS_MAC -Wall -Werror -Wno-deprecated -Wno-parentheses -Wno-unused-function")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D=_DEBUG")

IF(${COVERAGE})
  MESSAGE(STATUS "Building with coverage instrumentation")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-instr-generate")

  SET(coveragePath "${CMAKE_BINARY_DIR}/coverage")

  ADD_CUSTOM_TARGET(coverage-merge
                    COMMAND xcrun
                    llvm-profdata merge -output=code.profdata default.profraw
                    WORKING_DIRECTORY ${coveragePath}
                    COMMENT "Generating coverage profraw files" VERBATIM)

  ADD_CUSTOM_TARGET(coverage-show
                    COMMAND xcrun
                    llvm-cov show
                    ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${libname}.xop/Contents/MacOS/${libname}
                    -instr-profile=${coveragePath}/code.profdata
                    -show-instantiation-summary
                    ${SOURCES} ${HEADERS}
                    -path-equivalence -use-color --format html
                    -Xdemangler=c++filt
                    --output-dir=${coveragePath}
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMENT "Generating coverage HTML" VERBATIM)

  ADD_CUSTOM_TARGET(coverage-report
                    COMMAND xcrun
                    llvm-cov report
                    ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${libname}.xop/Contents/MacOS/${libname}
                    -instr-profile=${coveragePath}/code.profdata
                    ${COVERAGE_SOURCES}
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMENT "Generating coverage summary" VERBATIM)

  ADD_CUSTOM_TARGET(coverage)

  # ORDER: coverage -> coverage-merge -> coverage-show -> coverage-report
  ADD_DEPENDENCIES(coverage coverage-report)
  ADD_DEPENDENCIES(coverage-report coverage-show)
  ADD_DEPENDENCIES(coverage-show coverage-merge)
ENDIF()

  LIST(APPEND CMAKE_PROGRAM_PATH "/usr/local/opt/llvm/bin")
ELSEIF(WIN32)

  ADD_LIBRARY(${libname} SHARED ${SOURCES} ${SOURCES_EXT} ${HEADERS} ${RESOURCES})

  SET_TARGET_PROPERTIES(${libname} PROPERTIES SUFFIX ".xop")

  TARGET_LINK_LIBRARIES(${libname} version.lib ${EXTRA_LIBS}
                        ${CMAKE_SOURCE_DIR}/../XOPSupport/IGOR${bitness}.lib
                        ${CMAKE_SOURCE_DIR}/../XOPSupport/VC/XOPSupport${bitness}.lib)

  IF(MSVC)
    # Use static linking for all default targets
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if(${flag_var} MATCHES "/MT")
        string(REGEX REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
      endif(${flag_var} MATCHES "/MT")
    endforeach(flag_var)

    # Use parallel builds
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX /MP -D_CRT_SECURE_NO_WARNINGS")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
  ENDIF()
ENDIF()

TARGET_LINK_LIBRARIES(${libname} optimized ${CMAKE_SOURCE_DIR}/../hdf5/HDF5-1.10.6-win${hdf5bitness}/lib/libszip.lib)
TARGET_LINK_LIBRARIES(${libname} optimized ${CMAKE_SOURCE_DIR}/../hdf5/HDF5-1.10.6-win${hdf5bitness}/lib/libzlib.lib)
TARGET_LINK_LIBRARIES(${libname} optimized ${CMAKE_SOURCE_DIR}/../hdf5/HDF5-1.10.6-win${hdf5bitness}/lib/libhdf5.lib)
TARGET_LINK_LIBRARIES(${libname} optimized ${CMAKE_SOURCE_DIR}/../hdf5/HDF5-1.10.6-win${hdf5bitness}/lib/libhdf5_cpp.lib)
TARGET_LINK_LIBRARIES(${libname} optimized ${CMAKE_SOURCE_DIR}/../hdf5/HDF5-1.10.6-win${hdf5bitness}/lib/libhdf5_hl_cpp.lib)

TARGET_LINK_LIBRARIES(${libname} debug ${CMAKE_SOURCE_DIR}/../hdf5/HDF5-1.10.6-win${hdf5bitness}/lib/libszip_D.lib)
TARGET_LINK_LIBRARIES(${libname} debug ${CMAKE_SOURCE_DIR}/../hdf5/HDF5-1.10.6-win${hdf5bitness}/lib/libzlib_D.lib)
TARGET_LINK_LIBRARIES(${libname} debug ${CMAKE_SOURCE_DIR}/../hdf5/HDF5-1.10.6-win${hdf5bitness}/lib/libhdf5_D.lib)
TARGET_LINK_LIBRARIES(${libname} debug ${CMAKE_SOURCE_DIR}/../hdf5/HDF5-1.10.6-win${hdf5bitness}/lib/libhdf5_cpp_D.lib)
TARGET_LINK_LIBRARIES(${libname} debug ${CMAKE_SOURCE_DIR}/../hdf5/HDF5-1.10.6-win${hdf5bitness}/lib/libhdf5_hl_cpp_D.lib)

ADD_SUBDIRECTORY(fmt)

TARGET_LINK_LIBRARIES(${libname} fmt::fmt)

FIND_PROGRAM(PERL NAMES perl)

IF(${PERL-NOTFOUND})
    MESSAGE(FATAL_ERROR "Could not find perl.")
ENDIF()

ADD_CUSTOM_TARGET(xop-stub
  COMMAND ${PERL} ${CMAKE_SOURCE_DIR}/xop-function-stub-generator/xop-stub-generator.pl
  interface.h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Running xop stub generator" VERBATIM)

FIND_PROGRAM(CLANG_FORMAT NAMES clang-format)

IF(CLANG_FORMAT)
  # run clang-format before building
  ADD_CUSTOM_COMMAND(TARGET ${libname}
                     PRE_BUILD
                     COMMAND ${CLANG_FORMAT} -i ${SOURCES} ${HEADERS}
                     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                     COMMENT "Running clang-format" VERBATIM)
ENDIF()

FIND_PROGRAM(CLANG_TIDY NAMES clang-tidy)

IF(CLANG_TIDY)
  ADD_CUSTOM_TARGET(clang-tidy
                    COMMAND ${CLANG_TIDY} -fix -p=${CMAKE_BINARY_DIR} -checks=* ${SOURCES}
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMENT "Running clang-tidy" VERBATIM)
ENDIF()

IF(MSVC)
  INSTALL(TARGETS ${libname}
          RUNTIME
          DESTINATION ${installFolder})

  INSTALL(FILES ${CMAKE_BINARY_DIR}/Release/${libname}.pdb
          DESTINATION ${installFolder}
          CONFIGURATIONS Release)

ELSEIF(APPLE)
  INSTALL(FILES ${CMAKE_BINARY_DIR}/${libname}.zip
          DESTINATION ${installFolder})
ENDIF()

IF(MSVC)
  create_default_target_launcher(${libname} COMMAND ${igorPath})
ELSE()
  create_target_launcher(${libname} COMMAND ${igorPath})
ENDIF()
