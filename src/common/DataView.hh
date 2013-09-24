
#ifndef DataView_hh
#define DataView_hh

#include "EditAccess.hh"
#include "ModelAccess.hh"
#include "Sensor.hh"
#include "TimeRange.hh"

#include <vector>

class DataView
{
public:
  typedef std::vector<Sensor> Sensors_t;

  DataView();
  virtual ~DataView();

  virtual void setDataAccess(EditAccessPtr eda, ModelAccessPtr mda);
  virtual void setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits);

  virtual void navigateTo(const SensorTime&) = 0;

  boost::signal1<void, SensorTime> signalNavigateTo;

protected:
  virtual void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);

protected:
  EditAccessPtr  mDA;
  ModelAccessPtr mMA;
};

#endif // DataView_hh
