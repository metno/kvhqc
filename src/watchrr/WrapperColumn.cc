
#include "WrapperColumn.hh"

#include <QtGui/QBrush>
#include <boost/bind.hpp>

#define MILOGGER_CATEGORY "kvhqc.WrapperColumn"
#include "util/HqcLogging.hh"

WrapperColumn::WrapperColumn(DataColumn_p dc)
  : mDC(dc)
{
  connect(dc.get(), SIGNAL(columnChanged(const timeutil::ptime&, ObsColumn_p)),
      this, SIGNAL(columnChanged(const timeutil::ptime&, ObsColumn_p)));
  connect(dc.get(), SIGNAL(columnTimesChanged(ObsColumn_p)),
      this, SIGNAL(columnTimesChanged(ObsColumn_p)));
}

WrapperColumn::~WrapperColumn()
{
}
