
#ifndef SimpleCorrections_hh
#define SimpleCorrections_hh 1

#include "common/SingleObsBuffer.hh"

#include "common/ModelBuffer.hh"
#include "common/DataItem.hh"

#include <QWidget>
#include <memory>

class ChecksTableModel;
class DataHistoryTableModel;
QT_BEGIN_NAMESPACE
class Ui_SingleObservation;
QT_END_NAMESPACE

class SimpleCorrections : public QWidget
{ Q_OBJECT;
public:
  SimpleCorrections(EditAccess_p eda, ModelAccess_p mda, QWidget* parent=0);
  ~SimpleCorrections();
  
public Q_SLOTS:
  virtual void navigateTo(const SensorTime&);
  
protected:
  virtual void changeEvent(QEvent *event);
  
private:
  void enableEditing();

private Q_SLOTS:
  void onAcceptOriginal();
  void onAcceptModel();
  void onAcceptCorrected();
  void onReject();
  void onQc2Toggled(bool);

  void onNewCorrected();
  void onStartEditor();

  void update();
  void onDataChanged();

  void onHistoryTableUpdated();

private:
  std::unique_ptr<Ui_SingleObservation> ui;
  EditAccess_p mDA;
  ModelAccess_p mMA;
  ModelBuffer_p mModelBuffer;
  SingleObsBuffer_p mObsBuffer;

  ChecksTableModel *mChecksModel;
  DataHistoryTableModel *mHistoryModel;
  DataItem_p mItemFlags;
  DataItem_p mItemOriginal;
  DataItem_p mItemCorrected;
  SensorTime mSensorTime;
};

#endif // SimpleCorrections_hh
