
#ifndef HQC_LOGGING_HH
#define HQC_LOGGING_HH 1

#include <Qt/qglobal.h>
#include <iosfwd>

QT_BEGIN_NAMESPACE;
class QString;
QT_END_NAMESPACE;

#include <qUtilities/miLoggingQt.h>

extern bool HqcLoggingWarnOrError;

#define HQC_LOG_WARN(x)                                 \
  do { HqcLoggingWarnOrError = true;                    \
    METLIBS_LOG_WARN(x); } while(0)

#define HQC_LOG_ERROR(x)                     \
  do { HqcLoggingWarnOrError = true ;        \
    METLIBS_LOG_ERROR(x); } while(0)

#endif // HQC_LOGGING_HH
