
#ifndef LIB_KVHELPERS_HH
#define LIB_KVHELPERS_HH 1

#include "Sensor.hh"

#include <set>
#include <vector>

namespace kvalobs {
class kvControlInfo;
class kvData;
class kvModelData;
class kvParam;
class kvStation;
class kvTypes;
class kvUseInfo;
} // namespace kvalobs
class TimeRange;

// ------------------------------------------------------------------------

namespace Helpers {

int is_accumulation(const kvalobs::kvControlInfo& ci);
int is_endpoint(const kvalobs::kvControlInfo& ci);
bool is_rejected(const kvalobs::kvControlInfo& ci, float corr);
bool is_missing(const kvalobs::kvControlInfo& ci, float corr);
bool is_orig_missing(const kvalobs::kvControlInfo& ci, float orig);

enum { NO, BEFORE_REDIST, QC2_REDIST, HQC_REDIST };

inline bool is_accumulation(int fd)
{ return fd==2 or fd>=4; }

QString getFlagText(const kvalobs::kvControlInfo& cInfo);
QString getFlagExplanation(const kvalobs::kvControlInfo& cInfo);
QString getFlagName(int flagNumber);

int kvSensorNumber(const kvalobs::kvData& d);
Sensor sensorFromKvData(const kvalobs::kvData& d);
SensorTime sensorTimeFromKvData(const kvalobs::kvData& d);
SensorTime sensorTimeFromKvModelData(const kvalobs::kvModelData& d);
Sensor modelSensor(const Sensor& sensor);

void updateUseInfo(kvalobs::kvData& data);

struct stations_by_distance : public std::binary_function<bool, kvalobs::kvStation, kvalobs::kvStation>
{
  stations_by_distance(const kvalobs::kvStation& c) : center(c) { }
  bool operator()(const kvalobs::kvStation& a, const kvalobs::kvStation& b) const;
  float distance(const kvalobs::kvStation& neighbor) const;

private:
  const kvalobs::kvStation& center;
};

struct stations_by_id : public std::unary_function<bool, kvalobs::kvStation>
{
  stations_by_id(int s) : stationid(s) { }
  bool operator()(const kvalobs::kvStation& a) const;

private:
  int stationid;
};

int nearestStationId(float lon, float lat, float maxDistanceKm = 10);

void addNeighbors(std::vector<Sensor>& neighbors, const Sensor& center, const TimeRange& time, int maxNeighbors);

bool aggregatedParameter(int paramFrom, int paramTo);
void aggregatedParameters(int paramFrom, std::set<int>& paramTo);
std::vector<Sensor> relatedSensors(const Sensor& s, const TimeRange& time, const std::string& viewType);

float numericalValue(int paramId, float codeValue);
inline float numericalValue(const Sensor& sensor, float codeValue)
{ return numericalValue(sensor.paramId, codeValue); }
inline float numericalValue(const SensorTime& st, float codeValue)
{ return numericalValue(st.sensor.paramId, codeValue); }

inline QString formatValue(float v)
{ return QString::number(v, 'f', 1); }

inline bool same_sensor(int sensor1, int sensor2)
{ int diff = sensor1 - sensor2; return diff == 0 or diff == '0' or diff == -('0'); }

void updateCfailed(kvalobs::kvData& data, const std::string& add);

QString paramName(int paramId);
QString stationName(const kvalobs::kvStation& s);

QString paramInfo(int paramId);
QString typeInfo(int typeId);
QString stationInfo(int stationId);

} // namespace Helpers

// ########################################################################

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

} // namespace kvalobs

#endif // LIB_KVHELPERS_HH
