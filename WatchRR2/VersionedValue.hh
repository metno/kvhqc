
#ifndef VERSIONEDVALUE_HH
#define VERSIONEDVALUE_HH 1

#include <algorithm>
#include <functional>
#include <vector>

/**
 * Track versions of a value.
 *
 * Values for different versions are stored and it is possible to go
 * back to an old version, and, if newer versions have not been
 * dropped, to advance to a newer version again.
 *
 * \tparam T the type of the values to store
 * \tparam E comparison function to decide if the current value is different from the original value, defaults to std:.equal_to
 */
template< typename T, class E=std::equal_to<T> >
class VersionedValue {
public:
    VersionedValue(const T& originalValue)
        : mVersions(1, Version(0, originalValue)), mCurrent(0) { }

    /**
     * Reset to the given original value, drop all changes.
     *
     * \return true iff modified() or value() have changed
     */
    bool reset(const T& originalValue);

    /**
     * Change the original value without dropping changes.
     *
     * \return true iff modified() or value() have changed
     */
    bool changeOriginal(const T& originalValue);

    /**
     * Access the current value.
     *
     * \return the current value
     */
    const T& value() const
        { return mVersions[mCurrent].value; }

    /**
     * Access the value for a specific version.
     *
     * This function has to search for the version.
     *
     * \return the value at a specific version.
     */
    const T& value(int version) const;

    /**
     * Check if the value is different from the original value.
     *
     * \return true iff E()(current value, original value) is false
     */
    bool modified() const
        { return (mCurrent>0 and not E()(value(), mVersions[0].value)); }

    /**
     * Set a new current version.
     *
     * \param version the new current version
     * \param dropAbove whether to drop all newer versions
     * \return true iff modified() or value() have changed
     */
    bool setVersion(int version, bool dropAbove);

    /**
     * Set a new value for the current version.
     *
     * This drops all versions newer than version.
     *
     * \param version the current version
     * \param newValue the new value for 
     * \return true iff modified() or value() have changed
     */
    bool setValue(int version, const T& newValue);

    /**
     * Check if a value was set for a specific version.
     *
     * This function has to search for the version.
     *
     * \return true iff a value was set (and has not been dropped) for the given version.
     */
    bool hasVersion(int version) const;

#ifndef VERSIONEDVALUE_TEST
private:
#endif // VERSIONEDVALUE_TEST
    struct Version {
        int version;
        T value;
        Version(int ve, const T& va) : version(ve), value(va) { }
        bool operator<(const Version& other) const
            { return version < other.version; }
    };
    struct VersionGE : public std::binary_function<Version, int, bool> {
        bool operator()(const Version& a, int version) const
            { return version >= a.version; }
    };
    struct VersionL {
        bool less(int va, int vb) const
            { return va < vb; }
        bool operator()(const Version& a, int version) const
            { return less(a.version, version); }
        bool operator()(int version, const Version& b) const
            { return less(version, b.version); }
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
    const bool changed = (not E()(originalValue, value())) or modified();
    mVersions = Versions_t(1, Version(0, originalValue));
    mCurrent = 0;
    return changed;
}

template< typename T, class E>
bool VersionedValue<T,E>::changeOriginal(const T& originalValue)
{
    const bool wasModified = modified();
    mVersions[0].value = originalValue;
    return (modified() != wasModified);
}

template< typename T, class E>
const T& VersionedValue<T,E>::value(int version) const
{
    typename Versions_t::const_iterator it = std::lower_bound(mVersions.begin(), mVersions.end(), version, VersionGE());
    it--;
    return it->value;
}

template< typename T, class E>
bool VersionedValue<T,E>::setVersion(int version, bool dropAbove)
{
    const bool wasModified = modified();
    const unsigned int oldCurrent = mCurrent;
    if (mVersions[mCurrent].version < version) {
        while(mCurrent+1 < mVersions.size() and mVersions[mCurrent+1].version <= version)
            mCurrent += 1;
    } else {
        while(mCurrent>0 and mVersions[mCurrent].version > version)
            mCurrent -= 1;
    }
    const bool changedValue = (mCurrent != oldCurrent)
        and (not E()(mVersions[oldCurrent].value, value()));
    if (dropAbove and mCurrent+1 < mVersions.size())
        mVersions.erase(mVersions.begin() + (mCurrent+1), mVersions.end());
    return changedValue or (modified() != wasModified);
}

template< typename T, class E>
bool VersionedValue<T,E>::setValue(int version, const T& newValue)
{
    const bool changedValue = (not E()(newValue, value())),
        wasModified = modified();
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
    return changedValue or (modified() != wasModified);
}

template< typename T, class E>
bool VersionedValue<T,E>::hasVersion(int version) const
{
    return std::binary_search(mVersions.begin(), mVersions.end(), version, VersionL());
}


#endif // VERSIONEDVALUE_HH
