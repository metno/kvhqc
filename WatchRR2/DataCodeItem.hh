
#ifndef DATACODEITEM_HH
#define DATACODEITEM_HH 1

#include "Code2Text.hh"
#include "DataValueItem.hh"

class DataCodeItem : public DataValueItem {
public:
  DataCodeItem(ObsColumn::Type columnType, Code2TextCPtr codes);

  virtual QVariant data(EditDataPtr obs, const SensorTime& st, int role) const;

protected:
  Code2TextCPtr mCodes;
};

typedef boost::shared_ptr<DataCodeItem> DataCodeItemPtr;

#endif // DATACODEITEM_HH
