
#ifndef TimeSeriesView_hh
#define TimeSeriesView_hh 1

#include "AbstractDataView.hh"
#include "common/ObsAccess.hh"
#include "common/TimeBuffer.hh"
#include "common/ModelAccess.hh"
#include "common/NavigateHelper.hh"
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

class TimeSeriesView : public AbstractDataView
{ Q_OBJECT
public:
  TimeSeriesView(QWidget* parent=0);
  ~TimeSeriesView();
  
  void setDataAccess(ObsAccess_p eda, ModelAccess_p mda)
    { mDA = eda; mMA = mda; }

private Q_SLOTS:
  void onActionAddColumn();
  void onActionRemoveColumns();
  void onActionResetColumns();

  void onRadioPlot();
  void onDateFromChanged(const QDateTime&);
  void onDateToChanged(const QDateTime&);

  void updatePlot();

private:
  void doNavigateTo();
  void updateSensors();
  void updateTime();
  void updateTimeEditors();
  void retranslateUi();

  std::string changes();
  void replay(const std::string& changes);
  void storeChanges();

  void setTimeSpan(const TimeSpan& t);

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
  SensorTime mStoreSensorTime;

  static std::vector<POptions::Colour> sDefinedColours;
  static std::vector<POptions::Linetype> sDefinedLinetypes;
};

#endif // TimeSeriesView_hh
