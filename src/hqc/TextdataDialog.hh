// -*- c++ -*-

#ifndef HQC_TEXTDATADIALOG_H
#define HQC_TEXTDATADIALOG_H

#include "common/TimeRange.hh"
#include <QtGui/qdialog.h>
#include <QtCore/qdatetime.h>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QLabel;
class QDateTimeEdit;
class QLineEdit;
class QCheckBox;
class QPushButton;

class TextDataDialog : public QDialog
{ Q_OBJECT;

public:
  TextDataDialog(QWidget* parent = 0);

  int getStationId() const
    { return stnr; }
  TimeRange getTimeRange() const;

protected:
  virtual void changeEvent(QEvent *event);

private:
  void retranslateUi();

  QLabel* textLabel0;
  QLabel* textLabel1;
  QLabel* textLabel2;
  QLabel* textLabel3;

  QLineEdit* stationEdit;

  QDateTimeEdit* fromEdit;
  QDateTimeEdit* toEdit;

  QPushButton* okButton;
  QPushButton* cancelButton;

  int stnr;
  QDateTime dtto;
  QDateTime dtfrom;

public Q_SLOTS:
  void setStation(const QString& st);
  void setFromTime(const QDateTime& dt);
  void setToTime(const QDateTime& dt);
  void checkStationId();

Q_SIGNALS:
  void textDataHide();
  void textDataApply();
};

#endif // HQC_TEXTDATADIALOG_H
