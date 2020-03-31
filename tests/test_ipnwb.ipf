#pragma TextEncoding = "UTF-8"
#pragma rtGlobals=3

#pragma ModuleName=IPNWBXOP_TEST_READWRITE
#include "unit-testing"
#include ":main"

/// @brief read a compound
static Function ReadCompound()

	string dataPath

	PathInfo home
	dataPath = ParseFilepath(5, S_path, "\\", 0, 0) + "test_existing.h5"

	IPNWB_ReadCompound /S=offset /C=size /REF=refs /LOC="/intervals/epochs/timeseries" dataPath

	Make/T base_refs = {"/acquisition/vcs", "/stimulus/presentation/ccss", "/acquisition/vcs", "/stimulus/presentation/ccss"}
	Make/I base_size = {2000, 1000, 400, 200}
	Make/I base_offset = {-2470000, -1235000, -2472000, -1236000}
	CHECK_EQUAL_WAVES(refs, base_refs)
	CHECK_EQUAL_WAVES(size, base_size)
	CHECK_EQUAL_WAVES(offset, base_offset)

End

/// @brief fail test read compound from non existing file
static Function ReadCompoundNoFile()

	variable err

	try
		IPNWB_ReadCompound /S=offset /C=size /REF=refs /LOC="/intervals/epochs/timeseries" "::notexisting::"; AbortOnRTE
		FAIL()
	catch
		err = getRTError(1)
		PASS()
	endtry
End

/// @brief fail test read compound from empty file name
static Function ReadCompoundEmptyFile()

	variable err

	try
		IPNWB_ReadCompound /S=offset /C=size /REF=refs /LOC="/intervals/epochs/timeseries" ""; AbortOnRTE
		FAIL()
	catch
		err = getRTError(1)
		PASS()
	endtry
End

/// @brief fail test read compound from wrong path inside hdf5
static Function ReadCompoundWrongLocation()

	variable err
	string dataPath

	PathInfo home
	dataPath = ParseFilepath(5, S_path, "\\", 0, 0) + "test_existing.h5"

	try
		IPNWB_ReadCompound /S=offset /C=size /REF=refs /LOC="/intervals/epochs/not_existing_timeseries" dataPath; AbortOnRTE
		FAIL()
	catch
		err = getRTError(1)
		PASS()
	endtry
End

/// @brief write compound
static Function WriteCompound()

	string dataPath

	PathInfo home
	dataPath = ParseFilepath(5, S_path, "\\", 0, 0) + "test_fresh1.h5"

	Make/T refs = {"/acquisition/vcs", "/stimulus/presentation/ccss", "/acquisition/vcs", "/stimulus/presentation/ccss"}
	Make/I size = {2000, 1000, 400, 200}
	Make/I offset = {-2470000, -1235000, -2472000, -1236000}

	IPNWB_WriteCompound /S=offset /C=size /REF=refs /LOC="/intervals/epochs/timeseries" dataPath
	PASS()

End

/// @brief write compound with second write -> append
static Function WriteCompoundAppend()

	string dataPath

	PathInfo home
	dataPath = ParseFilepath(5, S_path, "\\", 0, 0) + "test_fresh1.h5"

	Make/T refs = {"/acquisition/vcs", "/stimulus/presentation/ccss", "/acquisition/vcs", "/stimulus/presentation/ccss"}
	Make/I size = {2000, 1000, 400, 200}
	Make/I offset = {-2470000, -1235000, -2472000, -1236000}

	IPNWB_WriteCompound /S=offset /C=size /REF=refs /LOC="/intervals/epochs/timeseries" dataPath
	IPNWB_WriteCompound /S=offset /C=size /REF=refs /LOC="/intervals/epochs/timeseries" dataPath
	PASS()

End

/// @brief Fail test write to hdf5 file where dataset exists but is not chunked
static Function WriteCompoundAppendFail()

	variable err
	string dataPath

	PathInfo home
	dataPath = ParseFilepath(5, S_path, "\\", 0, 0) + "test_existing.h5"

	Make/T refs = {"/acquisition/vcs", "/stimulus/presentation/ccss", "/acquisition/vcs", "/stimulus/presentation/ccss"}
	Make/I size = {2000, 1000, 400, 200}
	Make/I offset = {-2470000, -1235000, -2472000, -1236000}

	try
		IPNWB_WriteCompound /S=offset /C=size /REF=refs /LOC="/intervals/epochs/timeseries" dataPath; AbortOnRTE
		FAIL()
	catch
		err = getRTError(1)
		PASS()
	endtry

End

/// @brief Fail test write to invalid path inside hdf5
static Function WriteCompoundPathFail()

	variable err
	string dataPath

	PathInfo home
	dataPath = ParseFilepath(5, S_path, "\\", 0, 0) + "test_existing.h5"

	Make/T refs = {"/acquisition/vcs", "/stimulus/presentation/ccss", "/acquisition/vcs", "/stimulus/presentation/ccss"}
	Make/I size = {2000, 1000, 400, 200}
	Make/I offset = {-2470000, -1235000, -2472000, -1236000}

	try
		IPNWB_WriteCompound /S=offset /C=size /REF=refs /LOC="" dataPath; AbortOnRTE
		FAIL()
	catch
		err = getRTError(1)
		PASS()
	endtry

End

/// @brief Fail test write to not existing
static Function WriteCompoundNoFile()

	variable err

	Make/T refs = {"/acquisition/vcs", "/stimulus/presentation/ccss", "/acquisition/vcs", "/stimulus/presentation/ccss"}
	Make/I size = {2000, 1000, 400, 200}
	Make/I offset = {-2470000, -1235000, -2472000, -1236000}

	try
		IPNWB_WriteCompound /S=offset /C=size /REF=refs /LOC="/intervals/epochs/timeseries" "::notexisting::"; AbortOnRTE
		FAIL()
	catch
		err = getRTError(1)
		PASS()
	endtry

End

/// @brief Fail test write to empty filename
static Function WriteCompoundEmptyFile()

	variable err

	Make/T refs = {"/acquisition/vcs", "/stimulus/presentation/ccss", "/acquisition/vcs", "/stimulus/presentation/ccss"}
	Make/I size = {2000, 1000, 400, 200}
	Make/I offset = {-2470000, -1235000, -2472000, -1236000}

	try
		IPNWB_WriteCompound /S=offset /C=size /REF=refs /LOC="/intervals/epochs/timeseries" ""; AbortOnRTE
		FAIL()
	catch
		err = getRTError(1)
		PASS()
	endtry

End

/// @brief Fail test write different sized waves
static Function WriteCompoundWrongWaves()

	variable err

	string dataPath

	PathInfo home
	dataPath = ParseFilepath(5, S_path, "\\", 0, 0) + "test_fresh2.h5"

	Make/T refs = {"/acquisition/vcs", "/stimulus/presentation/ccss", "/acquisition/vcs", "/stimulus/presentation/ccss"}
	Make/I size = {2000, 1000, 400}
	Make/I offset = {-2470000, -1235000}

	try
		IPNWB_WriteCompound /S=offset /C=size /REF=refs /LOC="/intervals/epochs/timeseries" dataPath; AbortOnRTE
		FAIL()
	catch
		err = getRTError(1)
		PASS()
	endtry

End

/// @brief Rountrip test
static Function WriteCompoundRoundTrip()

	string dataPath

	PathInfo home
	dataPath = ParseFilepath(5, S_path, "\\", 0, 0) + "test_fresh3.h5"

	Make/T refs = {"/acquisition/vcs", "/stimulus/presentation/ccss", "/acquisition/vcs", "/stimulus/presentation/ccss"}
	Make/I size = {2000, 1000, 400, 200}
	Make/I offset = {-2470000, -1235000, -2472000, -1236000}

	IPNWB_WriteCompound /S=offset /C=size /REF=refs /LOC="/intervals/epochs/timeseries" dataPath
	IPNWB_ReadCompound /S=offsetr /C=sizer /REF=refsr /LOC="/intervals/epochs/timeseries" dataPath

	CHECK_EQUAL_WAVES(offset, offsetr)
	CHECK_EQUAL_WAVES(size, sizer)
	CHECK_EQUAL_WAVES(refs, refsr)

	IPNWB_WriteCompound /S=offset /C=size /REF=refs /LOC="/intervals/epochs/timeseries" dataPath
	IPNWB_ReadCompound/FREE /S=foffsetr /C=fsizer /REF=frefsr /LOC="/intervals/epochs/timeseries" dataPath

	Make/FREE/T/N=8 ref8
	Make/FREE/I/N=8 off8, size8
	ref8[] = refs[mod(p, 4)]
	off8[] = offset[mod(p, 4)]
	size8[] = size[mod(p, 4)]
	CHECK_EQUAL_WAVES(off8, foffsetr)
	CHECK_EQUAL_WAVES(size8, fsizer)
	CHECK_EQUAL_WAVES(ref8, frefsr)

End
