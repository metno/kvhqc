
#ifndef DATAORIGINALITEM_HH
#define DATAORIGINALITEM_HH 1

#include "DataCodeItem.hh"

class DataOriginalItem : public DataCodeItem {
public:
  DataOriginalItem(Code2TextCPtr codes);

  virtual QVariant data(EditDataPtr obs, const SensorTime& st, int role) const;
  virtual QString description(bool mini) const;
};

typedef std::shared_ptr<DataOriginalItem> DataOriginalItemPtr;

#endif // DATAORIGINALITEM_HH
