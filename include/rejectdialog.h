#ifndef REJECTDIALOG_H
#define REJECTDIALOG_H

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

class RejectDialog : public QDialog
{
    Q_OBJECT

public:
    RejectDialog( QWidget* parent = 0);
    //    ~RejectDialog();

    QLabel* textLabel3;
    QLabel* textLabel1;
    QLabel* textLabel2;
    //    QCheckBox* defineTo;
    QDateTimeEdit* fromEdit;
    QDateTimeEdit* toEdit;
    //    QCheckBox* rmXmlCheckBox;
    QPushButton* okButton;
    QPushButton* cancelButton;

    TimeSpan getTimeSpan();

    QDateTime dtto;
    QDateTime dtfrom;

public slots:
    void setFromTime(const QDateTime& dt);
    void setToTime(const QDateTime& dt);
    //    Reject getReject();
    //    bool hideXml();
    /*
protected:
    QVBoxLayout* RejectDialogLayout;
    QGridLayout* layout2;
    QHBoxLayout* Layout1;
    QSpacerItem* Horizontal_Spacing2;

protected slots:
    virtual void languageChange();

private slots:
    void timeRangeCheck();
    */
signals:
    void rejectHide();
    void rejectApply();
};

#endif // REJECTDIALOG_H
