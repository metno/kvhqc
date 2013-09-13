
#ifndef DATAVALUEITEM_HH
#define DATAVALUEITEM_HH 1

#include "DataItem.hh"

class DataValueItem : public DataItem {
public:
  DataValueItem(ObsColumn::Type columnType);

  virtual Qt::ItemFlags flags(EditDataPtr obs) const;
  virtual QVariant data(EditDataPtr obs, const SensorTime& st, int role) const;
  virtual int type() const;
  
protected:
  virtual float getValue(EditDataPtr obs) const;

protected:
  ObsColumn::Type mColumnType;
};

typedef boost::shared_ptr<DataValueItem> DataValueItemPtr;

#endif // DATAVALUEITEM_HH
