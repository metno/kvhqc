/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

$Id$

Copyright (C) 2007 met.no

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

#include "Authenticator.hh"

#include <qlineedit.h>
#define LDAP_DEPRECATED 1 //  ouch!
#include <ldap.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#include <qvalidator.h>
#include <qregexp.h>
#include <qlineedit.h>
#include <qmessagebox.h>

#include "ui_authenticationdialog.h"

#define MILOGGER_CATEGORY "kvhqc.AcceptRejectButtons"
#include "util/HqcLogging.hh"

namespace Authentication
{

#define VERIFY(result)                                  \
  if (result !=  LDAP_SUCCESS) {                        \
    METLIBS_LOG_DEBUG(ldap_err2string(result));         \
    return false;                                       \
  }

/**
 * Warning: This authentication is, by itself, not secure: if
 * username is empty or something authentication will have been
 * successful.
 */
bool authenticate(const char *username, const char *password,
    const char *server, int port)
{
  METLIBS_LOG_SCOPE();

  LDAP * ldap;
  std::ostringstream uris;
  uris << "ldaps://" << server << ':' << port;
  std::string uri = uris.str();
  METLIBS_LOG_DEBUG("connecting to " << uri);
  VERIFY(ldap_initialize(& ldap, uri.c_str()));

  int ldapVersion = LDAP_VERSION3;
  ldap_set_option(ldap, LDAP_OPT_PROTOCOL_VERSION, & ldapVersion);
  
  std::ostringstream uns;
  uns << "uid=" << username << ",ou=user,ou=internal,dc=met,dc=no";
  std::string un = uns.str();
  
  VERIFY(ldap_simple_bind_s(ldap, un.c_str(), password));
  
  ldap_unbind(ldap);
  return true;
}

Authenticator::Authenticator(const char *server, int port,
    QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::AuthenticationDialog)
  , server(server)
  , port(port)
{
  ui->setupUi(this);
#ifdef Q_WS_X11
  const char* env_user = getenv("USER");
  if( env_user )
    ui->username->setText(env_user);
  ui->password->setFocus();
#endif

  QRegExpValidator *validUN = new QRegExpValidator(this);
  validUN->setRegExp(QRegExp("[-\\w]+"));
  ui->username->setValidator(validUN);

  QRegExpValidator *validPW = new QRegExpValidator(this);
  validPW->setRegExp(QRegExp("\\S+"));
  ui->password->setValidator(validPW);
}

Authenticator::~Authenticator()
{
}

void Authenticator::doAuthenticate()
{
  const QString un = ui->username->text();
  const QString pw = ui->password->text();
  ui->password->clear();

  if (un.isEmpty() or pw.isEmpty())
    return;

  bool result = Authentication::authenticate(un.toAscii(), pw.toAscii(), server.toAscii(), port);

  if ( result ) {
    return accept();
  } else {
    QMessageBox::information(this,
        tr("Wrong username or password"),
        tr("Wrong username or password. Please try again."),
        QMessageBox::Ok);
  }
}

const QString Authenticator::authenticate(QWidget* parent, const char *server, int port)
{
  Authenticator auth(server, port, parent);
  int result = auth.exec();
  if ( result == QDialog::Accepted )
    return auth.ui->username->text();
  
  return QString();
}

} // namespace Authentication
