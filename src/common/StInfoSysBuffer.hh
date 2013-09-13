// -*- c++ -*-

#ifndef StInfoSysBuffer_hh
#define StInfoSysBuffer_hh

#include "StationInfoBuffer.hh"

namespace miutil {
namespace conf {
class ConfSection;
} // namespace conf
} // namespace miutil

class StInfoSysBuffer : public StationInfoBuffer {
public:
    StInfoSysBuffer(miutil::conf::ConfSection* conf);
    ~StInfoSysBuffer();

    virtual bool isConnected();

protected:
    virtual void readStationInfo();

private:
    bool readFromStInfoSys();
};

#endif // StInfoSysBuffer_hh
