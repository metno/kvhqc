
#ifndef ERRORS_ERRORFLITER_HH
#define ERRORS_ERRORFLITER_HH 1

#include "common/ObsFilter.hh"

class ErrorFilter : public ObsFilter
{
public:
  ErrorFilter(bool errorsForSalen);
  ~ErrorFilter();

  virtual QString acceptingSql(const QString& data_alias, const TimeSpan& time) const override;

  /* \param afterSQL true if accept is called after running the SQL
   * contraints from acceptingSQL(...); if false, acceptingSQL may
   * have run or not */
  virtual bool accept(ObsData_p obs, bool afterSQL) const override;

  bool subsetOf(ObsFilter_p other) const override;

private:
  bool mErrorsForSalen;
};

HQC_TYPEDEF_P(ErrorFilter);

#endif // ERRORS_ERRORFLITER_HH
