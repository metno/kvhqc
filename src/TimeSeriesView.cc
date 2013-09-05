
#include "TimeSeriesView.hh"

TimeSeriesView::TimeSeriesView(QWidget* parent)
    : QWidget(parent)
{
}

TimeSeriesView::~TimeSeriesView()
{
}
                        
void TimeSeriesView::setDataAccess(EditAccessPtr eda, ModelAccessPtr mda)
{
    DataView::setDataAccess(eda, mda);
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

void TimeSeriesView::show()
{
#if 0
  timeutil::ptime stime;
  timeutil::ptime etime;
  std::vector<std::string> parameter;
  std::vector<POptions::PlotOptions> plotoptions;
  std::vector<int> parameterIndex;
  std::vector<int> stationIndex;

  tsdlg->getResults(parameter,stime,etime,stationIndex,plotoptions);

  // make timeseries
  TimeSeriesData::tsList tslist;

  int nTypes = tsdlg->obsCheckBox->isChecked() + tsdlg->modCheckBox->isChecked();

  for ( unsigned int ip = 0; ip < parameter.size(); ip++ ) {
      const std::list<kvalobs::kvParam>& plist = KvMetaDataBuffer::instance()->allParams();
      std::list<kvalobs::kvParam>::const_iterator it=plist.begin();
      BOOST_FOREACH(const kvalobs::kvParam& p, plist) {
          if (p.name() == parameter[ip]) {
              parameterIndex.push_back(it->paramID());
              break;
          }
      }

    TimeSeriesData::TimeSeries tseries;
    tseries.stationid(stationIndex[ip]);  // set stationid
    tseries.paramid(parameterIndex[ip]);     // set parameter-number

    tseries.plotoptions(plotoptions[ip]); // set plotoptions for this curve
    if (tsdlg->modCheckBox->isChecked() && ( nTypes == 1 || ip%nTypes != 0) ) {
      for ( unsigned int i = 0; i < modeldatalist.size(); i++) { // fill data
	if ( modeldatalist[i].stnr == stationIndex[ip] &&
	     modeldatalist[i].otime >= stime &&
	     modeldatalist[i].otime <= etime ){
	  tseries.add(TimeSeriesData::Data(timeutil::make_miTime(modeldatalist[i].otime),
					   modeldatalist[i].orig[parameterIndex[ip]]));
	}
      }
      if(tseries.dataOK()) {
	tslist.push_back(tseries);
      }
    }
    if ( tsdlg->obsCheckBox->isChecked() && (nTypes == 1 || ip%nTypes == 0) ) {
      for ( unsigned int i = 0; i < datalist->size(); i++) { // fill data
          const timeutil::ptime& otime = (*datalist)[i].otime();
          if ( (*datalist)[i].stnr() == stationIndex[ip] &&
	     otime >= stime &&
             otime <= etime &&
	     otime.time_of_day().minutes() == 0 ) {
	  if ( (*datalist)[i].corr(parameterIndex[ip]) > -32766.0 )
	    tseries.add(TimeSeriesData::Data(timeutil::make_miTime(otime),
					     (*datalist)[i].corr(parameterIndex[ip])));
	}
      }
      if(tseries.dataOK()) {
	tslist.push_back(tseries);
      }
    }
    else if (tsdlg->modCheckBox->isChecked() && tsdlg->obsCheckBox->isChecked() ) {
      for ( unsigned int i = 0; i < modeldatalist.size(); i++) { // fill data
	if ( modeldatalist[i].stnr == stationIndex[ip] &&
	     modeldatalist[i].otime >= stime &&
	     modeldatalist[i].otime <= etime ){
	  tseries.add(TimeSeriesData::Data(timeutil::make_miTime(modeldatalist[i].otime),
					  (*datalist)[i].corr(parameterIndex[ip])
					  - modeldatalist[i].orig[parameterIndex[ip]]));
	}
      }
      if(tseries.dataOK()) {
	tslist.push_back(tseries);
      }
    }
  }

  if(tslist.size() == 0){
    tspdialog->hide();
    return;
  }

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

  tspdialog->prepare(tsplot);
  tspdialog->show();
#endif
}

