
#ifndef EXTREMESFILTER_HH
#define EXTREMESFILTER_HH 1

#include "common/ObsFilter.hh"

class ExtremesFilter : public ObsFilter
{
public:
  ExtremesFilter(int paramid, int nExtremes)
    : mParamId(paramid), mExtremesCount(nExtremes) { }

  virtual QString acceptingSql(const QString& data_alias, const TimeSpan& time) const;
  virtual QString acceptingSqlExtraTables(const QString& data_alias, const TimeSpan& time) const;

  virtual bool needsSQL() const
    { return true; }

  virtual bool accept(ObsData_p obs, bool afterSQL) const
    { return false; }

  bool subsetOf(ObsFilter_p other) const;

private:
  void prepareParams(QString& paramIds, QString& function, QString& ordering) const;

private:
  int mParamId;
  int mExtremesCount;
};

HQC_TYPEDEF_P(ExtremesFilter);

#endif // EXTREMESFILTER_HH
