
#ifndef ChangeReplay_hh
#define ChangeReplay_hh 1

#include <boost/foreach.hpp>

#include <algorithm>
#include <set>
#include <vector>

template<class Column, typename Comp = std::less<Column> >
class ChangeReplay {
public:
    typedef std::vector<Column> ColumnVector;
    typedef std::set<Column, Comp> ColumnSet;

public:
    ColumnVector removals(const ColumnVector& original, const ColumnVector& actual);
    ColumnVector replay(const ColumnVector& original, const ColumnVector& actual, const ColumnVector& removed);
};

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

template<class Column, typename Comp>
typename ChangeReplay<Column,Comp>::ColumnVector ChangeReplay<Column,Comp>::replay(const ColumnVector& original, const ColumnVector& actual, const ColumnVector& removed)
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

#endif // ChangeReplay_hh
