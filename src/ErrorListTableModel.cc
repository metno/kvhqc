
#include "ErrorListTableModel.hh"

#include "hqc_utilities.hh"
#include "KvMetaDataBuffer.hh"

#include <kvalobs/kvDataOperations.h>

#include <QtCore/QCoreApplication>

//#define NDEBUG
#include "debug.hh"

namespace {

const char* headers[ErrorListTableModel::NCOLUMNS] = {
    QT_TRANSLATE_NOOP("ErrorList", "Stnr"),
    QT_TRANSLATE_NOOP("ErrorList", "Name"),
    QT_TRANSLATE_NOOP("ErrorList", "Mt"),
    QT_TRANSLATE_NOOP("ErrorList", "Dy"),
    QT_TRANSLATE_NOOP("ErrorList", "Hr"),
    QT_TRANSLATE_NOOP("ErrorList", "Mn"),
    QT_TRANSLATE_NOOP("ErrorList", "Para"),
    QT_TRANSLATE_NOOP("ErrorList", "Type"),
    QT_TRANSLATE_NOOP("ErrorList", "Orig.d"),
    QT_TRANSLATE_NOOP("ErrorList", "Corr.d"),
    QT_TRANSLATE_NOOP("ErrorList", "mod.v"),
    QT_TRANSLATE_NOOP("ErrorList", "Flag"),
    "=",
    QT_TRANSLATE_NOOP("ErrorList", "Fl.v"),
    QT_TRANSLATE_NOOP("ErrorList", "Corrected OK"),
    QT_TRANSLATE_NOOP("ErrorList", "Original OK"),
    QT_TRANSLATE_NOOP("ErrorList", "Interpolated"),
    QT_TRANSLATE_NOOP("ErrorList", "Corrected"),
    QT_TRANSLATE_NOOP("ErrorList", "Rejected")
};

const char* tooltips[ErrorListTableModel::NCOLUMNS] = {
    QT_TRANSLATE_NOOP("ErrorList", "Station number"),
    QT_TRANSLATE_NOOP("ErrorList", "Station name"),
    QT_TRANSLATE_NOOP("ErrorList", "Obs month"),
    QT_TRANSLATE_NOOP("ErrorList", "Obs day"),
    QT_TRANSLATE_NOOP("ErrorList", "Obs hour"),
    QT_TRANSLATE_NOOP("ErrorList", "Obs minute"),
    QT_TRANSLATE_NOOP("ErrorList", "Parameter name"),
    QT_TRANSLATE_NOOP("ErrorList", "Type ID"),
    QT_TRANSLATE_NOOP("ErrorList", "Original value"),
    QT_TRANSLATE_NOOP("ErrorList", "Corrected value"),
    QT_TRANSLATE_NOOP("ErrorList", "Model value"),
    QT_TRANSLATE_NOOP("ErrorList", "Flag name"),
    "",
    QT_TRANSLATE_NOOP("ErrorList", "Flag value"),
    "",
    "",
    "",
    "",
    ""
};

// may only be used 
ErrorList::mem_change change4column(int column)
{
    if (column==ErrorListTableModel::COL_CORR_OK)
        return ErrorList::CORR_OK;
    if (column==ErrorListTableModel::COL_ORIG_OK)
        return ErrorList::ORIG_OK;
    assert(column==ErrorListTableModel::COL_REJECTED);
    return ErrorList::REJECTED;
}

}

ErrorListTableModel::ErrorListTableModel(const std::vector<ErrorList::mem>& errorList)
    : mErrorList(errorList)
    , mShowStation(-1)
{
}

ErrorListTableModel::~ErrorListTableModel()
{
}

int ErrorListTableModel::rowCount(const QModelIndex&) const
{
    return mErrorList.size();
}

int ErrorListTableModel::columnCount(const QModelIndex&) const
{
    return NCOLUMNS;
}

Qt::ItemFlags ErrorListTableModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable|Qt::ItemIsEnabled;

    const int c = index.column();
    const ErrorList::mem& mo = mErrorList[index.row()];

    if (c>=COL_CORR_OK and c<NCOLUMNS) {
        const kvalobs::kvControlInfo cif(mo.controlinfo);
        const int fd = cif.flag(kvalobs::flag::fd);
        const int fmis = cif.flag(kvalobs::flag::fmis);
        if (Helpers::is_accumulation(fd)) {
            // for accumulations, always use WatchRR (or data list, for now)
            flags &= ~Qt::ItemIsEnabled;
        } else if (fmis == 3) {
            if (c==COL_INTERPOLATED)
                flags |= Qt::ItemIsEditable;
            else
                flags &= ~Qt::ItemIsEnabled;
        } else if ((c==COL_CORR_OK and fmis == 2) or (c==COL_REJECTED and fmis == 1)) {
            flags &= ~Qt::ItemIsEnabled;
        } else {
            flags |= Qt::ItemIsEditable;
            if (c==COL_CORR_OK or c==COL_ORIG_OK or c==COL_REJECTED)
                flags |= Qt::ItemIsUserCheckable;
        }
    }
    return flags;
}

static QString twoDigits(int number)
{
    return QString("%1").arg(number, 2, 10, QLatin1Char('0'));
}

QVariant ErrorListTableModel::data(const QModelIndex& index, int role) const
{
    const int column = index.column();
    const bool isTT = (role == Qt::ToolTipRole);
    if (role == Qt::DisplayRole or isTT) {
        const ErrorList::mem& mo = mErrorList[index.row()];
        if (isTT and column <= 6)
            return Helpers::stationInfo(mo.stnr) + " " + QString::fromStdString(timeutil::to_iso_extended_string(mo.obstime));
        if (isTT and column <= 12)
            return QVariant();
        switch (column) {
        case COL_STATION_ID:
            return mo.stnr;
        case COL_STATION_NAME:
            return mo.name;
        case COL_OBS_MONTH:
            return twoDigits(mo.obstime.date().month());
        case COL_OBS_DAY:
            return twoDigits(mo.obstime.date().day());
        case COL_OBS_HOUR:
            return twoDigits(mo.obstime.time_of_day().hours());
        case COL_OBS_MINUTE:
            return twoDigits(mo.obstime.time_of_day().minutes());
        case COL_OBS_PARAM:
            try {
                return QString::fromStdString(KvMetaDataBuffer::instance()->findParam(mo.parNo).name());
            } catch (std::runtime_error&) {
                return QVariant(); // unknown parameter
            }
        case COL_OBS_TYPEID:
            return mo.typeId;
        case COL_OBS_ORIG:
        case COL_OBS_CORR:
        case COL_OBS_MODEL: {
            const bool isCodeParam = KvMetaDataBuffer::instance()->isCodeParam(mo.parNo);
            const int nDigits = isCodeParam ? 0 : 1;
            float value;
            if (column == COL_OBS_ORIG)
                value = mo.orig;
            else if (column == COL_OBS_CORR)
                value = mo.corr;
            else
                value = mo.morig;
            if (value == -32767 or value == -32766)
                return (int)value;
            return QString::number(value ,'f', nDigits);
        }
        case COL_OBS_FLAG_NAME:
            return mo.flTyp;
        case COL_OBS_FLAG_EQ:
            return "=";
        case COL_OBS_FLAG_VAL:
            return mo.flg;
        case COL_CORR_OK:
        case COL_ORIG_OK:
        case COL_REJECTED: {
            const kvalobs::kvControlInfo cif(mo.controlinfo);
            const int fd = cif.flag(kvalobs::flag::fd);
            const int fmis = cif.flag(kvalobs::flag::fmis);
            if (fd == 2 or fd >= 4)
                return isTT ? tr("Accumulation. Use WatchRR to modify.") : "WatchRR!";
            if (column == COL_CORR_OK and fmis == 2) {
                return isTT ? tr("Corrected is missing. Use field 'Original OK' or 'Recjected'.") : "Orig OK/Rej!";
            } else if ((column==COL_CORR_OK or column==COL_ORIG_OK) and fmis == 3) {
                return isTT ? tr("Both original and corrected are missing. Use field 'Interpolated'.") : "Interp!";
            } else if (column==COL_REJECTED and fmis == 1) {
                return isTT ? tr("Cannot reject. Use field 'Original OK'.") : "Orig OK!";
            } else if (column==COL_REJECTED and fmis == 3) {
                return isTT ? tr("Cannot reject. Use field 'Interpolated'.") : "Interp!";
            }
            if (mo.change == change4column(column)) {
                if (mo.changed_qc2allowed)
                    return isTT ? tr("QC2 may override") : tr("QC2 ok");
                else
                    return isTT ? tr("QC2 may not override") : tr("no QC2");
            }
            break;
        }
        case COL_INTERPOLATED:
        case COL_CORRECTED: {
            const kvalobs::kvControlInfo cif(mo.controlinfo);
            const int fd = cif.flag(kvalobs::flag::fd);
            const int fmis = cif.flag(kvalobs::flag::fmis);
            if (fd == 2 or fd >= 4)
                return isTT ? tr("Accumulation. Use WatchRR to modify.") : "WatchRR!";
            if (column==COL_CORRECTED and fmis == 3)
                return isTT ? tr("Both original and corrected are missing. Use field 'Interpolated'.") : "Interp!";
            if (not isTT) {
                if (column==COL_CORRECTED and mo.change == ErrorList::ORIG_OK and fmis == 1)
                    return -32767;
                if ((column==COL_INTERPOLATED and mo.change != ErrorList::INTERPOLATED)
                    or (column==COL_CORRECTED and mo.change != ErrorList::CORRECTED))
                {
                    return QVariant();
                }
                
                const bool isCodeParam = KvMetaDataBuffer::instance()->isCodeParam(mo.parNo);
                const int nDigits = isCodeParam ? 0 : 1;
                return QString::number(mo.changed_value, 'f', nDigits);
            }
            break;
        }
        } // end of switch
    } else if (role == Qt::FontRole and (column == 0 or column == 1)) {
        const ErrorList::mem& mo = mErrorList[index.row()];
        if (mo.stnr == mShowStation) {
            QFont f;
            f.setBold(true);
            return f;
        }
    } else if (role == Qt::CheckStateRole and (column==COL_CORR_OK or column==COL_ORIG_OK or column==COL_REJECTED)) {
        const ErrorList::mem& mo = mErrorList[index.row()];
        return (mo.change == change4column(column)) ? Qt::Checked : Qt::Unchecked;
    } else if (role == Qt::TextColorRole and column == COL_OBS_CORR) {
        const ErrorList::mem& mo = mErrorList[index.row()];
        const kvalobs::kvControlInfo ci(mo.controlinfo);
        if (ci.flag(kvalobs::flag::fhqc) == 0) { // not hqc touched
            if (ci.qc2dDone())
                return Qt::darkMagenta;
            else if (ci.flag(kvalobs::flag::fnum) >= 6)
                return Qt::red;
        }
    } else if (role == Qt::TextAlignmentRole and (column==COL_OBS_ORIG or column==COL_OBS_CORR or column==COL_OBS_MODEL
                                                  or column==COL_INTERPOLATED or column==COL_CORRECTED))
    {
        return Qt::AlignRight;
    }
    return QVariant();
}

bool ErrorListTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    LOG_SCOPE("ErrorListTableModel");
    const int column = index.column();
    LOG4SCOPE_DEBUG(DBG1(column));
    if ((column==COL_CORR_OK or column==COL_ORIG_OK or column==COL_REJECTED) and role == Qt::CheckStateRole) {
        ErrorList::mem& mo = mErrorList[index.row()];
        const ErrorList::mem_change mc = change4column(column);
        if (mo.change != mc) {
            mo.change = mc;
            mo.changed_qc2allowed = false;
        } else if (mo.changed_qc2allowed) {
            mo.change = ErrorList::NO_CHANGE;
        } else {
            mo.changed_qc2allowed = true;
        }
        LOG4SCOPE_DEBUG("cb change " << DBG1(mo.change) << DBG1(mo.changed_qc2allowed));
    } else if ((column==COL_INTERPOLATED or column==COL_CORRECTED) and role == Qt::EditRole) {
        bool ok = false;
        const float fValue = value.toFloat(&ok);
        if (not ok) {
            LOG4SCOPE_DEBUG("bad float " << value.toString());
            return false;
        }

        ErrorList::mem& mo = mErrorList[index.row()];
        if (not KvMetaDataBuffer::instance()->checkPhysicalLimits(mo.parNo, fValue)) {
            LOG4SCOPE_DEBUG("value " << fValue << " outside physical range");
            return false;
        }

        const ErrorList::mem_change mc = (column==COL_INTERPOLATED) ? ErrorList::INTERPOLATED: ErrorList::CORRECTED;
        mo.change = mc;
        mo.changed_qc2allowed = false;
        mo.changed_value = fValue;
        LOG4SCOPE_DEBUG(DBG1(mo.changed_value) << DBG1(mo.change));
    } else {
        LOG4SCOPE_DEBUG(DBG1(role));
        return false;
    }
    QModelIndex index1 = createIndex(index.row(), COL_CORR_OK);
    QModelIndex index2 = createIndex(index.row(), NCOLUMNS-1);
    /*emit*/ dataChanged(index1, index2);
    return true;
}

QVariant ErrorListTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole)
            return qApp->translate("ErrorList", headers[section]);
        else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
            const QString tt = qApp->translate("ErrorList", tooltips[section]);
            if (not tt.isEmpty())
                return tt;
        }
    }
    return QVariant();
}

void ErrorListTableModel::showSameStation(int stationID)
{
    LOG_SCOPE("ErrorListTableModel");
    if (mShowStation == stationID)
        return;

    mShowStation = stationID;
    LOG4SCOPE_DEBUG(DBG1(mShowStation));
    QModelIndex index1 = createIndex(0, 0);
    QModelIndex index2 = createIndex(mErrorList.size()-1, 0);
    /*emit*/ dataChanged(index1, index2);
}
