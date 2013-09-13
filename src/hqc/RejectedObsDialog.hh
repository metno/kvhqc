// -*- c++ -*-

#ifndef HQC_REJECTEDOBSDIALOG_HH
#define HQC_REJECTEDOBSDIALOG_HH

#include "common/TimeRange.hh"

#include <QtCore/qvariant.h>
#include <QtGui/qdialog.h>
#include <QtCore/qdatetime.h>

#include <utility>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QLabel;
class QDateTimeEdit;
class QCheckBox;
class QPushButton;
typedef std::pair<QDateTime, QDateTime> TimeSpan;

class RejectedObsDialog : public QDialog
{
  Q_OBJECT;

public:
  RejectedObsDialog(QWidget* parent = 0);

  TimeRange getTimeRange() const;

Q_SIGNALS:
  void rejectHide();
  void rejectApply();

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
