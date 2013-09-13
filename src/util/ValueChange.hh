
#ifndef UTIL_VALUECHANGE_HH
#define UTIL_VALUECHANGE_HH 1

#include <functional>

/*! Keeps an old and a new value of something.
 */
template< typename T, class E=std::equal_to<T> >
class ValueChange {
public:
  /*! Initialize with old=new value.
   * \param old the initial old and new value
   */
  ValueChange(const T& old)
    : mOld(old), mNew(old) { }

  /*! Set old value to new value.
   * \return true if E(old, new) was false, i.e. if old has changed. */
  bool commit();

  /*! Set a new value.
   * \param n the new value
   */
  void set(const T& n)
    { mNew = n; }

  /*! Get the new value.
   * \return the new value
   */
  const T& get() const
    { return mNew; }

private:
  T mOld, mNew;
};

template< typename T, class E>
bool ValueChange<T,E>::commit()
{
  if (E()(mOld, mNew))
    return false;
  mOld = mNew;
  return true;
}

#endif // UTIL_VALUECHANGE_HH
