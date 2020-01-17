#include <cstdint>

#include "Helpers.h"
#include "CustomExceptions.h"

/// @brief Returns a Igor string Handle from a C++ string
///
/// Returns a Igor string Handle from a C++ string
/// returns nullptr if the Handle could not be created or any other error occured
///
/// @param str C++ string
/// @return Igor string handle with the content of the C++ string
Handle GetHandleFromString(std::string str)
{
  Handle h;
  if(!(h = WMNewHandle(str.size())))
    return nullptr;
  if(PutCStringInHandle(str.c_str(), h))
    return nullptr;
  return h;
}

/// @brief Returns a string from an Igor string Handle
///
/// Returns a string from an Igor string Handle
/// returns an empty string if the sring handle is nullptr
///
/// @param strHandle Igor string handle
/// @return string with the content of the Igor string Handle
std::string GetStringFromHandle(const Handle &strHandle)
{
  // Check for special case of null handle.
  if(strHandle == nullptr)
  {
    return std::string();
  }

  auto strLen = WMGetHandleSize(strHandle);
  return std::string(*strHandle, strLen);
}

/// @brief Returns a string from an Igor string Handle and disposes the Handle
///
/// Returns a string from an Igor string Handle
/// returns an empty string if the sring handle is nullptr
///
/// @param strHandle Igor string handle
/// @return string with the content of the Igor string Handle
std::string GetStringFromHandleWithDispose(const Handle &strHandle)
{
  const auto str = GetStringFromHandle(strHandle);
  WMDisposeHandle(strHandle);

  return str;
}

void OutputToHistory(std::string str, OutputMode mode /*= OutputMode::Normal*/)
{
  if(str.empty())
  {
    return;
  }

  XOPNotice2(str.append(CR_STR).c_str(), mode == OutputMode::Normal ? 0x1 : 0);
}

void OutputToHistory(const char *c_str, OutputMode mode /*= OutputMode::Normal*/)
{
  if(c_str == nullptr || *c_str == '\0')
  {
    return;
  }

  std::string str = c_str;

  OutputToHistory(str, mode);
}

template <>
float ConvertFromDouble(double val, std::string errorMsg)
{
  return static_cast<float>(val);
}

template <>
double ConvertFromDouble(double val, std::string errorMsg)
{
  return val;
}

template <>
bool ConvertFromDouble(double val, std::string errorMsg)
{
  // If value is NaN or inf, return an appropriate error.
  if(std::isnan(val) || std::isinf(val))
  {
    throw IgorException(kDoesNotSupportNaNorINF, errorMsg);
  }

  return std::abs(val) > 1e-8;
}

template <>
double ParseString<double>(std::string str, std::string msg)
{
  size_t pos = 0;
  double val = NAN;

  try
  {
    val = std::stod(str, &pos);
  }
  catch(std::exception)
  {
    throw IgorException(ERR_CONVERT, msg);
  }
  if(pos != str.size())
  {
    throw IgorException(ERR_CONVERT, msg);
  }
  return val;
}

template <>
float ParseString<float>(std::string str, std::string msg)
{
  size_t pos = 0;
  float val  = NAN;
  try
  {
    val = std::stof(str, &pos);
  }
  catch(std::exception)
  {
    throw IgorException(ERR_CONVERT, msg);
  }
  if(pos != str.size())
  {
    throw IgorException(ERR_CONVERT, msg);
  }
  return val;
}

template <>
int64_t ParseString<int64_t>(std::string str, std::string msg)
{
  size_t pos  = 0;
  int64_t val = 0;
  try
  {
    val = std::stoll(str, &pos);
  }
  catch(std::exception)
  {
    throw IgorException(ERR_CONVERT, msg);
  }
  if(pos != str.size())
  {
    throw IgorException(ERR_CONVERT, msg);
  }
  return val;
}

template <>
uint64_t ParseString<uint64_t>(std::string str, std::string msg)
{
  size_t pos   = 0;
  uint64_t val = 0;
  try
  {
    val = std::stoull(str, &pos);
  }
  catch(std::exception)
  {
    throw IgorException(ERR_CONVERT, msg);
  }
  if(pos != str.size())
  {
    throw IgorException(ERR_CONVERT, msg);
  }
  return val;
}

template <>
int32_t ParseString<int32_t>(std::string str, std::string msg)
{
  size_t pos  = 0;
  int32_t val = 0;
  try
  {
    val = std::stoi(str, &pos);
  }
  catch(std::exception)
  {
    throw IgorException(ERR_CONVERT, msg);
  }
  if(pos != str.size())
  {
    throw IgorException(ERR_CONVERT, msg);
  }
  return val;
}

template <>
uint32_t ParseString<uint32_t>(std::string str, std::string msg)
{
  size_t pos   = 0;
  uint32_t val = 0;
  try
  {
    val = std::stoul(str, &pos);
  }
  catch(std::exception)
  {
    throw IgorException(ERR_CONVERT, msg);
  }
  if(pos != str.size())
  {
    throw IgorException(ERR_CONVERT, msg);
  }
  return val;
}

#ifdef MACIGOR64

// MacOSX 64bit: size_t aka unsigned long (8 byte)
// is different from the above types
template <>
size_t ParseString<size_t>(std::string str, std::string msg)
{
  static_assert(sizeof(size_t) == sizeof(uint64_t), "Unexpected size_t");
  return ParseString<uint64_t>(str, msg);
}

#endif

void CheckStructType(Handle structType, std::string requiredType, std::string errorMsg)
{
  if(requiredType != GetStringFromHandle(structType))
  {
    throw IgorException(EXPECT_COMPAT_STRUCT, errorMsg);
  }
}

void CheckStructVersion(double version, int requiredVersion, std::string errorMsg)
{
  if(requiredVersion != ConvertFromDouble<int>(version, errorMsg))
  {
    throw IgorException(EXPECT_COMPAT_STRUCT, errorMsg);
  }
}

std::size_t GetWaveElementSize(int dataType)
{
  switch(dataType)
  {
  /// size assumptions about real/double values are
  // not guaranteed by the standard but should work
  case NT_CMPLX | NT_FP32:
    return 2 * sizeof(float);
    break;
  case NT_CMPLX | NT_FP64:
    return 2 * sizeof(double);
    break;
  case NT_FP32:
    return sizeof(float);
    break;
  case NT_FP64:
    return sizeof(double);
    break;
  case NT_I8:
    return sizeof(int8_t);
    break;
  case NT_I8 | NT_UNSIGNED:
    return sizeof(uint8_t);
    break;
  case NT_I16:
    return sizeof(int16_t);
    break;
  case NT_I16 | NT_UNSIGNED:
    return sizeof(uint16_t);
    break;
  case NT_I32:
    return sizeof(int32_t);
    break;
  case NT_I32 | NT_UNSIGNED:
    return sizeof(uint32_t);
    break;
  case NT_I64:
    return sizeof(int64_t);
    break;
  case NT_I64 | NT_UNSIGNED:
    return sizeof(uint64_t);
    break;
  default:
    throw IgorException(GENERAL_BAD_VIBS);
    break;
  }
}

void WaveClear(waveHndl wv)
{
  if(wv == nullptr)
  {
    throw IgorException(USING_NULL_REFVAR);
  }

  const auto numBytes = WavePoints(wv) * GetWaveElementSize(WaveType(wv));

  if(numBytes == 0) // nothing to do
  {
    return;
  }

  MemClear(WaveData(wv), numBytes);
}

void SetDimensionLabels(waveHndl h, int Dimension, const std::vector<std::string> &dimLabels)
{
  // Check wave exists.
  if(h == nullptr)
  {
    throw IgorException(NULL_WAVE_OP);
  }

  for(size_t k = 0; k < dimLabels.size(); k++)
  {
    if(dimLabels[k].size() == 0)
    {
      // Empty string.  Skip.
      continue;
    }

    if(int RetVal = MDSetDimensionLabel(h, Dimension, k, dimLabels[k].c_str()))
    {
      throw IgorException(RetVal);
    }
  }
}

void StringVectorToTextWave(const std::vector<std::string> &stringVector, waveHndl waveHandle)
{
  if(stringVector.empty())
  {
    return;
  }

  if(!waveHandle)
  {
    throw IgorException(USING_NULL_REFVAR);
  }

  size_t offset;
  size_t totalSize = 0;

  const size_t numEntriesPlusOne = stringVector.size() + 1;

  std::vector<size_t> stringSizes;
  stringSizes.reserve(numEntriesPlusOne);

  for(const auto &elem : stringVector)
  {
    auto size = elem.size();

    stringSizes.push_back(size);
    totalSize += size;
  }

  totalSize += numEntriesPlusOne * sizeof(size_t);

  Handle textHandle = WMNewHandle(totalSize);

  if(textHandle == nullptr)
  {
    throw IgorException(NOMEM);
  }

  for(size_t i = 0; i < numEntriesPlusOne; i++)
  {
    if(i == 0) // position of the first string
    {
      offset = numEntriesPlusOne * sizeof(size_t);
    }
    else // and of all the others
    {
      offset += stringSizes[i - 1];
    }

    // write offsets
    std::memcpy(*textHandle + i * sizeof(size_t), &offset, sizeof(size_t));

    if(i < stringVector.size())
    {
      // write strings
      std::memcpy(*textHandle + offset, stringVector[i].c_str(), stringSizes[i]);
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

  if(int ret = SetTextWaveData(waveHandle, mode, textHandle))
  {
    WMDisposeHandle(textHandle);
    throw IgorException(ret);
  };

  WMDisposeHandle(textHandle);
}

// works for 32 and 64 bit
void ClearTextWave(waveHndl w)
{
  Handle textH;
  size_t fullSize;
  int err;
  std::vector<CountInt> dims(MAX_DIMENSIONS + 1, 0);
  int numDims;

  if(err = MDGetWaveDimensions(w, &numDims, dims.data()))
    throw IgorException(err, "Error getting wave dimensions.");
  dims.resize(numDims);

  size_t size = 1;
  for(auto &e : dims)
    size *= e;

  fullSize = sizeof(size_t) * (size + 1) + size;
  if(!(textH = WMNewHandle(fullSize)))
    throw IgorException(ERR_CONVERT, "Error creating handle for text wave clear.");

  size_t *textHS = reinterpret_cast<size_t *>(textH);
  std::memset(textHS + size, 0, size + sizeof(size_t));
  for(size_t pos = 0; pos < size; pos++)
    *(textHS + pos) = pos;

  if(err = SetTextWaveData(w, 2, textH))
  {
    WMDisposeHandle(textH);
    throw IgorException(err, "Error clearing text wave.");
  }
  WMDisposeHandle(textH);
}

std::string UnescapePathSegment(std::string path)
{
  escapeCheck(path);
  return unescape(path);
}

void ASSERT(bool cond, std::string msg)
{
  if(!cond)
    throw IgorException(ERR_ASSERT, msg);
}

// The following code is copied/adapted from nlohmann json library
static void escapeCheck(std::string &reference_token)
{
  // check reference tokens are properly escaped
  for(std::size_t pos = reference_token.find_first_of('~'); pos != std::string::npos;
      pos             = reference_token.find_first_of('~', pos + 1))
  {
    if(reference_token[pos] != '~')
      throw IgorException(ERR_PARSE, "error parsing path");

    // ~ must be followed by 0 or 1
    if((pos == reference_token.size() - 1) || ((reference_token[pos + 1] != '0') && (reference_token[pos + 1] != '1')))
    {
      throw IgorException(ERR_PARSE, "escape character '~' must be followed with '0' or '1'");
    }
  }
}

/*!
@brief replace all occurrences of a substring by another string

@param[in,out] s  the string to manipulate; changed so that all
               occurrences of @a f are replaced with @a t
@param[in]     f  the substring to replace with @a t
@param[in]     t  the string to replace @a f

@pre The search string @a f must not be empty. **This precondition is
enforced with an assertion.**

@since version 2.0.0
*/
static void replace_substring(std::string &s, const std::string &f, const std::string &t)
{
  if(f.empty())
    throw IgorException(ERR_PARSE, "error parsing path");

  for(auto pos = s.find(f);            // find first occurrence of f
      pos != std::string::npos;        // make sure f was found
      s.replace(pos, f.size(), t),     // replace with t, and
      pos = s.find(f, pos + t.size())) // find next occurrence of f
  {
  }
}

/// unescape "~1" to tilde and "~0" to slash (order is important!)
std::string unescape(std::string s)
{
  if(s.empty())
    return s;

  replace_substring(s, "~1", "/");
  replace_substring(s, "~0", "~");
  return s;
}

/// escape "~" to "~0" and "/" to "~1"
std::string escape(std::string s)
{
  if(s.empty())
    return s;

  replace_substring(s, "~", "~0");
  replace_substring(s, "/", "~1");
  return s;
}
// The upper code is copied/adapted from nlohmann json library

std::string EscapeJSONString(std::string s)
{
  if(s.empty())
    return s;

  replace_substring(s, "\\", "\\\\");
  replace_substring(s, "\b", "\\b");
  replace_substring(s, "\f", "\\f");
  replace_substring(s, "\n", "\\n");
  replace_substring(s, "\r", "\\r");
  replace_substring(s, "\t", "\\t");
  replace_substring(s, "\"", "\\\"");
  return s;
}

std::string GetStackTrace() noexcept
{
  Handle h = WMNewHandle(0);

  if(int err = GetIgorRTStackInfo(3, &h))
  {
    return "unknown stacktrace";
  }
  else
  {
    return GetStringFromHandleWithDispose(h);
  }
}
