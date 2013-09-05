
#ifndef StationSelection_hh
#define StationSelection_hh 1

#include "StInfoSysBuffer.hh"

#include <QtGui/QDialog>
#include <Qt3Support/Q3Table>

#include <memory>
#include <set>

namespace Ui {
class StationSelectionDialog;
}

class StationTable : public Q3Table
{ Q_OBJECT;
public:
    StationTable(QWidget* parent=0);
    void setData(const listStat_l& listStat, const QStringList& counties);
    void sortColumn( int col, bool ascending, bool wholeRows );
private:
    QString getEnvironment(const int envID, const std::set<int>& typeIDs);
};

class StationSelection : public QDialog
{ Q_OBJECT;
public:
    StationSelection(const listStat_l& listStat,
                     const QStringList& counties,
                     QWidget* parent);

    std::vector<int> getSelectedStations();

public Q_SLOTS:
    void doSelectAllStations();

private Q_SLOTS:
    void tableCellClicked(int, int, int, const QPoint&);
    void tableCellClicked(int, int);
    void tableCellClicked();

Q_SIGNALS:
  void stationAppended(QString);
  void stationRemoved(QString);

private:
    void selectOrDeselectStation(int row);

private:
    std::auto_ptr<Ui::StationSelectionDialog> ui;
    std::set<int> mSelectedStations;
};

class StTableItem : public Q3TableItem{
public:
  StTableItem( Q3Table *t, EditType et, const QString &txt ) :
    Q3TableItem( t, et, txt ) {}
  QString key() const;
};

#endif // StationSelection_hh
