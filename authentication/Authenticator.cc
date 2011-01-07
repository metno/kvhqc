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
#include "Authenticator.h"
#include <qlineedit.h>
#define LDAP_DEPRECATED 1 //  ouch!
#include <ldap.h>
#include <cstring>
#include <iostream>
#include <sstream>

#include <qvalidator.h>
#include <qregexp.h>
#include <qlineedit.h>
#include <qmessagebox.h>

using namespace std;


//static const char *format  = "uid=%s,ou=People,o=dnmi.no";

namespace Authentication
{

#define VERIFY(operation) { \
	int result = operation; \
	if ( result !=  LDAP_SUCCESS ) { \
		cout << ldap_err2string(result) << endl; \
		return false; \
	} }


/**
 * Warning: This authentication is, by itself, not secure: if
 * username is empty or something authentication will have been
 * successful.
 */
bool authenticate(const char *username, const char *password,
                  const char *server, int port)
{
	LDAP * ldap;
	std::ostringstream uris;
	uris << "ldaps://" << server << ':' << port;
	std::string uri = uris.str();
	cout << "connecting to " << uri << endl;
	VERIFY(ldap_initialize(& ldap, uri.c_str()));


	int ldapVersion = LDAP_VERSION3;
	ldap_set_option(ldap, LDAP_OPT_PROTOCOL_VERSION, & ldapVersion);

	std::ostringstream uns;
	//uns << "uid=" << username << ", ou=People, o=dnmi.no";
	uns << "uid=" << username << ",ou=user,ou=internal,dc=met,dc=no";
	string un = uns.str();
	cout << un << endl;

	VERIFY(ldap_simple_bind_s(ldap, un.c_str(), password));

	ldap_unbind(ldap);
	cout << "YESSSSS" << endl;
	return true;
}


Authenticator::Authenticator( const char *server, int port,
                              QWidget* parent, const char* name, bool modal,  Qt::WindowFlags fl )
    : AuthenticationDialog( parent, name, modal, fl), server(server), port(port)
{
  QRegExpValidator *validUN = new QRegExpValidator(this, "unInputValidator");
  validUN->setRegExp(QRegExp("[-\\w]+"));
  username->setValidator(validUN);

  QRegExpValidator *validPW = new QRegExpValidator(this, "pwInputValidator");
  validPW->setRegExp(QRegExp("\\S+"));
  password->setValidator(validPW);
}

Authenticator::~Authenticator()
{}

void Authenticator::doAuthenticate()
{
  QString un = username->text();
  QString pw = password->text();
  password->clear();

  if ( un.isEmpty() or pw.isEmpty() )
    return;

  bool result = Authentication::authenticate(un.toAscii(), pw.toAscii(), server.toAscii(), port);

  if ( result ) {
    return accept();
  } else {
    QMessageBox::information(this,
                             "Feil brukernavn eller passord",
                             "Feil brukernavn eller passord. Vennligst prœóõ½v igjen.",
                             QMessageBox::Ok);

    return;
  }
}

const QString Authenticator::authenticate(const char *server, int port)
{
  Authenticator auth(server, port, 0,0,0);
  int result = auth.exec();
  if ( result == QDialog::Accepted )
    return auth.username->text();

  return QString();
}
}

