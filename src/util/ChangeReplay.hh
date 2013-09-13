
#ifndef UTIL_CHANGEREPLAY_HH
#define UTIL_CHANGEREPLAY_HH 1

#include <boost/foreach.hpp>

#include <algorithm>
#include <set>
#include <vector>

/*! Helper for recording and replaying user changes to table column
 *  lists, graph lines, etc.
 *
 * @see ViewChanges for an example
 */
template<class Column, typename Comp = std::less<Column> >
class ChangeReplay {
public:
    typedef std::vector<Column> ColumnVector;
    typedef std::set<Column, Comp> ColumnSet;

public:
    ColumnVector removals(const ColumnVector& original, const ColumnVector& actual);
    ColumnVector replay(const ColumnVector& original, const ColumnVector& actual, const ColumnVector& removed);
};

/*! Determine which columns have to be removed to go from original to
 *  actual: take the difference between the set of original columns
 *  and the set of actual columns -- duplicate columns are kind of
 *  ignored.
 */
template<class Column, typename Comp>
typename ChangeReplay<Column,Comp>::ColumnVector ChangeReplay<Column,Comp>::removals(const ColumnVector& original, const ColumnVector& actual)
{
    const ColumnSet originals(original.begin(), original.end());
    const ColumnSet actuals(actual.begin(), actual.end());

    ColumnVector removed;
    std::set_difference(originals.begin(), originals.end(), actuals.begin(), actuals.end(),
                        std::back_inserter(removed), Comp());
    return removed;
}

/*! Replay changes. Result is: 'actual' union ('original' - ('actual' union 'removed')).
 * 
 * @param original suggested column set (need not be the same as passed to 'removals' earlier)
 * @param actual   previous actual column set after user edits
 * @param removed  columns removed from the previous original column set (output from 'removals')
 */
template<class Column, typename Comp>
typename ChangeReplay<Column,Comp>::ColumnVector ChangeReplay<Column,Comp>::replay(const ColumnVector& original,
    const ColumnVector& actual, const ColumnVector& removed)
{
    const ColumnSet originals(original.begin(), original.end());
    ColumnSet no_add(actual.begin(), actual.end());
    no_add.insert(removed.begin(), removed.end());

    ColumnSet add;
    std::set_difference(originals.begin(), originals.end(), no_add.begin(), no_add.end(),
                        std::insert_iterator<ColumnSet>(add, add.end()), Comp());
    
    ColumnVector replayed(actual);
    BOOST_FOREACH(const Column& c, original) {
        if (add.find(c) != add.end())
            replayed.push_back(c);
    }
    return replayed;
}

#endif // UTIL_CHANGEREPLAY_HH
