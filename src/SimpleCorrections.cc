
#include "SimpleCorrections.hh"

#include "ModelData.hh"

#include <kvalobs/kvDataOperations.h>

#include "ui_simplecorrections.h"

#define MILOGGER_CATEGORY "kvhqc.SimpleCorrections"
#include "HqcLogging.hh"

namespace /* anonymous */ {

int preferredWidth(QWidget* w)
{ return w->sizeHint().width(); }

void setCommonMinWidth(QWidget* w[])
{
    int mw = preferredWidth(w[0]);
    for (int i=1; w[i]; ++i)
        mw = std::max(mw, preferredWidth(w[i]));
    for (int i=0; w[i]; ++i)
        w[i]->setMinimumSize(mw, w[i]->minimumSize().height());
}

// ----------------------------------------

void accept_original(EditAccessPtr eda, const SensorTime& sensorTime, bool qc2ok)
{
    EditDataPtr obs = eda->findE(sensorTime);
    if (not obs) {
        METLIBS_LOG_ERROR("accept_original without obs for " << sensorTime);
        return;
    }

    const kvalobs::kvControlInfo ci = obs->controlinfo();
    const int fmis = ci.flag(kvalobs::flag::fmis);
    if (fmis == 3) {
        METLIBS_LOG_ERROR("fmis=3, accept_original not possible for " << sensorTime);
        return;
    }
    if (ci.flag(kvalobs::flag::fnum) == 0 and not (fmis == 0 or fmis == 1 or fmis == 2)) {
        METLIBS_LOG_ERROR("bad accept_original, would not set fhqc for " << sensorTime);
        return;
    }

    EditDataEditorPtr editor = eda->editor(obs);
    editor->setCorrected(obs->original());

    Helpers::set_fhqc(editor, 1);
    if (fmis == 0 or fmis == 2) {
        Helpers::set_flag(editor, kvalobs::flag::fmis, 0);
        Helpers::set_flag(editor, kvalobs::flag::fd,   1);
    } else if (fmis == 1) {
        Helpers::set_flag(editor, kvalobs::flag::fmis, 3);
    }
    if (qc2ok)
        Helpers::set_fhqc(editor, 4);

    eda->newVersion();
    editor->commit();
}

void accept_corrected(EditAccessPtr eda, const SensorTime& sensorTime, bool qc2ok)
{
    EditDataPtr obs = eda->findE(sensorTime);
    if (not obs) {
        METLIBS_LOG_ERROR("accept_corrected without obs for " << sensorTime);
        return;
    }

    const int fmis = obs->controlinfo().flag(kvalobs::flag::fmis);
    EditDataEditorPtr editor = eda->editor(obs);

    if (Helpers::float_eq()(obs->original(), editor->corrected())
        and (not Helpers::is_accumulation(editor)) and fmis < 2)
    {
        Helpers::set_flag(editor, kvalobs::flag::fd, 1);
        Helpers::set_fhqc(editor, 1);
    } else if (fmis == 0) {
        Helpers::set_fhqc(editor, 7);
    } else if (fmis == 1) {
        Helpers::set_fhqc(editor, 5);
    } else {
        METLIBS_LOG_ERROR("bad accept_corrected for " << sensorTime);
        return;
    }
    if (qc2ok)
        Helpers::set_fhqc(editor, 4);

    eda->newVersion();
    editor->commit();
}

void reject(EditAccessPtr eda, const SensorTime& sensorTime, bool qc2ok)
{
    EditDataPtr obs = eda->findE(sensorTime);
    if (not obs) {
        METLIBS_LOG_ERROR("reject without obs for " << sensorTime);
        return;
    }

    const int fmis = obs->controlinfo().flag(kvalobs::flag::fmis);
    if (fmis == 1 or fmis == 3) {
        METLIBS_LOG_ERROR("bad reject with fmis=1/3 for " << sensorTime);
        return;
    }

    EditDataEditorPtr editor = eda->editor(obs);
    Helpers::reject(editor);
    if (qc2ok)
        Helpers::set_fhqc(editor, 4);

    eda->newVersion();
    editor->commit();
}

void interpolate_or_correct(EditAccessPtr eda, const SensorTime& sensorTime, float newC)
{
    EditDataPtr obs = eda->findE(sensorTime);
    if (not obs) {
        METLIBS_LOG_ERROR("interpolate_or_correct without obs for " << sensorTime);
        return;
    }
    if (Helpers::is_accumulation(obs)) {
        METLIBS_LOG_ERROR("accept_corrected for accumulation for " << sensorTime);
        return;
    }

    EditDataEditorPtr editor = eda->editor(obs);
    Helpers::auto_correct(editor, newC);

    eda->newVersion();
    editor->commit();
}

} // anonymous namespace

SimpleCorrections::SimpleCorrections(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SimpleCorrections)
{
    ui->setupUi(this);

    // TODO move setting of minimum sizes to retranslateUi somehow
    QWidget* labels1[] = { ui->labelStation, ui->labelObstime, ui->labelFlags, ui->labelOriginal, 0 };
    setCommonMinWidth(labels1);
    QWidget* labels2[] = { ui->labelType, ui->labelParam, 0 };
    setCommonMinWidth(labels2);

    setMaximumSize(QSize(minimumSize().width(), maximumSize().height()));

    update();
}

SimpleCorrections::~SimpleCorrections()
{
}

void SimpleCorrections::setDataAccess(EditAccessPtr eda, ModelAccessPtr mda)
{
    DataView::setDataAccess(eda, mda);
    update();
}

void SimpleCorrections::navigateTo(const SensorTime& st)
{
    METLIBS_LOG_SCOPE();
    if (eq_SensorTime()(mSensorTime, st))
        return;

    mSensorTime = st;
    METLIBS_LOG_DEBUG(LOGVAL(mSensorTime));

    update();
}

void SimpleCorrections::update()
{
    METLIBS_LOG_SCOPE();

    const Sensor& s = mSensorTime.sensor;
    ModelDataPtr mdl;
    EditDataPtr obs;
    if (s.valid()) {
        ui->textStation->setText(QString::number(s.stationId));
        ui->textParam->setText(Helpers::parameterName(s.paramId));
        ui->textType->setText(QString::number(s.typeId));

        ui->textObstime->setText(QString::fromStdString(timeutil::to_iso_extended_string(mSensorTime.time)));

        obs = mDA ? mDA->findE(mSensorTime) : EditDataPtr();
        mdl = mMA ? mMA->find(mSensorTime) : ModelDataPtr();
        if (not mMA)
            METLIBS_LOG_DEBUG("no model access");
        if (not mdl)
            METLIBS_LOG_DEBUG("no model value for " << mSensorTime);
    } else {
        ui->textStation->setText("");
        ui->textParam->setText("");
        ui->textType->setText("");

        ui->textObstime->setText("");
    }
    ui->textFlags->setText(obs ? Helpers::getFlagText(obs->controlinfo()) : "");
    ui->textOriginal->setText(obs ? QString::number(obs->original()) : "");
    ui->textCorrected->setText(obs ? QString::number(obs->corrected()) : "");
    ui->textModel->setText(mdl ? QString::number(mdl->value()) : "");

    ui->textNewCorrectedValue->setText("");

    enableEditing();
}

void SimpleCorrections::enableEditing()
{
    METLIBS_LOG_SCOPE();

    enum { ORIG_OK, ORIG_OK_QC2, CORR_OK, CORR_OK_QC2, REJECT, REJECT_QC2,
           NEW_CORRECTED, N_BUTTONS };
    QWidget* buttons[N_BUTTONS] = {
        ui->buttonAcceptOriginal,  ui->buttonAcceptOriginalQC2,
        ui->buttonAcceptCorrected, ui->buttonAcceptCorrectedQC2,
        ui->buttonReject,          ui->buttonRejectQC2,
        ui->textNewCorrectedValue
    };
    bool enable[N_BUTTONS];
    
    const Sensor& s = mSensorTime.sensor;
    EditDataPtr obs = (mDA and s.valid()) ? mDA->findE(mSensorTime) : EditDataPtr();
    if (not obs) {
        setEnabled(false);
        return;
    }
    setEnabled(true);
    std::fill(enable, enable+N_BUTTONS, true);

    const int fmis = obs->controlinfo().flag(kvalobs::flag::fmis);
    if (s.paramId == kvalobs::PARAMID_RR_24 or Helpers::is_accumulation(obs)) {
        // for accumulations, always use WatchRR
        std::fill(enable, enable+N_BUTTONS, false);
    } else if (fmis == 3) {
        std::fill(enable, enable+N_BUTTONS, false);
        enable[NEW_CORRECTED] = true;
    } else if (fmis == 2) {
        enable[CORR_OK] = enable[CORR_OK_QC2] = false;
    } else if (fmis == 1) {
        enable[REJECT] = enable[REJECT_QC2] = false;
    }

    for (int b=0; b<N_BUTTONS; ++b)
        buttons[b]->setEnabled(enable[b]);
}

void SimpleCorrections::onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr data)
{
    METLIBS_LOG_SCOPE();
    METLIBS_LOG_DEBUG(LOGVAL(data->sensorTime()) << LOGVAL(mSensorTime));
    if (data and eq_SensorTime()(data->sensorTime(), mSensorTime))
        update();
}

void SimpleCorrections::onAcceptOriginal()
{
    accept_original(mDA, mSensorTime, false);
}

void SimpleCorrections::onAcceptOriginalQC2()
{
    accept_original(mDA, mSensorTime, true);
}

void SimpleCorrections::onAcceptCorrected()
{
    accept_corrected(mDA, mSensorTime, false);
}

void SimpleCorrections::onAcceptCorrectedQC2()
{
    accept_corrected(mDA, mSensorTime, true);
}

void SimpleCorrections::onReject()
{
    reject(mDA, mSensorTime, false);
}

void SimpleCorrections::onRejectQC2()
{
    reject(mDA, mSensorTime, true);
}

void SimpleCorrections::onNewCorrected()
{
    bool ok;
    float newC = ui->textNewCorrectedValue->text().toFloat(&ok);
    if (ok)
        interpolate_or_correct(mDA, mSensorTime, newC);
}
