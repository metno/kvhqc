// -*- c++ -*-

#ifndef HQC_REJECTEDOBSDIALOG_HH
#define HQC_REJECTEDOBSDIALOG_HH

#include "common/TimeSpan.hh"

#include <QDialog>

class QLabel;
class QDateTimeEdit;
class QPushButton;

class RejectedObsDialog : public QDialog
{ Q_OBJECT;

public:
  RejectedObsDialog(QWidget* parent = 0);

  TimeSpan getTimeSpan() const;

protected:
  void changeEvent(QEvent *event);

private:
  void retranslateUi();

private:
  QLabel* textLabel3;
  QLabel* textLabel1;
  QLabel* textLabel2;

  QDateTimeEdit* fromEdit;
  QDateTimeEdit* toEdit;

  QPushButton* okButton;
  QPushButton* cancelButton;
};

#endif // HQC_REJECTEDOBSDIALOG_HH
