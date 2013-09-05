
#ifndef DATACORRECTEDITEM_HH
#define DATACORRECTEDITEM_HH 1

#include "Code2Text.hh"
#include "DataValueItem.hh"

class DataCorrectedItem : public DataValueItem {
public:
  DataCorrectedItem(bool showNew, Code2TextCPtr codes);
  
  virtual QVariant data(EditDataPtr obs, int role) const;
  virtual bool setData(EditDataPtr obs, EditAccessPtr da, const SensorTime& st, const QVariant& value, int role);
  virtual QString description(bool mini) const;

protected:
  Code2TextCPtr mCodes;
};

typedef boost::shared_ptr<DataCorrectedItem> DataCorrectedItemPtr;

#endif // DATACORRECTEDITEM_HH
