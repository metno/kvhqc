
#ifndef REDISTTABLEMODEL_HH
#define REDISTTABLEMODEL_HH

#include "common/Code2Text.hh"
#include "common/ObsTableModel.hh"

class RedistTableModel : public ObsTableModel {
public:
    RedistTableModel(EditAccessPtr da, const Sensor& sensor, const TimeSpan& time);
    virtual ~RedistTableModel();

    const std::vector<float>& newCorrected() const
        { return mNewValues; }

    void setNewCorrected(const std::vector<float>& nc);

    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role);

    float originalSum() const;
    float currentSum() const;
    bool hasManualChanges() const;

    const Sensor& sensor() const
        { return mSensor; }

    const TimeSpan& time() const
        { return mTime; }

private:
    Sensor mSensor;
    std::vector<float> mNewValues;
    Code2TextCPtr mRR24Codes;
};

#endif /* REDISTTABLEMODEL_HH */
