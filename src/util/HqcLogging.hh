
#ifndef HQC_LOGGING_HH
#define HQC_LOGGING_HH 1

#include <Qt/qglobal.h>
#include <iosfwd>

QT_BEGIN_NAMESPACE;
class QString;
QT_END_NAMESPACE;

std::ostream& operator<<(std::ostream& out, const QString& qs);

#include <miLogger/miLogging.h>

extern bool HqcLoggingWarnOrError;

#define HQC_LOG_WARN(x)                                 \
  do { HqcLoggingWarnOrError = true;                    \
    METLIBS_LOG_WARN(x); } while(0)

#define HQC_LOG_ERROR(x)                     \
  do { HqcLoggingWarnOrError = true ;        \
    METLIBS_LOG_ERROR(x); } while(0)

#define LOGTYPE(type)                           \
  '{' << typeid(type).name() << '}'

#define LOGMYTYPE()                             \
  LOGTYPE(*this)

#define LOG_CONSTRUCT_COUNTER \
  namespace { int count_contruct = 0; }

#define LOG_CONSTRUCT() \
  do { count_contruct += 1;                                             \
    METLIBS_LOG_DEBUG("this=" << this << " count=" << count_contruct);  \
  } while(0)
  
#define LOG_DESTRUCT() \
  do { count_contruct -= 1;                                             \
    METLIBS_LOG_DEBUG("this=" << this << " count=" << count_contruct);  \
  } while(0)

#endif // HQC_LOGGING_HH
