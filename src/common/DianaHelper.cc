/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2013-2018 met.no

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

#include "DianaHelper.hh"

#include "DianaClient.hh"

#include <coserver/ClientSelection.h>
#include <coserver/QLetterCommands.h>
#include <coserver/miMessage.h>

#define MILOGGER_CATEGORY "kvhqc.DianaHelper"
#include "util/HqcLogging.hh"

namespace {
timeutil::ptime qtime(const QString& text)
{
  return timeutil::from_iso_extended_string(text.toStdString());
}

QString qtime(const timeutil::ptime& time)
{
  return QString::fromStdString(timeutil::to_iso_extended_string(time));
}
} // namespace

DianaHelper::DianaHelper(ClientSelection* cb)
    : QObject(cb->parent())
    , mDianaButton(cb)
    , mActiveClient(0)
{
  connect(mDianaButton, SIGNAL(receivedMessage(int, const miQMessage&)), this, SLOT(processLetter(int, const miQMessage&)));
  connect(mDianaButton, SIGNAL(disconnected()), this, SLOT(handleDisconnect()));
  connect(mDianaButton, SIGNAL(renamed(const QString&)), this, SLOT(onRenamed(const QString&)));
}

DianaHelper::~DianaHelper()
{
  METLIBS_LOG_SCOPE(LOGVAL((mActiveClient != 0)));
  if (mActiveClient)
    mActiveClient->setActive(false);
}

void DianaHelper::tryConnect()
{
  mDianaButton->connect();
}

void DianaHelper::handleConnect()
{
}

void DianaHelper::handleDisconnect()
{
  mConnected.clear();
}

void DianaHelper::onRenamed(const QString&)
{
  // should delete or rename time list in diana, but diana cannot do that
}

void DianaHelper::setClientActive(DianaClient* client, bool active)
{
  METLIBS_LOG_SCOPE(LOGVAL(client));
  if (active && mActiveClient != client)
    replaceActiveClient(client);
  else if (!active && mActiveClient == client)
    replaceActiveClient(0);
}

void DianaHelper::replaceActiveClient(DianaClient* client)
{
  if (mActiveClient) {
    mActiveClient->setActive(false);
    disconnect(mActiveClient, &DianaClient::clientTimeSelected, this, &DianaHelper::onClientTimeSelected);
    disconnect(mActiveClient, &DianaClient::clientTimesChanged, this, &DianaHelper::onClientTimesChanged);
  }
  mActiveClient = client;
  if (mActiveClient) {
    mActiveClient->setActive(true);
    connect(mActiveClient, &DianaClient::clientTimeSelected, this, &DianaHelper::onClientTimeSelected);
    connect(mActiveClient, &DianaClient::clientTimesChanged, this, &DianaHelper::onClientTimesChanged);
  }

  sendActiveTimes(qmstrings::all);
}

void DianaHelper::sendActiveTimes(int clientId)
{
  METLIBS_LOG_SCOPE(LOGVAL(clientId));
  if (mActiveClient) {
    const std::vector<timeutil::ptime> times = mActiveClient->getClientTimes();
    METLIBS_LOG_SCOPE(LOGVAL(times.size()));
    if (!times.empty()) {
      sendTimes(clientId, times);
      const timeutil::ptime time = mActiveClient->getClientTime();
      if (!time.is_not_a_date_time())
        sendTime(clientId, time);
    }
  }
}

void DianaHelper::processLetter(int fromId, const miQMessage& m)
{
  METLIBS_LOG_SCOPE(LOGVAL(m));
  const bool cmd_newclient = (m.command() == qmstrings::newclient);
  const bool cmd_removeclient = (m.command() == qmstrings::removeclient);
  if (cmd_newclient || cmd_removeclient) {
    const int idx_id = m.findCommonDesc("id");
    const int idx_type = m.findCommonDesc("type");
    METLIBS_LOG_DEBUG(LOGVAL(idx_id) << LOGVAL(idx_type));
    if (idx_id >= 0 && idx_type >= 0 && m.getCommonValue(idx_type) == "Diana") {
      const int clientId = m.getCommonValue(idx_id).toInt();
      METLIBS_LOG_DEBUG(LOGVAL(clientId));
      if (cmd_newclient) {
        mConnected.insert(clientId);
        sendActiveTimes(clientId);
      } else if (cmd_removeclient) {
        mConnected.erase(clientId);
      }
    }
  }

  if (m.command() == qmstrings::timechanged && mConnected.count(fromId)) {
    const int idx_time = m.findCommonDesc("time");
    if (idx_time >= 0) {
      const timeutil::ptime newTime = qtime(m.getCommonValue(idx_time));
      if (newTime != mDianaTime) {
        mDianaTime = newTime;
        if (mActiveClient)
          mActiveClient->remoteTimeSelected(mDianaTime);
      }
    }
  }
}

void DianaHelper::onClientTimeSelected(const timeutil::ptime& time)
{
  sendTime(qmstrings::all, time);
}

void DianaHelper::onClientTimesChanged(const std::vector<timeutil::ptime>& times)
{
  sendTimes(qmstrings::all, times);
}

void DianaHelper::sendTimes(int clientId, const std::vector<timeutil::ptime>& times)
{
  METLIBS_LOG_SCOPE(LOGVAL(clientId));
  if (!isClient(clientId) || !mActiveClient)
    return;

  miQMessage m(qmstrings::settime);
  m.addCommon("datatype", mDianaButton->getClientName());
  m.addDataDesc("time");
  for (const timeutil::ptime& t : times)
    m.addDataValues(QStringList(qtime(t)));
  mDianaButton->sendMessage(m);
}

void DianaHelper::sendTime(int clientId, const timeutil::ptime& time)
{
  METLIBS_LOG_SCOPE();
  if (!isClient(clientId) || !mActiveClient)
    return;
  if (mDianaTime == time)
    return;

  mDianaTime = time;
  miQMessage m(qmstrings::settime);
  m.addCommon("time", QString::fromStdString(timeutil::to_iso_extended_string(mDianaTime)));
  mDianaButton->sendMessage(m);
}

void DianaHelper::sendMessage(int clientId, const miQMessage& m)
{
  if (clientId == qmstrings::all)
    mDianaButton->sendMessage(m, mConnected);
  else
    mDianaButton->sendMessage(m, clientId);
}

bool DianaHelper::isClient(int clientId) const
{
  return (clientId == qmstrings::all || mConnected.count(clientId));
}
