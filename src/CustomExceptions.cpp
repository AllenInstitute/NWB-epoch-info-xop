#include "CustomExceptions.h"

#include <utility>

#include "Helpers.h"

IgorException::IgorException() : m_errorCode(EXIT_FAILURE)
{
}

IgorException::IgorException(int errorCode) : m_errorCode(errorCode)
{
}

IgorException::IgorException(int errorCode, std::string errorMsg)
    : m_errorCode(errorCode), m_errorMsg(std::move(errorMsg))
{
}

const char *IgorException::what() const noexcept
{
  return m_errorMsg.c_str();
}

int IgorException::GetErrorCode() const noexcept
{
  return m_errorCode;
}

int IgorException::HandleException() const
{
  return HandleException(false);
}

int IgorException::HandleException(bool quiet) const
{
  if(!m_errorMsg.empty() && !quiet)
  {
    OutputToHistory(m_errorMsg);
  }

  return m_errorCode;
}

int HandleException(const IgorException &e)
{
  return e.HandleException();
}

int HandleException(const IgorException &e, bool quiet)
{
  return e.HandleException(quiet);
}

int HandleException(const std::exception &e)
{
  /// No Igor error code associated with this.
  OutputToHistory("BUG: Encountered C++ exception: {}"_format(e));

  OutputToHistory("Stacktrace: {}"_format(GetStackTrace()));

  return CPP_EXCEPTION;
}
