
#ifndef JumpToObservation_hh
#define JumpToObservation_hh 1

#include "common/ObsAccess.hh"

#include <boost/signals.hpp>

#include <QtGui/QDialog>

class Sensor;
class SensorChooser;
class SensorTime;
namespace Ui {
class JumpToObservation;
}

class JumpToObservation : public QDialog
{ Q_OBJECT;
public:
  JumpToObservation(ObsAccessPtr da, QWidget* parent=0);
  ~JumpToObservation();
  
  void navigateTo(const SensorTime&);

  SensorTime selectedSensorTime() const;
  
  boost::signal1<void, SensorTime> signalNavigateTo;
                                                   
protected:
  virtual void changeEvent(QEvent *event);

public Q_SLOTS:
  virtual void accept();

private Q_SLOTS:
  void slotValidSensor(bool);

private:
  std::auto_ptr<Ui::JumpToObservation> ui;
  ObsAccessPtr mDA;
  std::auto_ptr<SensorChooser> mSensorChooser;
};

#endif // JumpToObservation_hh
