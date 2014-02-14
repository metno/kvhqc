
#include "UiHelpers.hh"

#include <QtGui/QApplication>

namespace Helpers {

void processNonUserEvents()
{
  qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

} // namespace Helpers
