/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/


#include "Milog4cpp.hh"

#define MILOGGER_CATEGORY "Milog4cpp"
#include <miLogger/miLogging.h>

namespace {

const std::string s_MILOGGER_CATEGORY = MILOGGER_CATEGORY;

struct StringLoggerTag
{
  std::string tag;
  ::milogger::LoggerTag logger;
  StringLoggerTag(const std::string& t)
      : tag(t)
      , logger(tag.c_str())
  {
  }
};

} // namespace

Milog4cppLayout::Milog4cppLayout()
{
}

Milog4cppLayout::~Milog4cppLayout()
{
}

std::string Milog4cppLayout::formatMessage(const std::string &msg, milog::LogLevel ll, const std::string &context)
{
  milogger::Severity ml;
  switch (ll) {
  case milog::FATAL:
    ml = milogger::FATAL;
    break;
  case milog::ERROR:
    ml = milogger::ERROR;
    break;
  case milog::WARN:
    ml = milogger::WARN;
    break;
  case milog::INFO:
    ml = milogger::INFO;
    break;
  case milog::DEBUG:
  case milog::DEBUG1:
  case milog::DEBUG2:
  case milog::DEBUG3:
  case milog::DEBUG4:
  case milog::DEBUG5:
  case milog::DEBUG6:
    ml = milogger::DEBUG;
    break;
  default:
    ml = milogger::VERBOSE;
    break;
  }

  const std::string& cat = (not context.empty()) ? context : s_MILOGGER_CATEGORY;
  loggers_t::iterator it = loggers.find(cat);
  if (it == loggers.end())
    it = loggers.insert(std::make_pair(cat, std::make_shared<StringLoggerTag>(cat))).first;

  ::milogger::LoggerTag& tag = it->second->logger;
  if (milogger::RecordPtr record = tag.createRecord(ml)) {
    record->stream() << msg;
    tag.submitRecord(record);
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
