
#ifndef KVTYPES_HH
#define KVTYPES_HH 1

#include <kvalobs/kvObsPgm.h>
#include <kvalobs/kvParam.h>
#include <kvalobs/kvRejectdecode.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvTextData.h>
#include <kvalobs/kvTypes.h>

#include <kvalobs/kvData.h>
#include <kvalobs/kvModelData.h>

#include <vector>
#include <set>

namespace hqc {

typedef std::vector<kvalobs::kvObsPgm> kvObsPgm_v;
typedef std::vector<kvalobs::kvParam> kvParam_v;
typedef std::vector<kvalobs::kvRejectdecode> kvRejectdecode_v;
typedef std::vector<kvalobs::kvStation> kvStation_v;
typedef std::vector<kvalobs::kvTextData> kvTextData_v;
typedef std::vector<kvalobs::kvTypes> kvTypes_v;

typedef std::vector<kvalobs::kvData> kvData_v;
typedef std::vector<kvalobs::kvModelData> kvModelData_v;

typedef std::set<int> int_s;
typedef std::vector<int> int_v;

}

#endif // KVTYPES_HH
