
#ifndef HQC_SYSTEM_DB
#define HQC_SYSTEM_DB 1

#include "common/KvTypedefs.hh"

#include <QString>
#include <QStringList>

#include <map>

class HqcSystemDB {
public:
  static QString explainCheck(QString& check);
  static bool paramLimits(int paramid, float& low, float& high);
  static bool shownDecimals(int paramid, int& decimals);

  struct ParamCode {
    int value;
    QString longText;
    QStringList shortTexts;
  };
  typedef QList<ParamCode> ParamCode_ql;

  static ParamCode_ql paramCodes(int paramid);

  typedef std::map<int, int> station2prio_t;
  static station2prio_t stationPriorities();

  static hqc::int_s coastalStations();

  static QString remappedCounty(int countryid, int municip_code);

  static QString explainFlagValue(int fn, int fv);

  static hqc::int_v relatedParameters(int paramid, const QString& viewType);

  static void aggregatedParameters(int paramFrom, hqc::int_s& paramTo);

  struct Region {
    QString regionLabel;
    QStringList countyLabels;
    QStringList countyDbNames;
  };
  typedef QList<Region> Region_ql;

  static Region_ql regions();

  struct ParamGroup {
    QString label;
    hqc::int_v paramIds;
  };
  typedef QList<ParamGroup> ParamGroup_ql;

  static ParamGroup_ql paramGroups();
};

#endif // HQC_SYSTEM_DB
