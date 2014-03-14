
#ifndef UTIL_MAKE_SET_HH
#define UTIL_MAKE_SET_HH 1

#include <set>

template<class SetType>
SetType make_set(const typename SetType::value_type& single)
{
  SetType s;
  s.insert(single);
  return s;
}

template<class SetType>
class SetMaker
{
public:
  typedef typename SetType::value_type value_type;

  SetMaker& operator<<(const value_type& v)
    { mSet.insert(v); return *this; }

  SetType set() const
    { return mSet; }

private:
  SetType mSet;
};

#endif // UTIL_MAKE_SET_HH
