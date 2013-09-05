
#ifndef VERSIONEDVALUE_HH
#define VERSIONEDVALUE_HH 1

#include <functional>
#include <vector>

template< typename T, class E=std::equal_to<T> >
class VersionedValue {
public:
    VersionedValue(const T& originalValue)
        : mVersions(1, Version(0, originalValue)), mCurrent(0) { }
    bool reset(const T& originalValue);
    const T& value() const
        { return mVersions[mCurrent].value; }
    bool modified() const
        { return not E()(value(), mVersions[0].value); }
    bool setVersion(int version, bool dropAbove);
    bool setValue(int version, const T& newValue);

#ifndef VERSIONEDVALUE_TEST
private:
#endif // VERSIONEDVALUE_TEST
    struct Version {
        int version;
        T value;
        Version(int ve, const T& va) : version(ve), value(va) { }
    };
    typedef std::vector<Version> Versions_t;

#ifndef VERSIONEDVALUE_TEST
private:
#endif // VERSIONEDVALUE_TEST
    Versions_t mVersions;
    unsigned int mCurrent;
};

template< typename T, class E>
bool VersionedValue<T,E>::reset(const T& originalValue)
{
    const bool wasModified = modified();
    const T old = value();
    mVersions = Versions_t(1, Version(0, originalValue));
    mCurrent = 0;
    return wasModified or (not E()(old, value()));
}

template< typename T, class E>
bool VersionedValue<T,E>::setVersion(int version, bool dropAbove)
{
    const bool wasModified = modified();
    const T old = value();
    if (mVersions[mCurrent].version < version) {
        while(mCurrent+1 < mVersions.size() and mVersions[mCurrent+1].version <= version)
            mCurrent += 1;
    } else {
        while(mCurrent>0 and mVersions[mCurrent].version > version)
            mCurrent -= 1;
    }
    if (dropAbove and mCurrent+1 < mVersions.size())
        mVersions.erase(mVersions.begin() + (mCurrent+1), mVersions.end());
    return (modified() != wasModified) or (not E()(old, value()));
}

template< typename T, class E>
bool VersionedValue<T,E>::setValue(int version, const T& newValue)
{
    const bool wasModified = modified();
    const T old = value();
    if (mCurrent+1 < mVersions.size()) {
        // drop all versions above, only needed if setVersion was not called
        mVersions.erase(mVersions.begin() + (mCurrent+1), mVersions.end());
    }
    if (mVersions[mCurrent].version == version) {
        mVersions[mCurrent].value = newValue;
    } else {
        mCurrent = mVersions.size();
        mVersions.push_back(Version(version, newValue));
    }
    return (modified() != wasModified) or (not E()(old, value()));
}

#endif // VERSIONEDVALUE_HH
