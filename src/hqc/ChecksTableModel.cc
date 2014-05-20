
#include "ChecksTableModel.hh"

#include "common/HqcApplication.hh"

#include <QtCore/QVariant>
#include <QtSql/QSqlQuery>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.ChecksTableModel"
#include "common/ObsLogging.hh"

namespace {

enum { COL_CHECK, COL_EXPLAIN, NCOLUMNS };

const char* headers[NCOLUMNS] = {
    QT_TRANSLATE_NOOP("ChecksTableModel", "Check"),
    QT_TRANSLATE_NOOP("ChecksTableModel", "Description")
};

}

ChecksTableModel::ChecksTableModel(QObject* parent)
  : QAbstractTableModel(parent)
{
}

ChecksTableModel::~ChecksTableModel()
{
}

int ChecksTableModel::rowCount(const QModelIndex&) const
{
  return mExplanations.size();
}

int ChecksTableModel::columnCount(const QModelIndex&) const
{
  return NCOLUMNS;
}

Qt::ItemFlags ChecksTableModel::flags(const QModelIndex& /*index*/) const
{
  return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}

QVariant ChecksTableModel::data(const QModelIndex& index, int role) const
{
  const int row = index.row();
  switch (index.column()) {
  case COL_CHECK:
    if (role == Qt::DisplayRole)
      return mChecks.at(row);
  case COL_EXPLAIN:
    if (role == Qt::DisplayRole or role == Qt::ToolTipRole or role == Qt::StatusTipRole)
      return mExplanations.at(row);
  }
  return QVariant();
}

QVariant ChecksTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole)
          return tr(headers[section]);
    }
    return QVariant();
}

void ChecksTableModel::showChecks(ObsData_p obs)
{
  METLIBS_LOG_SCOPE();

  beginResetModel();
  buildModel(obs);
  endResetModel();
}

void ChecksTableModel::buildModel(ObsData_p obs)
{
  METLIBS_LOG_SCOPE();

  mChecks.clear();
  mExplanations.clear();
  if (not obs)
    return;

  const std::string& cfailed = obs->cfailed();
  if (cfailed.empty())
    return;
  
  mChecks = QString::fromStdString(cfailed).split(",");
  QSqlQuery query(hqcApp->systemDB());
  query.prepare("SELECT description FROM check_explain WHERE qcx = ? AND language = 'nb'");
  
  BOOST_FOREACH(QString& c, mChecks) {
    if (c.startsWith("QC2N_")) {
      QString n = c.mid(5);
      n.replace("_", ", ");
      n.prepend(tr("neighbors: "));
      c = "QC2-redist-N";
      mExplanations.push_back(n);
      continue;
    }
    
    query.bindValue(0, c);
    query.exec();
    if (query.next())
      mExplanations.push_back(query.value(0).toString());
    else
      mExplanations.push_back("?");
  }
}
