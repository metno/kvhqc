
#ifndef TimeSeriesAdd_hh
#define TimeSeriesAdd_hh 1

#include "TimeSeriesView.hh"
#include "common/Sensor.hh"

#include <QtGui/QDialog>

class SensorChooser;
namespace Ui {
class TimeSeriesAdd;
}

class TimeSeriesAdd : public QDialog
{ Q_OBJECT;
public:
  TimeSeriesAdd(QWidget* parent=0);
  ~TimeSeriesAdd();

  void init(const SensorTime& st);

  Sensor selectedSensor() const;

private Q_SLOTS:
  void slotValidSensor(bool);

private:
  std::auto_ptr<Ui::TimeSeriesAdd> ui;
  std::auto_ptr<SensorChooser> mSensorChooser;
};

#endif // TimeSeriesAdd_hh
