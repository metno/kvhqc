
#ifndef DATAORIGINALITEM_HH
#define DATAORIGINALITEM_HH 1

#include "DataCodeItem.hh"

class DataOriginalItem : public DataCodeItem {
public:
  DataOriginalItem(Code2TextCPtr codes);
  
  virtual QVariant data(ObsData_p obs, const SensorTime& st, int role) const;
  virtual QString description(bool mini) const;
};

HQC_TYPEDEF_P(DataOriginalItem);

#endif // DATAORIGINALITEM_HH
