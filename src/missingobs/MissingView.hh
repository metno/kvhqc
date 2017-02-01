
#ifndef MISSINGVIEW_H
#define MISSINGVIEW_H

#include "common/EditAccess.hh"
#include "common/Sensor.hh"
#include "util/BusyLabel.hh"

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

Q_SIGNALS:
  void signalNavigateTo(const SensorTime&);

protected:
  virtual void changeEvent(QEvent *event);

private Q_SLOTS:
  void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void onUpdateClicked();
  void onModelReset();

private:
  int getSelectedRow() const;
  int getTypeId() const;

private:
  std::unique_ptr<Ui::DialogMissingObservations> ui;
  BusyLabel* mBusy;
  std::unique_ptr<MissingTableModel> mMissingModel;
  int mLastSelectedRow;
  TimeSpanControl* mTimeControl;
};

#endif
