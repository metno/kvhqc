
#include "ModelColumn.hh"

#include "ModelAccess.hh"
#include "SensorHeader.hh"

#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.ModelColumn"
#include "common/ObsLogging.hh"

ModelColumn::ModelColumn(ModelAccess_p ma, const Sensor& sensor, const TimeSpan& time)
  : mBuffer(new ModelBuffer(ma))
  , mSensor(sensor)
  , mHeaderShowStation(true)
  , mCodes(boost::make_shared<Code2Text>())
{
}

ModelColumn::~ModelColumn()
{
}

Qt::ItemFlags ModelColumn::flags(const timeutil::ptime& /*time*/) const
{
  return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}

QVariant ModelColumn::data(const timeutil::ptime& time, int role) const
{
  ModelData_p mdl = get(time);
  if (not mdl)
    return QVariant();

  const float value = mdl->value();
  if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
    return mCodes->asTip(value);
  } else if (role == Qt::DisplayRole or role == Qt::EditRole) {
    return mCodes->asText(value);
  } else if (role == Qt::TextAlignmentRole) {
    return Qt::AlignRight;
  }
  return QVariant();
}

bool ModelColumn::setData(const timeutil::ptime& /*time*/, const QVariant& /*value*/, int /*role*/)
{
  return false;
}

QVariant ModelColumn::headerData(Qt::Orientation orientation, int role) const
{
  SensorHeader sh(mSensor, mHeaderShowStation ? SensorHeader::ALWAYS : SensorHeader::TOOLTIP,
      SensorHeader::ALWAYS, mTimeOffset.hours());
  return sh.modelHeader(orientation, role);
}

void ModelColumn::setTimeOffset(const boost::posix_time::time_duration& timeOffset)
{
  mTimeOffset = timeOffset;
}

void ModelColumn::setCodes(Code2TextCPtr codes)
{
  mCodes = codes;
}

void ModelColumn::bufferReceived(const ModelData_pv& mdata)
{
  for (ModelData_pv::const_iterator it = mdata.begin(); it != mdata.end(); ++it) {
    const SensorTime& st = (*it)->sensorTime();
    Q_EMIT columnChanged(st.time - mTimeOffset, shared_from_this());
  }
}

ModelData_p ModelColumn::get(const timeutil::ptime& time) const
{
  return mBuffer->get(SensorTime(mSensor, time + mTimeOffset));
}
