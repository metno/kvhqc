
#define MILOGGER_CATEGORY "kvhqc.HqcLogging" // only to get rid of warning
#include "HqcLogging.hh"

#include <QtCore/QString>

std::ostream& operator<<(std::ostream& out, const QString& qs)
{
    out << qs.toStdString();
    return out;
}

bool HqcLoggingWarnOrError = false;
