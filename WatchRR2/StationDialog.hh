
#ifndef STATIONDIALOG_HH
#define STATIONDIALOG_HH

#include "Sensor.hh"
#include "TimeRange.hh"
#include <QtGui/QDialog>
#include <memory>

class TimeRangeControl;
class TypeIdModel;
namespace kvalobs {
class kvObsPgm;
}
namespace Ui {
class DialogStation;
}

class StationDialog : public QDialog
{ Q_OBJECT;
public:
  StationDialog(const Sensor& sensor, const TimeRange& time, QDialog* parent=0);
  StationDialog(QDialog* parent=0);
  virtual ~StationDialog();
                            
  Sensor selectedSensor() const
    { return mSensor; }

  virtual TimeRange selectedTime() const;

  virtual bool valid() const;

protected:
  virtual int acceptThisObsPgm(const kvalobs::kvObsPgm& op) const;

protected Q_SLOTS:
  virtual void onEditStation();
  virtual void onEditTime();
  virtual void onSelectType(int);

protected:
  void init();
  virtual bool checkStation();
  virtual bool checkType();
  virtual void updateTypeList();
  virtual void enableOk();

protected:
  Sensor mSensor;
  std::auto_ptr<Ui::DialogStation> ui;
  TimeRangeControl* mTimeControl;
  std::auto_ptr<TypeIdModel> mTypesModel;
};

#endif // STATIONDIALOG_HH
