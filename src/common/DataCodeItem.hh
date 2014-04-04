
#ifndef DATACODEITEM_HH
#define DATACODEITEM_HH 1

#include "Code2Text.hh"
#include "DataValueItem.hh"

class DataCodeItem : public DataValueItem {
public:
  DataCodeItem(ObsColumn::Type columnType, Code2TextCPtr codes);

  virtual QVariant data(ObsData_p obs, const SensorTime& st, int role) const;

protected:
  Code2TextCPtr mCodes;
};

HQC_TYPEDEF_P(DataCodeItem);

#endif // DATACODEITEM_HH
