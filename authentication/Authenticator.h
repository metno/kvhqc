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
#ifndef __Authenticator_h__
#define __Authenticator_h__

// Compile with -lldap option.

#include "authenticationdialog.h"
#include <qstring.h>
#include <qdialog.h>


namespace Authentication {

  //  const int DEFAULT_LDAP_PORT = 389;
  const int DEFAULT_LDAP_PORT = 636;

  bool authenticate(const char *username, const char *password, 
		    const char *server, int port = DEFAULT_LDAP_PORT);



  class Authenticator : public AuthenticationDialog 
  {
    Q_OBJECT;

  public:
    Authenticator( const char *server, int port = DEFAULT_LDAP_PORT, 
		   QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WindowFlags fl = 0 );
    virtual ~Authenticator();

    static const QString authenticate(const char *server, int port = DEFAULT_LDAP_PORT);

  protected:
    QString server;
    int port;

  protected slots:
    virtual void doAuthenticate();
  };


}


#endif // __Authenticator_h__
