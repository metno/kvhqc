// -*- c++ -*-

#ifndef TEXTDATADIALOG_H
#define TEXTDATADIALOG_H

#include <kvalobs/kvStation.h>

#include <QtCore/qvariant.h>
#include <QtGui/qdialog.h>
#include <QtCore/qdatetime.h>
#include <QtGui/qlineedit.h>

#include <list>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QLabel;
class QDateTimeEdit;
class QLineEdit;
class QCheckBox;
class QPushButton;
typedef std::pair<QDateTime, QDateTime> TimeSpan;

class TextDataDialog : public QDialog
{
    Q_OBJECT

public:
  TextDataDialog(const std::list<kvalobs::kvStation>& slist, QWidget* parent = 0);
    //    ~TextDataDialog();

    QLabel* textLabel0;
    QLabel* textLabel1;
    QLabel* textLabel2;
    QLabel* textLabel3;

    QLineEdit* stationEdit;

    QDateTimeEdit* fromEdit;
    QDateTimeEdit* toEdit;

    QPushButton* okButton;
    QPushButton* cancelButton;

    TimeSpan getTimeSpan();

    int stnr;
    QDateTime dtto;
    QDateTime dtfrom;
private:
    const std::list<kvalobs::kvStation>& stationList;

public slots:
    void setStation(const QString& st);
    void setFromTime(const QDateTime& dt);
    void setToTime(const QDateTime& dt);
    void checkStationId();

signals:
    void textDataHide();
    void textDataApply();
};

#endif // REJECTDIALOG_H
