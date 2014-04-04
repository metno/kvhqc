
#ifndef WATCHRRDIALOG_HH
#define WATCHRRDIALOG_HH 1

#include "common/EditAccess.hh"
#include "common/ModelAccess.hh"
#include "common/TimeSpan.hh"

#include <QtGui/QDialog>

#include <memory>

class DianaHelper;

QT_BEGIN_NAMESPACE
class QItemSelection;
class QModelIndex;
QT_END_NAMESPACE

namespace Ui {
class DialogMain;
}
class StationCardModel;
class NeighborRR24Model;
class NeighborCardsModel;

class WatchRRDialog : public QDialog
{   Q_OBJECT;
public:
  WatchRRDialog(EditAccessPtr da, ModelAccessPtr ma, const Sensor& sensor, const TimeSpan& time, QWidget* parent=0);
  ~WatchRRDialog();

public Q_SLOTS:
  virtual void accept();
  virtual void reject();
  
protected:
  virtual void changeEvent(QEvent *event);

private Q_SLOTS:
  void onAcceptRow();
  void onEdit();
  void onRedistribute();
  void onRedistributeQC2();
  void onUndo();
  void onRedo();
  void onSelectionChanged(const QItemSelection&, const QItemSelection&);
  void onDataChanged(const QModelIndex&, const QModelIndex&);
  void onBackendDataChanged(ObsAccess::ObsDataChange what, EditDataPtr obs);
  void onNeighborDataDateChanged(const QDate&);
  void onNeighborDataTimeChanged(const timeutil::ptime& time);
  void onCurrentTabChanged(int tab);

  void dianaConnection(bool c);

private:
  struct Selection {
    TimeSpan selTime;
    int minCol;
    int maxCol;
    Selection()
      : minCol(-1), maxCol(-1) { }
    Selection(const TimeSpan& s, int mic, int mac)
      : selTime(s), minCol(mic), maxCol(mac) { }
    bool empty() const
      { return minCol<0 or maxCol<0; }
  };

private:
  void setStationInfoText();
  Selection findSelection();
  void clearSelection();
  bool isRR24Selection(const Selection& sel) const;
  bool isCompleteSingleRowSelection(const Selection& sel) const;

  void initializeRR24Data();
  void addRR24Task(const timeutil::ptime& time, QString task);
  void enableSave();

private:
  std::auto_ptr<Ui::DialogMain> ui;
  std::auto_ptr<DianaHelper> mDianaHelper;
  EditAccessPtr mParentDA, mDA;
  Sensor mSensor;
  TimeSpan mTime;
  TimeSpan mEditableTime;
  std::auto_ptr<StationCardModel> mStationCard;
  std::auto_ptr<NeighborRR24Model> mNeighborRR24;
  std::auto_ptr<NeighborCardsModel> mNeighborCards;
};

#endif // WATCHRRDIALOG_HH
