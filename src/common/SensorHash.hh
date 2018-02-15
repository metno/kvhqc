
#ifndef COMMON_SENSORHASH_HH
#define COMMON_SENSORHASH_HH 1

#include "Sensor.hh"
#include <boost/functional/hash/hash.hpp>

struct hash_Sensor
{
  size_t operator()(const Sensor& s) const
  {
    std::size_t seed = 0;
    boost::hash_combine(seed, s.stationId);
    boost::hash_combine(seed, s.paramId);
    boost::hash_combine(seed, s.level);
    boost::hash_combine(seed, s.sensor);
    boost::hash_combine(seed, s.typeId);
    return seed;
  }
};

#endif // COMMON_SENSORHASH_HH
