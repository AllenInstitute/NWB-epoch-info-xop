#include "CustomExceptions.h"
#include "Helpers.h"

IgorException::IgorException() : m_errorCode(EXIT_FAILURE)
{
}

IgorException::IgorException(int errorCode) : m_errorCode(errorCode)
{
}

IgorException::IgorException(int errorCode, std::string errorMsg) : m_errorCode(errorCode), m_errorMsg(errorMsg)
{
}

const char *IgorException::what() const noexcept
{
  return m_errorMsg.c_str();
}

int IgorException::HandleException() const
{
  if(!m_errorMsg.empty())
  {
    OutputToHistory(m_errorMsg);
  }

  return m_errorCode;
}

int HandleException(IgorException e)
{
  return e.HandleException();
}

int HandleException(std::exception e)
{
  /// No Igor error code associated with this.
  OutputToHistory("BUG: Encountered C++ exception: {}"_format(e));

  OutputToHistory("Stacktrace: {}"_format(GetStackTrace()));

  return CPP_EXCEPTION;
}
