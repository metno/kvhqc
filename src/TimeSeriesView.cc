
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

  QDateTime t = timeutil::nowWithMinutes0Seconds0();
  QDateTime f = t.addSecs(-2*24*3600 + 3600*(17-t.time().hour()) + 60*45);
  ui->timeFrom->setDateTime(f);
  ui->timeTo->setDateTime(t);

#if 0
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

  tsdlg->setFromTimeSlot(f);
  tsdlg->setToTimeSlot(t);
#endif
}

TimeSeriesView::~TimeSeriesView()
{
}
                        
void TimeSeriesView::setDataAccess(EditAccessPtr eda, ModelAccessPtr mda)
{
  DataView::setDataAccess(eda, mda);
}

namespace /*anonymous*/ {
class IdxHelper {
  int i;
public:
  IdxHelper(int ii) : i(ii) { }
  int next(int available)
    { int r = i % available; i /= available; return r; }
};
} // namespace anonymous

void TimeSeriesView::setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits)
{
  METLIBS_LOG_SCOPE();
  DataView::setSensorsAndTimes(sensors, limits);
  mSensors = mOriginalSensors = sensors;
  mOriginalTimeLimits = limits;

  ui->timeFrom->setDateTime(timeutil::to_QDateTime(limits.t0()));
  ui->timeTo  ->setDateTime(timeutil::to_QDateTime(limits.t1()));

  // TODO improve plot options
  initalizePlotOptions();

  std::vector<POptions::Colour> colours;
  POptions::Colour::definedColours(colours);

  std::vector<POptions::Linetype> linetypes;
  linetypes.push_back(POptions::Linetype("full"));
  linetypes.push_back(POptions::Linetype("dashed"));
  linetypes.push_back(POptions::Linetype("dotted"));
  linetypes.push_back(POptions::Linetype("dashdotted"));
  linetypes.push_back(POptions::Linetype("dashdashdotted"));

  std::vector<POptions::Marker> markers;
  markers.push_back(POptions::M_CIRCLE);
  markers.push_back(POptions::M_RECTANGLE);
  markers.push_back(POptions::M_TRIANGLE);
  markers.push_back(POptions::M_DIAMOND);
  markers.push_back(POptions::M_STAR);

  std::vector<POptions::yAxis> axes;
  axes.push_back(POptions::axis_left_left);
  axes.push_back(POptions::axis_right_right);
  axes.push_back(POptions::axis_left_right);
  axes.push_back(POptions::axis_right_left);
  axes.push_back(POptions::axis_right_left);
  axes.push_back(POptions::axis_right_left);

  mPlotOptions = std::vector<POptions::PlotOptions>(mSensors.size());
  int idx = -1;
  BOOST_FOREACH(const Sensor& s, mSensors) {
    POptions::PlotOptions& po = mPlotOptions[++idx];

    po.name = (tr("Station:") + QString::number(s.stationId)).toStdString();
    po.label= KvMetaDataBuffer::instance()->findParamName(s.paramId)
        + " (" + miutil::from_number(s.stationId)
        + "/" + miutil::from_number(s.typeId) + ")";

    IdxHelper poi(idx);
    const int ci = poi.next(colours.size()), mi = poi.next(markers.size()), li = poi.next(linetypes.size());
    METLIBS_LOG_DEBUG(LOGVAL(ci) << LOGVAL(mi)<< LOGVAL(li));

    po.linecolour = colours[ci];
    po.marker     = markers[mi];
    po.fillcolour = po.linecolour; // sets marker colour
    po.linetype   = linetypes[li];

    po.linewidth = (s.stationId == mSensors.front().stationId) ? 2 : 1; // if we come here, mSensors is not empty
    po.axis = axes[0];
  }  

  updatePlot();
}

void TimeSeriesView::navigateTo(const SensorTime& st)
{
  ui->plot->clearTimemarks();
  ui->plot->setTimemark(timeutil::make_miTime(st.time), "here");
}

std::string TimeSeriesView::changes()
{
  return "";
}

void TimeSeriesView::replay(const std::string& changes)
{
}

void TimeSeriesView::onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr)
{
  updatePlot();
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

#if 0
void TimeSeriesView::updateSensors()
{
  METLIBS_LOG_SCOPE();

  timeutil::ptime stime;
  timeutil::ptime etime;
  std::vector<std::string> parameter;
  std::vector<POptions::PlotOptions> plotoptions;
  std::vector<int> stationIndex;

  tsdlg->getResults(parameter,stime,etime,stationIndex,plotoptions);
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

  updatePlot();
}
#endif

void TimeSeriesView::updatePlot()
{
  METLIBS_LOG_SCOPE();

  const timeutil::ptime stime = timeutil::from_QDateTime(ui->timeFrom->dateTime());
  const timeutil::ptime etime = timeutil::from_QDateTime(ui->timeTo  ->dateTime());
  const TimeRange limits(stime, etime);

  METLIBS_LOG_DEBUG(LOGVAL(limits));

  TimeSeriesData::tsList tslist;

  int idx = -1;
  BOOST_FOREACH(const Sensor& sensor, mSensors) {
    idx += 1;
    if (idx >= 6)
      continue;
    if (idx >= mPlotOptions.size()) {
      METLIBS_LOG_ERROR("only " << mPlotOptions.size() << " plotoptions, idx " << idx << " is invalid");
      break;
    }

    const ObsAccess::TimeSet times = mDA->allTimes(sensor, limits);
    METLIBS_LOG_DEBUG(LOGVAL(times.size()));
    if (times.empty())
      continue;

    TimeSeriesData::TimeSeries tseries;
    tseries.stationid(sensor.stationId);
    tseries.paramid(sensor.paramId);
    tseries.plotoptions(mPlotOptions[idx]);

    BOOST_FOREACH(const timeutil::ptime& time, times) {
      const SensorTime st(sensor, time);
      ObsDataPtr obs = mDA->find(st);
      ModelDataPtr mdl = mMA->find(st);
      
      const miutil::miTime mtime = timeutil::make_miTime(time);
      if (mdl and ui->radioModel->isChecked()) {
        tseries.add(TimeSeriesData::Data(mtime, mdl->value()));
      } else if (obs and ui->radioObservations->isChecked()) {
        const float corr = obs->corrected();
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

  poptions.name = tr("HQC Time Series").toStdString();
  poptions.fillcolour= POptions::Colour("white");
  poptions.linecolour= POptions::Colour("black");
  poptions.linewidth= 1;
  tsplot.plotoptions(poptions); // set global plotoptions

  tsplot.tserieslist(tslist);      // set list of timeseries

  ui->plot->prepare(tsplot);
}

// static
void TimeSeriesView::initalizePlotOptions()
{
  std::vector<POptions::Colour> colours;
  POptions::Colour::definedColours(colours);
  if (not colours.empty())
    return;

  POptions::Colour::define("black",0,0,0);
  POptions::Colour::define("white",255,255,255);
  POptions::Colour::define("red",255,0,0);
  POptions::Colour::define("green",0,255,0);
  POptions::Colour::define("blue",0,0,255);
  POptions::Colour::define("cyan",0,255,255);
  POptions::Colour::define("magenta",255,0,255);
  POptions::Colour::define("yellow",255,255,0);
  POptions::Colour::define("grey25",64,64,64);
  POptions::Colour::define("grey40",102,102,102);
  POptions::Colour::define("grey45",115,115,115);
  POptions::Colour::define("grey50",128,128,128);
  POptions::Colour::define("grey55",140,140,140);
  POptions::Colour::define("grey60",153,153,153);
  POptions::Colour::define("grey65",166,166,166);
  POptions::Colour::define("grey70",179,179,179);
  POptions::Colour::define("grey75",192,192,192);
  POptions::Colour::define("grey80",204,204,204);
  POptions::Colour::define("grey85",217,217,217);
  POptions::Colour::define("grey90",230,230,230);
  POptions::Colour::define("grey95",243,243,243);
  POptions::Colour::define("mist_red",240,220,220);
  POptions::Colour::define("mist_green",220,240,220);
  POptions::Colour::define("mist_blue",220,240,240);
  POptions::Colour::define("dark_green",0,128,128);
  POptions::Colour::define("brown",179,36,0);
  POptions::Colour::define("orange",255,90,0);
  POptions::Colour::define("purple",90,0,90);
  POptions::Colour::define("light_blue",36,36,255);
  POptions::Colour::define("dark_yellow",179,179,0);
  POptions::Colour::define("dark_red",128,0,0);
  POptions::Colour::define("dark_blue",0,0,128);
  POptions::Colour::define("dark_cyan",0,128,128);
  POptions::Colour::define("dark_magenta",128,0,128);
  POptions::Colour::define("midnight_blue",26,26,110);
  POptions::Colour::define("dnmi_green",44,120,36);
  POptions::Colour::define("dnmi_blue",0,54,125);

  POptions::Linetype::define("full",0xFFFF,1);
  POptions::Linetype::define("dashed",0x00FF,1);
  POptions::Linetype::define("dashdotted",0x0C0F,1);
  POptions::Linetype::define("dashdashdotted",0x1C47,1);
  POptions::Linetype::define("dotted",0xAAAA,2);
}
