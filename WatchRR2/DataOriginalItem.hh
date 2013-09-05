
#ifndef DATAORIGINALITEM_HH
#define DATAORIGINALITEM_HH 1

#include "Code2Text.hh"
#include "DataValueItem.hh"

class DataOriginalItem : public DataValueItem {
public:
  DataOriginalItem(Code2TextCPtr codes);
  
  virtual QVariant data(EditDataPtr obs, const SensorTime& st, int role) const;
  virtual QString description(bool mini) const;

protected:
  Code2TextCPtr mCodes;
};

typedef boost::shared_ptr<DataOriginalItem> DataOriginalItemPtr;

#endif // DATAORIGINALITEM_HH
