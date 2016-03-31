/* -*- c++ -*-

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

#ifndef COMMON_HQCKAFKAREINSERTER_HH
#define COMMON_HQCKAFKAREINSERTER_HH

#include "common/AbstractReinserter.hh"

#include <string>

namespace kvalobs {
namespace service {
class KafkaProducerThread;
} // namespace service
} // namespace kvalobs

class HqcKafkaReinserter : public AbstractReinserter
{
public:
  HqcKafkaReinserter(const std::string& brokers, const std::string& domain, int operatorID);
  ~HqcKafkaReinserter();

  void shutdown();

  bool insert(std::list<kvalobs::kvData> &dl) const;

private:
  void updateUseAddCFailed(kvalobs::kvData &d) const;

private:
  std::unique_ptr<kvalobs::service::KafkaProducerThread> mProducerThread;
  int mOperatorId;
};

#endif // COMMON_HQCKAFKAREINSERTER_HH
