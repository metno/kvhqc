
#ifndef MAINTABLEMODEL_HH
#define MAINTABLEMODEL_HH 1

#include "EditTimeColumn.hh"
#include "ObsColumn.hh"
#include "ObsTableModel.hh"
#include "ModelAccess.hh"

class MainTableModel : public ObsTableModel
{
public:
    MainTableModel(EditAccessPtr da, ModelAccessPtr ma, const Sensor& sensor, const TimeRange& time);

    int getRR24Column() const;
    void setRR24TimeRange(const TimeRange& tr);

private:
    boost::shared_ptr<EditTimeColumn> mRR24EditTime;
};

#endif /* MAINTABLEMODEL_HH */
