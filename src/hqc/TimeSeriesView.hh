
#ifndef TimeSeriesView_hh
#define TimeSeriesView_hh 1

#include "common/ObsAccess.hh"
#include "common/TimeBuffer.hh"
#include "common/ModelAccess.hh"
#include "util/BusyLabel.hh"

#include "qtimeseries/PlotOptions.h"

#include <QtGui/QWidget>

#include <memory>
#include <vector>

class QAction;
class QMenu;
class TimeSpanControl;

namespace Ui {
class TimeSeriesView;
}

class TimeSeriesView : public QWidget
{ Q_OBJECT
public:
  TimeSeriesView(QWidget* parent=0);
  ~TimeSeriesView();
  
  void setDataAccess(ObsAccess_p eda, ModelAccess_p mda)
    { mDA = eda; mMA = mda; }

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

  void onDataComplete();
  void onDataChanged();

private:
  void doNavigateTo(const SensorTime& st);
  void updateSensors();
  void updatePlot();
  void updateTimeEditors();

  std::string changes();
  void replay(const std::string& changes);
  void storeChanges();

  void setTimeSpan(const TimeSpan& t);

  void updateVisible(bool visible);

  static void initalizePlotOptions();

private:
  std::auto_ptr<Ui::TimeSeriesView> ui;

  QMenu* mColumnMenu;
  QAction* mColumnAdd;
  QAction* mColumnRemove;
  QAction* mColumnReset;
  BusyLabel* mBusy;

  ObsAccess_p mDA;
  ModelAccess_p mMA;
  SensorTime mSensorTime;
  TimeSpan mTimeLimits, mOriginalTimeLimits;
  Sensor_v mSensors, mOriginalSensors;
  std::vector<POptions::PlotOptions> mPlotOptions;
  TimeSpanControl* mTimeControl;
  bool mChangingTimes;

  TimeBuffer_p mObsBuffer;
  bool mVisible;
  SensorTime mPendingSensorTime;

  static std::vector<POptions::Colour> sDefinedColours;
  static std::vector<POptions::Linetype> sDefinedLinetypes;
};

#endif // TimeSeriesView_hh
