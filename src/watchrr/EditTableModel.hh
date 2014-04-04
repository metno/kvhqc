
#ifndef EDITTABLEMODEL_HH
#define EDITTABLEMODEL_HH

#include "common/Code2Text.hh"
#include "common/ObsTableModel.hh"

class EditTableModel : public ObsTableModel {
public:
  EditTableModel(EditAccessPtr kda, const Sensor& sensor, const TimeSpan& time);

  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role);

  const std::vector<float>& newCorrected() const
    { return mNewValues; }

  const std::vector<int>& acceptReject() const
    { return mAcceptReject; }

  void acceptAll();
  void rejectAll();

  const Sensor& sensor() const
    { return mSensor; }

  const TimeSpan& time() const
    { return mTime; }

private:
  Sensor mSensor;
  std::vector<float> mNewValues;
  std::vector<int> mAcceptReject;
  Code2TextCPtr mRR24Codes;
};

#endif /* EDITTABLEMODEL_HH */
