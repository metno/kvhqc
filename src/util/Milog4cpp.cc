
#include "Milog4cpp.hh"

#define MILOGGER_CATEGORY "Milog4cpp"
#include <miLogger/miLogging.h>

Milog4cppLayout::Milog4cppLayout()
{
}

Milog4cppLayout::~Milog4cppLayout()
{
}

std::string Milog4cppLayout::formatMessage(const std::string &msg, milog::LogLevel ll, const std::string &context)
{
  milogger::detail::PriorityLevel ml;
  switch (ll) {
  case milog::FATAL: ml = milogger::detail::FATAL;  break;
  case milog::ERROR: ml = milogger::detail::ERROR;  break;
  case milog::WARN:  ml = milogger::detail::WARN;  break;
  case milog::INFO:  ml = milogger::detail::INFO;  break;
  case milog::DEBUG:
  case milog::DEBUG1:
  case milog::DEBUG2:
  case milog::DEBUG3:
  case milog::DEBUG4:
  case milog::DEBUG5:
  case milog::DEBUG6:
    ml = milogger::detail::DEBUG; break;
  default:
    ml = milogger::detail::VERBOSE; break;
  }

  ::milogger::detail::Category milogger((not context.empty()) ? context : MILOGGER_CATEGORY);
  if (milogger.isLoggingEnabled(ml)) {
    std::ostringstream logmessagestream;
    logmessagestream << msg;
    milogger.log(ml, logmessagestream.str());
  }

  return "";
}

// ================================================================================

Milog4cppStream::Milog4cppStream()
  : milog::LogStream(new Milog4cppLayout, milog::DEBUG6)
{
}

Milog4cppStream::~Milog4cppStream()
{
}

void Milog4cppStream::write(const std::string& /*message*/)
{
}
