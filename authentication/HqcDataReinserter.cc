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

#include "HqcDataReinserter.h"
#include "hqc_utilities.hh"
#include <kvalobs/kvDataOperations.h>
#include <boost/foreach.hpp>
#include <algorithm>

using namespace kvservice;
using namespace kvalobs;

#define MILOGGER_CATEGORY "kvhqc.HqcDataReinserter"
#include "HqcLogging.hh"

namespace internal_
{

void updateUseAddCFailed(kvalobs::kvData &d)
{
    const kvalobs::kvControlInfo cinfo = d.controlinfo();
    const int fhqc = cinfo.flag(flag::fhqc);
    if (fhqc == 0)
        HQC_LOG_ERROR("inserting data with fhqc==0: " << d);
    
    kvUseInfo ui = d.useinfo();
    ui.setUseFlags(cinfo);
    ui.addToErrorCount();
    d.useinfo(ui);
}

} // namespace internal_

HqcDataReinserter::HqcDataReinserter( KvApp *app, int operatorID )
  : DataReinserter<KvApp>( app, operatorID )
{
}

HqcDataReinserter::~HqcDataReinserter( )
{
}

const HqcDataReinserter::Result HqcDataReinserter::insert(kvalobs::kvData &d) const
{
    if (d.typeID() <= -32767)
        return fail("invalid typeid <= -32767");
    ::internal_::updateUseAddCFailed(d);
    return DataReinserter<KvApp>::insert(d);
}


const HqcDataReinserter::Result HqcDataReinserter::insert(std::list<kvalobs::kvData> &dl) const
{
    BOOST_FOREACH(const kvalobs::kvData &d, dl) {
        if (d.typeID() <= -32767)
            return fail("invalid typeid <= -32767");
    }
    std::for_each(dl.begin(), dl.end(), ::internal_::updateUseAddCFailed);
    return DataReinserter<KvApp>::insert(dl);
}

const HqcDataReinserter::Result HqcDataReinserter::insert(const kvalobs::serialize::KvalobsData& data) const
{
    HQC_LOG_WARN("inserting kvalobs::serialize::KvalobsData will not update useinfo or check typeid!");
    return DataReinserter<KvApp>::insert(data);
}

CKvalObs::CDataSource::Result_var HqcDataReinserter::fail(const std::string& why) const
{
    CKvalObs::CDataSource::Result_var ret(new CKvalObs::CDataSource::Result);
    ret->res = CKvalObs::CDataSource::ERROR;
    ret->message = why.c_str();
    return ret;
}
