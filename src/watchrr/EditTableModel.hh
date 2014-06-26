
#ifndef EDITTABLEMODEL_HH
#define EDITTABLEMODEL_HH

#include "TaskAccess.hh"
#include "common/Code2Text.hh"
#include "WatchRRTableModel.hh"

class EditTableModel : public WatchRRTableModel {
public:
  EditTableModel(TaskAccess_p kda, const Sensor& sensor, const TimeSpan& time, QObject* parent=0);

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
  TimeSpan mTime;
  std::vector<float> mNewValues;
  std::vector<int> mAcceptReject;
  Code2TextCPtr mRR24Codes;
};

#endif /* EDITTABLEMODEL_HH */
