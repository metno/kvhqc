
#ifndef EDITTIMECOLUMN_HH
#define EDITTIMECOLUMN_HH 1

#include "DataColumn.hh"
#include "TimeSpan.hh"

class EditTimeColumn : public ObsColumn {
public:
    EditTimeColumn(DataColumn_p oc);
    ~EditTimeColumn();

    virtual Qt::ItemFlags flags(const timeutil::ptime& time) const;
    virtual QVariant data(const timeutil::ptime& time, int role) const;
    virtual bool setData(const timeutil::ptime& time, const QVariant& value, int role);
    virtual QVariant headerData(Qt::Orientation orientation, int role) const;

    void setEditableTime(const TimeSpan& et);
    const boost::posix_time::time_duration& timeOffset() const;
    virtual int type() const
      { return mDC->type(); }

private:
    void onColumnChanged(const timeutil::ptime& time, ObsColumn_p c);

private:
    DataColumn_p mDC;
    TimeSpan mEditableTime;
};

#endif // EDITTIMECOLUMN_HH
