
#include "SimpleCorrections.hh"

#include "ModelData.hh"

#include <kvalobs/kvDataOperations.h>

#include "ui_simplecorrections.h"
//#define NDEBUG
#include "debug.hh"

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
        LOG4HQC_ERROR("SimpleCorrections", "accept_original without obs for " << sensorTime);
        return;
    }

    const kvalobs::kvControlInfo ci = obs->controlinfo();
    const int fmis = ci.flag(kvalobs::flag::fmis);
    if (fmis == 3) {
        LOG4HQC_ERROR("SimpleCorrections", "fmis=3, accept_original not possible for " << sensorTime);
        return;
    }
    if (ci.flag(kvalobs::flag::fnum) == 0 and not (fmis == 0 or fmis == 1 or fmis == 2)) {
        LOG4HQC_ERROR("SimpleCorrections", "bad accept_original, would not set fhqc for " << sensorTime);
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

    editor->commit();
}

void accept_corrected(EditAccessPtr eda, const SensorTime& sensorTime, bool qc2ok)
{
    EditDataPtr obs = eda->findE(sensorTime);
    if (not obs) {
        LOG4HQC_ERROR("SimpleCorrections", "accept_corrected without obs for " << sensorTime);
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
        LOG4HQC_ERROR("SimpleCorrections", "bad accept_corrected for " << sensorTime);
        return;
    }
    if (qc2ok)
        Helpers::set_fhqc(editor, 4);
    editor->commit();
}

void reject(EditAccessPtr eda, const SensorTime& sensorTime, bool qc2ok)
{
    EditDataPtr obs = eda->findE(sensorTime);
    if (not obs) {
        LOG4HQC_ERROR("SimpleCorrections", "reject without obs for " << sensorTime);
        return;
    }

    const int fmis = obs->controlinfo().flag(kvalobs::flag::fmis);
    if (fmis == 1 or fmis == 3) {
        LOG4HQC_ERROR("SimpleCorrections", "bad reject with fmis=1/3 for " << sensorTime);
        return;
    }

    EditDataEditorPtr editor = eda->editor(obs);
    Helpers::reject(editor);
    if (qc2ok)
        Helpers::set_fhqc(editor, 4);
    editor->commit();
}

void interpolate_or_correct(EditAccessPtr eda, const SensorTime& sensorTime, float newC)
{
    EditDataPtr obs = eda->findE(sensorTime);
    if (not obs) {
        LOG4HQC_ERROR("SimpleCorrections", "interpolate_or_correct without obs for " << sensorTime);
        return;
    }
    if (Helpers::is_accumulation(obs)) {
        LOG4HQC_ERROR("SimpleCorrections", "accept_corrected for accumulation for " << sensorTime);
        return;
    }

    EditDataEditorPtr editor = eda->editor(obs);
    Helpers::auto_correct(editor, newC);
    editor->commit();
}

} // anonymous namespace

SimpleCorrections::SimpleCorrections(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SimpleCorrections)
{
    ui->setupUi(this);

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
    LOG_SCOPE("SimpleCorrections");
    if (eq_SensorTime()(mSensorTime, st))
        return;

    mSensorTime = st;
    LOG4SCOPE_DEBUG(DBG1(mSensorTime));

    update();
}

void SimpleCorrections::update()
{
    LOG_SCOPE("SimpleCorrections");

    const Sensor& s = mSensorTime.sensor;
    if (s.stationId > 0) {
        ui->textStation->setText(QString::number(s.stationId));
        ui->textParam->setText(Helpers::parameterName(s.paramId));
        ui->textType->setText(QString::number(s.typeId));

        ui->textObstime->setText(QString::fromStdString(timeutil::to_iso_extended_string(mSensorTime.time)));

        EditDataPtr obs = mDA ? mDA->findE(mSensorTime) : EditDataPtr();
        if (obs) {
            ui->textFlags->setText(Helpers::getFlagText(obs->controlinfo()));
            ui->textOriginal->setText(QString::number(obs->original()));
            ui->textCorrected->setText(QString::number(obs->corrected()));
        }
        ModelDataPtr mdl = mMA ? mMA->find(mSensorTime) : ModelDataPtr();
        if (mdl)
            ui->textModel->setText(QString::number(mdl->value()));
    } else {
        ui->textStation->setText("--");
        ui->textParam->setText("--");
        ui->textType->setText("--");

        ui->textObstime->setText("");
    }
    enableEditing();
}

void SimpleCorrections::enableEditing()
{
    LOG_SCOPE("SimpleCorrections");

    enum { ORIG_OK, ORIG_OK_QC2, CORR_OK, CORR_OK_QC2, REJECT, REJECT_QC2,
           INTERPOLATED, CORRECTED, N_BUTTONS };
    QWidget* buttons[N_BUTTONS] = {
        ui->buttonAcceptOriginal,  ui->buttonAcceptOriginalQC2,
        ui->buttonAcceptCorrected, ui->buttonAcceptCorrectedQC2,
        ui->buttonReject,          ui->buttonRejectQC2,
        ui->textInterpolatedValue, ui->textCorrectedValue
    };
    bool enable[N_BUTTONS];
    
    const Sensor& s = mSensorTime.sensor;
    EditDataPtr obs = (mDA and s.stationId > 0 and s.paramId != 110) ? mDA->findE(mSensorTime) : EditDataPtr();
    std::fill(enable, enable+N_BUTTONS, obs);

    if (obs) {
        const int fmis = obs->controlinfo().flag(kvalobs::flag::fmis);
        if (s.paramId == kvalobs::PARAMID_RR_24 or Helpers::is_accumulation(obs)) {
            // for accumulations, always use WatchRR
            std::fill(enable, enable+N_BUTTONS, false);
        } else if (fmis == 3) {
            enable[INTERPOLATED] &= true;
        } else if (fmis == 2) {
            enable[CORR_OK] = enable[CORR_OK_QC2] = false;
        } else if (fmis == 1) {
            enable[REJECT] = enable[REJECT_QC2] = false;
        }
    }

    for (int b=0; b<N_BUTTONS; ++b)
        buttons[b]->setEnabled(enable[b]);
}

void SimpleCorrections::onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr data)
{
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

void SimpleCorrections::onInterpolated()
{
    bool ok;
    float newC = ui->textInterpolatedValue->text().toFloat(&ok);
    if (ok)
        interpolate_or_correct(mDA, mSensorTime, newC);
}

void SimpleCorrections::onCorrected()
{
    bool ok;
    float newC = ui->textCorrectedValue->text().toFloat(&ok);
    if (ok)
        interpolate_or_correct(mDA, mSensorTime, newC);
}
