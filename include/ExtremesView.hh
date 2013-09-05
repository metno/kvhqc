
#ifndef EXTREMESVIEW_H
#define EXTREMESVIEW_H

#include "EditAccess.hh"
#include "Sensor.hh"

#include <QtGui/QDialog>

#include <vector>

class ExtremesTableModel;
QT_BEGIN_NAMESPACE;
class QItemSelection;
QT_END_NAMESPACE;
namespace Ui {
class DialogExtremeValues;
}

class ExtremesView : public QDialog
{ Q_OBJECT;
public:
  ExtremesView(QWidget* parent=0);
  ~ExtremesView();

  virtual void setExtremes(EditAccessPtr eda, const std::vector<SensorTime>& extremes);

  void navigateTo(const SensorTime&) { }
                                    
  boost::signal1<void, SensorTime> signalNavigateTo;

private Q_SLOTS:
  void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
  int getSelectedRow() const;

private:
  std::auto_ptr<Ui::DialogExtremeValues> ui;
  std::auto_ptr<ExtremesTableModel> mExtremesModel;
  int mLastSelectedRow;
};

#endif
