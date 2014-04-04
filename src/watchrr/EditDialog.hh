
#ifndef EDITDIALOG_HH
#define EDITDIALOG_HH 1

#include "common/EditAccess.hh"
#include "common/TimeSpan.hh"

#include <QtGui/QDialog>

#include <memory>

namespace Ui {
class DialogEdit;
}
class EditTableModel;

class EditDialog : public QDialog
{   Q_OBJECT
public:
  EditDialog(QDialog* parent, EditAccessPtr da, const Sensor& sensor, const TimeSpan& time, const TimeSpan& editableTime);
  virtual ~EditDialog();

protected:
  virtual void changeEvent(QEvent *event);

private Q_SLOTS:
  void onAcceptAll();
  void onRejectAll();
  void onButtonOk();

private:
  EditAccessPtr mDA;
  TimeSpan mEditableTime;
  Sensor mSensor;
  std::auto_ptr<EditTableModel> etm;
  std::auto_ptr<Ui::DialogEdit> ui;
};

#endif // EDITDIALOG_HH
