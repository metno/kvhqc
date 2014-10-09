
#ifndef EXTREMESVIEW_H
#define EXTREMESVIEW_H

#include "common/EditAccess.hh"
#include "common/Sensor.hh"

#include <QtGui/QWidget>

#include <vector>

class ExtremesTableModel;
class TimeSpanControl;
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
  
protected:
  virtual void changeEvent(QEvent *event);
  
Q_SIGNALS:
  void signalNavigateTo(const SensorTime&);

private Q_SLOTS:
  void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void onUpdateClicked();
  void onModelReset();

private:
  int getSelectedRow() const;
  int getParamId() const;

private:
  std::auto_ptr<Ui::DialogExtremeValues> ui;
  std::auto_ptr<ExtremesTableModel> mExtremesModel;
  int mLastSelectedRow;
  TimeSpanControl* mTimeControl;
};

#endif
