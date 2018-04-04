
#ifndef WRAPPERCOLUMN_HH
#define WRAPPERCOLUMN_HH 1

#include "common/DataColumn.hh"

class WrapperColumn : public ObsColumn {
protected:
  WrapperColumn(DataColumn_p dc);

public:
  ~WrapperColumn();
  
  virtual void attach(ObsTableModel* table)
    { mDC->attach(table); }

  virtual void detach(ObsTableModel* table)
    { mDC->detach(table); }

  virtual Qt::ItemFlags flags(const timeutil::ptime& time) const
    { return mDC->flags(time); }

  virtual QVariant data(const timeutil::ptime& time, int role) const
    { return mDC->data(time, role); }

  virtual bool setData(const timeutil::ptime& time, const QVariant& value, int role)
    { return mDC->setData(time, value, role); }

  virtual QVariant headerData(Qt::Orientation orientation, int role) const
    { return mDC->headerData(orientation, role); }
  
  virtual int type() const
    { return mDC->type(); }

protected:
  DataColumn_p mDC;
};

#endif // WRAPPERCOLUMN_HH
