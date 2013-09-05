
#ifndef HELPERS_HH
#define HELPERS_HH 1

#include <Functors.hh>
#include "Sensor.hh"
#include <kvalobs/kvStation.h>
#include <QtCore/QString>

#include <set>

class FlagChange;
class ObsData;
typedef boost::shared_ptr<ObsData> ObsDataPtr;
class TimeRange;
class EditDataEditor;
typedef boost::shared_ptr<EditDataEditor> EditDataEditorPtr;
namespace kvalobs {
class kvControlInfo;
class kvData;
class kvModelData;
}

namespace Helpers {

enum { NO, BEFORE_REDIST, QC2_REDIST, HQC_REDIST };
int is_accumulation(ObsDataPtr obs);
int is_accumulation(EditDataEditorPtr editor);
int is_endpoint(ObsDataPtr obs);
int is_endpoint(EditDataEditorPtr editor);

bool is_rejected(ObsDataPtr obs);
bool is_rejected(EditDataEditorPtr editor);
bool is_missing(ObsDataPtr obs);
bool is_missing(EditDataEditorPtr editor);
bool is_orig_missing(ObsDataPtr obs);
bool is_orig_missing(EditDataEditorPtr editor);

void reject(EditDataEditorPtr editor);
void correct(EditDataEditorPtr editor, float newC);
void auto_correct(EditDataEditorPtr editor, float newC);

void set_flag(EditDataEditorPtr editor, int flag, int value);
void set_fhqc(EditDataEditorPtr editor, int fhqc);

char int2char(int i);

QString getFlagText(const kvalobs::kvControlInfo& cInfo);
QString getFlagExplanation(const kvalobs::kvControlInfo& cInfo);
QString getFlagName(int flagNumber);

QString parameterName(int paramId);

int kvSensorNumber(const kvalobs::kvData& d);
Sensor sensorFromKvData(const kvalobs::kvData& d);
SensorTime sensorTimeFromKvData(const kvalobs::kvData& d);
SensorTime sensorTimeFromKvModelData(const kvalobs::kvModelData& d);
Sensor modelSensor(const Sensor& sensor);

void updateUseInfo(kvalobs::kvData& data);

QString& appendText(QString& text, const QString& append, const QString& separator = ", ");
QString appendedText(const QString& text, const QString& append, const QString& separator = ", ");

double distance(double lon1, double lat1, double lon2, double lat2);

float round(float f, float factor);
float roundDecimals(float f, int decimals);
float parseFloat(const QString& text, int nDecimals);

int extract_ui2(ObsDataPtr obs);

QString stationName(const kvalobs::kvStation& s);

struct stations_by_distance : public std::binary_function<bool, kvalobs::kvStation, kvalobs::kvStation> {
    stations_by_distance(const kvalobs::kvStation& c) : center(c) { }
    bool operator()(const kvalobs::kvStation& a, const kvalobs::kvStation& b) const {
        if (a.stationID() == b.stationID())
            return false;
        return distance(a) < distance(b);
    }
    float distance(const kvalobs::kvStation& neighbor) const {
        return Helpers::distance(center.lon(), center.lat(), neighbor.lon(), neighbor.lat());
    }
private:
    const kvalobs::kvStation& center;
};

struct stations_by_id : public std::unary_function<bool, kvalobs::kvStation> {
    stations_by_id(int s) : stationid(s) { }
    bool operator()(const kvalobs::kvStation& a) const {
        return (a.stationID() == stationid);
    }
private:
    int stationid;
};

std::vector<Sensor> findNeighbors(const Sensor& sensor, const TimeRange& time, int maxNeighbors);
bool aggregatedParameter(int paramFrom, int paramTo);
void aggregatedParameters(int paramFrom, std::set<int>& paramTo);
std::vector<Sensor> relatedSensors(const SensorTime& st, const std::string& viewType);

float numericalValue(int paramId, float codeValue);
inline float numericalValue(const Sensor& sensor, float codeValue)
{ return numericalValue(sensor.paramId, codeValue); }
inline float numericalValue(const SensorTime& st, float codeValue)
{ return numericalValue(st.sensor.paramId, codeValue); }

bool askDiscardChanges(int nupdates, QWidget* parent);

} // namespace Helpers

namespace kvalobs {

enum {
    PARAMID_SD = 18,
    PARAMID_V4 = 34,
    PARAMID_V4S = 35,
    PARAMID_V5 = 36,
    PARAMID_V5S = 37,
    PARAMID_V6 = 38,
    PARAMID_V6S = 39,
    PARAMID_DD = 61,
    PARAMID_FF = 81,
    PARAMID_DD_02 = 62,
    PARAMID_FF_02 = 82,
    PARAMID_FG = 83,
    PARAMID_FX = 86,
    PARAMID_RR_01 = 105,
    PARAMID_RR_1  = 106,
    PARAMID_RR_3  = 107,
    PARAMID_RR_6  = 108,
    PARAMID_RR_12 = 109,
    PARAMID_RR_24 = 110,
    PARAMID_SA  = 112,
    PARAMID_TA  = 211,
    PARAMID_TAN = 213,
    PARAMID_TAX = 215,
    PARAMID_TD  = 217,
    PARAMID_UU  = 262,
    PARAMID_VV  = 273,
    PARAMID_MLAT = 401,
    PARAMID_MLON = 402,
    PARAMID_DD_01 = 10061,
    PARAMID_FF_01 = 10081
};

enum { REJECTED = -32766, MISSING = -32767, NEW_ROW = -32768 };

inline QString formatValue(float v)
{ return QString::number(v, 'f', 1); }

inline bool same_sensor(int sensor1, int sensor2)
{ int diff = sensor1 - sensor2; return diff == 0 or diff == '0' or diff == -('0'); }

} // namespace kvalobs

#endif // HELPERS_HH
