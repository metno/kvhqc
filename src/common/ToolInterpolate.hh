
#ifndef ToolInterpolate_hh
#define ToolInterpolate_hh 1

#include "common/EditAccess.hh"
#include "common/TimeBuffer.hh"
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

protected:
  virtual void changeEvent(QEvent *event);

private Q_SLOTS:
  void onInterpolate();
  void fetchData();
  void enableButtons();

private:
  void retranslateUi();
  bool checkEnabled();

private:
  EditAccess_p mDA;
  QTableView* mTableView;

  QToolButton *mButtonInterpolate;

  SensorTime mFirst, mLast;

  TimeBuffer_p mObsBuffer;

  typedef std::vector<Time> Time_v;
  Time_v mSelectedTimes;
};

#endif // ToolInterpolate_hh
