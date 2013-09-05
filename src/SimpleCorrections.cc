
#include "SimpleCorrections.hh"

#include <boost/bind.hpp>

#include "ui_simplecorrections.h"
//#define NDEBUG
#include "debug.hh"

static int preferredWidth(QWidget* w)
{ return w->sizeHint().width(); }

static void setCommonMinWidth(QWidget* w[])
{
    int mw = preferredWidth(w[0]);
    for (int i=1; w[i]; ++i)
        mw = std::max(mw, preferredWidth(w[i]));
    for (int i=0; w[i]; ++i)
        w[i]->setMinimumSize(mw, w[i]->minimumSize().height());
}

SimpleCorrections::SimpleCorrections(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SimpleCorrections)
    , mSensorTime(Sensor(0, 0, 0, 0, 0), timeutil::ptime())
{
    ui->setupUi(this);

    QWidget* labels1[] = { ui->labelStation, ui->labelObstime, ui->labelFlags, ui->labelOriginal, 0 };
    setCommonMinWidth(labels1);
    QWidget* labels2[] = { ui->labelType, ui->labelParam, 0 };
    setCommonMinWidth(labels2);

    setMaximumSize(QSize(minimumSize().width(), maximumSize().height()));
}

SimpleCorrections::~SimpleCorrections()
{
    if (mDA)
        mDA->obsDataChanged.disconnect(boost::bind(&SimpleCorrections::onDataChanged, this, _1, _2));
}

void SimpleCorrections::setDataAccess(EditAccessPtr eda)
{
    if (mDA)
        mDA->obsDataChanged.disconnect(boost::bind(&SimpleCorrections::onDataChanged, this, _1, _2));
    mDA = eda;
    if (mDA)
        mDA->obsDataChanged.connect(boost::bind(&SimpleCorrections::onDataChanged, this, _1, _2));

    navigateTo(mSensorTime);
}

void SimpleCorrections::navigateTo(const SensorTime& st)
{
    mSensorTime = st;

    const Sensor& s = mSensorTime.sensor;
    if (s.stationId > 0) {
        ui->textStation->setText(QString::number(s.stationId));
        ui->textParam->setText(QString::number(s.paramId));
        ui->textType->setText(QString::number(s.typeId));

        ui->textObstime->setText(QString::fromStdString(timeutil::to_iso_extended_string(mSensorTime.time)));
    } else {
        ui->textStation->setText("--");
        ui->textParam->setText("--");
        ui->textType->setText("--");

        ui->textObstime->setText("");
    }
}

void SimpleCorrections::onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr data)
{
    if (data and eq_SensorTime()(data->sensorTime(), mSensorTime))
        navigateTo(mSensorTime);
}

#if 0
enum mem_change { NO_CHANGE, CORR_OK, ORIG_OK, INTERPOLATED, REDISTRIBUTED, CORRECTED, REJECTED };
struct change {
    mem_change change;
    float changed_value;
    bool changed_qc2allowed;
    change() : change(NO_CHANGE), changed_value(0), changed_qc2allowed(false) { }
};
typedef std::map<SensorTime, change> Changes_t;

// from ErrorList::flags
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

// from ErrorList::data
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

if (role == Qt::CheckStateRole and (column==COL_CORR_OK or column==COL_ORIG_OK or column==COL_REJECTED)) {
    const ErrorList::mem& mo = mErrorList[index.row()];
    return (mo.change == change4column(column)) ? Qt::Checked : Qt::Unchecked;
}

// from ErrorList::setData
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
            mo.changed_qc2allowed = true;
        } else if (not mo.changed_qc2allowed) {
            mo.change = ErrorList::NO_CHANGE;
        } else {
            mo.changed_qc2allowed = false;
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
    } catch (std::runtime_error&) {
    }
    QModelIndex index1 = createIndex(index.row(), COL_CORR_OK);
    QModelIndex index2 = createIndex(index.row(), NCOLUMNS-1);
    /*emit*/ dataChanged(index1, index2);
    return true;
    return QVariant();
}

// from errorlist.cc
namespace /* anonymous */ {

bool set_no_accumulation(kvalobs::kvData& kd)
{
    kvalobs::kvControlInfo ci = kd.controlinfo();
    if (ci.flag(kvalobs::flag::fd) != 3)
        return false;
    ci.set(kvalobs::flag::fd, 1);
    kd.controlinfo(ci);
    return true;
}

void set_fhqc(kvalobs::kvData& kd, int fhqc)
{
    kvalobs::kvControlInfo ci = kd.controlinfo();
    ci.set(kvalobs::flag::fhqc, fhqc);
    kd.controlinfo(ci);
}

void set_fmis(kvalobs::kvData& kd, int fmis)
{
    kvalobs::kvControlInfo ci = kd.controlinfo();
    ci.set(kvalobs::flag::fmis, fmis);
    kd.controlinfo(ci);
}

} // anonymous namespace

void ErrorList::saveChanges()
{
    LOG_SCOPE("ErrorList");

    typedef std::list<kvData> kvDataList;
    kvDataList modData;
    BOOST_FOREACH(const mem& mo, mTableModel->errorList()) {
        if (mo.change == NO_CHANGE)
            continue;
        kvalobs::kvData kd = getKvData(mo);
        const kvalobs::kvControlInfo cif = kd.controlinfo();

        const int fmis = cif.flag(kvalobs::flag::fmis);
        const int fd   = cif.flag(kvalobs::flag::fd);
        bool qc2ok = mo.changed_qc2allowed;

        switch (mo.change) {
        case NO_CHANGE:
            continue;
        case CORR_OK:
            if (Helpers::float_eq()(kd.original(), kd.corrected())
                and (not Helpers::is_accumulation(fd)) and fmis < 2)
            {
                kvalobs::hqc::hqc_accept(kd);
                set_no_accumulation(kd);
            } else if (fmis == 0) {
                set_fhqc(kd, 7);
            } else if (fmis == 1) {
                set_fhqc(kd, 5);
            } else {
                LOG4SCOPE_ERROR("bad corr ok, would not set fhqc, kd=" << kd);
                continue;
            }
            break;
        case ORIG_OK:
            if (fmis == 3) {
                LOG4SCOPE_ERROR("fmis=3, orig ok not possible, should not have been here, kd=" << kd);
                continue;
            }
            if (cif.flag(kvalobs::flag::fnum) == 0 and not (fmis == 0 or fmis == 1 or fmis == 2)) {
                LOG4SCOPE_ERROR("bad orig ok, would not set fhqc, kd=" << kd);
                continue;
            }
            kd.corrected(kd.original());
            kvalobs::hqc::hqc_accept(kd);
            if (fmis == 0 or fmis == 2) {
                set_fmis(kd, 0);
                set_no_accumulation(kd);
            } else if (fmis == 1) {
                set_fmis(kd, 3);
            }
            break;
        case CORRECTED:
        case INTERPOLATED:
            if (Helpers::is_accumulation(fd)) {
                LOG4SCOPE_WARN("corr/interp for accumulation, should not have been here, kd=" << kd);
                continue;
            }
            kvalobs::hqc::hqc_auto_correct(kd, mo.changed_value);
            qc2ok = false;
            break;
        case REDISTRIBUTED:
            LOG4SCOPE_ERROR("should not make redistributions in error list, kd=" << kd);
            continue;
        case REJECTED:
            if (fmis == 1 or fmis == 3) {
                LOG4SCOPE_ERROR("fmis=1/3, cannot reject, should not have been here, kd=" << kd);
                continue;
            }
            kvalobs::hqc::hqc_reject(kd);
            break;
        }
        if (qc2ok)
            set_fhqc(kd, 4);

        modData.push_back( kd );
    }

    LOG4SCOPE_DEBUG("modData =\n" << decodeutility::kvdataformatter::createString(modData));

    if (modData.empty()) {
        QMessageBox::information(this,
                                 tr("No unsaved data"),
                                 tr("There are no unsaved data."),
                                 QMessageBox::Ok, Qt::NoButton);
        return;
    }

    if (mainWindow->saveDataToKvalobs(modData)) {
        QMessageBox::information(this,
                                 tr("Data saved"),
                                 tr("%1 rows have been saved to kvalobs. Warning: data shown in error "
                                    "and data list might no longer be consistent with kvalobs.").arg(modData.size()),
                                 QMessageBox::Ok, Qt::NoButton);
    }
}

bool ErrorList::maybeSave()
{
    bool modified = false;
    BOOST_FOREACH(const mem& mo, mTableModel->errorList()) {
        if (mo.change != NO_CHANGE) {
            modified = true;
            break;
        }
    }

    bool ret = true;
    if (modified) {
        int result =
            QMessageBox::warning( this, tr("HQC"),
                                  tr("You have unsaved changes in the error list. Do you want to save them?"),
                                  tr("&Yes"), tr("&No"), tr("&Cancel"),
                                  0, 2 );
        if ( ! result )
            saveChanges();
        ret = result != 2;
    }
    return ret;
}
#endif
