
#include "ObsFilter.hh"

bool ObsFilter::hasSQL() const
{
  return false;
}

// -- how to sanitize this? avoid TRUNCATE and friends
std::string ObsFilter::acceptingSQL(const std::string& data_alias) const
{
  return "";
}

bool ObsFilter::accept(const SensorTime&, bool /*afterSQL*/) const
{
  return true;
}

bool ObsFilter::accept(ObsData_p obs, bool afterSQL) const
{
  if (not obs)
    return false;
  return accept(obs->sensorTime(), afterSQL);
}

bool ObsFilter::equals(const ObsFilter& other) const
{
  return false;
}

bool ObsFilter::subsetOf(const ObsFilter& other) const
{
  return true;
}
