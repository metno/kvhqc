
#ifndef FUNCTORS_HH
#define FUNCTORS_HH 1

#include <kvalobs/kvParam.h>
#include <kvalobs/kvStation.h>

#include <functional>

namespace Helpers {

struct float_eq : public std::binary_function<float, float, bool>
{
    bool operator()(float a, float b) const;
};

struct station_by_id : public std::unary_function<bool, kvalobs::kvStation>
{
    station_by_id(int s) : stationid(s) { }
    bool operator()(const kvalobs::kvStation& s) const
        { return s.stationID() == stationid; }
private:
    int stationid;
};

struct param_by_id : public std::unary_function<bool, kvalobs::kvParam>
{
    param_by_id(int s) : paramid(s) { }
    bool operator()(const kvalobs::kvParam& p) const
        { return p.paramID() == paramid; }
private:
    int paramid;
};

} // namespace Helpers

#endif // FUNCTORS_HH
