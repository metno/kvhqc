
#include "Helpers.hh"

#include "util/stringutil.hh"

#include <miconfparser/confsection.h>

#include <QtCore/QVariant>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

#include <cmath>

#define MILOGGER_CATEGORY "kvhqc.Helpers"
#include "HqcLogging.hh"

namespace Helpers {

double distance(double lon1, double lat1, double lon2, double lat2)
{
  const double DEG_RAD = M_PI/180, EARTH_RADIUS = 6371.0;
  const double delta_lon=(lon1 - lon2)*DEG_RAD, slon = sin(delta_lon/2);
  const double delta_lat=(lat1 - lat2)*DEG_RAD, slat = sin(delta_lat/2);
  const double a = slat*slat + cos(lat1*DEG_RAD)*cos(lat2*DEG_RAD)*slon*slon;
  const double c =2.0 * atan2(sqrt(a), sqrt(1-a));
  return EARTH_RADIUS*c;
}

// ------------------------------------------------------------------------

float round(float f, float factor)
{
  f *= factor;
  if (f < 0.0f)
    f -= 0.5;
  else
    f += 0.5;
  float ff = 0;
  modff(f, &ff);
  return ff / factor;
}

// ------------------------------------------------------------------------

float roundDecimals(float f, int decimals)
{
  return round(f, std::pow(10, decimals));
}

// ------------------------------------------------------------------------

bool askDiscardChanges(int nupdates, QWidget* parent)
{
  if (nupdates == 0)
    return true;

  QMessageBox w(parent);
  w.setWindowTitle(parent->windowTitle());
  w.setIcon(QMessageBox::Warning);
  w.setText(qApp->translate("Helpers", "There are %1 unsaved data updates.").arg(nupdates));
  w.setInformativeText(qApp->translate("Helpers", "Are you sure that you want to lose them?"));
  QPushButton* discard = w.addButton(qApp->translate("Helpers", "Discard changes"), QMessageBox::ApplyRole);
  QPushButton* cont = w.addButton(qApp->translate("Helpers", "Continue"), QMessageBox::RejectRole);
  w.setDefaultButton(cont);
  w.exec();
  if (w.clickedButton() != discard)
    return false;
  return true;
}

bool connect2postgres(const QString& qname, const QString& host, const QString& dbname, const QString& user, const QString& password, int port)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG(LOGVAL(qname) << LOGVAL(host) << LOGVAL(dbname) << LOGVAL(user) << LOGVAL(password) << LOGVAL(port));

  QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", qname);
  db.setHostName    (host);
  db.setDatabaseName(dbname);
  db.setUserName    (user);
  db.setPassword    (password);
  db.setPort        (port);

  if (not db.open()) {
    HQC_LOG_ERROR("cannot connect to PSQL database: " << db.lastError().text());
    return false;
  } else {
    return true;
  }
}

bool connect2postgres(const QString& qname, std::shared_ptr<miutil::conf::ConfSection> conf, const std::string& prefix)
{
  METLIBS_LOG_SCOPE();
  if (not conf)
    return false;

  using namespace miutil::conf;
  const ValElementList valHost     = conf->getValue(prefix + ".host");
  const ValElementList valDbname   = conf->getValue(prefix + ".dbname");
  const ValElementList valUser     = conf->getValue(prefix + ".user");
  const ValElementList valPassword = conf->getValue(prefix + ".password");
  const ValElementList valPort     = conf->getValue(prefix + ".port");

  if (valHost.size() != 1 or valDbname.size() != 1 or valUser.size() != 1 or valPassword.size() != 1 or valPort.size() != 1)
    return false;

  try {
    return connect2postgres(qname, Helpers::fromUtf8(valHost.front().valAsString()),
        Helpers::fromUtf8(valDbname  .front().valAsString()),
        Helpers::fromUtf8(valUser    .front().valAsString()),
        Helpers::fromUtf8(valPassword.front().valAsString()),
        valPort.front().valAsInt());
  } catch (miutil::conf::InvalidTypeEx& e) {
    return false;
  }
}

} // namespace Helpers
