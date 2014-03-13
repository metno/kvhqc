
#ifndef BOOSTUTIL_HH
#define BOOSTUTIL_HH 1

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#define HQC_SHARED_NOCOPY(klazz) \
  public boost::enable_shared_from_this<klazz>, \
               private boost::noncopyable

#define HQC_TYPEDEF_P(klazz) \
  typedef boost::shared_ptr<klazz> klazz ## _p

#define HQC_TYPEDEF_V(klazz) \
  typedef std::vector<klazz> klazz ## _v

#define HQC_TYPEDEF_PV(klazz) \
  typedef std::vector<klazz ## _p> klazz ## _pv

#define HQC_TYPEDEF_X(klazz) \
  typedef klazz* klazz ## _x

#define HQC_TYPEDEF_S(klazz) \
  typedef std::set<klazz> klazz ## _s

#define HQC_TYPEDEF_PS(klazz) \
  typedef std::set<klazz ## _p> klazz ## _ps

#define HQC_TYPEDEF_PL(klazz) \
  typedef std::list<klazz ## _p> klazz ## _pl

#endif // BOOSTUTIL_HH
