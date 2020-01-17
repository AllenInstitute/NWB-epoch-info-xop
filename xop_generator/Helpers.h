#pragma once

#include "XOPStandardHeaders.h" // Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "CustomExceptions.h"
#include "xop_errors.h"

#include <cmath>
#include <vector>
#include <limits>
#include <string>
#include <type_traits>
#include <fmt/format.h>

#undef min
#undef max

using namespace fmt::literals;

// See http://fmtlib.net/latest/api.html#formatting-user-defined-types
namespace fmt
{
template <typename T, typename Char>
struct formatter<T, Char, std::enable_if_t<std::is_base_of<std::exception, T>::value>> : formatter<std::string>
{
  template <typename FormatContext>
  auto format(const std::exception &e, FormatContext &ctx)
  {
    return fmt::formatter<std::string>::format(e.what(), ctx);
  }
};
} // namespace fmt

/// @brief Converts a double value to the specified integer/float/bool type.
///
/// Returns an error if:
/// - The value is NaN or +/- inf (for integers and bools)
/// - The value lies outside the range of the integer representation
///
/// The value is truncated towards zero
/// That is:
/// - Positive numbers are rounded down
/// - Negative numbers are rounded up
///
/// @tparam  T    integer type to convert to.
/// @param   msg  exception message on error
///
/// @return convValue  converted value
template <typename T>
T ConvertFromDouble(double val, std::string errorMsg)
{
  // If value is NaN or inf, return an appropriate 'error'.
  if(std::isnan(val) || std::isinf(val))
  {
    throw IgorException(kDoesNotSupportNaNorINF, errorMsg);
  }
  // If value lies outside range of integer type, return an 'error'.
  if(val > (double) (std::numeric_limits<T>::max)() || val < (double) (std::numeric_limits<T>::min)())
  {
    throw IgorException(kParameterOutOfRange, errorMsg);
  }

  // Truncate towards zero.
  if(val > 0)
  {
    val = std::floor(val);
  }
  else if(val < 0)
  {
    val = std::ceil(val);
  }

  return static_cast<T>(val);
}

template <>
float ConvertFromDouble<float>(double val, std::string errorMsg);

template <>
double ConvertFromDouble<double>(double val, std::string errorMsg);

template <>
bool ConvertFromDouble<bool>(double val, std::string errorMsg);

std::string GetStringFromHandle(const Handle &strHandle);
std::string GetStringFromHandleWithDispose(const Handle &strHandle);

uint64_t StringToUint64_t(std::string s, std::string errorMsg);
Handle GetHandleFromString(std::string str);

enum class OutputMode
{
  Normal, ///< Mark experiment as modified
  Silent  ///< Don't mark the experiment as modified
};

void OutputToHistory(std::string str, OutputMode mode = OutputMode::Normal);
void OutputToHistory(const char *c_str, OutputMode mode = OutputMode::Normal);

/// @brief Checks structure type field
///
/// @param  structType Igor string with the current type
/// @param  RequiredType required type
void CheckStructType(Handle structType, std::string RequiredType, std::string errorMsg);

/// @brief Checks structure version field
///
/// @param  version from Igor with the current version
/// @param  requiredVersion required version
void CheckStructVersion(double version, int requiredVersion, std::string errorMsg);

/// @brief Return the size in bytes of the given Igor Pro wave types
///
/// The returned size is zero for non-numeric wave types
std::size_t GetWaveElementSize(int dataType);

/// @brief Set all elements of the given wave to zero
///
/// Does nothing for non-numeric waves
void WaveClear(waveHndl wv);

/// @brief Set dimension labels on wave
///
/// @param	h	Wave to set dimension labels on
/// @param	Dimension	Dimension to set labels on
/// @param	colLabels	vector of labels to set
///
/// dimLabels[k] will be assigned to index k of the wave
void SetDimensionLabels(waveHndl h, int dimension, const std::vector<std::string> &dimLabels);

/// Write stringVector to the text wave waveHandle, using memcpy this is quite fast
void StringVectorToTextWave(const std::vector<std::string> &stringVector, waveHndl waveHandle);

std::string UnescapePathSegment(std::string path);
/// Throws an IgorException if condition is not met with msg
void ASSERT(bool cond, std::string msg);

/// Sets all entries in a text wave to ""
void ClearTextWave(waveHndl w);
/// Sets all entries of a numerical wave to given value
template <typename T,
          typename std::enable_if_t<
              std::is_same<float, T>::value || std::is_same<double, T>::value || std::is_integral<T>::value, int> = 0>
void SetWaveNum(waveHndl w, T value)
{
  T *p        = static_cast<T *>(WaveData(w));
  size_t size = WaveMemorySize(w, 2) / sizeof(T);
  for(size_t s = 0; s < size; s++)
  {
    *p = value;
    p++;
  }
};

template <typename T>
T ParseString(std::string str, std::string msg)
{
  throw IgorException(ERR_CONVERT, "Type not supported by ParseString");
}

template <>
double ParseString<double>(std::string str, std::string msg);

template <>
float ParseString<float>(std::string str, std::string msg);

template <>
uint64_t ParseString<uint64_t>(std::string str, std::string msg);

template <>
uint32_t ParseString<uint32_t>(std::string str, std::string msg);

template <>
int32_t ParseString<int32_t>(std::string str, std::string msg);

template <>
int64_t ParseString<int64_t>(std::string str, std::string msg);

#ifdef MACIGOR64

template <>
size_t ParseString<size_t>(std::string str, std::string msg);

#endif

static void escapeCheck(std::string &reference_token);
static void replace_substring(std::string &s, const std::string &f, const std::string &t);
std::string unescape(std::string s);
std::string escape(std::string s);
std::string EscapeJSONString(std::string s);

/// @brief Return the stack trace of Igor Pro functions
std::string GetStackTrace() noexcept;

/// @brief Determine if T has the member ZFlagEncountered
///
/// From: https://stackoverflow.com/a/16000226
template <typename T, typename = int>
struct HasZ : std::false_type
{
};

template <typename T>
struct HasZ<T, decltype((void) T::ZFlagEncountered, 0)> : std::true_type
{
};

/// @brief Function to parse the /Z flag
///
/// Uses "tag dispatch".
///
/// @param  params function/operation structure which might not have the /Z flag
/// @tparam T      type of the structure
///
/// @return Returns the flag status, true for /Z enabled, false for disabled or not present
template <typename T>
bool ReadZFlag(T *params)
{
  return ReadZFlagImpl(params, HasZ<T>());
}

template <typename T>
bool ReadZFlagImpl(T *params, std::false_type)
{
  return false;
}

template <typename T>
bool ReadZFlagImpl(T *params, std::true_type)
{
  if(!params->ZFlagEncountered)
  {
    // No flag encountered.
    return false;
  }

  // Flag encountered.
  if(!params->ZFlagParamsSet[0])
  {
    // But no parameter not set, equal to true.
    return true;
  }

  // Evaluate set value
  return ConvertFromDouble<bool>(params->ZIn, "/Z flag value must be convertible to 0/1.");
}

#define BEGIN_OUTER_CATCH                                                                                              \
  /* BEGIN: NO-EXCEPTIONS-ALLOWED */                                                                                   \
                                                                                                                       \
  int err    = EXIT_SUCCESS;                                                                                           \
  bool ZFlag = false;                                                                                                  \
                                                                                                                       \
  /* END: NO-EXCEPTIONS-ALLOWED */                                                                                     \
                                                                                                                       \
  try                                                                                                                  \
  {                                                                                                                    \
    ZFlag = ReadZFlag(p);

#define END_OUTER_CATCH                                                                                                \
  }                                                                                                                    \
  catch(const IgorException &e)                                                                                        \
  {                                                                                                                    \
    err = HandleException(e);                                                                                          \
  }                                                                                                                    \
  catch(const std::exception &e)                                                                                       \
  {                                                                                                                    \
    err = HandleException(e);                                                                                          \
  }                                                                                                                    \
  catch(...)                                                                                                           \
  {                                                                                                                    \
    /* Unhandled exception */                                                                                          \
    err = UNHANDLED_CPP_EXCEPTION;                                                                                     \
  }                                                                                                                    \
                                                                                                                       \
  /* BEGIN: NO-EXCEPTIONS-ALLOWED */                                                                                   \
                                                                                                                       \
  if(ZFlag)                                                                                                            \
  {                                                                                                                    \
    SetOperationNumVar("V_flag", err);                                                                                 \
                                                                                                                       \
    return EXIT_SUCCESS;                                                                                               \
  }                                                                                                                    \
                                                                                                                       \
  return err;
