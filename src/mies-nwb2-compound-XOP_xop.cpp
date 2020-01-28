#include "mies-nwb2-compound-XOP_xop.h"
#include "mies-nwb2-compound-XOP_handler.h"

#include "Operations.h"
#include "functions.h"
#include "CustomExceptions.h"
#include "Helpers.h"
#include "xop_errors.h"

#include <mutex>

namespace
{
using LockGuard = std::lock_guard<std::recursive_mutex>;
std::recursive_mutex mutex;
} // namespace

// FUNCTIONS

// OPERATIONS

extern "C" int ExecuteIPNWB_WriteCompound(IPNWB_WriteCompoundRuntimeParamsPtr p)
{
  BEGIN_OUTER_CATCH

  LockGuard lock(mutex);
  XOPHandler().IPNWB_WriteCompound(p);

  END_OUTER_CATCH
}

extern "C" int ExecuteIPNWB_ReadCompound(IPNWB_ReadCompoundRuntimeParamsPtr p)
{
  BEGIN_OUTER_CATCH

  LockGuard lock(mutex);
  XOPHandler().IPNWB_ReadCompound(p);

  END_OUTER_CATCH
}

static int RegisterIPNWB_WriteCompound(void)
{
  const char *cmdTemplate;
  const char *runtimeNumVarList;
  const char *runtimeStrVarList;

  // NOTE: If you change this template, you must change the IPNWB_WriteCompoundRuntimeParams structure as well.
  cmdTemplate = "IPNWB_WriteCompound /Z[=number:ZIn] /Q[=number:QIn] /S=wave:offsetWave /C=wave:sizeWave "
                "/REF=wave:tsRefWave /LOC=string:compPath string:fullFileName";
  runtimeNumVarList = "";
  runtimeStrVarList = "";
  return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(IPNWB_WriteCompoundRuntimeParams),
                           (void *) ExecuteIPNWB_WriteCompound, kOperationIsThreadSafe);
}

static int RegisterIPNWB_ReadCompound(void)
{
  const char *cmdTemplate;
  const char *runtimeNumVarList;
  const char *runtimeStrVarList;

  // NOTE: If you change this template, you must change the IPNWB_ReadCompoundRuntimeParams structure as well.
  cmdTemplate = "IPNWB_ReadCompound /Z[=number:ZIn] /Q[=number:QIn] /FREE /S=DataFolderAndName:{offsetWave, real} "
                "/C=DataFolderAndName:{sizeWave, real} /REF=DataFolderAndName:{tsRefWave, text} /LOC=string:compPath "
                "string:fullFileName";
  runtimeNumVarList = "";
  runtimeStrVarList = "";
  return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(IPNWB_ReadCompoundRuntimeParams),
                           (void *) ExecuteIPNWB_ReadCompound, kOperationIsThreadSafe);
}

static int RegisterOperations(void) // Register any operations with Igor.
{
  int result;

  if(result = RegisterIPNWB_WriteCompound())
    return result;

  if(result = RegisterIPNWB_ReadCompound())
    return result;

  return 0;
}

/*	XOPEntry()

  This is the entry point from the host application to the XOP for all messages
  after the INIT message.
*/

extern "C" void XOPEntry(void)
{
  XOPIORecResult result = 0;

  switch(GetXOPMessage())
  {
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

HOST_IMPORT int XOPMain(IORecHandle ioRecHandle) // The use of XOPMain rather than main means
                                                 // this XOP requires Igor Pro 6.20 or later
{
  int result;

  XOPInit(ioRecHandle);  // Do standard XOP initialization
  SetXOPEntry(XOPEntry); // Set entry point for future calls

#if XOP_TOOLKIT_VERSION >= 800
  if(igorVersion < 800)
#else
  if(igorVersion < 700)
#endif
  {
    SetXOPResult(OLD_IGOR);
    return EXIT_FAILURE;
  }

  if(result = RegisterOperations())
  {
    SetXOPResult(result);
    return EXIT_FAILURE;
  }

  SetXOPResult(0);
  return EXIT_SUCCESS;
}
