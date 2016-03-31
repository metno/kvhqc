
#ifndef TimeSeriesView_hh
#define TimeSeriesView_hh 1

#include "common/DataView.hh"
#include "qtimeseries/PlotOptions.h"

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

protected:
  virtual void showEvent(QShowEvent* showEvent);
  virtual void hideEvent(QHideEvent* hideEvent);
  virtual void resizeEvent(QResizeEvent *resizeEvent);
  virtual void changeEvent(QEvent *event);

private Q_SLOTS:
  void onActionAddColumn();
  void onActionRemoveColumns();
  void onActionResetColumns();

  void onRadioPlot();
  void onDateFromChanged(const QDateTime&);
  void onDateToChanged(const QDateTime&);

private:
  void doNavigateTo(const SensorTime& st);
  void updateSensors();
  void updatePlot();
  void updateTimeEditors();

  std::string changes();
  void replay(const std::string& changes);
  void storeChanges();

  void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);
  void setTimeRange(const TimeRange& t);

  void updateVisible(bool visible);

  static void initalizePlotOptions();

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
  bool mChangingTimes;

  bool mVisible;
  SensorTime mPendingSensorTime;

  static std::vector<POptions::Colour> sDefinedColours;
  static std::vector<POptions::Linetype> sDefinedLinetypes;
};

#endif // TimeSeriesView_hh
