
#include "UiHelpers.hh"

#include <QApplication>

namespace Helpers {

void findMinMaxRowCol(const QModelIndexList& selected, int& minRow, int& maxRow, int& minCol, int& maxCol)
{
  if (not selected.isEmpty()) {
    maxRow = minRow = selected.at(0).row();
    maxCol = minCol = selected.at(0).column();
    for (int i=1; i<selected.count(); i++) {
        const int r = selected.at(i).row(), c = selected.at(i).column();
        if (r < minRow)
          minRow = r;
        if (maxRow < r)
          maxRow = r;
        if (c < minCol)
          minCol = c;
        if (maxCol < c)
          maxCol = c;
    }
  }
}

} // namespace Helpers
