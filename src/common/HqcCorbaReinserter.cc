/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

Copyright (C) 2013 met.no

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

#include "HqcCorbaReinserter.hh"

#include <kvalobs/kvDataOperations.h>

#include <boost/foreach.hpp>

#include <algorithm>

using namespace kvservice;
using namespace kvalobs;

#define MILOGGER_CATEGORY "kvhqc.HqcCorbaReinserter"
#include "util/HqcLogging.hh"

namespace internal_
{

void updateUseAddCFailed(kvalobs::kvData &d)
{
    kvalobs::kvControlInfo cinfo = d.controlinfo();
    if (cinfo.flag(flag::fhqc) == 0) {
      cinfo.set(flag::fhqc, 3);
      d.controlinfo(cinfo);
      HQC_LOG_ERROR("inserting data with fhqc==0, forced to fhqc==3: " << d);
    }

    kvUseInfo ui = d.useinfo();
    ui.setUseFlags(cinfo);
    ui.addToErrorCount();
    d.useinfo(ui);
}

} // namespace internal_

HqcCorbaReinserter::HqcCorbaReinserter(KvApp *app, int operatorID)
  : mReinserter(new KvAppReinserter(app, operatorID))
{
}

HqcCorbaReinserter::~HqcCorbaReinserter()
{
}

bool HqcCorbaReinserter::insert(std::list<kvalobs::kvData> &dl) const
{
  METLIBS_LOG_SCOPE();
  BOOST_FOREACH(const kvalobs::kvData &d, dl) {
    if (d.typeID() <= -32767)
      return false;
  }
  std::for_each(dl.begin(), dl.end(), ::internal_::updateUseAddCFailed);

  const CKvalObs::CDataSource::Result_var res = mReinserter->insert(dl);
  if (res->res == CKvalObs::CDataSource::OK) {
    return true;
  } else {
    METLIBS_LOG_WARN("could not store data, message='" << res->message << "'");
    return false;
  }
}
