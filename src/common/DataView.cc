
#include "DataView.hh"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.DataView"
#include "util/HqcLogging.hh"

DataView::DataView()
{
}

DataView::~DataView()
{
  if (mDA)
    mDA->obsDataChanged.disconnect(boost::bind(&DataView::onDataChanged, this, _1, _2));
}

void DataView::setDataAccess(EditAccessPtr eda, ModelAccessPtr mda)
{
  METLIBS_LOG_SCOPE();
  if (eda != mDA) {
    if (mDA)
      mDA->obsDataChanged.disconnect(boost::bind(&DataView::onDataChanged, this, _1, _2));
    mDA = eda;
    if (mDA)
      mDA->obsDataChanged.connect(boost::bind(&DataView::onDataChanged, this, _1, _2));
  }
  mMA = mda;
  if (not mMA)
    METLIBS_LOG_DEBUG("no model access");
}

void DataView::setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits)
{
  METLIBS_LOG_SCOPE();
}

void DataView::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs)
{
  METLIBS_LOG_SCOPE();
  // TODO
}
