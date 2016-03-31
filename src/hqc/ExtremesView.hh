
#ifndef EXTREMESVIEW_H
#define EXTREMESVIEW_H

#include "common/EditAccess.hh"
#include "common/Sensor.hh"

#include <QtGui/QWidget>

#include <vector>

class ExtremesTableModel;
class TimeRangeControl;
QT_BEGIN_NAMESPACE;
class QItemSelection;
QT_END_NAMESPACE;
namespace Ui {
class DialogExtremeValues;
}

class ExtremesView : public QWidget
{ Q_OBJECT;
public:
  ExtremesView(QWidget* parent=0);
  ~ExtremesView();

  void setDataAccess(EditAccessPtr eda)
    { mEDA = eda; }

  boost::signal1<void, SensorTime> signalNavigateTo;

protected:
  virtual void changeEvent(QEvent *event);

private Q_SLOTS:
  void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void onUpdateClicked();

private:
  int getSelectedRow() const;
  int getParamId() const;
  void setExtremes(const std::vector<SensorTime>& extremes);

private:
  std::auto_ptr<Ui::DialogExtremeValues> ui;
  std::shared_ptr<EditAccess> mEDA;
  std::auto_ptr<ExtremesTableModel> mExtremesModel;
  int mLastSelectedRow;
  TimeRangeControl* mTimeControl;
};

#endif
