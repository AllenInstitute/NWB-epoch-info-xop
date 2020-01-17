#pragma once

#include <XOPStandardHeaders.h> // Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h

// Operation template: MIESNWB2_WriteCompound /Z[=number:ZIn] /Q[=number:QIn]
// /S=wave:offsetWave /C=wave:sizeWave /REF=wave:tsRefWave /LOC=string:compPath
// string:fullFileName

// Runtime param structure for MIESNWB2_WriteCompound operation.
#pragma pack(2) // All structures passed to Igor are two-byte aligned.
struct MIESNWB2_WriteCompoundRuntimeParams {
  // Flag parameters.

  // Parameters for /Z flag group.
  int ZFlagEncountered;
  double ZIn; // Optional parameter.
  int ZFlagParamsSet[1];

  // Parameters for /Q flag group.
  int QFlagEncountered;
  double QIn; // Optional parameter.
  int QFlagParamsSet[1];

  // Parameters for /S flag group.
  int SFlagEncountered;
  waveHndl offsetWave;
  int SFlagParamsSet[1];

  // Parameters for /C flag group.
  int CFlagEncountered;
  waveHndl sizeWave;
  int CFlagParamsSet[1];

  // Parameters for /REF flag group.
  int REFFlagEncountered;
  waveHndl tsRefWave;
  int REFFlagParamsSet[1];

  // Parameters for /LOC flag group.
  int LOCFlagEncountered;
  Handle compPath;
  int LOCFlagParamsSet[1];

  // Main parameters.

  // Parameters for simple main group #0.
  int fullFileNameEncountered;
  Handle fullFileName;
  int fullFileNameParamsSet[1];

  // These are postamble fields that Igor sets.
  int calledFromFunction; // 1 if called from a user function, 0 otherwise.
  int calledFromMacro;    // 1 if called from a macro, 0 otherwise.
  UserFunctionThreadInfoPtr
      tp; // If not null, we are running from a ThreadSafe function.
};
typedef struct MIESNWB2_WriteCompoundRuntimeParams
    MIESNWB2_WriteCompoundRuntimeParams;
typedef struct MIESNWB2_WriteCompoundRuntimeParams
    *MIESNWB2_WriteCompoundRuntimeParamsPtr;
#pragma pack() // Reset structure alignment to default.

// Operation template: MIESNWB2_ReadCompound /Z[=number:ZIn] /Q[=number:QIn]
// /S=DataFolderAndName:{offsetWave, real} /C=DataFolderAndName:{sizeWave, real}
// /REF=DataFolderAndName:{tsRefWave, text} /LOC=string:compPath
// string:fullFileName

// Runtime param structure for MIESNWB2_ReadCompound operation.
#pragma pack(2) // All structures passed to Igor are two-byte aligned.
struct MIESNWB2_ReadCompoundRuntimeParams {
  // Flag parameters.

  // Parameters for /Z flag group.
  int ZFlagEncountered;
  double ZIn; // Optional parameter.
  int ZFlagParamsSet[1];

  // Parameters for /Q flag group.
  int QFlagEncountered;
  double QIn; // Optional parameter.
  int QFlagParamsSet[1];

  // Parameters for /S flag group.
  int SFlagEncountered;
  DataFolderAndName offsetWave;
  int SFlagParamsSet[1];

  // Parameters for /C flag group.
  int CFlagEncountered;
  DataFolderAndName sizeWave;
  int CFlagParamsSet[1];

  // Parameters for /REF flag group.
  int REFFlagEncountered;
  DataFolderAndName tsRefWave;
  int REFFlagParamsSet[1];

  // Parameters for /LOC flag group.
  int LOCFlagEncountered;
  Handle compPath;
  int LOCFlagParamsSet[1];

  // Main parameters.

  // Parameters for simple main group #0.
  int fullFileNameEncountered;
  Handle fullFileName;
  int fullFileNameParamsSet[1];

  // These are postamble fields that Igor sets.
  int calledFromFunction; // 1 if called from a user function, 0 otherwise.
  int calledFromMacro;    // 1 if called from a macro, 0 otherwise.
  UserFunctionThreadInfoPtr
      tp; // If not null, we are running from a ThreadSafe function.
};
typedef struct MIESNWB2_ReadCompoundRuntimeParams
    MIESNWB2_ReadCompoundRuntimeParams;
typedef struct MIESNWB2_ReadCompoundRuntimeParams
    *MIESNWB2_ReadCompoundRuntimeParamsPtr;
#pragma pack() // Reset structure alignment to default.
