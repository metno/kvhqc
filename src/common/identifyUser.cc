/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

Copyright (C) 2016 met.no

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

#include "HqcCorbaReinserter.hh"
#include "HqcKafkaReinserter.hh"

#include "common/KvServiceHelper.hh"
#include "common/gui/Authenticator.hh"
#include "util/stringutil.hh"

#include <kvalobs/kvOperator.h>

#include <list>

namespace Authentication {

namespace {
typedef std::list<kvalobs::kvOperator> opList;
};

int identifyUser(QWidget* widgetparent, const char *ldap_server, int ldap_port)
{
  const QString user = Authenticator::authenticate(widgetparent, ldap_server, ldap_port);
  if (user.isEmpty())
    return -1; // Not authenticated

  // Get list of operators from database, and find our operator:
  opList operators;
  KvServiceHelper::instance()->getKvOperator(operators);

  for (opList::const_iterator it = operators.begin(); it != operators.end(); ++it) {
    const QString op_username = Helpers::fromUtf8(it->username());
    if (user == op_username)
      return it->userID();
  }

  return -1;
}

AbstractReinserterPtr identifyUser(kvservice::KvApp *app, QWidget* widgetparent,
    const char *ldap_server, int ldap_port)
{
  const int id = identifyUser(widgetparent, ldap_server, ldap_port);
  if (id < 0)
    return AbstractReinserterPtr();

  return AbstractReinserterPtr(new HqcCorbaReinserter(app, id));
}

AbstractReinserterPtr identifyUser(kvservice::KvApp *app, QWidget* widgetparent,
    const char *ldap_server)
{
  return identifyUser(app, widgetparent, ldap_server, DEFAULT_LDAP_PORT);
}

AbstractReinserterPtr identifyUser(std::shared_ptr<miutil::conf::ConfSection> conf, QWidget* widgetparent,
    const char *ldap_server)
{
  const int ldap_port = DEFAULT_LDAP_PORT;
  const int id = identifyUser(widgetparent, ldap_server, ldap_port);
  if (id < 0)
    return AbstractReinserterPtr();

  const miutil::conf::ValElementList valBrokers = conf->getValue("kafka.brokers");
  const miutil::conf::ValElementList valDomain  = conf->getValue("kafka.domain");

  try {
    if (valBrokers.size() == 1 && valDomain.size() == 1) {
      const std::string brokers = valBrokers.front().valAsString();
      const std::string domain  = valDomain .front().valAsString();
      return AbstractReinserterPtr(new HqcKafkaReinserter(brokers, domain, id));
    }
  } catch (miutil::conf::InvalidTypeEx& e) {
  }

  return AbstractReinserterPtr();
}

} // namespace Authentication
