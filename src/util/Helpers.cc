/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

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


#include "Helpers.hh"

#include <miconfparser/confsection.h>

#include <QApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>

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
    return connect2postgres(qname, QString::fromStdString(valHost    .front().valAsString()),
        QString::fromStdString(valDbname  .front().valAsString()),
        QString::fromStdString(valUser    .front().valAsString()),
        QString::fromStdString(valPassword.front().valAsString()),
        valPort.front().valAsInt());
  } catch (miutil::conf::InvalidTypeEx& e) {
    return false;
  }
}

// ------------------------------------------------------------------------

QString timeStepAsText(int step)
{
  const int MINUTE = 60, HOUR = 60*MINUTE, DAY = 24*HOUR;

  if (step == 0)
    return qApp->translate("Helpers", "none");
  if (step < 0)
    step = -step;
  if (step >= DAY and (step % DAY) == 0)
    return qApp->translate("Helpers", "%1 d").arg(step / DAY);
  if (step >= HOUR and (step % HOUR) == 0)
    return qApp->translate("Helpers", "%1 h").arg(step / HOUR);
  if (step >= MINUTE and (step % MINUTE) == 0)
    return qApp->translate("Helpers", "%1 min").arg(step / MINUTE);
  return qApp->translate("Helpers", "%1 s").arg(step);
}

} // namespace Helpers
