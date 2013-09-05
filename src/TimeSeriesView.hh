
#ifndef TimeSeriesView_hh
#define TimeSeriesView_hh 1

#include "DataView.hh"

#include <QtGui/QWidget>
#include <memory>

class TimeseriesDialog;
namespace Ui {
class TimeSeriesView;
}

class TimeSeriesView : public QWidget, public DataView
{ Q_OBJECT
public:
  TimeSeriesView(QWidget* parent=0);
  ~TimeSeriesView();
                        
  void setDataAccess(EditAccessPtr eda, ModelAccessPtr mda);
  void setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits);

public Q_SLOTS:
  void navigateTo(const SensorTime&);

private:
  void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);
  void updatePlot();

private Q_SLOTS:
  void onConfigButton();
  void onRadioPlot();
  void onDateFromChanged(const QDateTime&);
  void onDateToChanged(const QDateTime&);

private:
  std::auto_ptr<Ui::TimeSeriesView> ui;
  std::auto_ptr<TimeseriesDialog> tsdlg;
};

#endif // TimeSeriesView_hh
