
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
  JumpToObservation(QWidget* parent=0);
  ~JumpToObservation();
  
  SensorTime selectedSensorTime() const;

public Q_SLOTS:  
  void navigateTo(const SensorTime&);
  virtual void accept();

Q_SIGNALS:
  void signalNavigateTo(const SensorTime&);
                                                   
protected:
  virtual void changeEvent(QEvent *event);

private Q_SLOTS:
  void slotValidSensor(bool);

private:
  std::auto_ptr<Ui::JumpToObservation> ui;
  ObsAccess_p mDA;
  std::auto_ptr<SensorChooser> mSensorChooser;
};

#endif // JumpToObservation_hh
