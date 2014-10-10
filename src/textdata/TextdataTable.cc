
#include "TextdataTable.hh"

#include "TextdataDialog.hh"
#include "TextDataQueryTask.hh"

#include "common/HqcApplication.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/QueryTaskHelper.hh"
#include "common/TimeHeader.hh"

#include "util/BusyLabel.hh"

#include <QtCore/QEvent>
#include <QtCore/QSettings>
#include <QtGui/QHeaderView>
#include <QtGui/QMessageBox>
#include <QtGui/QTableView>
#include <QtGui/QVBoxLayout>

#define MILOGGER_CATEGORY "kvhqc.TextData"
#include "util/HqcLogging.hh"

namespace {

enum TextData_Columns {
  COL_STATIONID,
  COL_OBSTIME,
  COL_ORIGINAL,
  COL_PARAM,
  COL_TBTIME,
  COL_TYPEID,
  NCOLUMNS
};

const char* headers[NCOLUMNS] = {
  QT_TRANSLATE_NOOP("TextDataTable", "Stationid"),
  QT_TRANSLATE_NOOP("TextDataTable", "Obstime"),
  QT_TRANSLATE_NOOP("TextDataTable", "Original"),
  QT_TRANSLATE_NOOP("TextDataTable", "ParamName"),
  QT_TRANSLATE_NOOP("TextDataTable", "Tbtime"),
  QT_TRANSLATE_NOOP("TextDataTable", "Typeid")
};

const char SETTINGS_GROUP[] = "textdatadialog";
const char SETTING_GEOMETRY[] = "geometry";
}

TextDataTableModel::TextDataTableModel(const std::vector<TxtDat>& txtList, QObject* parent)
  : QAbstractTableModel(parent)
  , mTxtList(txtList)
{
}

TextDataTableModel::~TextDataTableModel()
{
}

int TextDataTableModel::rowCount(const QModelIndex&) const
{
  return mTxtList.size();
}

int TextDataTableModel::columnCount(const QModelIndex&) const
{
  return NCOLUMNS;
}

QVariant TextDataTableModel::data(const QModelIndex& index, int role) const
{
  const int column = index.column();
  if (role == Qt::DisplayRole) {
    const TxtDat& td = mTxtList[index.row()];
    switch (column) {
    case COL_STATIONID:
      return QString::number(td.stationId);
    case COL_OBSTIME:
      return timeutil::shortenedTime(td.obstime);
    case COL_ORIGINAL:
      return QString::fromStdString(td.original);
    case COL_PARAM:
      return QString::fromStdString(KvMetaDataBuffer::instance()->findParamName(td.paramId));
    case COL_TBTIME:
      return timeutil::shortenedTime(td.tbtime);
    case COL_TYPEID:
      return QString::number(td.typeId);
    }
  } else if (role == Qt::ToolTipRole) {
    const TxtDat& td = mTxtList[index.row()];
    switch (column) {
    case COL_STATIONID:
      try {
        const kvalobs::kvStation& s = KvMetaDataBuffer::instance()->findStation(td.stationId);
        return QString::fromStdString(s.name());
      } catch (std::runtime_error& e) {
        METLIBS_LOG_INFO("station lookup error: " << e.what());
      }
      break;
    case COL_OBSTIME:
      return TimeHeader::headerData(td.obstime, Qt::Vertical, Qt::ToolTipRole);
    case COL_PARAM:
      return QString::number(td.paramId);
    case COL_TBTIME:
      return TimeHeader::headerData(td.tbtime, Qt::Vertical, Qt::ToolTipRole);
    case COL_TYPEID:
      try {
        const kvalobs::kvTypes& t = KvMetaDataBuffer::instance()->findType(td.typeId);
        return QString::fromStdString(t.format());
      } catch (std::runtime_error& e) {
        METLIBS_LOG_INFO("typeid lookup error: " << e.what());
      }
      break;
    }
  } else if (role == Qt::FontRole and (column == COL_OBSTIME or column == COL_TBTIME)) {
    return QFont("Monospace");
  }
  return QVariant();
}

QVariant TextDataTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal and role == Qt::DisplayRole) {
    return headers[section];
  }
  return QVariant();
}

TextData::TextData(const int stationId, const TimeSpan& time, QWidget* parent)
  : QDialog(parent)
  , mTableModel(0)
{
  setWindowIcon(QIcon("icons:textdata.svg"));
  setAttribute(Qt::WA_DeleteOnClose, true);

  mTableView = new QTableView(this);
  mTableView->verticalHeader()->setDefaultSectionSize(20);
  mTableView->verticalHeader()->hide();

  mBusy = new BusyLabel(this);

  QVBoxLayout* topLayout = new QVBoxLayout(this);
  topLayout->addWidget(mBusy);
  topLayout->addWidget(mTableView);
  retranslateUi();

  readSettings();

  mTask = new QueryTaskHelper(new TextDataQueryTask(stationId, time, QueryTask::PRIORITY_AUTOMATIC));
  connect(mTask, SIGNAL(done(QueryTask*)), this, SLOT(onQueryDone()));
  mTask->post(hqcApp->kvalobsHandler());
  mBusy->setBusy(true);
}

TextData::~TextData()
{
  writeSettings();
  delete mTask;
}

void TextData::retranslateUi()
{
  setWindowTitle(tr("TextData"));
}

void TextData::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    retranslateUi();
  QDialog::changeEvent(event);
}

void TextData::readSettings()
{
  QSettings settings;
  settings.beginGroup(SETTINGS_GROUP);
  if (not restoreGeometry(settings.value(SETTING_GEOMETRY).toByteArray())) {
    METLIBS_LOG_INFO("cannot restore text data window geometry");
    resize(800, 700);
  }
  settings.endGroup();
}

void TextData::writeSettings()
{
  QSettings settings;
  settings.beginGroup(SETTINGS_GROUP);
  settings.setValue(SETTING_GEOMETRY, saveGeometry());
  settings.endGroup();
}

void TextData::onQueryDone()
{
  delete mTableModel;

  const TextDataQueryTask* t = static_cast<const TextDataQueryTask*>(mTask->task());
  mTableModel = new TextDataTableModel(t->textData(), this);
  mTableView->setModel(mTableModel);
  mTableView->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  mTableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

  mBusy->setBusy(false);

  delete mTask;
  mTask = 0;
}

void TextData::showTextData(QWidget* parent)
{
  METLIBS_LOG_SCOPE();
  TextDataDialog dlg(parent);
  if (dlg.exec() == QDialog::Accepted) {
    TextData* td = new TextData(dlg.getStationId(), dlg.getTimeSpan(), parent);
    td->open();
  }
}
