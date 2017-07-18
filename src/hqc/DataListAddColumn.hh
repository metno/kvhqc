
#ifndef DataListAddColumn_hh
#define DataListAddColumn_hh 1

#include "AutoDataList.hh"

#include <QDialog>
#include <set>

class Sensor;
class SensorChooser;
class SensorTime;
namespace Ui {
class DataListAddColumn;
}

class DataListAddColumn : public QDialog
{ Q_OBJECT;
public:
  DataListAddColumn(QWidget* parent=0);
  ~DataListAddColumn();

  void setSensor(const Sensor& sensor);

  Sensor selectedSensor() const;
  AutoDataList::ColumnType selectedColumnType() const;
  int selectedTimeOffset() const;

protected:
  virtual void changeEvent(QEvent *event);

private Q_SLOTS:
  void slotValidSensor(bool);

private:
  void resetTimeOffset();

private:
  std::unique_ptr<Ui::DataListAddColumn> ui;
  std::unique_ptr<SensorChooser> mSensorChooser;
};

#endif // DataListAddColumn_hh
