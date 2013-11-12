
#ifndef StationIdCompletion_hh
#define StationIdCompletion_hh 1

class QLineEdit;
class QWidget;
class StationIdModel;

namespace Helpers {

void installStationIdCompleter(QWidget* parent, QLineEdit* editStation);
void installStationIdCompleter(QWidget* parent, QLineEdit* editStation, StationIdModel* cmodel);

} // namespace Helpers

#endif // StationIdCompletion_hh
