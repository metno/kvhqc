
#ifndef DataListAddColumn_hh
#define DataListAddColumn_hh 1

#include "AutoDataList.hh"

#include <QtGui/QDialog>
#include <set>

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
  void onStationEdited(const QString&);
  void onParameterSelected(int);
  void onTypeSelected(int);

private:
  void setLevels(const std::set<int>& levels);
  void setMaxSensor(int maxSensor);

  int getStationId() const;
  int getParamId() const;
  int getTypeId() const;
  int getLevel() const;
  int getSensorNumber() const;
  void resetTimeOffset();

private:
  std::auto_ptr<Ui::DataListAddColumn> ui;
};

#endif // DataListAddColumn_hh
