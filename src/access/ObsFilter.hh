
#ifndef ACCESS_OBSFILTER_HH
#define ACCESS_OBSFILTER_HH 1

#include "ObsData.hh"

#include <string>

class ObsFilter : HQC_SHARED_NOCOPY(ObsFilter) {
public:
  virtual ~ObsFilter();

  virtual bool hasSQL() const;
  virtual std::string acceptingSQL(const std::string& data_alias) const;

  /* \param afterSQL true if accept is called after running the SQL
   * contraints from acceptingSQL(...); if false, acceptingSQL may
   * have run or not */
  virtual bool accept(ObsData_p obs, bool afterSQL) const;

  virtual bool subsetOf(const ObsFilter& other) const;
};

HQC_TYPEDEF_P(ObsFilter);

#endif // ACCESS_OBSFILTER_HH
