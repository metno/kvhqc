
#ifndef DATACORRECTEDITEM_HH
#define DATACORRECTEDITEM_HH 1

#include "DataCodeItem.hh"

class DataCorrectedItem : public DataCodeItem {
public:
  DataCorrectedItem(bool showNew, Code2TextCPtr codes);

  virtual QVariant data(EditDataPtr obs, const SensorTime& st, int role) const;
  virtual bool setData(EditDataPtr obs, EditAccessPtr da, const SensorTime& st, const QVariant& value, int role);
  virtual QString description(bool mini) const;
};

typedef std::shared_ptr<DataCorrectedItem> DataCorrectedItemPtr;

#endif // DATACORRECTEDITEM_HH
