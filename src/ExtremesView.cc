
#include "ExtremesView.hh"
#include "ExtremesTableModel.hh"

#include <QtGui/QHeaderView>

#include <boost/foreach.hpp>

#include "ui_extremevalues.h"

#define MILOGGER_CATEGORY "kvhqc.ExtremesView"
#include "HqcLogging.hh"

ExtremesView::ExtremesView(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::DialogExtremeValues)
  , mLastSelectedRow(-1)
{
  METLIBS_LOG_SCOPE();
  ui->setupUi(this);
  ui->tableExtremes->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  ui->tableExtremes->verticalHeader()->setDefaultSectionSize(20);
  ui->tableExtremes->verticalHeader()->hide();
  ui->tableExtremes->setSelectionBehavior(QTableView::SelectRows);
  ui->tableExtremes->setSelectionMode(QTableView::SingleSelection);
}

ExtremesView::~ExtremesView()
{
}

void ExtremesView::setExtremes(EditAccessPtr eda, const std::vector<SensorTime>& extremes)
{
  mExtremesModel.reset(new ExtremesTableModel(eda, extremes));
  ui->tableExtremes->setModel(mExtremesModel.get());
  connect(ui->tableExtremes->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
}

void ExtremesView::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  METLIBS_LOG_SCOPE();

  const int row = getSelectedRow();
  if (row < 0 or row == mLastSelectedRow)
    return;
  mLastSelectedRow = row;

  EditDataPtr obs = mExtremesModel->getObs(row);
  /*emit*/ signalNavigateTo(obs->sensorTime());
}

int ExtremesView::getSelectedRow() const
{
  QModelIndexList selectedRows = ui->tableExtremes->selectionModel()->selectedRows();
  if (selectedRows.size() != 1)
    return -1;
  const QModelIndex indexModel = selectedRows.at(0);
  return indexModel.row();
}
