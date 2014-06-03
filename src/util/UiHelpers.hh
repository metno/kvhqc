
#ifndef UTIL_GUI_UIHELPERS_HH
#define UTIL_GUI_UIHELPERS_HH 1

#include <QtCore/QModelIndex>

namespace Helpers {

void findMinMaxRowCol(const QModelIndexList& selected, int& minRow, int& maxRow, int& minCol, int& maxCol);

} // namespace Helpers

#endif // UTIL_GUI_UIHELPERS_HH
