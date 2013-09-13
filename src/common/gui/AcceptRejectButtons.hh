
#ifndef AcceptRejectButtons_hh
#define AcceptRejectButtons_hh 1

#include "common/EditAccess.hh"
#include "common/Sensor.hh"

#include <QtGui/QWidget>

#include <vector>

class QTableView;
class QItemSelection;
class QToolButton;
class QCheckBox;

class AcceptRejectButtons : public QWidget
{ Q_OBJECT;
public:
  AcceptRejectButtons(QWidget* parent=0);

  void updateModel(EditAccessPtr da, QTableView* table);

public Q_SLOTS:
  void enableButtons();
                                                           
private Q_SLOTS:
  void onAccept();
  void onReject();

private:
  void retranslateUi();

private:
  EditAccessPtr mDA;
  QTableView* mTableView;

  QToolButton *mButtonAccept;
  QToolButton *mButtonReject;
  QCheckBox *mCheckQC2;

  std::vector<SensorTime> mSelectedObs;
  bool mSelectedColumnIsOriginal;
};

#endif // AcceptRejectButtons_hh
