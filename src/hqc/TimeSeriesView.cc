/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "TimeSeriesView.hh"

#include "TimeSeriesAdd.hh"
#include "ViewChanges.hh"

#include "common/EditAccess.hh"
#include "common/HqcApplication.hh"
#include "common/KvHelpers.hh"
#include "common/ModelData.hh"
#include "common/ObsPgmRequest.hh"
#include "common/TimeSpanControl.hh"

#include "util/ChangeReplay.hh"

#include <puTools/miStringFunctions.h>

#include <QDomElement>
#include <QMenu>
#include <QResizeEvent>
#include <QStringListModel>

#include <boost/range/adaptor/reversed.hpp>

#include <algorithm>

#include "ui_timeseriesview.h"
#include "ui_ts_remove.h"

#define MILOGGER_CATEGORY "kvhqc.TimeSeriesView"
#include "common/ObsLogging.hh"

namespace {
const std::string VIEW_TYPE = "TimeSeries";
const std::string ID = "1";
}

// ########################################################################

namespace /*anonymous*/ {
class IdxHelper {
  int i;
public:
  IdxHelper(int ii) : i(ii) { }
  int next(int available)
    { int r = i % available; i /= available; return r; }
};

const size_t MAX_LINES = 8;
const int NMARKERS = 5;
} // namespace anonymous

// ########################################################################

TimeSeriesView::TimeSeriesView(QWidget* parent)
  : AbstractDataView(parent)
  , ui(new Ui::TimeSeriesView)
  , mTimeControl(new TimeSpanControl(this))
  , mChangingTimes(false)
  , mObsPgmRequest(0)
{
  METLIBS_LOG_SCOPE();
  ui->setupUi(this);
  ui->comboWhatToPlot->addItems(QStringList()
      << tr("Corrected")
      << tr("Model")
      << tr("Difference"));
  ui->comboWhatToPlot->setCurrentIndex(0);

  mBusy = new BusyLabel(this);
  ui->topRow->addWidget(mBusy);

  mColumnMenu = new QMenu(this);
  mColumnAdd = mColumnMenu->addAction(QIcon("icons:dl_columns_add.svg"), tr("Add..."), this, SLOT(onActionAddColumn()));
  mColumnRemove = mColumnMenu->addAction(QIcon("icons:dl_columns_remove.svg"), tr("Remove..."), this, SLOT(onActionRemoveColumns()));
  mColumnReset = mColumnMenu->addAction(QIcon("icons:dl_columns_reset.svg"), tr("Reset"), this, SLOT(onActionResetColumns()));
  mColumnAdd->setEnabled(false);
  mColumnRemove->setEnabled(false);
  mColumnReset->setEnabled(false);

  ui->buttonLinesMenu->setIcon(QIcon("icons:dl_columns.svg"));
  ui->buttonLinesMenu->setMenu(mColumnMenu);

  mTimeControl->setMinimumGap(24);
  mTimeControl->setMaximumGap(24*7*6); // maximum 6 weeks
  mTimeControl->install(ui->timeFrom, ui->timeTo);

  QDateTime t = timeutil::nowWithMinutes0Seconds0();
  QDateTime f = t.addSecs(-2*24*3600 + 3600*(17-t.time().hour()) + 60*45);
  ui->timeFrom->setDateTime(f);
  ui->timeTo->setDateTime(t);
  ui->timeFrom->setCurrentSection(QDateTimeEdit::HourSection);
  ui->timeTo  ->setCurrentSection(QDateTimeEdit::HourSection);

  // TODO improve plot options
  initalizePlotOptions();

  mDA = hqcApp->editAccess();

  mModelBuffer = std::make_shared<ModelBuffer>(hqcApp->modelAccess());
  connect(mModelBuffer.get(), SIGNAL(received(const ModelData_pv&)),
      this, SLOT(updatePlot()));
}

TimeSeriesView::~TimeSeriesView()
{
  delete mObsPgmRequest;
}
                        
void TimeSeriesView::storeChanges()
{
  METLIBS_LOG_TIME(LOGVAL(mStoreSensorTime));
  if (mStoreSensorTime.valid())
    ViewChanges::store(mStoreSensorTime.sensor, VIEW_TYPE, ID, changes());
  mStoreSensorTime = mNavigate.current();
}

void TimeSeriesView::updateSensors()
{
  METLIBS_LOG_TIME(LOGVAL(mSensors.size()));

  std::vector<POptions::yAxis> axes;
  axes.push_back(POptions::axis_left_left);
  axes.push_back(POptions::axis_right_right);
  axes.push_back(POptions::axis_left_right);
  axes.push_back(POptions::axis_right_left);

  mPlotOptions = std::vector<POptions::PlotOptions>(mSensors.size());
  int idx = -1;
  for (const Sensor& s : mSensors) {
    POptions::PlotOptions& po = mPlotOptions[++idx];

    po.name = (tr("Station:") + QString::number(s.stationId)).toStdString();
    po.label = KvMetaDataBuffer::instance()->findParamName(s)
        + " (" + miutil::from_number(s.stationId)
        + "/T" + miutil::from_number(s.typeId);
    if (s.level != 0)
      po.label += "/L" + miutil::from_number(s.level);
    if (s.sensor != 0)
      po.label += "/S" + miutil::from_number(s.sensor);
    po.label += ")";
    METLIBS_LOG_DEBUG(LOGVAL(s) << LOGVAL(po.label));

    IdxHelper poi(idx);
    const int ci = poi.next(sDefinedColours.size()), li = poi.next(sDefinedLinetypes.size());
    const int mi = (ci % NMARKERS);
    METLIBS_LOG_DEBUG(LOGVAL(ci) << LOGVAL(mi)<< LOGVAL(li));

    po.linecolour = sDefinedColours[ci];
    po.marker     = (POptions::Marker)(POptions::M_RECTANGLE + mi);
    po.fillcolour = po.linecolour; // sets marker colour
    po.linetype   = sDefinedLinetypes[li];

    po.linewidth = (s.stationId == mSensors.front().stationId) ? 3 : 1; // if we come here, mSensors is not empty
    po.axis = axes[0];

    try {
      const kvalobs::kvParam& param = KvMetaDataBuffer::instance()->findParam(s);
      if (param.unit().find("grader") != std::string::npos
          and s.paramId != kvalobs::PARAMID_MLON and s.paramId != kvalobs::PARAMID_MLAT)
      {
        po.plottype = POptions::type_vector;
        po.linewidth= 1;
      } else if (param.description().find("Nedb") != std::string::npos and param.description().find("tilvekst") != std::string::npos) {
        po.plottype = POptions::type_histogram;
        po.axisname= "";
      }
    } catch (std::exception& ex) {
      HQC_LOG_WARN("exception while retrieving kvParam for " << s);
    }
  }
  updateTime();
}

void TimeSeriesView::retranslateUi()
{
  ui->retranslateUi(this);
  
  ui->comboWhatToPlot->setItemText(0, tr("Corrected"));
  ui->comboWhatToPlot->setItemText(1, tr("Model"));
  ui->comboWhatToPlot->setItemText(2, tr("Difference"));
  
  mColumnAdd->setText(tr("Add..."));
  mColumnRemove->setText(tr("Remove..."));
  mColumnReset->setText(tr("Reset"));
  
  updateSensors(); // language-specific legend
}

void TimeSeriesView::doNavigateTo()
{
  METLIBS_LOG_TIME();

  const SensorTime& st = mNavigate.current();

  const bool changedSensor = (not mStoreSensorTime.valid()
      or not eq_Sensor()(st.sensor, mStoreSensorTime.sensor));
  const bool changedTime = changedSensor
      or not mTimeLimits.contains(st.time);

  if (changedTime) {
    mTimeLimits = ViewChanges::defaultTimeLimits(st);
    mOriginalTimeLimits = mTimeLimits;
    updateTimeEditors();
  }
  if (changedSensor) {
    findNeighbors();
  } else if (changedTime) {
    updateTime();
  } else {
    highlightTime();
  }
}

void TimeSeriesView::findNeighbors()
{
  METLIBS_LOG_TIME();
  mColumnAdd->setEnabled(false);
  mColumnRemove->setEnabled(false);

  const SensorTime& st = mNavigate.current();
  hqc::int_s stationIds = KvMetaDataBuffer::instance()->findNeighborStationIds(st.sensor.stationId);
  stationIds.insert(st.sensor.stationId);
  delete mObsPgmRequest;
  mObsPgmRequest = new ObsPgmRequest(stationIds);
  connect(mObsPgmRequest, SIGNAL(complete()), this, SLOT(haveNeighbors()));
  mObsPgmRequest->post();
}

void TimeSeriesView::haveNeighbors()
{
  METLIBS_LOG_TIME();
  // need to update related parameters and neighbor list
  
  // set original columns
  const SensorTime& st = mNavigate.current();
  mSensors = Sensor_v(1, st.sensor);
  KvMetaDataBuffer::instance()->addNeighbors(mSensors, st.sensor, mTimeLimits, mObsPgmRequest, MAX_LINES*2);
  mOriginalSensors = mSensors;
  
  mModelBuffer->clear();
  replay(ViewChanges::fetch(mSensorTime.sensor, VIEW_TYPE, ID));
  
  mColumnAdd->setEnabled(true);
  mColumnRemove->setEnabled(true);
  
  updateSensors();
}

void TimeSeriesView::highlightTime()
{
  ui->plot->clearTimemarks();
  ui->plot->setTimemark(timeutil::make_miTime(mNavigate.current().time), "here");
}

namespace /* anonymous */ {
static const char C_ATTR_STATIONID[] = "stationid";
static const char C_ATTR_PARAMID[]   = "paramid";
static const char C_ATTR_TYPEID[]    = "typeid";
static const char C_ATTR_SENSORNR[]  = "sensornr";
static const char C_ATTR_LEVEL[]     = "level";

static const char T_ATTR_START[] = "start";
static const char T_ATTR_END[]   = "end";

static const char E_TAG_CHANGES[] = "changes";
static const char E_TAG_COLUMNS[] = "columns";
static const char E_TAG_REMOVED[] = "removed";
static const char E_TAG_COLUMN[]  = "column";
static const char E_TAG_TSHIFT[]  = "timeshift";

void toText(const Sensor& sensor, QDomElement& ce)
{
  ce.setAttribute(C_ATTR_STATIONID, sensor.stationId);
  ce.setAttribute(C_ATTR_PARAMID,   sensor.paramId);
  ce.setAttribute(C_ATTR_TYPEID,    sensor.typeId);
  if (sensor.level != 0)
    ce.setAttribute(C_ATTR_LEVEL, sensor.level);
  if (sensor.sensor != 0)
    ce.setAttribute(C_ATTR_SENSORNR, sensor.sensor);
}

void fromText(const QDomElement& ce, Sensor& sensor)
{
  sensor.stationId = ce.attribute(C_ATTR_STATIONID).toInt();
  sensor.paramId   = ce.attribute(C_ATTR_PARAMID)  .toInt();
  sensor.typeId    = ce.attribute(C_ATTR_TYPEID)   .toInt();
  if (ce.hasAttribute(C_ATTR_LEVEL))
    sensor.level = ce.attribute(C_ATTR_LEVEL).toInt();
  if (ce.hasAttribute(C_ATTR_SENSORNR))
    sensor.sensor = ce.attribute(C_ATTR_SENSORNR).toInt();
}
} // anonymous namespace

std::string TimeSeriesView::changes()
{
  METLIBS_LOG_SCOPE();
  QDomDocument doc("changes");
  QDomElement doc_changes = doc.createElement(E_TAG_CHANGES);
  doc.appendChild(doc_changes);

  ChangeReplay<Sensor, lt_Sensor> cr;
  const Sensor_v removed = cr.removals(mOriginalSensors, mSensors);
  if (not removed.empty()) {
    QDomElement doc_removed = doc.createElement(E_TAG_REMOVED);
    for (const Sensor& r : removed) {
      QDomElement doc_column = doc.createElement(E_TAG_COLUMN);
      toText(r, doc_column);
      doc_removed.appendChild(doc_column);
    } 
    doc_changes.appendChild(doc_removed);
  }

  QDomElement doc_columns = doc.createElement(E_TAG_COLUMNS);
  for (const Sensor& s : mSensors) {
    QDomElement doc_column = doc.createElement(E_TAG_COLUMN);
    toText(s, doc_column);
    doc_columns.appendChild(doc_column);
  } 
  doc_changes.appendChild(doc_columns);

  if (mOriginalTimeLimits.t0() != mTimeLimits.t0() or mOriginalTimeLimits.t1() != mTimeLimits.t1()) {
    QDomElement doc_timeshift = doc.createElement(E_TAG_TSHIFT);
    doc_timeshift.setAttribute(T_ATTR_START, static_cast<int>((mTimeLimits.t0() - mOriginalTimeLimits.t0()).hours()));
    doc_timeshift.setAttribute(T_ATTR_END,   static_cast<int>((mTimeLimits.t1() - mOriginalTimeLimits.t1()).hours()));
    doc_changes.appendChild(doc_timeshift);
  }

  METLIBS_LOG_DEBUG("changes=" << doc.toString());
  return doc.toString().toStdString();
}

void TimeSeriesView::replay(const std::string& changesText)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG("replaying " << changesText);

  QDomDocument doc;
  doc.setContent(QString::fromStdString(changesText));

  const QDomElement doc_changes = doc.documentElement();

  Sensor_v removed;
  const QDomElement doc_removed = doc_changes.firstChildElement(E_TAG_REMOVED);
  if (not doc_removed.isNull()) {
    for(QDomElement c = doc_removed.firstChildElement(E_TAG_COLUMN); not c.isNull(); c = c.nextSiblingElement(E_TAG_COLUMN)) {
      Sensor sensor;
      fromText(c, sensor);
      removed.push_back(sensor);
      METLIBS_LOG_DEBUG("removed " << sensor.stationId << ';' << sensor.paramId << ';' << sensor.typeId);
    }
  }

  Sensor_v actual;
  const QDomElement doc_actual = doc_changes.firstChildElement(E_TAG_COLUMNS);
  if (not doc_actual.isNull()) {
    for(QDomElement c = doc_actual.firstChildElement(E_TAG_COLUMN); not c.isNull(); c = c.nextSiblingElement(E_TAG_COLUMN)) {
      Sensor sensor;
      fromText(c, sensor);
      actual.push_back(sensor);
      METLIBS_LOG_DEBUG("sensor " << sensor.stationId << ';' << sensor.paramId << ';' << sensor.typeId);
    }
  }
  
  ChangeReplay<Sensor, lt_Sensor> cr;
  mSensors = cr.replay(mOriginalSensors, actual, removed);

  TimeSpan newTimeLimits(mOriginalTimeLimits);
  const QDomElement doc_timeshift = doc_changes.firstChildElement(E_TAG_TSHIFT);
  if (not doc_timeshift.isNull()) {
    const int dT0 = doc_timeshift.attribute(T_ATTR_START).toInt();
    const int dT1 = doc_timeshift.attribute(T_ATTR_END)  .toInt();
    const timeutil::ptime t0 = mOriginalTimeLimits.t0() + boost::posix_time::hours(dT0);
    const timeutil::ptime t1 = mOriginalTimeLimits.t1() + boost::posix_time::hours(dT1);
    TimeSpan newTimeLimits(t0, t1);
    METLIBS_LOG_DEBUG(LOGVAL(newTimeLimits));
  }
  mTimeLimits = newTimeLimits;
  updateTimeEditors();

  bool changed = (mSensors.size() != mOriginalSensors.size())
      or mTimeLimits != mOriginalTimeLimits;
  if (not changed)
    changed = not std::equal(mSensors.begin(), mSensors.end(), mOriginalSensors.begin(), eq_Sensor());
  mColumnReset->setEnabled(changed);
}

void TimeSeriesView::onActionAddColumn()
{
  METLIBS_LOG_SCOPE();
  TimeSeriesAdd ta(this);
  if (ta.exec() != QDialog::Accepted)
    return;

  const Sensor s = ta.selectedSensor();
  mSensors.push_back(s);
  METLIBS_LOG_DEBUG(s);
  
  mColumnReset->setEnabled(true);
  storeChanges();
  updateSensors();
}

void TimeSeriesView::onActionRemoveColumns()
{
  METLIBS_LOG_SCOPE();
  QStringList lines;
  for (const Sensor& s : mSensors)
    lines << QString("%1 P%2 T%3 L%4 S%5").arg(s.stationId).arg(s.paramId)
      .arg(s.typeId).arg(s.level).arg(s.sensor);

  QDialog tr(this);
  Ui::TimeSeriesRemove tr_ui;
  tr_ui.setupUi(&tr);
  tr_ui.tableLines->setModel(new QStringListModel(lines, &tr));

  if (tr.exec() != QDialog::Accepted)
    return;

  const QModelIndexList selected = tr_ui.tableLines->selectionModel()->selectedRows();
  METLIBS_LOG_DEBUG(LOGVAL(selected.size()));
  for (const QModelIndex& s : boost::adaptors::reverse(selected)) {
    Sensor_v::iterator it = mSensors.begin() + s.row();
    METLIBS_LOG_DEBUG(LOGVAL(*it));
    mSensors.erase(it);
  }
  
  mColumnReset->setEnabled(true);
  storeChanges();
  updateSensors();
}

void TimeSeriesView::onActionResetColumns()
{
  mTimeLimits = mOriginalTimeLimits;
  mSensors = mOriginalSensors;
  mColumnReset->setEnabled(false);
  storeChanges();
  updateTimeEditors();
  updateSensors();
}

void TimeSeriesView::onRadioPlot()
{
  METLIBS_LOG_SCOPE();
  updateTime();
}

void TimeSeriesView::updateTimeEditors()
{
  mChangingTimes = true;
  ui->timeFrom->setDateTime(timeutil::to_QDateTime(mTimeLimits.t0()));
  ui->timeTo  ->setDateTime(timeutil::to_QDateTime(mTimeLimits.t1()));
  mChangingTimes = false;
}

void TimeSeriesView::setTimeSpan(const TimeSpan& t)
{
  METLIBS_LOG_SCOPE();
  if (mChangingTimes or t == mTimeLimits)
    return;

  mTimeLimits = t;
  storeChanges();
  updateTime();
}

void TimeSeriesView::onDateFromChanged(const QDateTime&)
{
  const timeutil::ptime stime = timeutil::from_QDateTime(ui->timeFrom->dateTime());
  if (stime == mTimeLimits.t0())
    return;

  const timeutil::ptime etime = timeutil::from_QDateTime(ui->timeTo  ->dateTime());
  setTimeSpan(TimeSpan(stime, etime));
}

void TimeSeriesView::onDateToChanged(const QDateTime&)
{
  const timeutil::ptime etime = timeutil::from_QDateTime(ui->timeTo  ->dateTime());
  if (etime == mTimeLimits.t1())
    return;

  const timeutil::ptime stime = timeutil::from_QDateTime(ui->timeFrom->dateTime());
  setTimeSpan(TimeSpan(stime, etime));
}

void TimeSeriesView::updateTime()
{
  METLIBS_LOG_TIME();
  if (not mDA)
    return;

  mObsBuffer = std::make_shared<TimeBuffer>(Sensor_s(mSensors.begin(), mSensors.end()), mTimeLimits);
  connect(mObsBuffer.get(), SIGNAL(bufferCompleted(const QString&)), this, SLOT(updatePlot()));
  connect(mObsBuffer.get(), SIGNAL(newDataEnd(const ObsData_pv&)), this, SLOT(updatePlot()));
  connect(mObsBuffer.get(), SIGNAL(updateDataEnd(const ObsData_pv&)), this, SLOT(updatePlot()));
  connect(mObsBuffer.get(), SIGNAL(dropDataEnd(const SensorTime_v&)), this, SLOT(updatePlot()));
  mBusy->setBusy(true);
  mObsBuffer->postRequest(mDA);
}

void TimeSeriesView::updatePlot()
{
  METLIBS_LOG_SCOPE();
  mBusy->setBusy(false);
  if (!mObsBuffer || mPlotOptions.empty() || !mSensorTime.valid())
    return;

  const int whatToPlot = ui->comboWhatToPlot->currentIndex();

  METLIBS_LOG_DEBUG(LOGVAL(mTimeLimits) << LOGVAL(whatToPlot));

  TimeSeriesData::tsList tslist;

  int idx = -1;
  for (const Sensor& sensor : mSensors) {
    idx += 1;
    if (idx >= (int)mPlotOptions.size()) {
      HQC_LOG_ERROR("only " << mPlotOptions.size() << " plotoptions for " << mSensors.size()
          << " sensors,, idx " << idx << " is invalid, mST=" << mSensorTime << " s=" << sensor);
      break;
    }

    const Time_s times = mObsBuffer->times();
    METLIBS_LOG_DEBUG(LOGVAL(sensor) << LOGVAL(times.size()));
    if (times.empty())
      continue;

    TimeSeriesData::TimeSeries tseries;
    tseries.stationid(sensor.stationId);
    tseries.paramid(sensor.paramId);
    tseries.plotoptions(mPlotOptions[idx]);

    for (const timeutil::ptime& time : times) {
      const SensorTime st(sensor, time);
      ObsData_p obs;
      ModelData_p mdl;

      float value = 0;
      if (whatToPlot == 0 or whatToPlot == 2) {
        obs = mObsBuffer->get(st);
        if (not obs)
          continue;
        value = obs->corrected();
        if (value <= kvalobs::REJECTED)
          continue;
        value = Helpers::numericalValue(sensor, value);
        if (KvMetaDataBuffer::instance()->checkPhysicalLimits(st, value) == CachedParamLimits::OutsideMinMax)
          continue;
      }

      if (whatToPlot == 1 or whatToPlot == 2) {
        mdl = mModelBuffer->get(st);
        if (not mdl)
          continue;
        const float mv = mdl->value();
        value = (whatToPlot == 1) ? mv : (value - mv);
      }
      
      const miutil::miTime mtime = timeutil::make_miTime(time);
      tseries.add(TimeSeriesData::Data(mtime, value));
    }

    if (tseries.dataOK()) {
      tslist.insert(tslist.begin(), tseries);
      if (tslist.size() >= MAX_LINES)
        break;
    }
  }
  
  if (tslist.empty())
    return; // FIXME otherwise segfault
   
  // finally: one complete plot-structure
  TimeSeriesData::TSPlot tsplot;
  POptions::PlotOptions poptions;
  poptions.time_types.push_back(POptions::T_DATE);
  poptions.time_types.push_back(POptions::T_DAY);
  poptions.time_types.push_back(POptions::T_HOUR);

  poptions.name = "";
  poptions.fillcolour= POptions::Colour("white");
  poptions.linecolour= POptions::Colour("black");
  poptions.linewidth= 1;
  tsplot.plotoptions(poptions); // set global plotoptions

  tsplot.tserieslist(tslist);      // set list of timeseries

  ui->plot->prepare(tsplot);

  highlightTime();
}

// static
void TimeSeriesView::initalizePlotOptions()
{
  if (sDefinedColours.empty()) {
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

    POptions::Colour::definedColours(sDefinedColours);
  }

  if (sDefinedLinetypes.empty()) {
    POptions::Linetype::define("full",0xFFFF,1);
    POptions::Linetype::define("dashed",0x00FF,1);
    POptions::Linetype::define("dashdotted",0x0C0F,1);
    POptions::Linetype::define("dashdashdotted",0x1C47,1);
    POptions::Linetype::define("dotted",0xAAAA,2);

    sDefinedLinetypes.push_back(POptions::Linetype("full"));
    sDefinedLinetypes.push_back(POptions::Linetype("dashed"));
    sDefinedLinetypes.push_back(POptions::Linetype("dotted"));
    sDefinedLinetypes.push_back(POptions::Linetype("dashdotted"));
    sDefinedLinetypes.push_back(POptions::Linetype("dashdashdotted"));
  }
}

std::vector<POptions::Colour>   TimeSeriesView::sDefinedColours;
std::vector<POptions::Linetype> TimeSeriesView::sDefinedLinetypes;
