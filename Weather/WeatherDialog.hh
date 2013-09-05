
#ifndef WEATHERDIALOG_HH
#define WEATHERDIALOG_HH 1

#include "EditAccess.hh"
#include "TimeRange.hh"

#include <QtGui/QDialog>

#include <memory>

QT_BEGIN_NAMESPACE
class QItemSelection;
class QModelIndex;
class QTableView;
QT_END_NAMESPACE

namespace Ui {
class WeatherDialog;
}
class WeatherTableModel;

class WeatherDialog : public QDialog
{ Q_OBJECT;
public:
  WeatherDialog(EditAccessPtr da, const Sensor& sensor, const TimeRange& time, QWidget* parent=0);
  ~WeatherDialog();

private Q_SLOTS:
  void onUndo();
  void onRedo();
  void onDataChanged(const QModelIndex&, const QModelIndex&);

private:
  void onBackendDataChanged(ObsAccess::ObsDataChange what, EditDataPtr obs);
  void enableSave();
  void clearSelection();
  void clearSelection(QTableView* tv);

private:
  std::auto_ptr<Ui::WeatherDialog> ui;
  EditAccessPtr mDA;
  Sensor mSensor;
  TimeRange mTime;
  std::auto_ptr<WeatherTableModel> mModelCorrected;
  std::auto_ptr<WeatherTableModel> mModelOriginal;
  std::auto_ptr<WeatherTableModel> mModelFlags;
};

#endif // WEATHERDIALOG_HH
