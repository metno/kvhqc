
#ifndef REDISTDIALOG_HH
#define REDISTDIALOG_HH

#include "common/EditAccess.hh"
#include "common/TimeRange.hh"

#include <QtGui/QDialog>

#include <memory>

QT_BEGIN_NAMESPACE
class QItemSelection;
class QModelIndex;
QT_END_NAMESPACE

namespace Ui {
class DialogRedist;
}
class RedistTableModel;

class RedistDialog : public QDialog
{   Q_OBJECT;
public:
    RedistDialog(QDialog* parent, EditAccessPtr da, const Sensor& sensor, const TimeRange& time, const TimeRange& editableTime);
    virtual ~RedistDialog();

private Q_SLOTS:
    void onButtonOk();
    void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

private:
    void updateSumInfo();

private:
    EditAccessPtr mDA;
    TimeRange mEditableTime;
    std::auto_ptr<RedistTableModel> rtm;
    std::auto_ptr<Ui::DialogRedist> ui;
};

#endif // REDISTDIALOG_HH
