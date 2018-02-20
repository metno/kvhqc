
#ifndef EXTREMESFILTER_HH
#define EXTREMESFILTER_HH 1

#include "common/KvTypedefs.hh"
#include "common/ObsFilter.hh"

class ExtremesFilter : public ObsFilter
{
public:
  ExtremesFilter(int paramid, int nExtremes);

  QString acceptingSql(const QString& data_alias, const TimeSpan& time) const override;
  QString acceptingSqlExtraTables(const QString& data_alias, const TimeSpan& time) const override;

  bool needsSQL() const override
    { return true; }

  bool accept(ObsData_p obs, bool afterSQL) const override;

  bool subsetOf(ObsFilter_p other) const override;

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
