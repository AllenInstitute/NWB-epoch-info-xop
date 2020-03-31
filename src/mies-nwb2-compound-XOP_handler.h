#pragma once

#include "Operations.h"
#include "functions.h"

class Handler
{
public:
  // Singleton access
  static Handler &Instance();

  // Set Quiet Mode for Output
  void SetQuietMode(bool quietMode);

  // Operations
  void IPNWB_WriteCompound(IPNWB_WriteCompoundRuntimeParamsPtr p);

  void IPNWB_ReadCompound(IPNWB_ReadCompoundRuntimeParamsPtr p);

  // Functions

private:
  Handler()        = default;
  bool m_quietMode = false;
};

Handler &XOPHandler();
