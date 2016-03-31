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

#include "HqcKafkaReinserter.hh"

#include <kvalobs/kvDataOperations.h>
#include <kvsubscribe/KafkaProducerThread.h>
#include <kvsubscribe/KvDataSerializeCommand.h>
#include <kvsubscribe/queue.h>

#include <boost/foreach.hpp>

#include <algorithm>

#define MILOGGER_CATEGORY "kvhqc.HqcKafkaReinserter"
#include "util/HqcLogging.hh"

namespace /* anonymous */ {

typedef std::list<kvalobs::kvData> kvData_l;

} /* anonymous namespace */

HqcKafkaReinserter::HqcKafkaReinserter(const std::string& brokers, const std::string& domain, int operatorID)
  : mProducerThread(new kvalobs::service::KafkaProducerThread)
  , mOperatorId(operatorID)
{
  mProducerThread->start(brokers, kvalobs::subscribe::queue::checked(domain));
}

HqcKafkaReinserter::~HqcKafkaReinserter()
{
}

void HqcKafkaReinserter::shutdown()
{
  mProducerThread->shutdown();
  mProducerThread->join(std::chrono::seconds(15));
}

bool HqcKafkaReinserter::insert(kvData_l& dl) const
{
  for (kvData_l::iterator it = dl.begin(); it != dl.end(); ++it) {
    if (it->typeID() <= -32767)
      return false;
    updateUseAddCFailed(*it);
  }

  using kvalobs::service::KvDataSerializeCommand;
  KvDataSerializeCommand* dataCmd(new KvDataSerializeCommand(dl));
  mProducerThread->send(dataCmd);

  return true; // FIXME
}

void HqcKafkaReinserter::updateUseAddCFailed(kvalobs::kvData &d) const
{
  kvalobs::kvControlInfo cinfo = d.controlinfo();
  if (cinfo.flag(kvalobs::flag::fhqc) == 0) {
    cinfo.set(kvalobs::flag::fhqc, 3);
    d.controlinfo(cinfo);
    HQC_LOG_ERROR("inserting data with fhqc==0, forced to fhqc==3: " << d);
  }

  kvalobs::kvUseInfo ui = d.useinfo();
  ui.setUseFlags(cinfo);
  ui.addToErrorCount();
  ui.HQCid(mOperatorId);
  d.useinfo(ui);
}
