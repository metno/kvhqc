
#include "RejectedObs.hh"

#include "RejectedObsDialog.hh"
#include "RejectedQueryTask.hh"

#include "common/HqcApplication.hh"
#include "common/QueryTaskHelper.hh"

#include "util/BusyLabel.hh"

#include <QtCore/QEvent>
#include <QtCore/QSettings>
#include <QtGui/QHeaderView>
#include <QtGui/QTableView>
#include <QtGui/QVBoxLayout>

#define MILOGGER_CATEGORY "kvhqc.RejectedObs"
#include "util/HqcLogging.hh"

namespace {
enum RejectedObs_Columns {
  COL_TBTIME,
  COL_OBS,
  COL_COMMENT,
  NCOLUMNS
};

const char* headers[NCOLUMNS] = {
  QT_TRANSLATE_NOOP("RejectDecodeTable", "Tbtime"),
  QT_TRANSLATE_NOOP("RejectDecodeTable", "Observation"),
  QT_TRANSLATE_NOOP("RejectDecodeTable", "Comment")
};

const char SETTINGS_GROUP[] = "rejecteddialog";
const char SETTING_GEOMETRY[] = "geometry";
}

RejectDecodeTableModel::RejectDecodeTableModel(const hqc::kvRejectdecode_v& rList)
  : mRecjected(rList)
{
}

RejectDecodeTableModel::~RejectDecodeTableModel()
{
}

int RejectDecodeTableModel::rowCount(const QModelIndex&) const
{
  return mRecjected.size();
}

int RejectDecodeTableModel::columnCount(const QModelIndex&) const
{
  return NCOLUMNS;
}

QVariant RejectDecodeTableModel::data(const QModelIndex& index, int role) const
{
  const int column = index.column();
  if (role == Qt::DisplayRole) {
    const kvalobs::kvRejectdecode& rd = mRecjected[index.row()];
    switch (column) {
    case 0:
      return timeutil::shortenedTime(rd.tbtime());
    case 1: {
      QString msg = QString::fromStdString(rd.message());
      const QRegExp REJ_DAT(".*\\<data\\>(.*)\\<\\/data\\>.*", false);
      if (REJ_DAT.exactMatch(msg))
        msg = REJ_DAT.cap(1);
      return msg;
    }
    case 2:
      return QString::fromStdString(rd.comment());
    }
  } else if (role == Qt::FontRole and (column == 1 or column == 2)) {
    return QFont("Monospace");
  }
  return QVariant();
}

QVariant RejectDecodeTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal and role == Qt::DisplayRole) {
    return headers[section];
  }
  return QVariant();
}

// ########################################################################

Rejects::Rejects(const TimeSpan& time, QWidget* parent)
  : QDialog(parent)
  , mTableModel(0)
{
  setWindowIcon(QIcon("icons:rejectedobs.svg"));
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

  mTask = new QueryTaskHelper(new RejectedQueryTask(time, QueryTask::PRIORITY_AUTOMATIC));
  connect(mTask, SIGNAL(done(SignalTask*)), this, SLOT(onQueryDone()));
  mTask->post(hqcApp->kvalobsHandler());
  mBusy->setBusy(true);
}

Rejects::~Rejects()
{
  writeSettings();
  delete mTask;
}

void Rejects::retranslateUi()
{
  setCaption(tr("RejectDecode"));
}

void Rejects::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    retranslateUi();
  QDialog::changeEvent(event);
}

void Rejects::readSettings()
{
  QSettings settings;
  settings.beginGroup(SETTINGS_GROUP);
  if (not restoreGeometry(settings.value(SETTING_GEOMETRY).toByteArray())) {
    METLIBS_LOG_INFO("cannot restore reject window geometry");
    resize(800, 700);
  }
  settings.endGroup();
}

void Rejects::writeSettings()
{
  QSettings settings;
  settings.beginGroup(SETTINGS_GROUP);
  settings.setValue(SETTING_GEOMETRY, saveGeometry());
  settings.endGroup();
}

void Rejects::onQueryDone()
{
  delete mTableModel;

  const RejectedQueryTask* t = static_cast<const RejectedQueryTask*>(mTask->task());
  mTableModel = new RejectDecodeTableModel(t->rejected());
  mTableView->setModel(mTableModel);
  mTableView->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  mTableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

  mBusy->setBusy(false);

  delete mTask;
  mTask = 0;
}


void Rejects::showRejected(QWidget* parent)
{
  METLIBS_LOG_SCOPE();
  RejectedObsDialog dlg(parent);
  if (dlg.exec() == QDialog::Accepted) {
    Rejects* r = new Rejects(dlg.getTimeSpan(), parent);
    r->open();
  }
}
