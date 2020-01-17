#include "XOPStandardHeaders.r"
// vim: set ft=rc:

resource 'vers' (1) {						/* XOP version info */
	0x01, 0x00, final, 0x00, 0,		/* version bytes and country integer */
	"1.00",
	"1.00, Copyright 2019 () byte physics, all rights reserved."
};

resource 'vers' (2) {						/* Igor version info */
	0x08, 0x00, release, 0x00, 0,	/* version bytes and country integer */
	"8.00",
	"(for Igor 8.00 or later)"
};

resource 'STR#' (1100) {					/* custom error messages */
	{
#if XOP_TOOLKIT_VERSION >= 800
  "mies-nwb2-compound-XOP XOP requires Igor 8.00 or later.",                                                             // OLD_IGOR
#else
  "mies-nwb2-compound-XOP XOP requires Igor 7.00 or later.",                                                             // OLD_IGOR
#endif
  "A unhandled C++ exception occured.",                                                                // UNHANDLED_CPP_EXCEPTION
  "A C++ exception occured.",                                                                          // CPP_EXCEPTION

  "ASSERTion encountered.",                                                                            // ERR_ASSERT
  "Error when parsing input",                                                                          // ERR_PARSE
  "Can not convert value to requested type.",                                                          // ERR_CONVERT
  "Invalid type specified.",                                                                           // ERR_INVALID_TYPE
  "Parameters missing in operation call",                                                              // ERR_FLAGPARAMS
	}
};

/* no menu item */

resource 'XOPI' (1100) {
	XOP_VERSION,							// XOP protocol version.
	DEV_SYS_CODE,							// Code for development system used to make XOP
	XOP_FEATURE_FLAGS,			  // Tells Igor about XOP features
	XOPI_RESERVED,					  // Reserved - must be zero.
	XOP_TOOLKIT_VERSION,			// XOP Toolkit version.
};

resource 'XOPC' (1100) {
	{
	"MIESNWB2_WriteCompound",
	utilOp + XOPOp + compilableOp + threadSafeOp,

	"MIESNWB2_ReadCompound",
	utilOp + XOPOp + compilableOp + threadSafeOp,

  }
};

#include "functions.r"
