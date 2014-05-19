
#include "NavigationHistory.hh"

#include "JumpToObservation.hh"
#include "common/KvMetaDataBuffer.hh"

#include <QApplication>
#include <QAbstractTableModel>
#include <QHeaderView>

#include "ui_navigationhistory.h"

class NavigationHistoryModel : public QAbstractTableModel
{
public:
  NavigationHistoryModel(QObject* parent=0)
    : QAbstractTableModel(parent) { }

  enum COLUMNS {
    COL_STATIONID = 0,
    COL_LEVEL,
    COL_SENSORNR,
    COL_TYPEID,
    COL_PARAMID,
    COL_OBSTIME,
    NCOLUMNS
  };

  void clear();
  QModelIndex add(const SensorTime& st);
  const SensorTime& get(int row) const
    { return mHistory.at(row); }

  int rowCount(const QModelIndex&) const
    { return mHistory.size(); }

  int columnCount(const QModelIndex&) const
    { return NCOLUMNS; }

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QVariant data(const QModelIndex& index, int role) const;

private:
  SensorTime_v mHistory;
};

namespace {

const char* headers[NavigationHistoryModel::NCOLUMNS] = {
  QT_TRANSLATE_NOOP("NavigationHistory", "Stnr"),
  QT_TRANSLATE_NOOP("NavigationHistory", "Lvl"),
  QT_TRANSLATE_NOOP("NavigationHistory", "Snsr"),
  QT_TRANSLATE_NOOP("NavigationHistory", "Type"),
  QT_TRANSLATE_NOOP("NavigationHistory", "Para"),
  QT_TRANSLATE_NOOP("NavigationHistory", "Obstime"),
};

const char* tooltips[NavigationHistoryModel::NCOLUMNS] = {
  QT_TRANSLATE_NOOP("NavigationHistory", "Station number"),
  QT_TRANSLATE_NOOP("NavigationHistory", "Level"),
  QT_TRANSLATE_NOOP("NavigationHistory", "Sensor Number"),
  QT_TRANSLATE_NOOP("NavigationHistory", "Type ID"),
  QT_TRANSLATE_NOOP("NavigationHistory", "Parameter name"),
  QT_TRANSLATE_NOOP("NavigationHistory", "Observation time"),
};

} // anonymous namespace

QVariant NavigationHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal) {
    if (role == Qt::DisplayRole) {
      return qApp->translate("NavigationHistory", headers[section]);
    } else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
      const QString tt = qApp->translate("NavigationHistory", tooltips[section]);
      if (not tt.isEmpty())
        return tt;
    }
  }
  return QVariant();
}

QVariant NavigationHistoryModel::data(const QModelIndex& index, int role) const
{
  if (role == Qt::DisplayRole) {
    const SensorTime& st = mHistory.at(index.row());
    switch (index.column()) {
    case COL_STATIONID:
      return st.sensor.stationId;
    case COL_LEVEL:
      return st.sensor.level;
    case COL_SENSORNR:
      return st.sensor.sensor;
    case COL_TYPEID:
      return st.sensor.typeId;
    case COL_PARAMID:
      return QString::fromStdString(KvMetaDataBuffer::instance()->findParam(st.sensor.paramId).name());
    case COL_OBSTIME:
      return QString::fromStdString(timeutil::to_iso_extended_string(st.time));
    }
  }
  return QVariant();
}

void NavigationHistoryModel::clear()
{
  beginResetModel();
  mHistory.clear();
  endResetModel();
}

QModelIndex NavigationHistoryModel::add(const SensorTime& st)
{
  SensorTime_v::const_iterator it = std::find_if(mHistory.begin(), mHistory.end(),
      std::bind1st(eq_SensorTime(), st));
  const int row = (it - mHistory.begin());
  if (it == mHistory.end()) {
    beginInsertRows(QModelIndex(), row, row);
    mHistory.push_back(st);
    endInsertRows();
  }
  return createIndex(0, row);
}

// ########################################################################

NavigationHistory::NavigationHistory(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui_NavigationHistory)
{
  ui->setupUi(this);
  setFocusProxy(ui->tableHistory);

  ui->tableHistory->setModel(new NavigationHistoryModel(this));
  connect(ui->tableHistory->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));

  QHeaderView* hh = ui->tableHistory->horizontalHeader();
  hh->setResizeMode(QHeaderView::Interactive);
  hh->resizeSections(QHeaderView::ResizeToContents);
  hh->resizeSection(NavigationHistoryModel::COL_STATIONID, 100);
  hh->resizeSection(NavigationHistoryModel::COL_LEVEL,      30);
  hh->resizeSection(NavigationHistoryModel::COL_SENSORNR,   30);
  hh->resizeSection(NavigationHistoryModel::COL_OBSTIME,   150);
  hh->resizeSection(NavigationHistoryModel::COL_PARAMID,    60);
  hh->resizeSection(NavigationHistoryModel::COL_TYPEID,     40);

  mJump = new JumpToObservation(this);

  connect(mJump, SIGNAL(signalNavigateTo(const SensorTime&)),
      this, SIGNAL(signalNavigateTo(const SensorTime&)));

  connect(ui->buttonJump, SIGNAL(clicked()), this, SLOT(onJump()));
  connect(ui->buttonClear, SIGNAL(clicked()), this, SLOT(onClear()));
}

void NavigationHistory::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    ui->retranslateUi(this);
  QWidget::changeEvent(event);
}

NavigationHistoryModel* NavigationHistory::model() const
{
  return static_cast<NavigationHistoryModel*>(ui->tableHistory->model());
}

void NavigationHistory::onSelectionChanged(const QItemSelection& selected, const QItemSelection&)
{
  if (selected.empty())
    return;

  const SensorTime& st = model()->get(selected.indexes().front().row());
  NavigateHelper::Blocker block(mNavigate);
  if (mNavigate.go(st))
    Q_EMIT signalNavigateTo(st);
}

void NavigationHistory::navigateTo(const SensorTime& st)
{
  QModelIndex idx = model()->add(st);
  NavigateHelper::Blocker block(mNavigate);
  if (idx.isValid() and mNavigate.go(st)) {
    ui->tableHistory->scrollTo(idx);

    if (QItemSelectionModel* sm = ui->tableHistory->selectionModel())
      sm->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
  }
}

void NavigationHistory::onJump()
{
  mJump->exec();
}

void NavigationHistory::onClear()
{
  mNavigate.invalidate();
  model()->clear();
}
