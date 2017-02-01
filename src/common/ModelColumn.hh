
#ifndef MODELCOLUMN_HH
#define MODELCOLUMN_HH 1

#include "Code2Text.hh"
#include "ModelBuffer.hh"
#include "ModelData.hh"
#include "ObsColumn.hh"

class ModelAccess;
HQC_TYPEDEF_P(ModelAccess);

class ModelColumn : public ObsColumn
{
public:
  ModelColumn(ModelAccess_p ma, const Sensor& sensor, const TimeSpan& time);
  ~ModelColumn();

  void setHeaderShowStation(bool show)
    { mHeaderShowStation = show; }

  Qt::ItemFlags flags(const timeutil::ptime& time) const;
  QVariant data(const timeutil::ptime& time, int role) const;
  bool setData(const timeutil::ptime& time, const QVariant& value, int role);
  QVariant headerData(Qt::Orientation orientation, int role) const;

  const boost::posix_time::time_duration& timeOffset() const
    { return mTimeOffset; }

  void setTimeOffset(const boost::posix_time::time_duration& timeOffset);

  const Sensor& sensor() const
    { return mSensor; }

  int type() const
    { return ObsColumn::MODEL; }

  void setCodes(Code2TextCPtr codes);

private Q_SLOTS:
  void bufferReceived(const ModelData_pv&);

private:
  ModelData_p get(const timeutil::ptime& time) const;

private:
  std::unique_ptr<ModelBuffer> mBuffer;

  Sensor mSensor;
  bool mHeaderShowStation;
  boost::posix_time::time_duration mTimeOffset;

  Code2TextCPtr mCodes;
};

HQC_TYPEDEF_P(ModelColumn);

#endif // MODELCOLUMN_HH
