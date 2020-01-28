#pragma once

#include "CustomExceptions.h"
#include "xop_errors.h"

#include <XOPStandardHeaders.h> // Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

#ifdef MACIGOR64
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wswitch-enum"
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#pragma clang diagnostic ignored "-Wheader-hygiene"
#pragma clang diagnostic ignored "-Wweak-vtables"
#pragma clang diagnostic ignored "-Wunused-member-function"
#pragma clang diagnostic ignored "-Wsigned-enum-bitfield"
#pragma clang diagnostic ignored "-Wdouble-promotion"
#endif

#include <fmt/format.h>

using namespace fmt::literals; // NOLINT

#ifdef MACIGOR64
#pragma clang diagnostic pop
#endif

#undef min
#undef max

#ifdef MACIGOR64
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wdouble-promotion"
#endif

#include "SafeInt/SafeInt.hpp"

#ifdef MACIGOR64
#pragma clang diagnostic pop
#endif

using StrStrMap = std::map<std::string, std::string>;

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

/// @brief Safe type conversion
template <typename T, typename U>
T To(U val)
{
  T result;

  if(!SafeCast(val, result))
  {
    throw IgorException(PNTS_INCOMPATIBLE);
  }

  return result;
}

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
/// @tparam  T        integer type to convert to.
/// @param   errorMsg exception message on error
///
/// @return convValue  converted value
template <typename T>
T ConvertFromDouble(double val, const std::string &errorMsg)
{
  // If value is NaN or inf, return an appropriate 'error'.
  if(std::isnan(val) || std::isinf(val))
  {
    throw IgorException(kDoesNotSupportNaNorINF, errorMsg);
  }
  // If value lies outside range of integer type, return an 'error'.
  if(val > static_cast<double>(std::numeric_limits<T>::max()) ||
     val < static_cast<double>(std::numeric_limits<T>::min()))
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
float ConvertFromDouble<float>(double val, const std::string &errorMsg);

template <>
double ConvertFromDouble<double>(double val, const std::string &errorMsg);

template <>
bool ConvertFromDouble<bool>(double val, const std::string &errorMsg);

std::string GetStringFromHandle(Handle strHandle);
std::string GetStringFromHandleWithDispose(Handle strHandle);

Handle GetHandleFromString(const std::string &str);

enum class OutputMode
{
  Normal, ///< Mark experiment as modified
  Silent  ///< Don't mark the experiment as modified
};

void OutputToHistory(std::string str, OutputMode mode = OutputMode::Normal);
void OutputToHistory(const char *c_str, OutputMode mode = OutputMode::Normal);

/// @brief Checks structure type field
///
/// @param  structType    Igor string with the current type
/// @param  requiredType  required type
/// @param  errorMsg      exception message on error
void CheckStructType(Handle structType, const std::string &requiredType, const std::string &errorMsg);

/// @brief Checks structure version field
///
/// @param  version         from Igor with the current version
/// @param  requiredVersion required version
/// @param  errorMsg        exception message on error
void CheckStructVersion(double version, int requiredVersion, const std::string &errorMsg);

/// @brief Return the size in bytes of the given Igor Pro wave types
///
/// The returned size is zero for non-numeric wave types
std::size_t GetWaveElementSize(int dataType);

/// @brief Set all elements of the given wave to zero or "" for text waves
void WaveClear(waveHndl wv);

/// @brief Sets all elements of a text wave to ""
void ClearTextWave(waveHndl wv);

/// @brief Set dimension labels on wave
///
/// @param	h	        Wave to set dimension labels on
/// @param	Dimension	Dimension to set labels on
/// @param	dimLabels	vector of labels to set
///
/// dimLabels[k] will be assigned to index k of the wave
void SetDimensionLabels(waveHndl h, int Dimension, const std::vector<std::string> &dimLabels);

/// Write stringVector to the text wave waveHandle, using memcpy this is quite
/// fast
void StringVectorToTextWave(const std::vector<std::string> &stringVector, waveHndl waveHandle);

/// Throws an IgorException if condition is not met with msg
void ASSERT(bool cond, const std::string &errorMsg);

/// Sets all entries in a text wave to a given string
void SetTextWave(waveHndl w, const std::string &value);

/// Sets all entries of a numerical wave to given value
template <typename T,
          typename std::enable_if_t<
              std::is_same<float, T>::value || std::is_same<double, T>::value || std::is_integral<T>::value, int> = 0>
void SetWaveNum(waveHndl w, T value)
{
  T *p        = static_cast<T *>(WaveData(w));
  size_t size = To<size_t>(WaveMemorySize(w, 2)) / sizeof(T);
  for(size_t s = 0; s < size; s++)
  {
    *p = value;
    p++; // NOLINT
  }
}

template <typename T>
T ParseString(const std::string & /*str*/, const std::string & /*errorMsg*/)
{
  throw IgorException(ERR_CONVERT, "Type not supported by ParseString");
}

template <>
double ParseString<double>(const std::string &str, const std::string &errorMsg);

template <>
float ParseString<float>(const std::string &str, const std::string &errorMsg);

template <>
uint64_t ParseString<uint64_t>(const std::string &str, const std::string &errorMsg);

template <>
uint32_t ParseString<uint32_t>(const std::string &str, const std::string &errorMsg);

template <>
int32_t ParseString<int32_t>(const std::string &str, const std::string &errorMsg);

template <>
int64_t ParseString<int64_t>(const std::string &str, const std::string &errorMsg);

#ifdef MACIGOR64

template <>
size_t ParseString<size_t>(const std::string &str, const std::string &errorMsg);

#endif

/// @brief Return the stack trace of Igor Pro functions
std::string GetStackTrace();

/// @brief Convenience wrapper for SetOperationStrVar
void SetOperationReturn(const std::string &name, const std::string &value);

/// @brief Convenience wrapper for SetOperationNumVar
void SetOperationReturn(const std::string &name, double value);

/// @brief Wrapper for XOPCommand3
std::string ExecuteCommand(const std::string &cmd, OutputMode mode = OutputMode::Silent);

/// @brief Return XOP version info as Key/value pairs
StrStrMap GetVersionInfo(const std::string &xopName);

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
/// @return Returns the flag status, true for /Z enabled, false for disabled or
/// not present
template <typename T>
bool ReadZFlag(T *params)
{
  return ReadZFlagImpl(params, HasZ<T>());
}

template <typename T>
bool ReadZFlagImpl(T * /*params*/, std::false_type /*unused*/)
{
  return false;
}

template <typename T>
bool ReadZFlagImpl(T *params, std::true_type /*unused*/)
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

/// @brief Uses the same logic as for the Z flag above
template <typename T, typename = int>
struct HasQ : std::false_type
{
};

template <typename T>
struct HasQ<T, decltype((void) T::QFlagEncountered, 0)> : std::true_type
{
};

template <typename T>
bool ReadQFlag(T *params)
{
  return ReadQFlagImpl(params, HasQ<T>());
}

template <typename T>
bool ReadQFlagImpl(T * /*params*/, std::false_type /*unused*/)
{
  return false;
}

template <typename T>
bool ReadQFlagImpl(T *params, std::true_type /*unused*/)
{
  if(!params->QFlagEncountered)
  {
    // No flag encountered.
    return false;
  }

  // Flag encountered.
  if(!params->QFlagParamsSet[0])
  {
    // But no parameter not set, equal to true.
    return true;
  }

  // Evaluate set value
  return ConvertFromDouble<bool>(params->QIn, "/Q flag value must be convertible to 0/1.");
}

#define BEGIN_OUTER_CATCH                                                                                              \
  /* BEGIN: NO-EXCEPTIONS-ALLOWED */                                                                                   \
                                                                                                                       \
  int err    = EXIT_SUCCESS;                                                                                           \
  bool ZFlag = false;                                                                                                  \
  bool QFlag = false;                                                                                                  \
                                                                                                                       \
  /* END: NO-EXCEPTIONS-ALLOWED */                                                                                     \
                                                                                                                       \
  try                                                                                                                  \
  {                                                                                                                    \
    ZFlag = ReadZFlag(p);                                                                                              \
    QFlag = ReadQFlag(p);                                                                                              \
    XOPHandler().SetQuietMode(QFlag);

#define END_OUTER_CATCH                                                                                                \
  }                                                                                                                    \
  catch(const IgorException &e)                                                                                        \
  {                                                                                                                    \
    err = HandleException(e, QFlag);                                                                                   \
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

/// @brief Handles operations using datafolderAndName={name, type} parameters.
/// @param[in] FlagParamsSet array from the operation structure from the
/// datafolderAndName parameter block
/// @param[in] dfAndName datafolderAndName structure from the operation
/// parameter block
/// @param[in] calledFromFunction flag if the operation was called from a
/// function from the operation strucutre
/// @param[in] dims vector with wave dimension sizes
/// @param[in] checkWaveProperties function that allows to check properties of
/// existing wave, called only for previously existing waves
///            e.g. check if only certain types are allowed.
/// @param[in] typeGetter function that returns the type for the wave creation
/// or overwrite
/// @param[in] setWaveContents function that sets the contents of the final wave
void HandleDestWave(int FlagParamsSet, const DataFolderAndName &dfAndName, const int freeFlagEncountered,
                    std::vector<CountInt> dims, const std::function<void(waveHndl)> &checkWaveProperties,
                    const std::function<int(waveHndl)> &typeGetter,
                    const std::function<void(waveHndl)> &setWaveContents);

/// @brief Retrieves dimensions of a wave.
/// @param[in] w wave handle of wave
/// @return vector with size of each dimension. The vector has size
/// MAX_DIMENSIONS + 1 and the first unused dimension is set to size 0
std::vector<CountInt> GetWaveDimension(waveHndl w);

/// @brief Retrieves dimensions and number of dimensions of a wave.
/// @param[in] w wave handle of wave
/// @param[out] numDims number of dimensions
/// @return vector with size of each dimension. The vector has size
/// MAX_DIMENSIONS + 1.
std::vector<CountInt> GetWaveDimension(waveHndl w, int &numDims);

/// @brief Compares expected wave dimensions vs actual wave dimensions. Throws
/// an IgorException when not equal.
/// @param[in] w wave handle of wave
/// @param[in] expectedDims vector with expected dimension sizes, only up to
/// MAX_DIMENSIONS entries will be considered
/// @param[in] errorMsg string containing a custom error message
void CheckWaveDimension(waveHndl w, const std::vector<CountInt> &expectedDims, const std::string &errorMsg);

/// @brief Sets an element of a wave. Supports numeric, text, Int64, UInt64
/// waves (Not WaveRef and DatafolderRef)
///        The template type must fit the wave type.
/// @param[in] w handle of target wave
/// @param[in] dims IndexInt vector of MAX_DIMENSIONS size that points to the
/// elements location within the wave.
/// @param[in] value value that should be written to the location given by dims
/// in w
template <typename T, typename std::enable_if_t<std::is_same<std::string, T>::value || std::is_same<float, T>::value ||
                                                    std::is_same<double, T>::value || std::is_integral<T>::value,
                                                int> = 0>
void SetWaveElement(waveHndl w, std::vector<IndexInt> &dims, const T &value)
{
  int err;

  if(!w)
  {
    throw IgorException(NOWAV);
  }

  ASSERT(dims.size() == MAX_DIMENSIONS, "dims vector for SetWaveElement must have size MAX_DIMENSIONS");

  const int type = WaveType(w);
  if(((type == NT_FP32) && std::is_same<float, T>::value) || ((type == NT_FP64) && std::is_same<double, T>::value) ||
     ((type & NT_I8) && std::is_integral<T>::value && (sizeof(T) == sizeof(int8_t))) ||
     ((type & NT_I16) && std::is_integral<T>::value && (sizeof(T) == sizeof(int16_t))) ||
     ((type & NT_I32) && std::is_integral<T>::value && (sizeof(T) == sizeof(int32_t))))

  {
    std::array<double, 2> v;
    v[0] = static_cast<double>(value);
    if(err = MDSetNumericWavePointValue(w, dims.data(), v.data()))
    {
      throw IgorException(err, "Error writing values to numeric wave");
    }
  }
  else if((type == NT_I64) && std::is_integral<T>::value && (sizeof(T) == sizeof(int64_t)) && std::is_signed<T>::value)
  {
    std::array<SInt64, 2> v;
    v[0] = static_cast<SInt64>(value);
    if(err = MDSetNumericWavePointValueSInt64(w, dims.data(), v.data()))
    {
      throw IgorException(err, "Error writing values to INT64 wave");
    }
  }
  else if((type == (NT_I64 | NT_UNSIGNED)) && std::is_integral<T>::value && (sizeof(T) == sizeof(uint64_t)) &&
          std::is_unsigned<T>::value)
  {
    std::array<UInt64, 2> v;
    v[0] = static_cast<UInt64>(value);
    if(err = MDSetNumericWavePointValueUInt64(w, dims.data(), v.data()))
    {
      throw IgorException(err, "Error writing values to UINT64 wave");
    }
  }
  else
  {
    throw IgorException(ERR_INVALID_TYPE, "XOP Bug: Unsupported wave type or wave type and input "
                                          "type are not the same.");
  }
}

template <>
void SetWaveElement<std::string>(waveHndl w, std::vector<IndexInt> &dims, const std::string &value);

/// @brief Gets an element of a wave. Supports numeric, text, Int64, UInt64
/// waves (Not WaveRef and DatafolderRef)
///        The template type must fit the wave type.
/// @param[in] w handle of target wave
/// @param[in] dims IndexInt vector of MAX_DIMENSIONS size that points to the
/// elements location within the wave.
/// @return value value that is read at the location given by dims in w
template <typename T, typename std::enable_if_t<std::is_same<std::string, T>::value || std::is_same<float, T>::value ||
                                                    std::is_same<double, T>::value || std::is_integral<T>::value,
                                                int> = 0>
T GetWaveElement(waveHndl w, std::vector<IndexInt> &dims)
{
  int err;

  if(!w)
  {
    throw IgorException(NOWAV);
  }

  ASSERT(dims.size() == MAX_DIMENSIONS, "dims vector for SetWaveElement must have size MAX_DIMENSIONS");

  const int type = WaveType(w);
  if(((type == NT_FP32) && std::is_same<float, T>::value) || ((type == NT_FP64) && std::is_same<double, T>::value) ||
     ((type & NT_I8) && std::is_integral<T>::value && (sizeof(T) == sizeof(int8_t))) ||
     ((type & NT_I16) && std::is_integral<T>::value && (sizeof(T) == sizeof(int16_t))) ||
     ((type & NT_I32) && std::is_integral<T>::value && (sizeof(T) == sizeof(int32_t))))

  {
    std::array<double, 2> v;
    if(err = MDGetNumericWavePointValue(w, dims.data(), v.data()))
    {
      throw IgorException(err, "Error reading values from numeric wave");
    }
    return static_cast<T>(v[0]);
  }
  if((type == NT_I64) && std::is_integral<T>::value && (sizeof(T) == sizeof(int64_t)) && std::is_signed<T>::value)
  {
    std::array<SInt64, 2> v;
    if(err = MDGetNumericWavePointValueSInt64(w, dims.data(), v.data()))
    {
      throw IgorException(err, "Error reading values from INT64 wave");
    }
    return static_cast<T>(v[0]);
  }
  if((type == (NT_I64 | NT_UNSIGNED)) && std::is_integral<T>::value && (sizeof(T) == sizeof(uint64_t)) &&
     std::is_unsigned<T>::value)
  {
    std::array<UInt64, 2> v;
    if(err = MDGetNumericWavePointValueUInt64(w, dims.data(), v.data()))
    {
      throw IgorException(err, "Error reading values from UINT64 wave");
    }
    return static_cast<T>(v[0]);
  }
  throw IgorException(ERR_INVALID_TYPE, "XOP Bug: Unsupported wave type or wave type and "
                                        "template type are not the same.");
}

template <>
std::string GetWaveElement<std::string>(waveHndl w, std::vector<IndexInt> &dims);

/// Convenience defines for using IgorToCType<> and CToIgorType<>
/// @{
#define NT_UI8 0x48
#define NT_UI16 0x50
#define NT_UI32 0x60
#define NT_UI64 0xC0
/// @}

/// @brief Convert the igor types (NT_FP32, ...) to the corresponding C/C++
/// types
///
/// @tparam T Igor type constant (non-type parameter)
template <int T>
struct IgorToCType;

// no specializations for complex/WAVE/DFREF waves

template <>
struct IgorToCType<NT_FP32>
{
  using type = float;
};

template <>
struct IgorToCType<NT_FP64>
{
  using type = double;
};

template <>
struct IgorToCType<NT_I8>
{
  using type = int8_t;
};

template <>
struct IgorToCType<NT_UI8>
{
  using type = uint8_t;
};

template <>
struct IgorToCType<NT_I16>
{
  using type = int16_t;
};

template <>
struct IgorToCType<NT_UI16>
{
  using type = uint16_t;
};

template <>
struct IgorToCType<NT_I32>
{
  using type = int32_t;
};

template <>
struct IgorToCType<NT_UI32>
{
  using type = uint32_t;
};

template <>
struct IgorToCType<NT_I64>
{
  using type = SInt64;
};

template <>
struct IgorToCType<NT_UI64>
{
  using type = UInt64;
};

template <>
struct IgorToCType<TEXT_WAVE_TYPE>
{
  using type = std::string;
};

/// @brief Convert the types from IgorToCType back to Igor Pro constants
///
/// @tparam T C/C++ type from IgorToCType<>
template <typename T>
struct CToIgorType;

// no specializations for complex/WAVE/DFREF waves

template <>
struct CToIgorType<typename IgorToCType<NT_FP32>::type>
{
  const static int type = NT_FP32;
};

template <>
struct CToIgorType<typename IgorToCType<NT_FP64>::type>
{
  const static int type = NT_FP64;
};

template <>
struct CToIgorType<typename IgorToCType<NT_I8>::type>
{
  const static int type = NT_I8;
};

template <>
struct CToIgorType<typename IgorToCType<NT_UI8>::type>
{
  const static int type = NT_UI8;
};

template <>
struct CToIgorType<typename IgorToCType<NT_I16>::type>
{
  const static int type = NT_I16;
};

template <>
struct CToIgorType<typename IgorToCType<NT_UI16>::type>
{
  const static int type = NT_UI16;
};

template <>
struct CToIgorType<typename IgorToCType<NT_I32>::type>
{
  const static int type = NT_I32;
};

template <>
struct CToIgorType<typename IgorToCType<NT_UI32>::type>
{
  const static int type = NT_UI32;
};

template <>
struct CToIgorType<typename IgorToCType<NT_I64>::type>
{
  const static int type = NT_I64;
};

template <>
struct CToIgorType<typename IgorToCType<NT_UI64>::type>
{
  const static int type = NT_UI64;
};

template <>
struct CToIgorType<typename IgorToCType<TEXT_WAVE_TYPE>::type>
{
  const static int type = TEXT_WAVE_TYPE;
};
