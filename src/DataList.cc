
#include "DataList.hh"

#include "DataListModel.hh"
#include "ObsDelegate.hh"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QFileDialog>
#include <QtGui/QFont>
#include <QtGui/QHeaderView>

#include <boost/foreach.hpp>

#include <set>

#include "ui_datalist.h"

#define M_TIME
#define MILOGGER_CATEGORY "kvhqc.DataList"
#include "HqcLogging.hh"

namespace /* anonymous */ {
QString protectForCSV(QString txt)
{
  txt.replace('\n', " ");
  txt.replace('\"', "'");
  txt.replace('\t', " ");
  return txt;
}
QString protectForCSV(const QVariant& v)
{
  return protectForCSV(v.toString());
}
} // namespace anonymous

DataList::DataList(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::DataList)
  , mEmittingNavigateTo(false)
{
  ui->setupUi(this);
  ui->buttonSaveAs->setIcon(QIcon("icons:dl_save_as.svg"));

  QFont mono("Monospace");
  ui->table->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  ui->table->horizontalHeader()->setMovable(true);
  ui->table->verticalHeader()->setFont(mono);
  ui->table->verticalHeader()->setDefaultSectionSize(20);

  ui->table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  ui->table->setSelectionBehavior(QAbstractItemView::SelectItems);
  ui->table->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui->table->setItemDelegate(new ObsDelegate(this));

  connect(ui->table, SIGNAL(currentChanged(const QModelIndex&)),
      this, SLOT(currentChanged(const QModelIndex&)));
}

DataList::~DataList()
{
}

void DataList::navigateTo(const SensorTime& st)
{
  METLIBS_LOG_TIME();
  if (mEmittingNavigateTo or not mTableModel.get() or not st.valid() or eq_SensorTime()(mSensorTime, st))
    return;

  mSensorTime = st;
  METLIBS_LOG_DEBUG(LOGVAL(mSensorTime));

  const QModelIndexList idxs = mTableModel->findIndexes(st);

  const QModelIndex& currentIdx = ui->table->currentIndex();
  QItemSelection selection;
  bool scroll = (not idxs.empty());
  BOOST_FOREACH(const QModelIndex idx, idxs) {
    selection.select(idx, idx);
    if (idx == currentIdx)
      scroll = false;
  }
  if (scroll) {
    ui->table->scrollTo(idxs.front());
    ui->table->scrollTo(idxs.back());
  }
  if (QItemSelectionModel* sm = ui->table->selectionModel())
    sm->select(selection, QItemSelectionModel::ClearAndSelect);
}

void DataList::currentChanged(const QModelIndex& current)
{
  METLIBS_LOG_SCOPE();
  const SensorTime st = mTableModel->findSensorTime(current);
  METLIBS_LOG_DEBUG(LOGVAL(st));
  if (st.valid() and not eq_SensorTime()(mSensorTime, st)) {
    mSensorTime = st;
    mEmittingNavigateTo = true;
    /*emit*/ signalNavigateTo(st);
    mEmittingNavigateTo = false;
  }
}

void DataList::updateModel(DataListModel* newModel)
{
  mTableModel.reset(newModel);
  ui->table->setModel(mTableModel.get());
  connect(ui->table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
  
  ui->buttonsAcceptReject->updateModel(mDA, ui->table);
}

void DataList::onSelectionChanged(const QItemSelection&, const QItemSelection&)
{
}

void DataList::onButtonSaveAs()
{
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save Table"),
      "datalist.csv",
      tr("CSV Table (*.csv)"));
  if (fileName.isEmpty())
    return;
  
  QFile file(fileName);
  file.open(QIODevice::WriteOnly | QIODevice::Text);

  int r0, r1, c0, c1;

  std::set<int> selectedRows, selectedColumns;
  std::set< std::pair<int,int> > selectedCells;
  QModelIndexList selected;
  if (QItemSelectionModel* sm = ui->table->selectionModel())
    selected = sm->selectedIndexes();
  if (not selected.isEmpty()) {
    for (int i=0; i<selected.count(); i++) {
      const int r = selected.at(i).row(), c = selected.at(i).column();
      selectedRows.insert(r);
      selectedColumns.insert(c);
      selectedCells.insert(std::make_pair(r, c));
    }
    r0 = *selectedRows.begin();
    r1 = *selectedRows.rbegin();
    c0 = *selectedColumns.begin();
    c1 = *selectedColumns.rbegin();
  } else {
    r0 = 0;
    r1 = mTableModel->rowCount(QModelIndex())-1; // no problem if < 0
    c0 = 0;
    c1 = mTableModel->columnCount(QModelIndex())-1; // no problem if < 0
  }

  QTextStream out(&file);

  out << "\"\"";
  for (int c=c0; c<=c1; ++c) {
    if (selectedColumns.empty() or selectedColumns.find(c) != selectedColumns.end())
      out << "\t\"" << protectForCSV(mTableModel->headerData(c, Qt::Horizontal, Qt::ToolTipRole)) << '\"';
  }
  out << "\n";

  for (int r=r0; r<=r1; ++r) {
    if (selectedRows.empty() or selectedRows.find(r) != selectedRows.end()) {
      out << '\"' << protectForCSV(mTableModel->headerData(r, Qt::Vertical, Qt::DisplayRole)) << '\"';
      for (int c=c0; c<=c1; ++c) {
        if (selectedColumns.empty() or selectedColumns.find(c) != selectedColumns.end()) {
          QString cell;
          if (selectedCells.empty() or selectedCells.find(std::make_pair(r, c)) != selectedCells.end())
            cell = protectForCSV(mTableModel->data(mTableModel->index(r, c), Qt::DisplayRole));
          out << "\t\"" << cell << '\"';
        }
      }
      out << "\n";
    }
  }
 
  file.close(); 
}
