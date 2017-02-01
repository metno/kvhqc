
#ifndef TimeSeriesView_hh
#define TimeSeriesView_hh 1

#include "AbstractDataView.hh"
#include "common/ObsAccess.hh"
#include "common/TimeBuffer.hh"
#include "common/ModelBuffer.hh"
#include "common/NavigateHelper.hh"
#include "util/BusyLabel.hh"

#include "qtimeseries/PlotOptions.h"

#include <QtGui/QWidget>

#include <memory>
#include <vector>

class ObsPgmRequest;
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
  
protected:
  void doNavigateTo();
  std::string changes();
  void replay(const std::string& changes);
  void prepareReplay();

private Q_SLOTS:
  void onActionAddColumn();
  void onActionRemoveColumns();
  void onActionResetColumns();

  void onRadioPlot();
  void onDateFromChanged(const QDateTime&);
  void onDateToChanged(const QDateTime&);

  void updatePlot();
  void haveNeighbors();

private:
  void findNeighbors();
  void updateSensors();
  void updateTime();
  void updateTimeEditors();
  void highlightTime();
  void retranslateUi();

  void storeChanges();

  void setTimeSpan(const TimeSpan& t);

  static void initalizePlotOptions();

private:
  std::unique_ptr<Ui::TimeSeriesView> ui;

  QMenu* mColumnMenu;
  QAction* mColumnAdd;
  QAction* mColumnRemove;
  QAction* mColumnReset;
  BusyLabel* mBusy;

  ObsAccess_p mDA;
  ModelBuffer_p mModelBuffer;
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

  ObsPgmRequest* mObsPgmRequest;
};

#endif // TimeSeriesView_hh
