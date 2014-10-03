/* -*- c++ -*-

   HQC - Free Software for Manual Quality Control of Meteorological Observations

   Copyright (C) 2007-2014 met.no

   Contact information:
   Norwegian Meteorological Institute
   Box 43 Blindern
   0313 OSLO
   NORWAY
   email: kvalobs-dev@met.no

   This file is part of HQC

   HQC is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   HQC is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with HQC; if not, write to the Free Software Foundation Inc.,
   51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef COMMON_AUTHENTICATOR_HH
#define COMMON_AUTHENTICATOR_HH

#include <QtCore/QString>
#include <QtGui/QDialog>

#include <memory>

namespace Ui {
class AuthenticationDialog;
}

namespace Authentication {

const int DEFAULT_LDAP_PORT = 636;

bool authenticate(const char *username, const char *password,
    const char *server, int port = DEFAULT_LDAP_PORT);

class Authenticator : public QDialog
{
  Q_OBJECT;

public:
  Authenticator(const char *server, int port = DEFAULT_LDAP_PORT, QWidget* parent = 0);
  virtual ~Authenticator();

  static const QString authenticate(QWidget* parent, const char *server, int port = DEFAULT_LDAP_PORT);

protected Q_SLOTS:
  virtual void doAuthenticate();

private:
  std::auto_ptr<Ui::AuthenticationDialog> ui;

  QString server;
  int port;
};

} // namespace Authentication

#endif // COMMON_AUTHENTICATOR_HH
