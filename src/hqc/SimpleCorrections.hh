
#ifndef SimpleCorrections_hh
#define SimpleCorrections_hh 1

#include "common/DataItem.hh"
#include "common/DataView.hh"

#include <QtGui/QWidget>
#include <memory>

class ChecksTableModel;
namespace Ui {
class SimpleCorrections;
}

class SimpleCorrections : public QWidget, public DataView
{   Q_OBJECT;
public:
  SimpleCorrections(QWidget* parent=0);
  ~SimpleCorrections();
                        
  virtual void setDataAccess(EditAccessPtr eda, ModelAccessPtr mda);
  virtual void navigateTo(const SensorTime&);

protected:
  virtual void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);

private:
  void enableEditing();
  void update();

private Q_SLOTS:
  void onAcceptOriginal();
  void onAcceptOriginalQC2();
  void onAcceptCorrected();
  void onAcceptCorrectedQC2();
  void onReject();
  void onRejectQC2();

  void onNewCorrected();

private:
  std::auto_ptr<Ui::SimpleCorrections> ui;
  std::auto_ptr<ChecksTableModel> mChecksModel;
  DataItemPtr mItemFlags;
  DataItemPtr mItemOriginal;
  DataItemPtr mItemCorrected;
  SensorTime mSensorTime;
};

#endif // SimpleCorrections_hh