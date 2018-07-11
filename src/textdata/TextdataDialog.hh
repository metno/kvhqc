// -*- c++ -*-

#ifndef HQC_TEXTDATADIALOG_H
#define HQC_TEXTDATADIALOG_H

#include "common/TimeSpan.hh"
#include "common/Sensor.hh"

#include <QDialog>
#include <QDateTime>

class QLabel;
class QDateTimeEdit;
class QLineEdit;
class QPushButton;

class TextDataDialog : public QDialog
{ Q_OBJECT;

public:
  TextDataDialog(const SensorTime& st, QWidget* parent = 0);

  int getStationId() const
    { return stnr; }
  TimeSpan getTimeSpan() const;

public Q_SLOTS:
  void accept();

protected:
  void changeEvent(QEvent *event);

Q_SIGNALS:
  void textDataHide();
  void textDataApply();

private Q_SLOTS:
  void setStation(const QString& st);
  void setFromTime(const QDateTime& dt);
  void setToTime(const QDateTime& dt);

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
};

#endif // HQC_TEXTDATADIALOG_H
