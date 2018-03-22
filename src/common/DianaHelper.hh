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

#ifndef DIANAHELPER_HH
#define DIANAHELPER_HH 1

#include "util/timeutil.hh"

#include <QObject>

#include <string>
#include <set>
#include <vector>

class DianaClient;
class ClientSelection;
class miQMessage;

class DianaHelper : public QObject
{
  Q_OBJECT

public:
  DianaHelper(ClientSelection* cb);
  ~DianaHelper();

  void setClientActive(DianaClient* client, bool active);
  void tryConnect();

Q_SIGNALS:
  void connection(bool);
  void receivedTime(const timeutil::ptime& time);

private Q_SLOTS:
  void handleConnect();
  void handleDisconnect();
  void onRenamed(const QString& name);
  void processLetter(int fromId, const miQMessage& m);

  //! time selected in hqc
  void onClientTimeSelected(const timeutil::ptime& time);

  //! time list changed in hqc
  void onClientTimesChanged(const std::vector<timeutil::ptime>& times);

private:
  void replaceActiveClient(DianaClient* client);
  void sendActiveTimes(int clientId);

  void sendTimes(int clientId, const std::vector<timeutil::ptime>& times);
  void sendTime(int clientId, const timeutil::ptime& time);
  bool isClient(int clientId) const;
  void sendMessage(int clientId, const miQMessage& m);

private:
  ClientSelection* mDianaButton;
  std::set<int> mConnected;

  DianaClient* mActiveClient;

  timeutil::ptime mDianaTime;
};

#endif /* DIANAHELPER_HH */
