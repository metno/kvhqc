
#ifndef SimpleCorrections_hh
#define SimpleCorrections_hh 1

#include "common/DataItem.hh"

#include <QtGui/QWidget>
#include <memory>

class ChecksTableModel;
namespace Ui {
class SimpleCorrections;
}

class SimpleCorrections : public QWidget
{ Q_OBJECT;
public:
  SimpleCorrections(QWidget* parent=0);
  ~SimpleCorrections();
  
  virtual void setDataAccess(EditAccess_p eda, ModelAccess_p mda);

public Q_SLOTS:
  virtual void navigateTo(const SensorTime&);
  
protected:
  //virtual void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);
  virtual void changeEvent(QEvent *event);
  
private:
  void enableEditing();
  void update();
  void adjustSizes();

private Q_SLOTS:
  void onAcceptOriginal();
  void onAcceptCorrected();
  void onAcceptCorrectedQC2();
  void onReject();
  void onRejectQC2();

  void onNewCorrected();

private:
  std::auto_ptr<Ui::SimpleCorrections> ui;
  std::auto_ptr<ChecksTableModel> mChecksModel;
  DataItem_p mItemFlags;
  DataItem_p mItemOriginal;
  DataItem_p mItemCorrected;
  SensorTime mSensorTime;
};

#endif // SimpleCorrections_hh
