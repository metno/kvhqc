
#ifndef DATACORRECTEDITEM_HH
#define DATACORRECTEDITEM_HH 1

#include "DataCodeItem.hh"

class DataCorrectedItem : public DataCodeItem {
public:
  DataCorrectedItem(Code2TextCPtr codes);
  
  virtual QVariant data(ObsData_p obs, const SensorTime& st, int role) const;
  virtual bool setData(ObsData_p obs, EditAccess_p da, const SensorTime& st, const QVariant& value, int role);
  virtual QString description(bool mini) const;
};

HQC_TYPEDEF_P(DataCorrectedItem);

#endif // DATACORRECTEDITEM_HH
