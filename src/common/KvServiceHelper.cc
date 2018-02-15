/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

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


#include "KvServiceHelper.hh"

#include <kvalobs/kvOperator.h>
#include <kvalobs/kvStationParam.h>

#define MILOGGER_CATEGORY "kvhqc.KvServiceHelper"
#include "util/HqcLogging.hh"

KvServiceHelper* KvServiceHelper::sInstance = 0;

KvServiceHelper::KvServiceHelper(std::shared_ptr<kvservice::KvApp> app)
    : mApp(app)
    , mKvalobsAvailable((bool)mApp)
{
  sInstance = this;
}

KvServiceHelper::~KvServiceHelper()
{
  sInstance = 0;
}

bool KvServiceHelper::checkKvalobsAvailability()
{
  if (!mApp)
    return false;
  std::list<kvalobs::kvStationParam> stParam;
  return updateKvalobsAvailability(mApp->getKvStationParam(stParam, 345345, 345345, 0));
}

bool KvServiceHelper::updateKvalobsAvailability(bool available)
{
  if (available != mKvalobsAvailable) {
    mKvalobsAvailable = available;
    Q_EMIT kvalobsAvailable(mKvalobsAvailable);
  }
  return available;
}

int KvServiceHelper::identifyOperator(const QString& username)
{
  if (!mApp) {
    updateKvalobsAvailability(false);
    return -1;
  }

  typedef std::list<kvalobs::kvOperator> kvOperator_l;
  kvOperator_l operators;
  if (!mApp->getKvOperator(operators)) {
    updateKvalobsAvailability(false);
    return -1;
  }
  updateKvalobsAvailability(true);

  for (kvOperator_l::const_iterator it = operators.begin(); it != operators.end(); ++it) {
    if (username == QString::fromStdString(it->username()))
      return it->userID();
  }
  return -1;
}
