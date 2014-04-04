
#ifndef ToolInterpolate_hh
#define ToolInterpolate_hh 1

#include "access/EditAccess.hh"
#include "common/ModelAccess.hh"
#include "common/Sensor.hh"

#include <QtGui/QWidget>

#include <vector>

class QTableView;
class QItemSelection;
class QToolButton;
class QCheckBox;

class ToolInterpolate : public QWidget
{ Q_OBJECT;
public:
  ToolInterpolate(QWidget* parent=0);

  void updateModel(EditAccess_p da, QTableView* table);

public Q_SLOTS:
  void enableButtons();
                                                           
protected:
  virtual void changeEvent(QEvent *event);

private Q_SLOTS:
  void onInterpolate();

private:
  void retranslateUi();
  bool checkEnabled();

private:
  EditAccess_p mDA;
  QTableView* mTableView;

  QToolButton *mButtonInterpolate;

  Sensor mSensor;
  TimeSpan mTime;

  typedef std::vector<SensorTime> SensorTime_v;
  SensorTime_v mSelectedObs;
};

#endif // ToolInterpolate_hh
