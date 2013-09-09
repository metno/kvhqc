
#ifndef JumpToObservation_hh
#define JumpToObservation_hh 1

#include "ObsAccess.hh"
#include <boost/signals.hpp>

#include <QtGui/QDialog>

class Sensor;
class SensorTime;
namespace Ui {
class JumpToObservation;
}

class JumpToObservation : public QDialog
{ Q_OBJECT;
public:
  JumpToObservation(ObsAccessPtr da, QWidget* parent=0);
  ~JumpToObservation();
  
  SensorTime selectedSensorTime() const;
  
  boost::signal1<void, SensorTime> signalNavigateTo;
                                                   
public Q_SLOTS:
  virtual void accept();

private Q_SLOTS:
  void onStationEdited();
  void onParameterSelected(int);

private:
  int getStationId() const;
  int getParamId() const;
  int getTypeId() const;
  int getLevel() const;
  int getSensorNr() const;

private:
  std::auto_ptr<Ui::JumpToObservation> ui;
  ObsAccessPtr mDA;
};

#endif // JumpToObservation_hh
