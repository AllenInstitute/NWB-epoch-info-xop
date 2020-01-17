#pragma once

#include "Operations.h"
#include "functions.h"

class Handler {
public:
  // Singleton access
  static Handler &Instance();

  // Set Quiet Mode for Output
  void SetQuietMode(bool quietMode);

  // Operations
  void MIESNWB2_WriteCompound(MIESNWB2_WriteCompoundRuntimeParamsPtr p);

  void MIESNWB2_ReadCompound(MIESNWB2_ReadCompoundRuntimeParamsPtr p);

  // Functions

private:
  Handler() = default;
  bool m_quietMode = false;
};

Handler &XOPHandler();
