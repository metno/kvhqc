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
#define LDAP_DEPRECATED 1
#include "Authenticator.h"
#include <qlineedit.h>
#include <ldap.h>
#include <cstring>
#include <iostream>
#include <sstream>

//#include <qvalidator.h>
#include <QValidator>
//#include <qregexp.h>
#include <QRegExp>
#include <qlineedit.h>
#include <qmessagebox.h>

using namespace std;


//static const char *format  = "uid=%s,ou=People,o=dnmi.no";

namespace Authentication {

  /**
   * Warning: This authentication is, by itself, not secure: if
   * username is empty or something authentication will have been
   * successful.
   */ 
  
  bool authenticate(const char *username, const char *password, 
		    const char *server, int port) 
  {
    LDAP *ld = NULL;
    LDAP *ldc;
    int result;
    ostringstream user;
    
    user << "uid=" << username << ",ou=People,o=dnmi.no";

    ld = ldap_init(server, port);
    string userS = user.str();
    result = ldap_bind_s( ld, userS.c_str(), password, LDAP_AUTH_SIMPLE);
    ldap_unbind(ld);
    //    ld = ldap_initialize(&ldc,"ldap.oslo.dnmi.no");
    //    result = ldap_simple_bind( ld, userS.c_str(), password);
    //    ldap_unbind_ext_s(ld, NULL, NULL);
    
    cerr << "Authentication of " << userS << ":" << endl;
    cerr << "\t" << ldap_err2string(result) << endl;

    if ( result == LDAP_SUCCESS )
      return true;
    
    return false;
  }



  Authenticator::Authenticator( const char *server, int port, 
				QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl )
    : AuthenticationDialog( parent, name, modal, fl), server(server), port(port)
  {
    //    QRegExpValidator *validUN = new QRegExpValidator(this, "unInputValidator");
    QRegExpValidator *validUN = new QRegExpValidator(this);
    validUN->setRegExp(QRegExp("[-\\w]+"));
    username->setValidator(validUN);

    //    QRegExpValidator *validPW = new QRegExpValidator(this, "pwInputValidator");
    QRegExpValidator *validPW = new QRegExpValidator(this);
    validPW->setRegExp(QRegExp("\\S+"));
    password->setValidator(validPW);
  }

  Authenticator::~Authenticator()
  {
  }

  void Authenticator::doAuthenticate()
  {
    QString un = username->text();
    QString pw = password->text();
    password->clear();

    if ( un.isEmpty() or pw.isEmpty() )
      return;

    bool result = Authentication::authenticate(un.ascii(), pw.ascii(), server.ascii(), port);

    cerr << "NUH!" << endl;

    if ( result ) {
      return accept();
    }
    else {
      QMessageBox::information(this, 
			       "Feil brukernavn eller passord",
			       "Feil brukernavn eller passord. Vennligst prøv igjen.",
			       QMessageBox::Ok);
      
      return;
    } 
  }

  const QString Authenticator::authenticate(const char *server, int port) {
    Authenticator auth(server, port, 0,0,0);
    int result = auth.exec();
    if ( result == QDialog::Accepted )
      return auth.username->text();

    return QString();
  }
}

