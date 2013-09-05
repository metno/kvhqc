
#include "textdatatable.h"

#include "GetTextData.h"
#include "KvMetaDataBuffer.hh"
#include "TimeHeader.hh"

#include <kvcpp/KvApp.h>
#include <kvcpp/WhichDataHelper.h>

#include <QtGui/QHeaderView>
#include <QtGui/QMessageBox>
#include <QtGui/QTableView>
#include <QtGui/QVBoxLayout>

#define NDEBUG
#include "HqcLogging.hh"

namespace {
const int NCOLUMNS = 7;
const char* headers[NCOLUMNS] = {
    QT_TRANSLATE_NOOP("TextDataTable", "Stationid"),
    QT_TRANSLATE_NOOP("TextDataTable", "Obstime"),
    QT_TRANSLATE_NOOP("TextDataTable", "Original"),
    QT_TRANSLATE_NOOP("TextDataTable", "Paramid"),
    QT_TRANSLATE_NOOP("TextDataTable", "ParamName"),
    QT_TRANSLATE_NOOP("TextDataTable", "Tbtime"),
    QT_TRANSLATE_NOOP("TextDataTable", "Typeid")
};
}

TextDataTableModel::TextDataTableModel(const std::vector<TxtDat>& txtList)
    : mTxtList(txtList)
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
        case 0: return QString::number(td.stationId);
        case 1: return TimeHeader::headerData(td.obstime, Qt::Vertical, Qt::ToolTipRole);
        case 2: return QString::fromStdString(td.original);
        case 3: return QString::number(td.paramId);
        case 4: return QString::fromStdString(KvMetaDataBuffer::instance()->findParam(td.paramId).name());
        case 5: return QString::fromStdString(timeutil::to_iso_extended_string(td.tbtime));
        case 6: return QString::number(td.typeId);
        }
    } else if (role == Qt::FontRole and (column == 1 or column == 2)) {
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

TextData::TextData(const std::vector<TxtDat>& txtList, QWidget* parent)
    : QDialog(parent)
    , mTableModel(new TextDataTableModel(txtList))
{
    setCaption(tr("TextData"));
    resize(700, 1000);

    QTableView* tv = new QTableView(this);
    tv->setModel(mTableModel.get());
    tv->verticalHeader()->setDefaultSectionSize(20);
    tv->verticalHeader()->hide();
    tv->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    tv->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

    QVBoxLayout* topLayout = new QVBoxLayout(this);
    topLayout->addWidget(tv);
    show();
}

void TextData::showTextData(int stationId, const TimeRange& timeLimits, QWidget* parent)
{
    kvservice::WhichDataHelper whichData;
    whichData.addStation(stationId, timeLimits.t0(), timeLimits.t1());

    GetTextData textDataReceiver;
    bool ok = false;
    try {
      ok = kvservice::KvApp::kvApp->getKvData(textDataReceiver, whichData);
    } catch (std::exception& e) {
      LOG4HQC_ERROR("TextData", "exception while retrieving text data: " << e.what());
    }
    if (not ok) {
      QMessageBox::critical(parent, tr("No Textdata"), tr("Could not read text data."),
          QMessageBox::Ok, QMessageBox::NoButton);
      return;
    }

    new TextData(textDataReceiver.textData(), parent);
}

