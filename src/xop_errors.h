#pragma once
#include "XOPStandardHeaders.h" // Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h

// clang-format off
#define ERR_NO_ERROR             0
#define OLD_IGOR                 1 + FIRST_XOP_ERR
#define UNHANDLED_CPP_EXCEPTION  2 + FIRST_XOP_ERR
#define CPP_EXCEPTION            3 + FIRST_XOP_ERR
#define ERR_ASSERT               4 + FIRST_XOP_ERR
#define ERR_PARSE                5 + FIRST_XOP_ERR
#define ERR_CONVERT              6 + FIRST_XOP_ERR
#define ERR_INVALID_TYPE         7 + FIRST_XOP_ERR
#define ERR_FLAGPARAMS           8 + FIRST_XOP_ERR
// clang-format on
