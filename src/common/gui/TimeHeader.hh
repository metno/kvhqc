
#ifndef TIMEHEADER_HH
#define TIMEHEADER_HH 1

#include "util/timeutil.hh"

#include <QtCore/QAbstractTableModel>

class TimeHeader
{
public:
    static QVariant headerData(const timeutil::ptime& time, Qt::Orientation orientation, int role);
};

#endif /* TIMEHEADER_HH */
