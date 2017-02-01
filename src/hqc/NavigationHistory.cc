
#include "NavigationHistory.hh"

#include "JumpToObservation.hh"
#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"

#include <QApplication>
#include <QAbstractTableModel>
#include <QHeaderView>
#include <QtCore/QSettings>

#include "ui_navigationhistory.h"

#define MILOGGER_CATEGORY "kvhqc.NavigationHistory"
#include "common/ObsLogging.hh"

namespace /* anonymous */ {
const char QSETTINGS_GROUP[] = "navigationhistory";
const char QSETTINGS_RECENT_MAX[] = "recent_max";
const char QSETTINGS_RECENT[] = "recent_%1";

QString recentKey(int i)
{
  return QString(QSETTINGS_RECENT).arg(i);
}
} // anonymous namespace

// ########################################################################

class NavigationHistoryModel : public QAbstractTableModel
{
public:
  NavigationHistoryModel(QObject* parent=0)
    : QAbstractTableModel(parent), mMaximumSize(100) { }

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
  void set(const SensorTime_v& history);
  const SensorTime& get(int row) const
    { return mHistory.at(row); }
  int size() const
    { return mHistory.size(); }

  QModelIndex add(const SensorTime& st);

  int rowCount(const QModelIndex&) const
    { return size(); }

  int columnCount(const QModelIndex&) const
    { return NCOLUMNS; }

  void setMaximumSize(int maxi)
    { mMaximumSize = std::max(3, maxi); trim(); }

  int maximumSize() const
    { return mMaximumSize; }

  void trim();

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QVariant data(const QModelIndex& index, int role) const;

private:
  SensorTime_v mHistory;
  size_t mMaximumSize;
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
    if (mHistory.size() > mMaximumSize) {
      beginRemoveRows(QModelIndex(), 0, 0);
      mHistory.erase(mHistory.begin());
      endRemoveRows();
    }
  }
  return createIndex(0, row);
}

void NavigationHistoryModel::trim()
{
  const int tooMuch = size() - mMaximumSize;
  if (tooMuch <= 0)
    return;

  beginRemoveRows(QModelIndex(), 0, tooMuch-1);
  mHistory.erase(mHistory.begin(), mHistory.begin() + tooMuch);
  endRemoveRows();
}

void NavigationHistoryModel::set(const SensorTime_v& history)
{
  beginResetModel();
  mHistory = history;
  endResetModel();
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

NavigationHistory::~NavigationHistory()
{
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
  QPoint pos = ui->buttonJump->mapToGlobal(ui->buttonJump->rect().topRight());
  mJump->move(pos);

  mJump->navigateTo(mNavigate.current());
  mJump->exec();
}

void NavigationHistory::onClear()
{
  mNavigate.invalidate();
  model()->clear();
}

void NavigationHistory::saveSettings(QSettings& settings)
{
  settings.beginGroup(QSETTINGS_GROUP);
  NavigationHistoryModel* m = model();
  for (int i = 0; true; ++i) {
    const QString key = recentKey(i);
    if (i < m->size())
      settings.setValue(key, Helpers::sensorTimeToString(m->get(i)));
    else if (settings.contains(key))
      settings.remove(key);
    else
      break;
  }
  settings.endGroup();
}

void NavigationHistory::restoreSettings(QSettings& settings)
{
  METLIBS_LOG_SCOPE();
  settings.beginGroup(QSETTINGS_GROUP);
  SensorTime_v history;
  for (int i = 0; true; ++i) {
    const QVariant value = settings.value(recentKey(i));
    if (not value.isValid())
      break;
    const SensorTime st = Helpers::sensorTimeFromString(value.toString());
    METLIBS_LOG_DEBUG(LOGVAL(i) << LOGVAL(st));
    if (st.valid())
      history.push_back(st);
  }
  settings.endGroup();

  model()->set(history);
}
