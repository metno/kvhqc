
#ifndef EDITTIMECOLUMN_HH
#define EDITTIMECOLUMN_HH 1

#include "DataColumn.hh"
#include "TimeRange.hh"

class EditTimeColumn : public ObsColumn {
public:
    EditTimeColumn(DataColumnPtr oc);
    ~EditTimeColumn();

    virtual Qt::ItemFlags flags(const timeutil::ptime& time) const;
    virtual QVariant data(const timeutil::ptime& time, int role) const;
    virtual bool setData(const timeutil::ptime& time, const QVariant& value, int role);
    virtual QVariant headerData(Qt::Orientation orientation, int role) const;

    void setEditableTime(const TimeRange& et);
    const boost::posix_time::time_duration& timeOffset() const;
    virtual int type() const
      { return mDC->type(); }

private:
    void onColumnChanged(const timeutil::ptime& time, ObsColumn* c);

private:
    DataColumnPtr mDC;
    TimeRange mEditableTime;
};

#endif // EDITTIMECOLUMN_HH
