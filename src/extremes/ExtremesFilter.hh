
#ifndef EXTREMESFILTER_HH
#define EXTREMESFILTER_HH 1

#include "common/ObsFilter.hh"

class ExtremesFilter : public ObsFilter
{
public:
  ExtremesFilter(int paramid)
    : mParamId(paramid) { }

  virtual QString acceptingSql(const QString& data_alias, const TimeSpan& time) const;
  virtual QString acceptingSqlExtraTables(const QString& data_alias, const TimeSpan& time) const;

  virtual bool needsSQL() const
    { return true; }

  virtual bool accept(ObsData_p obs, bool afterSQL) const
    { return false; }

  bool subsetOf(const ObsFilter& other) const
    { return false; }

private:
  void prepareParams(QString& paramIds, QString& function, QString& ordering) const;

private:
  int mParamId;
};

HQC_TYPEDEF_P(ExtremesFilter);

#endif // EXTREMESFILTER_HH
