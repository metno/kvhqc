
#ifndef StationIdCompletion_hh
#define StationIdCompletion_hh 1

class QLineEdit;
class QObject;
class StationIdModel;

namespace Helpers {

void installStationIdCompleter(QObject* parent, QLineEdit* editStation);
void installStationIdCompleter(QObject* parent, QLineEdit* editStation, StationIdModel* cmodel);

} // namespace Helpers

#endif // StationIdCompletion_hh
