
#include "EditTimeColumn.hh"

#include <QtGui/QBrush>
#include <boost/bind.hpp>

#define NDEBUG
#include "debug.hh"

EditTimeColumn::EditTimeColumn(DataColumnPtr dc)
    : mDC(dc)
{
    mDC->columnChanged.connect(boost::bind(&EditTimeColumn::onColumnChanged, this, _1, _2));
}

EditTimeColumn::~EditTimeColumn()
{
    mDC->columnChanged.disconnect(boost::bind(&EditTimeColumn::onColumnChanged, this, _1, _2));
}

Qt::ItemFlags EditTimeColumn::flags(const timeutil::ptime& time) const
{
    LOG_SCOPE();
    Qt::ItemFlags of = mDC->flags(time), f = of;
    const int removeFlags = Qt::ItemIsSelectable|Qt::ItemIsEditable;
    if( (f & removeFlags) and not mEditableTime.contains(time) ) {
        f &= ~removeFlags;
    }
    DBG(DBG1(time) << DBG1(of) << DBG1(f) << DBG1(mEditableTime.t0()));
    return f;
}

QVariant EditTimeColumn::data(const timeutil::ptime& time, int role) const
{
    if (role == Qt::BackgroundRole and not mEditableTime.contains(time))
        return QBrush(Qt::lightGray);
    return mDC->data(time, role);
}

bool EditTimeColumn::setData(const timeutil::ptime& time, const QVariant& value, int role)
{
    if( not mEditableTime.contains(time) )
        return false;
    return mDC->setData(time, value, role);
}

QVariant EditTimeColumn::headerData(int role, bool verticalHeader) const
{
    return mDC->headerData(role, verticalHeader);
}

void EditTimeColumn::setEditableTime(const TimeRange& et)
{
    mEditableTime = et;
}

const boost::posix_time::time_duration& EditTimeColumn::timeOffset() const
{
    return mDC->timeOffset();
}

void EditTimeColumn::onColumnChanged(const timeutil::ptime& time, ObsColumn* column)
{
    if (column == mDC.get())
        columnChanged(time, this);
}
