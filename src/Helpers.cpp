#include "Helpers.h"
#include "CustomExceptions.h"

#include <cstdint>
#include <utility>

namespace {

static_assert((NT_I8 | NT_UNSIGNED) == NT_UI8, "invalid definition");
static_assert((NT_I16 | NT_UNSIGNED) == NT_UI16, "invalid definition");
static_assert((NT_I32 | NT_UNSIGNED) == NT_UI32, "invalid definition");
static_assert((NT_I64 | NT_UNSIGNED) == NT_UI64, "invalid definition");

} // anonymous namespace

/// @brief Returns a Igor string Handle from a C++ string
///
/// Returns a Igor string Handle from a C++ string
/// returns nullptr if the Handle could not be created or any other error
/// occured
///
/// @param str C++ string
/// @return Igor string handle with the content of the C++ string
Handle GetHandleFromString(const std::string &str) {
  Handle h;
  if (!(h = WMNewHandle(To<BCInt>(str.size())))) {
    return nullptr;
  }
  if (PutCStringInHandle(str.c_str(), h)) {
    return nullptr;
  }
  return h;
}

/// @brief Returns a string from an Igor string Handle
///
/// Returns a string from an Igor string Handle
/// returns an empty string if the string handle is nullptr
///
/// @param strHandle Igor string handle
/// @return string with the content of the Igor string Handle
std::string GetStringFromHandle(Handle strHandle) {
  // Check for special case of null handle.
  if (strHandle == nullptr) {
    return std::string();
  }

  auto strLen = To<size_t>(WMGetHandleSize(strHandle));
  return std::string(*strHandle, strLen);
}

/// @brief Returns a string from an Igor string Handle and disposes the Handle
///
/// Returns a string from an Igor string Handle
/// returns an empty string if the sring handle is nullptr
///
/// @param strHandle Igor string handle
/// @return string with the content of the Igor string Handle
std::string GetStringFromHandleWithDispose(Handle strHandle) {
  const auto str = GetStringFromHandle(strHandle);
  WMDisposeHandle(strHandle);

  return str;
}

void OutputToHistory(std::string str,
                     OutputMode mode /*= OutputMode::Normal*/) {
  if (str.empty()) {
    return;
  }

  XOPNotice2(str.append(CR_STR).c_str(), mode == OutputMode::Normal ? 0x1 : 0);
}

void OutputToHistory(const char *c_str,
                     OutputMode mode /*= OutputMode::Normal*/) {
  if (c_str == nullptr || *c_str == '\0') {
    return;
  }

  std::string str = c_str;

  OutputToHistory(str, mode);
}

template <>
float ConvertFromDouble(double val, const std::string & /* errorMsg  */) {
  return static_cast<float>(val);
}

template <>
double ConvertFromDouble(double val, const std::string & /* errorMsg  */) {
  return val;
}

template <> bool ConvertFromDouble(double val, const std::string &errorMsg) {
  // If value is NaN or inf, return an appropriate error.
  if (std::isnan(val) || std::isinf(val)) {
    throw IgorException(kDoesNotSupportNaNorINF, errorMsg);
  }

  const double TOLERANCE = 1e-8;

  return std::abs(val) > TOLERANCE;
}

template <>
double ParseString<double>(const std::string &str,
                           const std::string &errorMsg) {
  size_t pos = 0;
  auto val = std::numeric_limits<double>::quiet_NaN();

  try {
    val = std::stod(str, &pos);
  } catch (std::exception &) {
    throw IgorException(ERR_CONVERT, errorMsg);
  }
  if (pos != str.size()) {
    throw IgorException(ERR_CONVERT, errorMsg);
  }
  return val;
}

template <>
float ParseString<float>(const std::string &str, const std::string &errorMsg) {
  size_t pos = 0;
  auto val = std::numeric_limits<float>::quiet_NaN();

  try {
    val = std::stof(str, &pos);
  } catch (std::exception &) {
    throw IgorException(ERR_CONVERT, errorMsg);
  }
  if (pos != str.size()) {
    throw IgorException(ERR_CONVERT, errorMsg);
  }
  return val;
}

template <>
int64_t ParseString<int64_t>(const std::string &str,
                             const std::string &errorMsg) {
  size_t pos = 0;
  int64_t val = 0;
  try {
    val = std::stoll(str, &pos);
  } catch (std::exception &) {
    throw IgorException(ERR_CONVERT, errorMsg);
  }
  if (pos != str.size()) {
    throw IgorException(ERR_CONVERT, errorMsg);
  }
  return val;
}

template <>
uint64_t ParseString<uint64_t>(const std::string &str,
                               const std::string &errorMsg) {
  size_t pos = 0;
  uint64_t val = 0;
  try {
    val = std::stoull(str, &pos);
  } catch (std::exception &) {
    throw IgorException(ERR_CONVERT, errorMsg);
  }
  if (pos != str.size()) {
    throw IgorException(ERR_CONVERT, errorMsg);
  }
  return val;
}

template <>
int32_t ParseString<int32_t>(const std::string &str,
                             const std::string &errorMsg) {
  size_t pos = 0;
  int32_t val = 0;
  try {
    val = std::stoi(str, &pos);
  } catch (std::exception &) {
    throw IgorException(ERR_CONVERT, errorMsg);
  }
  if (pos != str.size()) {
    throw IgorException(ERR_CONVERT, errorMsg);
  }
  return val;
}

template <>
uint32_t ParseString<uint32_t>(const std::string &str,
                               const std::string &errorMsg) {
  size_t pos = 0;
  uint32_t val = 0;
  try {
#ifdef MACIGOR64
    uint64_t val_long = std::stoul(str, &pos);

    if (val_long >
        static_cast<uint64_t>(std::numeric_limits<uint32_t>::max())) {
      throw IgorException(ERR_CONVERT, errorMsg);
    }

    val = static_cast<uint32_t>(val_long);
#else
    val = std::stoul(str, &pos);
#endif
  } catch (std::exception &) {
    throw IgorException(ERR_CONVERT, errorMsg);
  }
  if (pos != str.size()) {
    throw IgorException(ERR_CONVERT, errorMsg);
  }
  return val;
}

#ifdef MACIGOR64

// MacOSX 64bit: size_t aka unsigned long aka unsigned long long (8 byte)
// is different from the above types
template <>
size_t ParseString<size_t>(const std::string &str,
                           const std::string &errorMsg) {
  static_assert(sizeof(size_t) == sizeof(uint64_t), "Unexpected size_t");
  return ParseString<uint64_t>(str, errorMsg);
}

#endif

void CheckStructType(Handle structType, const std::string &requiredType,
                     const std::string &errorMsg) {
  if (requiredType != GetStringFromHandle(structType)) {
    throw IgorException(EXPECT_COMPAT_STRUCT, errorMsg);
  }
}

void CheckStructVersion(double version, int requiredVersion,
                        const std::string &errorMsg) {
  if (requiredVersion != ConvertFromDouble<int>(version, errorMsg)) {
    throw IgorException(EXPECT_COMPAT_STRUCT, errorMsg);
  }
}

std::size_t GetWaveElementSize(int dataType) {
  switch (dataType) {
  /// size assumptions about real/double values are
  // not guaranteed by the standard but should work
  case NT_CMPLX | NT_FP32:
    return 2 * sizeof(float);
  case NT_CMPLX | NT_FP64:
    return 2 * sizeof(double);
  case NT_FP32:
    return sizeof(float);
  case NT_FP64:
    return sizeof(double);
  case NT_I8:
    return sizeof(int8_t);
  case NT_I8 | NT_UNSIGNED:
    return sizeof(uint8_t);
  case NT_I16:
    return sizeof(int16_t);
  case NT_I16 | NT_UNSIGNED:
    return sizeof(uint16_t);
  case NT_I32:
    return sizeof(int32_t);
  case NT_I32 | NT_UNSIGNED:
    return sizeof(uint32_t);
  case NT_I64:
    return sizeof(int64_t);
  case NT_I64 | NT_UNSIGNED:
    return sizeof(uint64_t);
  default:
    throw IgorException(GENERAL_BAD_VIBS);
  }
}

void WaveClear(waveHndl wv) {
  if (wv == nullptr) {
    throw IgorException(USING_NULL_REFVAR);
  }

  int type = WaveType(wv);
  if (type == TEXT_WAVE_TYPE) {
    ClearTextWave(wv);
  } else {
    const auto numBytes = To<size_t>(WaveMemorySize(wv, 2));

    if (numBytes == 0) // nothing to do
    {
      return;
    }

    std::memset(WaveData(wv), 0, numBytes);
  }
}

void SetDimensionLabels(waveHndl h, int Dimension,
                        const std::vector<std::string> &dimLabels) {
  // Check wave exists.
  if (h == nullptr) {
    throw IgorException(NULL_WAVE_OP);
  }

  for (size_t k = 0; k < dimLabels.size(); k++) {
    if (dimLabels[k].empty()) {
      // Empty string.  Skip.
      continue;
    }

    if (int RetVal = MDSetDimensionLabel(h, Dimension, To<IndexInt>(k),
                                         dimLabels[k].c_str())) {
      throw IgorException(RetVal);
    }
  }
}

void StringVectorToTextWave(const std::vector<std::string> &stringVector,
                            waveHndl waveHandle) {
  if (stringVector.empty()) {
    return;
  }

  if (!waveHandle) {
    throw IgorException(USING_NULL_REFVAR);
  }

  size_t offset = 0;
  size_t totalSize = 0;

  const size_t numEntriesPlusOne = stringVector.size() + 1;

  std::vector<size_t> stringSizes;
  stringSizes.reserve(numEntriesPlusOne);

  for (const auto &elem : stringVector) {
    auto size = elem.size();

    stringSizes.push_back(size);
    totalSize += size;
  }

  totalSize += numEntriesPlusOne * sizeof(size_t);

  Handle textHandle = WMNewHandle(To<BCInt>(totalSize));

  if (textHandle == nullptr) {
    throw IgorException(NOMEM);
  }

  for (size_t i = 0; i < numEntriesPlusOne; i++) {
    if (i == 0) // position of the first string
    {
      offset = numEntriesPlusOne * sizeof(size_t);
    } else // and of all the others
    {
      offset += stringSizes[i - 1];
    }

    // write offsets
    std::memcpy(*textHandle + i * sizeof(size_t), &offset, sizeof(size_t));

    if (i < stringVector.size()) {
      // write strings
      std::memcpy(*textHandle + offset, stringVector[i].c_str(),
                  stringSizes[i]);
    }
  }

  // mode = 2 defines the format of the handle contents to
  // offsetToFirstString
  // offsetToSecondString
  // ...
  // offsetToPositionAfterLastString
  // firstString
  //...
  const int mode = 2;

  if (int ret = SetTextWaveData(waveHandle, mode, textHandle)) {
    WMDisposeHandle(textHandle);
    throw IgorException(ret);
  }

  WMDisposeHandle(textHandle);
}

// @brief Clears a text wave, sets all elements to zero sized strings. Works for
// 32 and 64 bit
void ClearTextWave(waveHndl w) {
  Handle textH;
  size_t fullSize;
  int err;
  std::vector<CountInt> dims(MAX_DIMENSIONS + 1, 0);
  int numDims;

  if (err = MDGetWaveDimensions(w, &numDims, dims.data())) {
    throw IgorException(err, "Error getting wave dimensions.");
  }
  dims.resize(To<size_t>(numDims));

  size_t size = 1;
  for (auto &e : dims) {
    size *= To<size_t>(e);
  }

  if (!size) {
    return;
  }

  fullSize = sizeof(size_t) * (size + 1);
  if (!(textH = WMNewHandle(To<BCInt>(fullSize)))) {
    throw IgorException(ERR_CONVERT,
                        "Error creating handle for text wave clear.");
  }

  auto *textHS = reinterpret_cast<size_t *>(*textH); // NOLINT
  // Fill offset area: offsets including the 'extra offset', as offsets increase
  // by one each string has size 1
  std::memset(textHS, 0, fullSize); // NOLINT
  // no data area

  if (err = SetTextWaveData(w, 2, textH)) {
    WMDisposeHandle(textH);
    throw IgorException(err, "Error clearing text wave.");
  }
  WMDisposeHandle(textH);
}

// @brief sets all elements of a text wave to the string value. Works for 32 and
// 64 bit
void SetTextWave(waveHndl w, const std::string &value) {
  Handle textH;
  size_t fullSize;
  size_t offsetSize;
  size_t offset;
  int err;
  std::vector<CountInt> dims(MAX_DIMENSIONS + 1, 0);
  int numDims;

  if (err = MDGetWaveDimensions(w, &numDims, dims.data())) {
    throw IgorException(err, "Error getting wave dimensions.");
  }
  dims.resize(To<size_t>(numDims));

  size_t size = 1;
  for (auto &e : dims) {
    size *= To<size_t>(e);
  }

  if (!size) {
    return;
  }

  offsetSize = sizeof(size_t) * (size + 1);
  fullSize = offsetSize + value.size() * size;
  if (!(textH = WMNewHandle(To<BCInt>(fullSize)))) {
    throw IgorException(ERR_CONVERT,
                        "Error creating handle for set text wave.");
  }

  auto *textOffs = reinterpret_cast<size_t *>(*textH); // NOLINT
  // Fill offset area: offsets including the 'extra offset'
  for (size_t pos = 0; pos < size; pos++) {
    offset = pos * value.size();
    *(textOffs + pos) = offset; // NOLINT
    std::memcpy(*textH + offsetSize + offset, value.c_str(),
                value.size()); // NOLINT
  }
  // set "extra offset"
  *(textOffs + size) = size * value.size(); // NOLINT

  if (err = SetTextWaveData(w, 2, textH)) {
    WMDisposeHandle(textH);
    throw IgorException(err, "Error clearing text wave.");
  }
  WMDisposeHandle(textH);
}

void ASSERT(bool cond, const std::string &errorMsg) {
  if (!cond) {
    throw IgorException(ERR_ASSERT, errorMsg);
  }
}

std::string GetStackTrace() {
  Handle h = WMNewHandle(0);

  if (int err = GetIgorRTStackInfo(3, &h)) {
    return "unknown stacktrace";
  }

  return GetStringFromHandleWithDispose(h);
}

void SetOperationReturn(const std::string &name, const std::string &value) {
  if (int err = SetOperationStrVar(name.c_str(), value.c_str())) {
    throw IgorException(err, "Error setting {}."_format(name));
  }
}

void SetOperationReturn(const std::string &name, double value) {
  if (int err = SetOperationNumVar(name.c_str(), value)) {
    throw IgorException(err, "Error setting {}."_format(name));
  }
}

void HandleDestWave(int FlagParamsSet, const DataFolderAndName &dfAndName,
                    const int freeFlagEncountered, std::vector<CountInt> dims,
                    const std::function<void(waveHndl)> &checkWaveProperties,
                    const std::function<int(waveHndl)> &typeGetter,
                    const std::function<void(waveHndl)> &setWaveContents) {
  int err;

  ASSERT(dims.size() == MAX_DIMENSIONS + 1,
         "dims vector for HandleDestWave must have size MAX_DIMENSIONS + 1");

  // Igor Pro wave handling code
  waveHndl destWaveH = nullptr;
  const int optExists = kOpDestWaveOverwriteOK | kOpDestWaveMustAlreadyExist;
  const int optOverwrite =
      kOpDestWaveOverwriteOK | kOpDestWaveOverwriteExistingWave;
  const int optNewFree = kOpDestWaveMakeFreeWave;
  int waveOptions = optOverwrite;
  int type = TEXT_WAVE_TYPE;

  if (freeFlagEncountered) {
    waveOptions = optNewFree;
  } else {
    // recover type of preexisting wave if it does
    err = GetOperationDestWave(
        dfAndName.dfH, static_cast<const char *>(dfAndName.name), FlagParamsSet,
        optExists, dims.data(), type, &destWaveH, nullptr);
    if (!((err == 0) || (err == NOWAV))) {
      throw IgorException(err, "Error retrieving wave from operation.");
    }
    if (err == 0) {
      checkWaveProperties(destWaveH);
    }
  }
  type = typeGetter(destWaveH);

  // change/create wave
  if (err = GetOperationDestWave(
          dfAndName.dfH, static_cast<const char *>(dfAndName.name),
          FlagParamsSet, waveOptions, dims.data(), type, &destWaveH, nullptr)) {
    throw IgorException(err, "Error creating or changing wave.");
  }

  setWaveContents(destWaveH);

  WaveHandleModified(destWaveH);
  if (FlagParamsSet) {
    SetOperationWaveRef(destWaveH, FlagParamsSet);
  }
}

std::vector<CountInt> GetWaveDimension(waveHndl w) {
  int numDims;
  return GetWaveDimension(w, numDims);
}

std::vector<CountInt> GetWaveDimension(waveHndl w, int &numDims) {
  if (!w) {
    throw IgorException(NOWAV);
  }
  std::vector<CountInt> dims(MAX_DIMENSIONS + 1, 0);
  if (int err = MDGetWaveDimensions(w, &numDims, dims.data())) {
    throw IgorException(err, "Error retrieving wave dimension.");
  }

  return dims;
}

void CheckWaveDimension(waveHndl w, const std::vector<CountInt> &expectedDims,
                        const std::string &errorMsg) {
  std::vector<CountInt> compDims(MAX_DIMENSIONS + 1, 0);
  auto size = To<std::ptrdiff_t>(expectedDims.size() < compDims.size() - 1
                                     ? expectedDims.size()
                                     : compDims.size() - 1);
  std::copy(expectedDims.begin(), expectedDims.begin() + size,
            compDims.begin());

  std::vector<CountInt> dims = GetWaveDimension(w);
  if (compDims != dims) {
    throw IgorException(ERR_INVALID_TYPE, errorMsg);
  }
}

template <>
void SetWaveElement<std::string>(waveHndl w, std::vector<IndexInt> &dims,
                                 const std::string &value) {
  if (!w) {
    throw IgorException(NOWAV);
  }

  ASSERT(dims.size() == MAX_DIMENSIONS,
         "dims vector for SetWaveElement must have size MAX_DIMENSIONS");

  if (WaveType(w) == TEXT_WAVE_TYPE) {
    Handle textH = GetHandleFromString(value);
    int err = MDSetTextWavePointValue(w, dims.data(), textH);
    WMDisposeHandle(textH);
    if (err) {
      throw IgorException(err, "Error writing values to text wave");
    }
  } else {
    throw IgorException(ERR_INVALID_TYPE, "Wave is not a text wave.");
  }
}

template <>
std::string GetWaveElement<std::string>(waveHndl w,
                                        std::vector<IndexInt> &dims) {
  if (!w) {
    throw IgorException(NOWAV);
  }

  ASSERT(dims.size() == MAX_DIMENSIONS,
         "dims vector for SetWaveElement must have size MAX_DIMENSIONS");

  if (WaveType(w) == TEXT_WAVE_TYPE) {
    Handle textH;
    if (!(textH = WMNewHandle(0L))) {
      throw IgorException(NOMEM, "Error creating Igor Handle.");
    }
    if (int err = MDGetTextWavePointValue(w, dims.data(), textH)) {
      WMDisposeHandle(textH);
      throw IgorException(err, "Error reading value from text wave");
    }
    return GetStringFromHandleWithDispose(textH);
  }

  throw IgorException(ERR_INVALID_TYPE, "XOP Bug: Wave is not a text wave.");
}

std::string ExecuteCommand(const std::string &cmd,
                           OutputMode mode /* = OutputMode::Silent */) {
  Handle text;
  int err =
      XOPCommand3(cmd.c_str(), mode == OutputMode::Silent ? 1 : 0, 0, &text);

  if (err) {
    throw IgorException(err, "Error executing the cmd {}"_format(cmd));
  }

  return GetStringFromHandleWithDispose(text);
}

StrStrMap GetVersionInfo(const std::string &xopName) {
  ASSERT(!xopName.empty(), "GetVersionInfo: xopName can not be empty");

  std::map<std::string, std::string> m;

  m["name"] = xopName;
  //  m["version"] = GIT_REVISION;

#ifdef MACIGOR64
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdate-time"
#endif

  m["builddate"] = "{} {}"_format(__DATE__, __TIME__); // NOLINT

#ifdef MACIGOR64
#pragma clang diagnostic pop
#endif

#ifdef MACIGOR64
  m["compiler"] = "Clang {}.{}.{}"_format(__clang_major__, __clang_minor__,
                                          __clang_patchlevel__);
#else
  m["compiler"] = "Visual Studio {}"_format(_MSC_VER);
#endif

  return m;
}
