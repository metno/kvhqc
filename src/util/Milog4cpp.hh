
#ifndef UTIL_MILOG4CPP_HH
#define UTIL_MILOG4CPP_HH 1

#include <milog/Layout.h>
#include <milog/LogManager.h>
#include <milog/LogStream.h>

#include "miLogger/miLoggingLogger.h"

class Milog4cppLayout : public milog::Layout {
public:
  Milog4cppLayout();
  ~Milog4cppLayout();

  std::string formatMessage(const std::string &msg, milog::LogLevel ll, const std::string &context);

private:
  struct StringLoggerTag {
    std::string tag;
    ::milogger::LoggerTag logger;
    StringLoggerTag(const std::string& t)
      : tag(t), logger(tag.c_str()) { }
  };
  typedef std::shared_ptr<StringLoggerTag> StringLoggerTag_p;

  typedef std::map<std::string, StringLoggerTag_p> loggers_t;
  loggers_t loggers;
};

// ================================================================================

class Milog4cppStream : public milog::LogStream
{
public:
  Milog4cppStream();
  ~Milog4cppStream();

protected:
  void write(const std::string &message);	
};

#endif // UTIL_MILOG4CPP_HH
