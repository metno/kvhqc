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
#include "identifyUser.h"
#include "Authenticator.h"
#include "HqcDataReinserter.h"
#include <kvalobs/kvOperator.h>
#include <list>
#include <cstring>

#include <iostream>

using namespace std;
using namespace kvalobs;
using namespace kvservice;

namespace Authentication {

  namespace {
    typedef list<kvalobs::kvOperator> opList;
    typedef opList::const_iterator opIter;
  };

  DataReinserter<KvApp> *identifyUser(KvApp *app, const char *ldap_server, QString& userName) 
  { 
    return identifyUser(app, ldap_server, userName, DEFAULT_LDAP_PORT);
  }

  DataReinserter<KvApp> *identifyUser(KvApp *app, const char *ldap_server, QString& userName, int ldap_port) 
  {
    // Authenticate user:
    
    QString user = Authenticator::authenticate(ldap_server, ldap_port);
    if ( user.isEmpty() )
      return NULL; // Not authenticated
    
    // Get list of operators from database, and find our operator:
    opList operators;

    cerr << "KvApp is " << (app ? "not null" : "null") << endl;

    app->getKvOperator(operators);  // FEIL SKJER HER!
    for (opIter it = operators.begin(); it != operators.end(); it++) {
      cerr << it->username().cStr() << "  " << it->userID() << endl;
      if ( strcmp(it->username().cStr(), user.ascii()) == 0 ) {
	string uname = it->username();
	userName = uname.c_str();
	return new HqcDataReinserter( app, it->userID() );
      }
    }

    // Could not find user in database:
    return NULL;
  }
}
