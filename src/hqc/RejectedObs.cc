
#include "RejectedObs.hh"

#include "util/stringutil.hh"
#include "util/timeutil.hh"

#include <kvalobs/kvRejectdecode.h>

#include <QtCore/QEvent>
#include <QtGui/QHeaderView>
#include <QtGui/QTableView>
#include <QtGui/QVBoxLayout>

namespace {
const int NCOLUMNS = 3;
const char* headers[NCOLUMNS] = {
  QT_TRANSLATE_NOOP("RejectDecodeTable", "Tbtime"),
  QT_TRANSLATE_NOOP("RejectDecodeTable", "Observation"),
  QT_TRANSLATE_NOOP("RejectDecodeTable", "Comment")
};
}

RejectDecodeTableModel::RejectDecodeTableModel(const std::vector<kvalobs::kvRejectdecode>& rList)
  : mRecjectList(rList)
  , mDataRegexp(".*\\<data\\>(.*)\\<\\/data\\>.*", false)
{
}

RejectDecodeTableModel::~RejectDecodeTableModel()
{
}

int RejectDecodeTableModel::rowCount(const QModelIndex&) const
{
  return mRecjectList.size();
}

int RejectDecodeTableModel::columnCount(const QModelIndex&) const
{
  return NCOLUMNS;
}

QVariant RejectDecodeTableModel::data(const QModelIndex& index, int role) const
{
  const int column = index.column();
  if (role == Qt::DisplayRole) {
    const kvalobs::kvRejectdecode& rd = mRecjectList[index.row()];
    switch (column) {
    case 0: return timeutil::to_iso_extended_qstring(rd.tbtime());
    case 1: {
      QString msg = Helpers::fromUtf8(rd.message());
      if (mDataRegexp.exactMatch(msg))
        msg = mDataRegexp.cap(1);
      return msg;
    }
    case 2: return Helpers::fromUtf8(rd.comment());
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

Rejects::Rejects(const std::vector<kvalobs::kvRejectdecode>& rejList, QWidget* parent)
  : QDialog(parent)
  , mTableModel(new RejectDecodeTableModel(rejList))
{
  resize(1200, 700);

  QTableView* tv = new QTableView(this);
  tv->setModel(mTableModel.get());
  tv->verticalHeader()->setDefaultSectionSize(20);
  tv->verticalHeader()->hide();
  tv->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  tv->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

  QVBoxLayout* topLayout = new QVBoxLayout(this);
  topLayout->addWidget(tv);
  retranslateUi();
  show();
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
