
#ifndef MISSINGVIEW_H
#define MISSINGVIEW_H

#include "common/EditAccess.hh"
#include "common/Sensor.hh"

#include <QtGui/QWidget>

#include <vector>

class MissingTableModel;
class TimeSpanControl;
QT_BEGIN_NAMESPACE;
class QItemSelection;
QT_END_NAMESPACE;
namespace Ui {
class DialogMissingObservations;
}

class MissingView : public QWidget
{ Q_OBJECT;
public:
  MissingView(QWidget* parent=0);
  ~MissingView();

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
  int getTypeId() const;
  void setMissing(const std::vector<SensorTime>& Missing);

private:
  std::auto_ptr<Ui::DialogMissingObservations> ui;
  boost::shared_ptr<EditAccess> mEDA;
  std::auto_ptr<MissingTableModel> mMissingModel;
  int mLastSelectedRow;
  TimeSpanControl* mTimeControl;
};

#endif
