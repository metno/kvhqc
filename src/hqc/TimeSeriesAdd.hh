
#ifndef TimeSeriesAdd_hh
#define TimeSeriesAdd_hh 1

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
  std::unique_ptr<Ui::TimeSeriesAdd> ui;
  std::unique_ptr<SensorChooser> mSensorChooser;
};

#endif // TimeSeriesAdd_hh
