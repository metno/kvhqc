
#ifndef EXTREMESFILTER_HH
#define EXTREMESFILTER_HH 1

#include "common/KvTypedefs.hh"
#include "common/ObsFilter.hh"

class ExtremesFilter : public ObsFilter
{
public:
  ExtremesFilter(int paramid, int nExtremes);

  virtual QString acceptingSql(const QString& data_alias, const TimeSpan& time) const;
  virtual QString acceptingSqlExtraTables(const QString& data_alias, const TimeSpan& time) const;

  virtual bool needsSQL() const
    { return true; }

  virtual bool accept(ObsData_p obs, bool afterSQL) const;

  virtual bool subsetOf(ObsFilter_p other) const;

  bool isMaximumSearch() const
    { return mFindMaximum; }

private:
  QString findExcludedIds() const;

private:
  int mParamId;
  int mExtremesCount;

  hqc::int_s mParamIds;
  bool mFindMaximum;
};

HQC_TYPEDEF_P(ExtremesFilter);

#endif // EXTREMESFILTER_HH
