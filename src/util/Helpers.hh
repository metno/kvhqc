
#ifndef UTIL_HELPERS_HH
#define UTIL_HELPERS_HH 1

#include <QtCore/QString>
#include <string>

class QWidget;
namespace miutil {
namespace conf {
class ConfSection;
} // namespace conf
} // namespace miutil

namespace Helpers {

char int2char(int i);

QString& appendText(QString& text, const QString& append, const QString& separator = ", ");
QString appendedText(const QString& text, const QString& append, const QString& separator = ", ");

double distance(double lon1, double lat1, double lon2, double lat2);

float round(float f, float factor);
float roundDecimals(float f, int decimals);
float parseFloat(const QString& text, int nDecimals);

bool askDiscardChanges(int nupdates, QWidget* parent);

bool connect2postgres(const QString& qname, const QString& host, const QString& dbname, const QString& user, const QString& password, int port);
bool connect2postgres(const QString& qname, miutil::conf::ConfSection *conf, const std::string& prefix);

} // namespace Helpers

#endif // UTIL_HELPERS_HH
