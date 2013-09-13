
#ifndef TimeSeriesView_hh
#define TimeSeriesView_hh 1

#include "common/DataView.hh"

#include <qTimeseries/PlotOptions.h>

#include <QtGui/QWidget>

#include <memory>
#include <vector>

class QAction;
class QMenu;
class TimeRangeControl;
class TimeseriesDialog;
namespace Ui {
class TimeSeriesView;
}

class TimeSeriesView : public QWidget, public DataView
{ Q_OBJECT
public:
  TimeSeriesView(QWidget* parent=0);
  ~TimeSeriesView();
                        
public Q_SLOTS:
  void navigateTo(const SensorTime&);

private:
  void updateSensors();
  void updatePlot();

  std::string changes();
  void replay(const std::string& changes);
  void storeChanges();

  void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);

  static void initalizePlotOptions();

private Q_SLOTS:
  void onActionAddColumn();
  void onActionRemoveColumns();
  void onActionResetColumns();

  void onRadioPlot();
  void onDateFromChanged(const QDateTime&);
  void onDateToChanged(const QDateTime&);

private:
  std::auto_ptr<Ui::TimeSeriesView> ui;
  std::auto_ptr<TimeseriesDialog> tsdlg;

  QMenu* mColumnMenu;
  QAction* mColumnAdd;
  QAction* mColumnRemove;
  QAction* mColumnReset;

  SensorTime mSensorTime;
  TimeRange mTimeLimits, mOriginalTimeLimits;
  Sensors_t mSensors, mOriginalSensors;
  std::vector<POptions::PlotOptions> mPlotOptions;
  TimeRangeControl* mTimeControl;
};

#endif // TimeSeriesView_hh
