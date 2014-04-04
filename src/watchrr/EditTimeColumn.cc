
#include "EditTimeColumn.hh"

#include <QtGui/QBrush>
#include <boost/bind.hpp>

#define MILOGGER_CATEGORY "kvhqc.EditTimeColumn"
#include "util/HqcLogging.hh"

EditTimeColumn::EditTimeColumn(DataColumn_p dc)
  : mDC(dc)
{
  connect(dc.get(), SIGNAL(columnChanged(ObsColumn_p)),
      this, SIGNAL(onColumnChanged(ObsColumn_p)));
  //mDC->columnChanged.connect(boost::bind(&EditTimeColumn::onColumnChanged, this, _1, _2));
}

EditTimeColumn::~EditTimeColumn()
{
  mDC->columnChanged.disconnect(boost::bind(&EditTimeColumn::onColumnChanged, this, _1, _2));
}

Qt::ItemFlags EditTimeColumn::flags(const timeutil::ptime& time) const
{
  METLIBS_LOG_SCOPE();
  Qt::ItemFlags of = mDC->flags(time), f = of;
  const int removeFlags = Qt::ItemIsSelectable|Qt::ItemIsEditable;
  if( (f & removeFlags) and not mEditableTime.contains(time) ) {
    f &= ~removeFlags;
  }
  METLIBS_LOG_DEBUG(LOGVAL(time) << LOGVAL(of) << LOGVAL(f) << LOGVAL(mEditableTime.t0()));
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

QVariant EditTimeColumn::headerData(Qt::Orientation orientation, int role) const
{
  return mDC->headerData(orientation, role);
}

void EditTimeColumn::setEditableTime(const TimeSpan& et)
{
  mEditableTime = et;
}

const boost::posix_time::time_duration& EditTimeColumn::timeOffset() const
{
  return mDC->timeOffset();
}

void EditTimeColumn::onColumnChanged(const timeutil::ptime& time, ObsColumn_p column)
{
  if (column == mDC)
    columnChanged(time, shared_from_this());
}
