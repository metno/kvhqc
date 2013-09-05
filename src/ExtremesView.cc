
#include "ExtremesView.hh"
#include "ExtremesTableModel.hh"

#include <QtGui/QHeaderView>
#include <QtGui/QMessageBox>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.ExtremesView"
#include "HqcLogging.hh"

ExtremesView::ExtremesView(QWidget* parent)
  : QTableView(0)
  , mLastSelectedRow(-1)
{
  METLIBS_LOG_SCOPE();

  horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  verticalHeader()->setDefaultSectionSize(20);
  verticalHeader()->hide();
  setSelectionBehavior(SelectRows);
  setSelectionMode(SingleSelection);
}

ExtremesView::~ExtremesView()
{
}

void ExtremesView::setExtremes(EditAccessPtr eda, const std::vector<SensorTime>& extremes)
{
  mExtremesModel.reset(new ExtremesTableModel(eda, extremes));
  setModel(mExtremesModel.get());
  connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
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
  QModelIndexList selectedRows = selectionModel()->selectedRows();
  if (selectedRows.size() != 1)
    return -1;
  const QModelIndex indexModel = selectedRows.at(0);
  return indexModel.row();
}
