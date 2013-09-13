
#ifndef VALUECHANGE_HH
#define VALUECHANGE_HH 1

#include <functional>

template< typename T, class E=std::equal_to<T> >
class ValueChange {
public:
    ValueChange(const T& old)
        : mOld(old), mNew(old) { }
    bool commit();
    void set(const T& n)
        { mNew = n; }
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

#endif // VALUECHANGE_HH
