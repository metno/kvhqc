
#ifndef DataListAddColumn_hh
#define DataListAddColumn_hh 1

#include "AutoDataList.hh"

#include <QtGui/QDialog>
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

  Sensor selectedSensor() const;
  AutoDataList::ColumnType selectedColumnType() const;
  int selectedTimeOffset() const;

private Q_SLOTS:
  void slotValidSensor(bool);

private:
  void resetTimeOffset();

private:
  std::auto_ptr<Ui::DataListAddColumn> ui;
  SensorChooser* mSensorChooser;
};

#endif // DataListAddColumn_hh
