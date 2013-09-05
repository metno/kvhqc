
#ifndef VERSIONEDVALUE_HH
#define VERSIONEDVALUE_HH 1

#include <functional>
#include <vector>

#ifdef VERSIONEDVALUE_TEST
#include <iostream>
#define DBGV(x) do { std::cout << __LINE__ << ": '" << #x << "'='" << x << "'" << std::endl; } while (0)
#else
#define DBGV(x) do { } while (0)
#endif

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
    mVersions = Versions_t(1, Version(0, originalValue));
    mCurrent = 0;
    return wasModified;
}

template< typename T, class E>
bool VersionedValue<T,E>::setVersion(int version, bool dropAbove)
{
    const T old = value();
    if (mVersions[mCurrent].version < version) {
        DBGV(mCurrent);
        while(mCurrent+1 < mVersions.size() and mVersions[mCurrent+1].version <= version) {
            mCurrent += 1;
            DBGV(mCurrent);
        }
    } else if (mVersions[mCurrent].version > version) {
        DBGV(mCurrent);
        while(mCurrent>0 and mVersions[mCurrent].version > version) {
            mCurrent -= 1;
            DBGV(mCurrent);
        }
    }
    DBGV(mCurrent);
    if (dropAbove and mCurrent+1 < mVersions.size()) {
        mVersions.erase(mVersions.begin() + (mCurrent+1), mVersions.end());
        DBGV(mVersions.size());
    }
    return not E()(old, value());
}

template< typename T, class E>
bool VersionedValue<T,E>::setValue(int version, const T& newValue)
{
    const T old = value();
    if (mVersions[mCurrent].version > version) {
        // drop all versions > currentVersion? only needed if somehow not dropped in setVersion
        mVersions.erase(mVersions.begin() + mCurrent, mVersions.end());
    }
    if (mVersions[mCurrent].version == version) {
        mVersions[mCurrent].value = newValue;
    } else {
        mCurrent = mVersions.size();
        mVersions.push_back(Version(version, newValue));
    }
    return not E()(old, value());
}

#endif // VERSIONEDVALUE_HH
