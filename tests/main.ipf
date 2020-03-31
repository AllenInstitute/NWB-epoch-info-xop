#pragma TextEncoding = "UTF-8"
#pragma rtGlobals=3

#pragma ModuleName=IPNWBXOP_TEST_MAIN

#include "unit-testing"

Function run()
	RunTest("test.*", name = "IPNWBXOP_TESTS", enableJU = 1, enableRegexp = 1)
End
