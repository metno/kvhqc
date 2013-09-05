
#ifndef OBSCOLUMN_HH
#define OBSCOLUMN_HH 1

#include "EditData.hh"
#include "ObsAccess.hh"
#include "timeutil.hh"
#include <QtCore/QAbstractTableModel>

class ObsColumn {
public:
    enum ValueType { Numerical=1, TextCode=2, Text=4 };
    enum { ValueTypeRole = Qt::UserRole, TextCodesRole, TextCodeExplanationsRole };

    ObsColumn() { }
    virtual ~ObsColumn() { }

    virtual Qt::ItemFlags flags(const timeutil::ptime& time) const;
    virtual QVariant data(const timeutil::ptime& time, int role) const = 0;
    virtual bool setData(const timeutil::ptime& time, const QVariant& value, int role);
    virtual QVariant headerData(Qt::Orientation orientation, int role) const = 0;

    virtual const boost::posix_time::time_duration& timeOffset() const = 0;
    virtual Sensor sensor() const;

    boost::signal2<void, timeutil::ptime, ObsColumn*> columnChanged;

protected:
    boost::posix_time::time_duration mTimeOffset;
};

typedef boost::shared_ptr<ObsColumn> ObsColumnPtr;

#endif // OBSCOLUMN_HH
