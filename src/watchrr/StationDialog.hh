
#ifndef WATCHRR_STATIONDIALOG_HH
#define WATCHRR_STATIONDIALOG_HH

#include "common/Sensor.hh"
#include "common/TimeSpan.hh"
#include "common/TypeIdModel.hh"

#include <QDialog>

#include <memory>

class TimeSpanControl;
class ObsPgmRequest;
namespace kvalobs {
class kvObsPgm;
}
namespace Ui {
class DialogStation;
}

class StationDialog : public QDialog
{ Q_OBJECT;
public:
  StationDialog(const Sensor& sensor, const TimeSpan& time, QDialog* parent=0);
  StationDialog(QDialog* parent=0);
  virtual ~StationDialog();
                            
  Sensor selectedSensor() const
    { return mSensor; }

  virtual TimeSpan selectedTime() const;

  virtual bool valid() const;

protected:
  virtual int acceptThisObsPgm(const kvalobs::kvObsPgm& op) const;

protected Q_SLOTS:
  virtual void onEditStation();
  virtual void onEditTime();
  virtual void onSelectType(int);
  void onObsPgmDone();

protected:
  void init();
  void updateStationInfoText();
  virtual bool checkStation();
  virtual bool checkType();
  virtual void updateTypeList();
  virtual void enableOk();

protected:
  Sensor mSensor;
  std::unique_ptr<Ui::DialogStation> ui;
  TimeSpanControl* mTimeControl;
  std::unique_ptr<TypeIdModel> mTypesModel;
  ObsPgmRequest* mObsPgmRequest;
};

#endif // WATCHRR_STATIONDIALOG_HH
