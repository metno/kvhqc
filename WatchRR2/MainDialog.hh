
#ifndef MAINDIALOG_HH
#define MAINDIALOG_HH 1

#include "EditAccess.hh"
#include "ModelAccess.hh"
#include "TimeRange.hh"

#include <kvcpp/kvservicetypes.h>

#include <QtGui/QDialog>

#include <memory>

class DianaHelper;

QT_BEGIN_NAMESPACE
class QItemSelection;
class QModelIndex;
QT_END_NAMESPACE

namespace Ui {
class DialogMain;
}
class MainTableModel;
class NeighborDataModel;
class NeighborTableModel;

class MainDialog : public QDialog
{   Q_OBJECT;
public:
    MainDialog(EditAccessPtr da, ModelAccessPtr ma, const Sensor& sensor, const TimeRange& time, QWidget* parent=0);
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
    void onBackendDataChanged(ObsAccess::ObsDataChange what, EditDataPtr obs);
    void onNeighborDataDateChanged(const QDate&);
    void onNeighborDataTimeChanged(const timeutil::ptime& time);

    void dianaConnection(bool c);

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
    bool isRR24Selection(const Selection& sel) const;
    bool isCompleteSingleRowSelection(const Selection& sel) const;

    void initializeRR24Data();
    void addRR24Task(const timeutil::ptime& time, QString task);
    void enableSave();

private:
    std::auto_ptr<Ui::DialogMain> ui;
    std::auto_ptr<DianaHelper> mDianaHelper;
    EditAccessPtr mDA;
    Sensor mSensor;
    TimeRange mTime;
    TimeRange mEditableTime;
    std::auto_ptr<MainTableModel> mRRModel;
    std::auto_ptr<NeighborTableModel> mNeighborModel;
    std::auto_ptr<NeighborDataModel> mNeighborData;
};

#endif // MAINDIALOG_HH
