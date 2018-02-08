
#ifndef HQC_OBSPGM_H
#define HQC_OBSPGM_H 1

#include <kvalobs/kvObsPgm.h>

#include <vector>

namespace hqc {

//! Struct for obs_pgm including prority_message.
class hqcObsPgm : public kvalobs::kvObsPgm
{
public:
  hqcObsPgm(int stationid, int paramid, int level, int nr_sensor, int typ, bool priority_message, bool collector, bool kl00, bool kl01, bool kl02, bool kl03,
            bool kl04, bool kl05, bool kl06, bool kl07, bool kl08, bool kl09, bool kl10, bool kl11, bool kl12, bool kl13, bool kl14, bool kl15, bool kl16,
            bool kl17, bool kl18, bool kl19, bool kl20, bool kl21, bool kl22, bool kl23, bool mon, bool tue, bool wed, bool thu, bool fri, bool sat, bool sun,
            const boost::posix_time::ptime& fromtime, const boost::posix_time::ptime& totime);

  explicit hqcObsPgm(const kvalobs::kvObsPgm& kvo, bool priority_message);

  bool priority_message() const { return priority_message_; }

private:
  bool priority_message_;
};

typedef std::vector<hqcObsPgm> hqcObsPgm_v;

} // namespace hqc

#endif // HQC_OBSPGM_H
