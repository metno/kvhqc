
#include "ObsFilter.hh"

ObsFilter::~ObsFilter()
{
}

QString ObsFilter::acceptingSql(const std::string&, const TimeSpan&) const
{
  return QString();
}

QString ObsFilter::acceptingSqlExtraTables(const std::string&, const TimeSpan&) const
{
  return QString();
}

bool ObsFilter::needsSQL() const
{
  return false;
 }

bool ObsFilter::accept(ObsData_p obs, bool afterSQL) const
{
  if (not obs)
    return false;
  return true;
}

bool ObsFilter::subsetOf(const ObsFilter& other) const
{
  return true;
}
