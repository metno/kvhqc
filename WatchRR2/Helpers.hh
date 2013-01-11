
#ifndef HELPERS_HH
#define HELPERS_HH 1

#include "Sensor.hh"
#include <QtCore/QString>

class FlagChange;
class ObsData;
typedef boost::shared_ptr<ObsData> ObsDataPtr;
class EditDataEditor;
typedef boost::shared_ptr<EditDataEditor> EditDataEditorPtr;
namespace kvalobs {
class kvControlInfo;
class kvData;
class kvModelData;
}

namespace Helpers {

struct float_eq : public std::binary_function<float, float, bool>
{
    bool operator()(float a, float b) const;
};

enum { NO, BEFORE_REDIST, QC2_REDIST, HQC_REDIST };
int is_accumulation(ObsDataPtr obs);
int is_endpoint(ObsDataPtr obs);

bool is_rejected(ObsDataPtr obs);
bool is_missing(ObsDataPtr obs);
bool is_orig_missing(ObsDataPtr obs);

void reject(EditDataEditorPtr editor);
void correct(EditDataEditorPtr editor, float newC);

char int2char(int i);

QString getFlagText(const kvalobs::kvControlInfo& cInfo);
QString getFlagExplanation(const kvalobs::kvControlInfo& cInfo);

QString parameterName(int paramId);

int kvSensorNumber(const kvalobs::kvData& d);
Sensor sensorFromKvData(const kvalobs::kvData& d);
SensorTime sensorTimeFromKvData(const kvalobs::kvData& d);
SensorTime sensorTimeFromKvModelData(const kvalobs::kvModelData& d);
Sensor modelSensor(const Sensor& sensor);

void updateUseInfo(kvalobs::kvData& data);
void updateCfailed(kvalobs::kvData& data, const std::string& add);

QString appendText(QString& text, const QString& append, const QString& separator = ", ");
QString appendedText(const QString& text, const QString& append, const QString& separator = ", ");

double distance(double lon1, double lat1, double lon2, double lat2);

} // namespace Helpers

namespace kvalobs {

enum {
    PARAMID_RR = 110,
    PARAMID_SD = 18,
    PARAMID_SA = 112,
    PARAMID_V4 = 34,
    PARAMID_V5 = 36,
    PARAMID_V6 = 38
};

enum { REJECTED = -32766, MISSING = -32767, NEW_ROW = -32768 };

inline QString formatValue(float v)
{ return QString::number(v, 'f', 1); }

inline bool same_sensor(int sensor1, int sensor2)
{ int diff = sensor1 - sensor2; return diff == 0 or diff == '0' or diff == -('0'); }

} // namespace kvalobs

#endif // HELPERS_HH
