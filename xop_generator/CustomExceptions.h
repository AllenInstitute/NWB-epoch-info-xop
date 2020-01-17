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
               "with a custom error code")]] IgorException();
  IgorException(int errorCode);
  IgorException(int errorCode, std::string errorMessage);

  const char *what() const noexcept override;

  /// Displays the exception if required; gets the return code.
  int HandleException() const;

private:
  const int m_errorCode;
  const std::string m_errorMsg;
};

int HandleException(std::exception e);
int HandleException(IgorException e);
