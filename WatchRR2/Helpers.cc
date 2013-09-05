
#include "Helpers.hh"

#include "EditAccess.hh"
#include "EditData.hh"
#include "EditDataEditor.hh"
#include "FlagChange.hh"
#include "HqcApplication.hh"
#include "KvMetaDataBuffer.hh"
#include "ModelAccess.hh"
#include "timeutil.hh"

#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvModelData.h>
#include <kvalobs/kvObsPgm.h>

#include <QtCore/QVariant>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <boost/foreach.hpp>

#include <sstream>

#define M_TIME
#define MILOGGER_CATEGORY "kvhqc.Helpers"
#include "HqcLogging.hh"

namespace {
static const char* flagnames[16] = {
    "fagg", "fr", "fcc", "fs", "fnum", 
    "fpos", "fmis", "ftime", "fw", "fstat", 
    "fcp", "fclim", "fd", "fpre", "fcombi", "fhqc" 
};
}

namespace Helpers {

bool float_eq::operator()(float a, float b) const
{
    return std::fabs(a - b) < 0.01f;
}

int kvSensorNumber(const kvalobs::kvData& d)
{
    const int s = d.sensor();
    return (s>='0') ? (s-'0') : s;
}

Sensor sensorFromKvData(const kvalobs::kvData& d)
{
    return Sensor(d.stationID(), d.paramID(), d.level(), kvSensorNumber(d), d.typeID());
}

SensorTime sensorTimeFromKvData(const kvalobs::kvData& d)
{
    return SensorTime(sensorFromKvData(d), timeutil::from_miTime(d.obstime()));
}

Sensor sensorFromKvModelData(const kvalobs::kvModelData& d)
{
    return Sensor(d.stationID(), d.paramID(), d.level(), ModelAccess::MODEL_SENSOR, ModelAccess::MODEL_TYPEID);
}

SensorTime sensorTimeFromKvModelData(const kvalobs::kvModelData& d)
{
    return SensorTime(sensorFromKvModelData(d), timeutil::from_miTime(d.obstime()));
}

// ------------------------------------------------------------------------

int is_accumulation(const kvalobs::kvControlInfo& ci)
{
    using namespace kvalobs::flag;
    const int f_fd = ci.flag(fd);
    if( f_fd == 2 or f_fd == 4 )
        return BEFORE_REDIST;
    if( f_fd == 7 or f_fd == 8 )
        return QC2_REDIST;
    if( f_fd == 9 or f_fd == 0xA )
        return HQC_REDIST;
    return NO;
}

int is_accumulation(ObsDataPtr obs)
{ return is_accumulation(obs->controlinfo()); }

int is_accumulation(EditDataEditorPtr editor)
{ return is_accumulation(editor->controlinfo()); }

// ------------------------------------------------------------------------

int is_endpoint(const kvalobs::kvControlInfo& ci)
{
    using namespace kvalobs::flag;
    const int f_fd = ci.flag(fd);
    if( f_fd == 4 )
        return BEFORE_REDIST;
    if( f_fd == 8 )
        return QC2_REDIST;
    if( f_fd == 0xA )
        return HQC_REDIST;
    return NO;
}

int is_endpoint(ObsDataPtr obs)
{ return is_endpoint(obs->controlinfo()); }

int is_endpoint(EditDataEditorPtr editor)
{ return is_endpoint(editor->controlinfo()); }

// ------------------------------------------------------------------------

bool is_rejected(const kvalobs::kvControlInfo& ci, float corr)
{
    return (ci.flag(kvalobs::flag::fmis) == 2) // same as kvDataOperations.cc
        or (corr == kvalobs::REJECTED);
}

bool is_rejected(ObsDataPtr obs)
{ return is_rejected(obs->controlinfo(), obs->corrected()); }

bool is_rejected(EditDataEditorPtr editor)
{ return is_rejected(editor->controlinfo(), editor->corrected()); }

// ------------------------------------------------------------------------

bool is_missing(const kvalobs::kvControlInfo& ci, float corr)
{
    return (ci.flag(kvalobs::flag::fmis) == 3) // same as kvDataOperations.cc
        or (corr == kvalobs::MISSING);
}

bool is_missing(ObsDataPtr obs)
{ return is_missing(obs->controlinfo(), obs->corrected()); }

bool is_missing(EditDataEditorPtr editor)
{ return is_missing(editor->controlinfo(), editor->corrected()); }

// ------------------------------------------------------------------------

bool is_orig_missing(const kvalobs::kvControlInfo& ci, float orig)
{
    return (ci.flag(kvalobs::flag::fmis) & 1) // same as kvDataOperations.cc
        or (orig == kvalobs::MISSING);
}

bool is_orig_missing(ObsDataPtr obs)
{ return is_missing(obs->controlinfo(), obs->original()); }

bool is_orig_missing(EditDataEditorPtr editor)
{ return is_orig_missing(editor->controlinfo(), editor->obs()->original()); }

// ------------------------------------------------------------------------

bool is_valid(ObsDataPtr obs) // same as kvDataOperations.cc
{ return not is_missing(obs) and not is_rejected(obs); }

bool is_valid(EditDataEditorPtr editor) // same as kvDataOperations.cc
{ return not is_missing(editor) and not is_rejected(editor); }

// ------------------------------------------------------------------------

void reject(EditDataEditorPtr editor) // same as kvDataOperations.cc
{
    if (not is_valid(editor))
        return;
    
    const FlagChange fc_reject("fmis=[04]->fmis=2;fmis=1->fmis=3;fhqc=A");
    editor->changeControlinfo(fc_reject);
    if (is_orig_missing(editor))
        editor->setCorrected(kvalobs::MISSING);
    else
        editor->setCorrected(kvalobs::REJECTED);
}

void correct(EditDataEditorPtr editor, float newC)
{
    const FlagChange fc_diff("fmis=3->fmis=1;fmis=[02]->fmis=4;fhqc=7");
    editor->changeControlinfo(fc_diff);
    editor->setCorrected(newC);
}

void set_flag(EditDataEditorPtr editor, int flag, int value)
{
    kvalobs::kvControlInfo ci = editor->controlinfo();
    ci.set(flag, value);
    editor->setControlinfo(ci);
}

void set_fhqc(EditDataEditorPtr editor, int fhqc)
{
    set_flag(editor, kvalobs::flag::fhqc, fhqc);
}

void auto_correct(EditDataEditorPtr editor, float newC)
{
    const bool interpolation = is_orig_missing(editor);
    correct(editor, newC);
    if (interpolation)
        set_fhqc(editor, 5);
}

// ========================================================================

char int2char(int i)
{
    if( i<10 )
        return ('0' + i);
    else
        return ('A' + (i-10));
}

static QString formatFlag(const kvalobs::kvControlInfo & cInfo, bool explain)
{
  METLIBS_LOG_TIME();
  std::auto_ptr<QSqlQuery> query;
  if (explain and hqcApp) {
    query.reset(new QSqlQuery(hqcApp->systemDB()));
    query->prepare("SELECT description FROM flag_explain WHERE flag = :fn AND flagvalue = :fv AND language = 'nb'");
  }

    std::ostringstream ss;
    bool first = true;

    const int showFlagAbove[16] = { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0 };
    for(int f=0; f<16; f++) {
	const int flag = cInfo.flag(f);
        using namespace kvalobs::flag;
	if (flag > showFlagAbove[f]) {
            if( not first )
                ss << (query.get() ? '\n' : ' ');
            ss << flagnames[f] << '=' << int2char(flag);
            if (query.get()) {
              query->bindValue("fn", f);
              query->bindValue("fv", flag);
              query->exec();
              QString explanation;
              if (query->next())
                explanation = query->value(0).toString();
              else
                explanation = qApp->translate("Helpers", "Unknown or invalid flag value");
              query->finish();
              ss << ": " << explanation.toStdString();
            }
            first = false;
	}
    }
    return QString::fromStdString(ss.str());
}


QString getFlagText(const kvalobs::kvControlInfo & cInfo)
{
    return formatFlag(cInfo, false);
}

QString getFlagExplanation(const kvalobs::kvControlInfo & cInfo)
{
    return formatFlag(cInfo, true);
}

QString getFlagName(int flagNumber)
{
    if (flagNumber < 0 or flagNumber >= 16)
        return "";
    return flagnames[flagNumber];
}

QString parameterName(int paramId)
{
    return QString::fromStdString(KvMetaDataBuffer::instance()->findParamName(paramId));
}

int parameterIdByName(const std::string& paramName)
{
  try {
    const std::list<kvalobs::kvParam>& allParams = KvMetaDataBuffer::instance()->allParams();
    BOOST_FOREACH(const kvalobs::kvParam& p, allParams) {
      if (p.name() == paramName)
        return p.paramID();
    }
  } catch (std::exception& e) {
    HQC_LOG_ERROR("exception while fetching params: " << e.what());
  }
  return -1;
}

void updateUseInfo(kvalobs::kvData& data)
{
    kvalobs::kvUseInfo ui = data.useinfo();
    ui.setUseFlags( data.controlinfo() );
    data.useinfo( ui );
}

QString& appendText(QString& text, const QString& append, const QString& separator)
{
    if (append.isEmpty())
        return text;
    if (not text.isEmpty())
        text += separator;
    text += append;
    return text;
}

QString appendedText(const QString& text, const QString& append, const QString& separator)
{
    QString t(text);
    appendText(t, append, separator);
    return t;
}

double distance(double lon1, double lat1, double lon2, double lat2)
{
    const double DEG_RAD = M_PI/180, EARTH_RADIUS = 6371.0;
    const double delta_lon=(lon1 - lon2)*DEG_RAD, slon = sin(delta_lon/2);
    const double delta_lat=(lat1 - lat2)*DEG_RAD, slat = sin(delta_lat/2);
    const double a = slat*slat + cos(lat1*DEG_RAD)*cos(lat2*DEG_RAD)*slon*slon;
    const double c =2.0 * atan2(sqrt(a), sqrt(1-a));
    return EARTH_RADIUS*c;
}

float round(float f, float factor)
{
    f *= factor;
    if (f < 0.0f)
        f -= 0.5;
    else
        f += 0.5;
    float ff = 0;
    modff(f, &ff);
    return ff / factor;
}

float roundDecimals(float f, int decimals)
{
    return round(f, std::pow(10, decimals));
}

float parseFloat(const QString& text, int nDecimals)
{
    bool numOk = false;
    const float num = text.toFloat(&numOk);
    if (not numOk)
        throw std::runtime_error("cannot parse number");
    if (not Helpers::float_eq()(num, Helpers::roundDecimals(num, nDecimals))) {
        std::ostringstream w;
        w << "text '" << text.toStdString() << "' converted to value " << num
          << " has unsupported precision (rounded value is "
          << Helpers::roundDecimals(num, nDecimals) << ")";
        throw std::runtime_error(w.str());
    }
    return num;
}

int extract_ui2(ObsDataPtr obs)
{
  kvalobs::kvUseInfo ui;
  ui.setUseFlags(obs->controlinfo());
  return ui.flag(2);
}

QString stationName(const kvalobs::kvStation& s)
{
  return QString::fromLatin1(s.name().c_str());
}

bool aggregatedParameter(int paramFrom, int paramTo)
{
  std::set<int> pTo;
  aggregatedParameters(paramFrom, pTo);
  return pTo.find(paramTo) != pTo.end();
}

void aggregatedParameters(int paramFrom, std::set<int>& paramTo)
{
  METLIBS_LOG_SCOPE();
  if (hqcApp) {
    QSqlQuery query(hqcApp->systemDB());
    query.prepare("SELECT paramid_to FROM param_aggregated WHERE paramid_from = ?");
    query.bindValue(0, paramFrom);
    if (query.exec()) {
      while (query.next())
        paramTo.insert(query.value(0).toInt());
    } else {
      HQC_LOG_WARN("error getting aggregated parameters for " << paramFrom
          << ": " << query.lastError().text());
    }
  }
}

void addNeighbors(std::vector<Sensor>& neighbors, const Sensor& sensor, const TimeRange& time, int maxNeighbors)
{
  METLIBS_LOG_SCOPE();

  if (not hqcApp) {
    METLIBS_LOG_INFO("no KvApp -- no neighbors, probably running a test program");
    return;
  }
  
  const std::list<kvalobs::kvStation>& stationsList = KvMetaDataBuffer::instance()->allStations();
  
  std::vector<kvalobs::kvStation> stations;
  try {
    Helpers::stations_by_distance ordering(KvMetaDataBuffer::instance()->findStation(sensor.stationId));
    
    BOOST_FOREACH(const kvalobs::kvStation& s, stationsList) {
      const int sid = s.stationID();
      if (sid < 60 or sid >= 100000 or sid == sensor.stationId)
        continue;
      if (ordering.distance(s) > 100 /*km*/)
        continue;
      stations.push_back(s);
      METLIBS_LOG_DEBUG(LOGVAL(s.stationID()));
    }
    std::sort(stations.begin(), stations.end(), ordering);
  } catch (std::exception& e) {
    METLIBS_LOG_WARN("exception while searching neighbor stations: " << e.what());
    return;
  }
  
  int count = 0;
  BOOST_FOREACH(const kvalobs::kvStation& s, stations) {
    const std::list<kvalobs::kvObsPgm>& obs_pgm = KvMetaDataBuffer::instance()->findObsPgm(s.stationID());
    BOOST_FOREACH (const kvalobs::kvObsPgm& op, obs_pgm) {
      if (time.intersection(TimeRange(op.fromtime(), op.totime())).undef())
        continue;
      // FIXME this is not correct if there is more than one klXX or collector or typeid or ...
      
      const bool eql = op.paramID() == sensor.paramId;
      const bool agg = aggregatedParameter(op.paramID(), sensor.paramId);
      if (eql or agg) {
        const int typeId = eql ? op.typeID() : -op.typeID();
        const Sensor n(s.stationID(), sensor.paramId, op.level(), 0, typeId);
        const bool duplicate = (std::find_if(neighbors.begin(), neighbors.end(), std::bind1st(eq_Sensor(), n)) != neighbors.end());
        METLIBS_LOG_DEBUG(LOGVAL(n) << LOGVAL(duplicate));
        neighbors.push_back(n);
        if (++count >= maxNeighbors)
          break;
      }
    }
    if (count >= maxNeighbors)
      break;
  }
}

float numericalValue(int paramId, float codeValue)
{
  if (paramId == kvalobs::PARAMID_RR_24 and codeValue == -1.0)
    return 0.0;
  return codeValue;
}

typedef std::vector<Sensor> Sensors_t;
Sensors_t relatedSensors(const SensorTime& st, const std::string& viewType)
{
  METLIBS_LOG_TIME();
  const Sensor& s = st.sensor;

  std::vector<int> stationPar, neighborPar;
  int nNeighbors = 8;

  if (hqcApp) {
    QSqlQuery query(hqcApp->systemDB());
    query.prepare("SELECT pr1.paramid FROM param_related AS pr1"
        " WHERE pr1.groupid = (SELECT pr2.groupid FROM param_related AS pr2 WHERE pr2.paramid = :pid)"
        "   AND (pr1.view_types_excluded IS NULL OR pr1.view_types_excluded NOT LIKE :vt)"
        " ORDER BY pr1.sortkey");

    query.bindValue(":pid", s.paramId);
    query.bindValue(":vt",  "%" + QString::fromStdString(viewType) + "%");
    query.exec();
    while (query.next())
      stationPar.push_back(query.value(0).toInt());
  }
#if 1
  METLIBS_LOG_DEBUG("found " << stationPar.size() << " station pars for " << LOGVAL(st) << LOGVAL(viewType));
  BOOST_FOREACH(int pid, stationPar) {
    METLIBS_LOG_DEBUG(LOGVAL(pid));
  }
#endif

  if (stationPar.empty())
    stationPar.push_back(s.paramId);
  if (neighborPar.empty())
    neighborPar.push_back(s.paramId);

  Sensors_t sensors;
  Sensor s2(s);
  BOOST_FOREACH(int par, stationPar) {
    s2.paramId = par;
    sensors.push_back(s2);
  }
  BOOST_FOREACH(int par, neighborPar) {
    Sensor sn(s);
    sn.paramId = par;
    addNeighbors(sensors, sn, TimeRange(st.time, st.time), nNeighbors);
  }
#if 0
  METLIBS_LOG_DEBUG("found " << sensors.size() << " default sensors for " << LOGVAL(st) << LOGVAL(viewType));
  BOOST_FOREACH(const Sensor& ds, sensors) {
    METLIBS_LOG_DEBUG(LOGVAL(ds));
  }
#endif
  return sensors;
}

bool askDiscardChanges(int nupdates, QWidget* parent)
{
  if (nupdates == 0)
    return true;

  QMessageBox w(parent);
  w.setWindowTitle(parent->windowTitle());
  w.setIcon(QMessageBox::Warning);
  w.setText(qApp->translate("Helpers", "There are %1 unsaved data updates.").arg(nupdates));
  w.setInformativeText(qApp->translate("Helpers", "Are you sure that you want to lose them?"));
  QPushButton* discard = w.addButton(qApp->translate("Helpers", "Discard changes"), QMessageBox::ApplyRole);
  QPushButton* cont = w.addButton(qApp->translate("Helpers", "Continue"), QMessageBox::RejectRole);
  w.setDefaultButton(cont);
  w.exec();
  if (w.clickedButton() != discard)
    return false;
  return true;
}

} // namespace Helpers
