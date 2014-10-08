
#ifndef ACCESS_OBSFILTER_HH
#define ACCESS_OBSFILTER_HH 1

#include "ObsData.hh"
#include "TimeSpan.hh"

#include <string>

class ObsFilter;
HQC_TYPEDEF_P(ObsFilter);

class ObsFilter : HQC_SHARED_NOCOPY(ObsFilter) {
public:
  virtual ~ObsFilter();

  virtual QString acceptingSql(const std::string& data_alias, const TimeSpan& time) const;
  virtual QString acceptingSqlExtraTables(const std::string& data_alias, const TimeSpan& time) const;

  virtual bool needsSQL() const;

  /* \param afterSQL true if accept is called after running the SQL
   * contraints from acceptingSQL(...); if false, acceptingSQL may
   * have run or not */
  virtual bool accept(ObsData_p obs, bool afterSQL) const;

  virtual bool subsetOf(ObsFilter_p other) const;
};

#endif // ACCESS_OBSFILTER_HH
