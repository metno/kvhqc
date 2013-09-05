
#ifndef WEATHERDIALOG_HH
#define WEATHERDIALOG_HH 1

#include "EditAccess.hh"
#include "TimeRange.hh"

#include <QtGui/QDialog>

#include <memory>

QT_BEGIN_NAMESPACE
class QItemSelection;
class QModelIndex;
QT_END_NAMESPACE

namespace Ui {
class WeatherDialog;
}
class WeatherTableModel;

class WeatherDialog : public QDialog
{ Q_OBJECT;
public:
  WeatherDialog(EditAccessPtr da, const Sensor& sensor, const TimeRange& time, QWidget* parent=0);
  ~WeatherDialog();

//public Q_SLOTS:
//    virtual void reject();
//                                                                                     
//private Q_SLOTS:
//    void onAcceptRow();
//    void onEdit();
//    void onRedistribute();
//    void onRedistributeQC2();
//    void onUndo();
//    void onSelectionChanged(const QItemSelection&, const QItemSelection&);
//    void onDataChanged(const QModelIndex&, const QModelIndex&);
//    void onNeighborDataDateChanged(const QDate&);
//    void onNeighborDataTimeChanged(const timeutil::ptime& time);
//
//    void dianaConnection(bool c);
//
//private:
//    struct Selection {
//        TimeRange selTime;
//        int minCol;
//        int maxCol;
//        Selection()
//            : minCol(-1), maxCol(-1) { }
//        Selection(const TimeRange& s, int mic, int mac)
//            : selTime(s), minCol(mic), maxCol(mac) { }
//        bool empty() const
//            { return minCol<0 or maxCol<0; }
//    };
//
private:
  void onBackendDataChanged(ObsAccess::ObsDataChange what, EditDataPtr obs);
//    Selection findSelection();
//    void clearSelection();
//    bool isRR24Selection(const Selection& sel) const;
//    bool isCompleteSingleRowSelection(const Selection& sel) const;
//
//    void initializeRR24Data();
//    void addRR24Task(const timeutil::ptime& time, QString task);
//    void enableSave();

private:
    std::auto_ptr<Ui::WeatherDialog> ui;
    EditAccessPtr mDA;
    Sensor mSensor;
    TimeRange mTime;
    std::auto_ptr<WeatherTableModel> mModelCorrected;
    std::auto_ptr<WeatherTableModel> mModelOriginal;
    std::auto_ptr<WeatherTableModel> mModelFlags;
};

#endif // WEATHERDIALOG_HH
