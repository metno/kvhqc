
#include "DataList.hh"

#include "common/DataListModel.hh"
#include "common/gui/ObsDelegate.hh"

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
#include "common/ObsLogging.hh"

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

const int NHOURS = 5, HOURS[NHOURS] = { 1, 3, 6, 12, 24 };
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

  ui->comboTimeStep->addItem(tr("none"), QVariant(0));
  { const int NMINUTES = 5, MINUTES[NMINUTES] = { 1, 5, 10, 15, 30 };
    for (int i=0; i<NMINUTES; ++i)
      ui->comboTimeStep->addItem(tr("%1 min").arg(MINUTES[i]), QVariant(MINUTES[i]*60));
  }

  { const int NHOURS = 5, HOURS[NHOURS] = { 1, 3, 6, 12, 24 };
    for (int i=0; i<NHOURS; ++i)
      ui->comboTimeStep->addItem(tr("%1 h").arg(HOURS[i]), QVariant(HOURS[i]*60*60));
  }
  ui->checkFilterTimes->setEnabled(false);

  connect(ui->table, SIGNAL(signalCurrentChanged(const QModelIndex&)),
      this, SLOT(onCurrentChanged(const QModelIndex&)));
}

DataList::~DataList()
{
}

void DataList::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange) {
    ui->retranslateUi(this);
    ui->comboTimeStep->setItemText(0, tr("none"));
    for (int i=0; i<NHOURS; ++i)
      ui->comboTimeStep->setItemText(i+1, tr("%1 h").arg(HOURS[i]));
  }
  QWidget::changeEvent(event);
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
  BOOST_FOREACH(const QModelIndex& idx, idxs) {
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

void DataList::onCurrentChanged(const QModelIndex& current)
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
  onUITimeStepChanged(ui->comboTimeStep->currentIndex());
  ui->table->setModel(mTableModel.get());
  connect(mTableModel.get(), SIGNAL(changedTimeStep(int)),
      this, SLOT(onModelTimeStepChanged(int)));
  connect(mTableModel.get(), SIGNAL(changedFilterByTimestep(bool, bool)),
      this, SLOT(onModelFilterByTimeStepChanged(bool, bool)));
  connect(ui->table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
  
  ui->buttonsAcceptReject->updateModel(mDA, mMA, ui->table);
  ui->toolInterpolate->updateModel(mDA, ui->table);
}

void DataList::onSelectionChanged(const QItemSelection&, const QItemSelection&)
{
}

void DataList::onCheckFilter(bool filterByTimestep)
{
  mTableModel->setFilterByTimestep(filterByTimestep);
}

void DataList::onUITimeStepChanged(int index)
{
  if (not mTableModel.get())
    return;

  int step = 0;
  if (index >= 0)
    step = ui->comboTimeStep->itemData(index).toInt();
  mTableModel->setTimeStep(step);
}

void DataList::onModelTimeStepChanged(int step)
{
  if (not mTableModel.get())
    return;

  for (int i=0; i<ui->comboTimeStep->count(); ++i) {
    const int combostep = ui->comboTimeStep->itemData(i).toInt();
    if (step == combostep) {
      ui->comboTimeStep->setCurrentIndex(i);
      return;
    }
  }
  METLIBS_LOG_WARN("datalist model time step not in combo list");
}

void DataList::onModelFilterByTimeStepChanged(bool enabled, bool ftbs)
{
  ui->checkFilterTimes->setEnabled(enabled);
  ui->checkFilterTimes->setChecked(ftbs);
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
