
#ifndef AcceptRejectButtons_hh
#define AcceptRejectButtons_hh 1

#include "common/EditAccess.hh"
#include "common/ModelAccess.hh"
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

  void updateModel(EditAccess_p da, ModelAccess_p ma, QTableView* table);

public Q_SLOTS:
  void enableButtons();
                                                           
protected:
  virtual void changeEvent(QEvent *event);

private Q_SLOTS:
  void onAccept();
  void onReject();

private:
  void retranslateUi();

private:
  EditAccess_p mDA;
  ModelAccess_p mMA;
  QTableView* mTableView;

  QToolButton *mButtonAccept;
  QToolButton *mButtonReject;
  QCheckBox *mCheckQC2;

  ObsData_pv mSelectedObs;

  enum SelectedColumnType { OTHER, ORIGINAL, CORRECTED, MODEL };
  SelectedColumnType mSelectedColumnType;
};

#endif // AcceptRejectButtons_hh
