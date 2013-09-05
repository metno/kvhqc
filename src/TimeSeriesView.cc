
#include "TimeSeriesView.hh"

#include "KvMetaDataBuffer.hh"
#include "ModelData.hh"
#include "TimeseriesDialog.h"

#include <boost/foreach.hpp>

#include "ui_timeseriesview.h"

#define MILOGGER_CATEGORY "kvhqc.TimeSeriesView"
#include "HqcLogging.hh"

TimeSeriesView::TimeSeriesView(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TimeSeriesView)
    , tsdlg(new TimeseriesDialog(this))
{
  METLIBS_LOG_SCOPE();
  ui->setupUi(this);

  std::vector<QString> stations;
  stations.push_back("10380");
  stations.push_back("17980");
  stations.push_back("18700");
  tsdlg->newStationList(stations);

  std::vector<int> params;
  params.push_back(211);
  params.push_back(213);
  params.push_back(215);
  params.push_back(105);
  tsdlg->newParameterList(params);

  QDateTime t = timeutil::nowWithMinutes0Seconds0();
  QDateTime f = t.addSecs(-2*24*3600 + 3600*(17-t.time().hour()) + 60*45);
  ui->timeFrom->setDateTime(f);
  ui->timeTo->setDateTime(t);
  tsdlg->setFromTimeSlot(f);
  tsdlg->setToTimeSlot(t);
}

TimeSeriesView::~TimeSeriesView()
{
}
                        
void TimeSeriesView::setDataAccess(EditAccessPtr eda, ModelAccessPtr mda)
{
  DataView::setDataAccess(eda, mda);

  const timeutil::ptime stime = timeutil::from_QDateTime(ui->timeFrom->dateTime());
  const timeutil::ptime etime = timeutil::from_QDateTime(ui->timeTo  ->dateTime());
  const TimeRange limits(stime, etime);
  mDA->addSubscription(ObsSubscription(18700, limits));
}

void TimeSeriesView::setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits)
{
  DataView::setSensorsAndTimes(sensors, limits);
}

void TimeSeriesView::navigateTo(const SensorTime&)
{
}

void TimeSeriesView::onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr)
{
}

void TimeSeriesView::onConfigButton()
{
  tsdlg->exec();
  updatePlot();
}

void TimeSeriesView::onRadioPlot()
{
  updatePlot();
}

void TimeSeriesView::onDateFromChanged(const QDateTime&)
{
  updatePlot();
}

void TimeSeriesView::onDateToChanged(const QDateTime&)
{
  updatePlot();
}

void TimeSeriesView::updatePlot()
{
  METLIBS_LOG_SCOPE();

  timeutil::ptime stime;
  timeutil::ptime etime;
  std::vector<std::string> parameter;
  std::vector<POptions::PlotOptions> plotoptions;
  std::vector<int> stationIndex;

  tsdlg->getResults(parameter,stime,etime,stationIndex,plotoptions);

  stime = timeutil::from_QDateTime(ui->timeFrom->dateTime());
  etime = timeutil::from_QDateTime(ui->timeTo  ->dateTime());
  const TimeRange limits(stime, etime);

  METLIBS_LOG_DEBUG(LOGVAL(parameter.size()) << LOGVAL(limits));

  TimeSeriesData::tsList tslist;

  const std::list<kvalobs::kvParam>& plist = KvMetaDataBuffer::instance()->allParams();
  for (unsigned int ip = 0; ip < parameter.size(); ip++) {
    Sensor sensor(stationIndex[ip], 0, 0, 0, 0);

    BOOST_FOREACH(const kvalobs::kvParam& p, plist) {
      if (p.name() == parameter[ip]) {
        sensor.paramId = p.paramID();
        METLIBS_LOG_DEBUG(LOGVAL(sensor.paramId));
        break;
      }
    }
    
    try {
      const KvMetaDataBuffer::ObsPgmList opgm = KvMetaDataBuffer::instance()->findObsPgm(sensor.stationId);
      BOOST_FOREACH(const kvalobs::kvObsPgm& op, opgm) {
        if (op.paramID() == sensor.paramId
            and op.fromtime() <= stime
            and (op.totime().is_not_a_date_time() or op.totime() >= etime))
        {
          sensor.typeId = op.typeID();
          METLIBS_LOG_DEBUG(LOGVAL(sensor.typeId));
          break;
        }
      }
    } catch (std::exception& e) {
      METLIBS_LOG_WARN("exception while reading obs_pgm for " << sensor);
      continue;
    }
    
    const ObsAccess::TimeSet times = mDA->allTimes(sensor, limits);
    METLIBS_LOG_DEBUG(LOGVAL(times.size()));
    if (times.empty())
      continue;


    TimeSeriesData::TimeSeries tseries;
    tseries.stationid(sensor.stationId);
    tseries.paramid(sensor.paramId);
    tseries.plotoptions(plotoptions[ip]);

    BOOST_FOREACH(const timeutil::ptime& time, times) {
      const SensorTime st(sensor, time);
      ObsDataPtr obs = mDA->find(st);
      ModelDataPtr mdl = mMA->find(st);
      METLIBS_LOG_DEBUG(st << " have obs=" << (obs ? "yes" : "no") << " mdl=" << (mdl ? "yes" : "no"));
      
      const miutil::miTime mtime = timeutil::make_miTime(time);
      if (mdl and ui->radioModel->isChecked()) {
        tseries.add(TimeSeriesData::Data(mtime, mdl->value()));
      } else if (obs and ui->radioObservations->isChecked()) {
        const float corr = obs->corrected();
        METLIBS_LOG_DEBUG(LOGVAL(corr));
        if (corr > -32766.0)
          tseries.add(TimeSeriesData::Data(mtime, corr));
      } else if (obs and mdl and ui->radioDifference->isChecked()) {
        const float corr = obs->corrected();
        if (corr > -32766.0)
	  tseries.add(TimeSeriesData::Data(mtime, corr - mdl->value()));
      }
    }

    if (tseries.dataOK())
      tslist.push_back(tseries);
  }
  
  if (tslist.empty())
    return;

  // finally: one complete plot-structure
  TimeSeriesData::TSPlot tsplot;
  POptions::PlotOptions poptions;
  poptions.time_types.push_back(POptions::T_DATE);
  poptions.time_types.push_back(POptions::T_DAY);
  poptions.time_types.push_back(POptions::T_HOUR);

  poptions.name= "HQC Tidsserie";
  poptions.fillcolour= POptions::Colour("white");
  poptions.linecolour= POptions::Colour("black");
  poptions.linewidth= 1;
  tsplot.plotoptions(poptions); // set global plotoptions

  tsplot.tserieslist(tslist);      // set list of timeseries

  ui->plot->prepare(tsplot);
}

