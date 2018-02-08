#include "hqcObsPgm.h"

namespace hqc {

hqcObsPgm::hqcObsPgm(int stationid, int paramid, int level, int nr_sensor, int typ, bool priority_message, bool collector, bool kl00, bool kl01, bool kl02,
                     bool kl03, bool kl04, bool kl05, bool kl06, bool kl07, bool kl08, bool kl09, bool kl10, bool kl11, bool kl12, bool kl13, bool kl14,
                     bool kl15, bool kl16, bool kl17, bool kl18, bool kl19, bool kl20, bool kl21, bool kl22, bool kl23, bool mon, bool tue, bool wed, bool thu,
                     bool fri, bool sat, bool sun, const boost::posix_time::ptime& fromtime, const boost::posix_time::ptime& totime)
    : kvalobs::kvObsPgm(stationid, paramid, level, nr_sensor, typ, collector, kl00, kl01, kl02, kl03, kl04, kl05, kl06, kl07, kl08, kl09, kl10, kl11, kl12,
                        kl13, kl14, kl15, kl16, kl17, kl18, kl19, kl20, kl21, kl22, kl23, mon, tue, wed, thu, fri, sat, sun, fromtime, totime)
    , priority_message_(priority_message)
{
}

hqcObsPgm::hqcObsPgm(const kvObsPgm& kvo, bool priority_message)
    : kvalobs::kvObsPgm(kvo)
    , priority_message_(priority_message)
{
}

} // namespace hqc
