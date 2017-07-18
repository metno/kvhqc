
#ifndef UTIL_HELPERS_HH
#define UTIL_HELPERS_HH 1

#include <QString>

#include <memory>
#include <string>

class QWidget;
namespace miutil {
namespace conf {
class ConfSection;
} // namespace conf
} // namespace miutil

namespace Helpers {

/*! Great circle distance between two points.
 * \param lon1 longitude of first point
 * \param lat1 latitude of first point
 * \param lon2 longitude of second point
 * \param lat2 latitude of second point
 * \return great cirlce distance in kilometer
 */
double distance(double lon1, double lat1, double lon2, double lat2);

/*! Round a value.
 * \param f value to be rounded
 * \param factor rounding factor
 * \return the rounded value, such that (\c f * \c factor) is integer
 */
float round(float f, float factor);

/*! Round to a given number of decimals
 * Rounding factor is \f$ 10 ^ decimals \f$.
 * \sa Helpers::round
 */
float roundDecimals(float f, int decimals);

/*! Shows a dialog asking whether changes should be discarded.
 * \param nupdates number of changes
 * \return true if user chose to discard changes
 */
bool askDiscardChanges(int nupdates, QWidget* parent);

/*! Connect QSQL to a postgres database.
 * \param qname the QSQL name of the database
 * \return true if the database is open
 */
bool connect2postgres(const QString& qname, const QString& host, const QString& dbname, const QString& user, const QString& password, int port);

/*! Connect QSQL to a postgres database, using .
 * Hostname etc. are fetched from the ConfSection prefix+".host", …
 * \param qname the QSQL name of the database
 * \param conf \c ConfSection to use
 * \param prefix for connection parameters (\c prefix + ".host", \c prefix + ".dbname", etc)
 * \return true if the database is open
 */
bool connect2postgres(const QString& qname, std::shared_ptr<miutil::conf::ConfSection> conf, const std::string& prefix);

/*! Format a time step as localized text.
 * Largest unit is day, step 0 is "none"
 * \param step time step in seconds (absolute value is used)
 */
QString timeStepAsText(int step);

} // namespace Helpers

#endif // UTIL_HELPERS_HH
