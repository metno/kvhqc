
#ifndef TimeSeriesView_hh
#define TimeSeriesView_hh 1

#include "ChangeableDataView.hh"

#include <qTimeseries/PlotOptions.h>

#include <QtGui/QWidget>

#include <memory>
#include <vector>

class TimeRangeControl;
class TimeseriesDialog;
namespace Ui {
class TimeSeriesView;
}

class TimeSeriesView : public QWidget, public ChangeableDataView
{ Q_OBJECT
public:
  TimeSeriesView(QWidget* parent=0);
  ~TimeSeriesView();
                        
  virtual void setDataAccess(EditAccessPtr eda, ModelAccessPtr mda);
  virtual void setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits);

  virtual std::string changes();
  virtual void replay(const std::string& changes);
  virtual std::string type() const;
  virtual std::string id() const;

public Q_SLOTS:
  void navigateTo(const SensorTime&);

private:
  void updateSensors();
  void updatePlot();

  void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);

  static void initalizePlotOptions();

private Q_SLOTS:
  void onButtonAdd();
  void onButtonRemove();
  void onRadioPlot();
  void onDateFromChanged(const QDateTime&);
  void onDateToChanged(const QDateTime&);

private:
  std::auto_ptr<Ui::TimeSeriesView> ui;
  std::auto_ptr<TimeseriesDialog> tsdlg;

  TimeRange mOriginalTimeLimits;
  Sensors_t mSensors, mOriginalSensors;
  std::vector<POptions::PlotOptions> mPlotOptions;
  TimeRangeControl* mTimeControl;
};

#endif // TimeSeriesView_hh
