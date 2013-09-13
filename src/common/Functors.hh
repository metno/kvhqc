
#ifndef FUNCTORS_HH
#define FUNCTORS_HH 1

#include <kvalobs/kvParam.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvTypes.h>

#include <functional>

namespace Helpers {

struct float_eq : public std::binary_function<float, float, bool>
{
    bool operator()(float a, float b) const;
};

template<class kvX>
struct extractKvId {
};

template<>
struct extractKvId<kvalobs::kvStation> {
  int operator()(const kvalobs::kvStation& s) const
    { return s.stationID(); }
};

template<>
struct extractKvId<kvalobs::kvParam> {
  int operator()(const kvalobs::kvParam& p) const
    { return p.paramID(); }
};

template<>
struct extractKvId<kvalobs::kvTypes> {
  int operator()(const kvalobs::kvTypes& t) const
    { return t.typeID(); }
};

template<class kvX>
struct kvX_lt : public std::binary_function<bool, kvX, kvX>
{
  bool operator()(const kvX& a, const kvX& b) const
    { const extractKvId<kvX> idx; return idx(a) < idx(b); }
};

typedef kvX_lt<kvalobs::kvStation> kvStation_lt;
typedef kvX_lt<kvalobs::kvParam>   kvParam_lt;
typedef kvX_lt<kvalobs::kvTypes>   kvTypes_lt;

template<class kvX>
struct kvX_lt_id
{
  bool operator()(const kvX& a, int b) const
    { const extractKvId<kvX> idx; return idx(a) < b; }
  bool operator()(int a, const kvX& b) const
    { const extractKvId<kvX> idx; return a < idx(b); }
};

typedef kvX_lt_id<kvalobs::kvStation> kvStation_lt_id;
typedef kvX_lt_id<kvalobs::kvParam>   kvParam_lt_id;
typedef kvX_lt_id<kvalobs::kvTypes>   kvTypes_lt_id;

template<class kvX>
struct kvX_eq_id
{
  kvX_eq_id(int i) : id(i) { }
  bool operator()(const kvX& x) const
    { const extractKvId<kvX> idx; return idx(x) == id; }
private:
  int id;
};

typedef kvX_eq_id<kvalobs::kvStation> kvStation_eq_id;
typedef kvX_eq_id<kvalobs::kvParam>   kvParam_eq_id;
typedef kvX_eq_id<kvalobs::kvTypes>   kvTypes_eq_id;

} // namespace Helpers

#endif // FUNCTORS_HH
