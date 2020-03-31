#pragma once

#include <exception>
#include <string>

class IgorException : public std::exception
{
public:
  /// Constructors
  // Mark default constructor as deprecated
  // - Allows use of default constructor when a custom error code hasn't been
  // implemented
  // - Compiler warning allows us to find usages later
  [[deprecated("Using default error code.  You should replace this "
               "with a custom error code")]] explicit IgorException();
  explicit IgorException(int errorCode);
  explicit IgorException(int errorCode, std::string errorMsg);

  const char *what() const noexcept override;
  int GetErrorCode() const noexcept;

  /// Displays the exception if required; gets the return code.
  int HandleException() const;
  /// Displays the exception if not quiet; gets the return code.
  int HandleException(bool quiet) const;

private:
  const int m_errorCode;
  const std::string m_errorMsg;
};

int HandleException(const std::exception &e);
int HandleException(const IgorException &e);

/// @brief Handler allows to suppress output to Igor History when quiet = true
int HandleException(const IgorException &e, bool quiet);
