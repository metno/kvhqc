
#ifndef SensorChooser_hh
#define SensorChooser_hh 1

#include "common/Sensor.hh"
#include "common/ParamIdModel.hh"
#include "common/TimeSpan.hh"
#include "common/TypeIdModel.hh"

#include <QtCore/QObject>

#include <set>

class QComboBox;
class QLineEdit;
class QSpinBox;

class SensorChooser : public QObject
{ Q_OBJECT;
public:
  SensorChooser(QLineEdit* station, QComboBox* param, QComboBox* type, QComboBox* level, QSpinBox* sensorNr, QObject* parent = 0);
  ~SensorChooser();

  void setSensor(const Sensor& sensor);
  void setTimeSpan(const TimeSpan& time);

  bool isValid();
  Sensor getSensor();
  
Q_SIGNALS:
  void valid(bool);

private Q_SLOTS:
  void onStationEdited(const QString&);
  void onParameterSelected(int);
  void onTypeSelected(int);
  void onLevelSelected(int);

private:
  void setLevels(const std::set<int>& levels);
  void setMaxSensor(int maxSensor);

  int getStationId() const;
  void setStationId(int stationId);
  int getParamId() const;
  void setParamId(int paramId);
  int getTypeId() const;
  void setTypeId(int typeId);
  int getLevel() const;
  void setLevel(int level);
  int getSensorNr() const;
  void setSensorNr(int snr);

  ParamIdModel* paramModel() const;
  TypeIdModel* typeModel() const;

private:
  QLineEdit* mStation;
  QComboBox* mParam;
  QComboBox* mType;
  QComboBox* mLevel;
  QSpinBox* mSensorNr;

  TimeSpan mTime;
};

#endif // SensorChooser_hh
