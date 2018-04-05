/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2013-2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef UTIL_CHANGEREPLAY_HH
#define UTIL_CHANGEREPLAY_HH 1

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
    for (const Column& c : original) {
        if (add.find(c) != add.end())
            replayed.push_back(c);
    }
    return replayed;
}

#endif // UTIL_CHANGEREPLAY_HH
