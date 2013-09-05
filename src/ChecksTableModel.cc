
#include "ChecksTableModel.hh"

#include "HqcApplication.hh"

#include <QtCore/QVariant>
#include <QtSql/QSqlQuery>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.ChecksTableModel"
#include "HqcLogging.hh"

namespace {

enum { COL_CHECK, COL_EXPLAIN, NCOLUMNS };

const char* headers[NCOLUMNS] = {
    QT_TRANSLATE_NOOP("ChecksTable", "Check"),
    QT_TRANSLATE_NOOP("ChecksTable", "Description")
};

}

ChecksTableModel::ChecksTableModel(ObsAccessPtr da)
    : mDA(da)
{
    mDA->obsDataChanged.connect(boost::bind(&ChecksTableModel::onDataChanged, this, _1, _2));
}

ChecksTableModel::~ChecksTableModel()
{
    mDA->obsDataChanged.disconnect(boost::bind(&ChecksTableModel::onDataChanged, this, _1, _2));
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
  if (role == Qt::DisplayRole) {
    const int row = index.row();
    switch (index.column()) {
    case COL_CHECK:
      return mChecks.at(row);
    case COL_EXPLAIN:
      return mExplanations.at(row);
    }
  }
  return QVariant();
}

QVariant ChecksTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole)
          return qApp->translate("ChecksTable", headers[section]);
    }
    return QVariant();
}

void ChecksTableModel::navigateTo(const SensorTime& st)
{
  METLIBS_LOG_SCOPE();
  if (eq_SensorTime()(st, mSensorTime))
    return;

  beginResetModel();
  mChecks.clear();
  mExplanations.clear();
  mSensorTime = st;
  if (mSensorTime.valid()) {
    if (ObsDataPtr obs = mDA->find(mSensorTime)) {
      mChecks = QString::fromStdString(obs->cfailed()).split(",");
      QSqlQuery query(hqcApp->systemDB());
      query.prepare("SELECT description FROM check_explain WHERE qcx = ? AND language = 'nb'");

      BOOST_FOREACH(const QString& c, mChecks) {
        query.bindValue(0, c);
        query.exec();
        if (query.next())
          mExplanations.push_back(query.value(0).toString());
        else
          mExplanations.push_back("?");
      }
    }
  }
  endResetModel();
}

void ChecksTableModel::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr data)
{
    METLIBS_LOG_SCOPE();
    METLIBS_LOG_DEBUG(LOGVAL(data->sensorTime()) << LOGVAL(what));
    if (what == ObsAccess::DESTROYED) {
      navigateTo(SensorTime()); // make invalid
    } else {
      navigateTo(data->sensorTime());
    }
}
