
#ifndef MAINDIALOG_HH
#define MAINDIALOG_HH 1

#include "EditAccess.hh"
#include "ModelAccess.hh"
#include "TimeRange.hh"

#include <kvcpp/kvservicetypes.h>

#include <QtGui/QDialog>

#include <memory>

QT_BEGIN_NAMESPACE
class QItemSelection;
class QModelIndex;
QT_END_NAMESPACE

namespace Ui {
class DialogMain;
}
class MainTableModel;

class MainDialog : public QDialog
{   Q_OBJECT;
public:
    MainDialog(EditAccessPtr da, ModelAccessPtr ma, const Sensor& sensor, const TimeRange& time);
    ~MainDialog();

public Q_SLOTS:
    virtual void reject();
                                                                                     
private Q_SLOTS:
    void onAcceptRow();
    void onEdit();
    void onRedistribute();
    void onRedistributeQC2();
    void onUndo();
    void onSelectionChanged(const QItemSelection&, const QItemSelection&);
    void onDataChanged(const QModelIndex&, const QModelIndex&);

    void onKvData(kvservice::KvObsDataListPtr data);

private:
    struct Selection {
        TimeRange selTime;
        int minCol;
        int maxCol;
        Selection()
            : minCol(-1), maxCol(-1) { }
        Selection(const TimeRange& s, int mic, int mac)
            : selTime(s), minCol(mic), maxCol(mac) { }
        bool empty() const
            { return minCol<0 or maxCol<0; }
    };

private:
    Selection findSelection();
    void clearSelection();

    void initializeRR24Data();
    void addRR24Task(const timeutil::ptime& time, QString task);
    void enableSave();

private:
    std::auto_ptr<Ui::DialogMain> ui;
    EditAccessPtr mDA;
    Sensor mSensor;
    TimeRange mTime;
    TimeRange mEditableTime;
    MainTableModel* mModel;
};

#endif // MAINDIALOG_HH
