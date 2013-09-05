
#ifndef DataListAddColumn_hh
#define DataListAddColumn_hh 1

#include "AutoDataList.hh"

#include <QtGui/QDialog>

class Sensor;
class SensorTime;
namespace Ui {
class DataListAddColumn;
}

class DataListAddColumn : public QDialog
{ Q_OBJECT;
public:
  DataListAddColumn(QWidget* parent=0);
  ~DataListAddColumn();

  void init(const SensorTime& st);

  Sensor selectedSensor() const;
  AutoDataList::ColumnType selectedColumnType() const;
  int selectedTimeOffset() const;

private Q_SLOTS:
  void onStationEdited();
  void onParameterSelected(int);

private:
  int getStationId() const;
  int getParamId() const;
  int getTypeId() const;

private:
  std::auto_ptr<Ui::DataListAddColumn> ui;
};

#endif // DataListAddColumn_hh
