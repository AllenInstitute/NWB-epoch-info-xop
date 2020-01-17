#include "mies-nwb2-compound-XOP_xop.h"
#include "mies-nwb2-compound-XOP_handler.h"

#include "CustomExceptions.h"
#include "Helpers.h"
#include "Operations.h"
#include "functions.h"
#include "xop_errors.h"

#include <mutex>

namespace {
using LockGuard = std::lock_guard<std::recursive_mutex>;
std::recursive_mutex mutex;
} // namespace

// FUNCTIONS

// OPERATIONS

extern "C" int
ExecuteMIESNWB2_WriteCompound(MIESNWB2_WriteCompoundRuntimeParamsPtr p) {
  BEGIN_OUTER_CATCH

  LockGuard lock(mutex);
  XOPHandler().MIESNWB2_WriteCompound(p);

  END_OUTER_CATCH
}

extern "C" int
ExecuteMIESNWB2_ReadCompound(MIESNWB2_ReadCompoundRuntimeParamsPtr p) {
  BEGIN_OUTER_CATCH

  LockGuard lock(mutex);
  XOPHandler().MIESNWB2_ReadCompound(p);

  END_OUTER_CATCH
}

static int RegisterMIESNWB2_WriteCompound(void) {
  const char *cmdTemplate;
  const char *runtimeNumVarList;
  const char *runtimeStrVarList;

  // NOTE: If you change this template, you must change the
  // MIESNWB2_WriteCompoundRuntimeParams structure as well.
  cmdTemplate = "MIESNWB2_WriteCompound /Z[=number:ZIn] /Q[=number:QIn] "
                "/S=wave:offsetWave /C=wave:sizeWave /REF=wave:tsRefWave "
                "/LOC=string:compPath string:fullFileName";
  runtimeNumVarList = "";
  runtimeStrVarList = "";
  return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList,
                           sizeof(MIESNWB2_WriteCompoundRuntimeParams),
                           (void *)ExecuteMIESNWB2_WriteCompound,
                           kOperationIsThreadSafe);
}

static int RegisterMIESNWB2_ReadCompound(void) {
  const char *cmdTemplate;
  const char *runtimeNumVarList;
  const char *runtimeStrVarList;

  // NOTE: If you change this template, you must change the
  // MIESNWB2_ReadCompoundRuntimeParams structure as well.
  cmdTemplate =
      "MIESNWB2_ReadCompound /Z[=number:ZIn] /Q[=number:QIn] "
      "/S=DataFolderAndName:{offsetWave, real} /C=DataFolderAndName:{sizeWave, "
      "real} /REF=DataFolderAndName:{tsRefWave, text} /LOC=string:compPath "
      "string:fullFileName";
  runtimeNumVarList = "";
  runtimeStrVarList = "";
  return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList,
                           sizeof(MIESNWB2_ReadCompoundRuntimeParams),
                           (void *)ExecuteMIESNWB2_ReadCompound,
                           kOperationIsThreadSafe);
}

static int RegisterOperations(void) // Register any operations with Igor.
{
  int result;

  if (result = RegisterMIESNWB2_WriteCompound())
    return result;

  if (result = RegisterMIESNWB2_ReadCompound())
    return result;

  return 0;
}

/*	XOPEntry()

  This is the entry point from the host application to the XOP for all messages
  after the INIT message.
*/

extern "C" void XOPEntry(void) {
  XOPIORecResult result = 0;

  switch (GetXOPMessage()) {
  case FUNCADDRS:
    result = RegisterFunction();
    break;
  case CLEANUP:
    break;
  }
  SetXOPResult(result);
}

/*	XOPMain(ioRecHandle)

  This is the initial entry point at which the host application calls XOP.
  The message sent by the host must be INIT.

  XOPMain does any necessary initialization and then sets the XOPEntry field of
  the ioRecHandle to the address to be called for future messages.
*/

HOST_IMPORT int
XOPMain(IORecHandle ioRecHandle) // The use of XOPMain rather than main means
                                 // this XOP requires Igor Pro 6.20 or later
{
  int result;

  XOPInit(ioRecHandle);  // Do standard XOP initialization
  SetXOPEntry(XOPEntry); // Set entry point for future calls

#if XOP_TOOLKIT_VERSION >= 800
  if (igorVersion < 800)
#else
  if (igorVersion < 700)
#endif
  {
    SetXOPResult(OLD_IGOR);
    return EXIT_FAILURE;
  }

  if (result = RegisterOperations()) {
    SetXOPResult(result);
    return EXIT_FAILURE;
  }

  SetXOPResult(0);
  return EXIT_SUCCESS;
}
