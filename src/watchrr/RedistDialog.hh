
#ifndef REDISTDIALOG_HH
#define REDISTDIALOG_HH

#include "TaskAccess.hh"
#include "common/TimeSpan.hh"

#include <QDialog>

#include <memory>

QT_BEGIN_NAMESPACE
class QItemSelection;
class QModelIndex;
QT_END_NAMESPACE

namespace Ui {
class DialogRedist;
}
class RedistTableModel;

class RedistDialog : public QDialog
{   Q_OBJECT;
public:
  RedistDialog(QDialog* parent, TaskAccess_p da, const Sensor& sensor, const TimeSpan& time, const TimeSpan& editableTime);
  virtual ~RedistDialog();

protected:
  virtual void changeEvent(QEvent *event);

private Q_SLOTS:
  void onButtonOk();
  void onButtonAuto();
  void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

private:
  void updateSumInfo();

private:
  TaskAccess_p mDA;
  TimeSpan mEditableTime;
  Sensor mSensor;
  std::unique_ptr<RedistTableModel> rtm;
  std::unique_ptr<Ui::DialogRedist> ui;
};

#endif // REDISTDIALOG_HH
