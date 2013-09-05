
#ifndef TimeSeriesAdd_hh
#define TimeSeriesAdd_hh 1

#include "TimeSeriesView.hh"
#include "Sensor.hh"

#include <QtGui/QDialog>

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
  void onStationEdited();
  void onParameterSelected(int);

private:
  int getStationId() const;
  int getParamId() const;
  int getTypeId() const;

private:
  std::auto_ptr<Ui::TimeSeriesAdd> ui;
};

#endif // TimeSeriesAdd_hh
