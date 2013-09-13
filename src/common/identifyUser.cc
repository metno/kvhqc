/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

Copyright (C) 2007-2013 met.no

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

#include "common/gui/Authenticator.hh"
#include "HqcDataReinserter.hh"
#include "KvServiceHelper.hh"

#include <kvalobs/kvOperator.h>

#include <boost/foreach.hpp>

#include <list>
#include <cstring>

namespace Authentication {

namespace {
typedef std::list<kvalobs::kvOperator> opList;
typedef opList::const_iterator opIter;
};

kvalobs::DataReinserter<kvservice::KvApp> *identifyUser(QWidget* widgetparent, kvservice::KvApp *app,
    const char *ldap_server, QString& userName)
{
  return identifyUser(widgetparent, app, ldap_server, userName, DEFAULT_LDAP_PORT);
}

kvalobs::DataReinserter<kvservice::KvApp> *identifyUser(QWidget* widgetparent, kvservice::KvApp *app,
    const char *ldap_server, QString& userName, int ldap_port)
{
  const QString user = Authenticator::authenticate(widgetparent, ldap_server, ldap_port);
  if (user.isEmpty())
    return 0; // Not authenticated
  
  // Get list of operators from database, and find our operator:
  opList operators;
  KvServiceHelper::instance()->getKvOperator(operators);

  BOOST_FOREACH(const kvalobs::kvOperator& op, operators) {
    const QString uname = QString::fromStdString(op.username());
    if (user == uname) {
      userName = uname;
      return new HqcDataReinserter(app, op.userID());
    }
  }

  return 0;
}

} // namespace Authentication
