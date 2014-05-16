
#ifndef NAVIGATIONHISTORY_HH
#define NAVIGATIONHISTORY_HH 1

#include "common/NavigateHelper.hh"
#include "common/Sensor.hh"

#include <QWidget>

class QItemSelection;

class JumpToObservation;
class NavigationHistoryModel;

class Ui_NavigationHistory;

class NavigationHistory : public QWidget
{ Q_OBJECT;

public:
  NavigationHistory(QWidget* parent=0);

public Q_SLOTS:
  void navigateTo(const SensorTime& st);

Q_SIGNALS:
  void signalNavigateTo(const SensorTime&);

protected:
  void changeEvent(QEvent *event);

private Q_SLOTS:
  void onClear();
  void onJump();
  void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
  NavigationHistoryModel* model() const;
  
private:
  std::auto_ptr<Ui_NavigationHistory> ui;
  JumpToObservation* mJump;
  NavigateHelper mNavigate;
};

#endif // NAVIGATIONHISTORY_HH
