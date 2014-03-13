
#ifndef ACCESS_OBSFILTER_HH
#define ACCESS_OBSFILTER_HH 1

#include "ObsData.hh"

#include <string>

class ObsFilter : HQC_SHARED_NOCOPY(ObsFilter) {
public:
  virtual ~ObsFilter();

  virtual bool hasSQL() const;
  virtual std::string acceptingSQL(const std::string& data_alias) const;
  virtual bool accept(const SensorTime&, bool afterSQL) const;
  virtual bool accept(ObsData_p obs, bool afterSQL) const;

  virtual bool equals(const ObsFilter& other) const;

  bool operator == (const ObsFilter& other) const
    { return equals(other); }

  bool operator != (const ObsFilter& other) const
    { return not equals(other); }
};

HQC_TYPEDEF_P(ObsFilter);

#endif // ACCESS_OBSFILTER_HH
