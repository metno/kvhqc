
#include "SimpleCorrections.hh"

#include "ChecksTableModel.hh"
#include "ColumnFactory.hh"
#include "ModelData.hh"
#include "ToolTipStringListModel.hh"

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
    METLIBS_LOG_SCOPE();
    ui->setupUi(this);
    ToolTipStringListModel* ttl = new ToolTipStringListModel(ui->comboCorrected);
    ui->comboCorrected->setModel(ttl);
    ui->tableChecks->horizontalHeader()->setResizeMode(QHeaderView::Interactive);

    // TODO move setting of minimum sizes to retranslateUi somehow
    QWidget* labels1[] = { ui->labelStation, ui->labelObstime, ui->labelFlags, ui->labelOriginal, 0 };
    setCommonMinWidth(labels1);
    QWidget* labels2[] = { ui->labelType, ui->labelParam, 0 };
    setCommonMinWidth(labels2);

    METLIBS_LOG_DEBUG(LOGVAL(minimumSize().height()) << LOGVAL(ui->tableChecks->minimumSize().height()));

    setMaximumSize(QSize(minimumSize().width(), maximumSize().height()));

#if 0
#include "../src/icon_accept.xpm"
#include "../src/icon_reject.xpm"
    QIcon iconAccept, iconReject;
    iconAccept.addPixmap(QPixmap(icon_accept));
    iconReject.addPixmap(QPixmap(icon_reject));
    ui->buttonAcceptCorrected   ->setIcon(iconAccept);
    ui->buttonAcceptCorrectedQC2->setIcon(iconAccept);
    ui->buttonAcceptOriginal    ->setIcon(iconAccept);
    ui->buttonAcceptOriginalQC2 ->setIcon(iconAccept);
    ui->buttonReject   ->setIcon(iconReject);
    ui->buttonRejectQC2->setIcon(iconReject);
#endif

    update();
}

SimpleCorrections::~SimpleCorrections()
{
}

void SimpleCorrections::setDataAccess(EditAccessPtr eda, ModelAccessPtr mda)
{
    DataView::setDataAccess(eda, mda);
    mChecksModel.reset(new ChecksTableModel(eda));
    ui->tableChecks->setModel(mChecksModel.get());
    update();
}

void SimpleCorrections::navigateTo(const SensorTime& st)
{
    METLIBS_LOG_SCOPE();
    mChecksModel->navigateTo(st);
    ui->tableChecks->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

    if (eq_SensorTime()(mSensorTime, st))
        return;

    mSensorTime = st;
    METLIBS_LOG_DEBUG(LOGVAL(mSensorTime));

    mItemFlags     = ColumnFactory::itemForSensor(mDA, mSensorTime.sensor, ColumnFactory::NEW_CONTROLINFO);
    mItemOriginal  = ColumnFactory::itemForSensor(mDA, mSensorTime.sensor, ColumnFactory::ORIGINAL);
    mItemCorrected = ColumnFactory::itemForSensor(mDA, mSensorTime.sensor, ColumnFactory::NEW_CORRECTED);

    update();
}

namespace /* anonymous */
{
static void setFBF(QWidget* w, DataItemPtr item, EditDataPtr obs)
{
  if (not (w and w->parentWidget()))
    return;
  
  QPalette palette = w->parentWidget()->palette();
  QFont font = w->parentWidget()->font();
  QString toolTip;

  if (item and obs) {
    const QVariant vFont = item->data(obs, Qt::FontRole);
    if (vFont.isValid())
      font = vFont.value<QFont>();
    
    const QVariant vFG = item->data(obs, Qt::ForegroundRole);
    if (vFG.isValid())
      palette.setColor(w->foregroundRole(), vFG.value<QBrush>());

    const QVariant vBG = item->data(obs, Qt::BackgroundRole);
    if (vBG.isValid())
      palette.setColor(w->backgroundRole(), vBG.value<QBrush>());

    toolTip = item->data(obs, Qt::ToolTipRole).toString();
  }
  
  w->setFont(font);
  w->setPalette(palette);
  w->setToolTip(toolTip);
}
} // namespace anonymous

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
    } else {
        ui->textStation->setText("");
        ui->textParam->setText("");
        ui->textType->setText("");

        ui->textObstime->setText("");
    }

    ui->textFlags->setText((obs and mItemFlags) ? mItemFlags->data(obs, Qt::DisplayRole).toString() : "");
    setFBF(ui->textFlags, mItemFlags, obs);

    ui->textOriginal->setText((obs and mItemOriginal) ? mItemOriginal->data(obs, Qt::DisplayRole).toString() : "");
    setFBF(ui->textOriginal, mItemOriginal, obs);

    ui->textModel->setText(mdl ? QString::number(mdl->value()) : "");

    { QComboBox*& c = ui->comboCorrected;
      ToolTipStringListModel* ttl = static_cast<ToolTipStringListModel*>(c->model());
      if (not mItemCorrected or not obs) {
        ttl->setStringList(QStringList());
        ttl->setToolTipList(QStringList());
        c->setEditable(false);
        c->setEnabled(false);
        c->setCurrentText("");
      } else {
      // FIXME this is almost identical to ObsDelegate code
        ttl->setStringList(mItemCorrected->data(obs, ObsColumn::TextCodesRole).toStringList());
        ttl->setToolTipList(mItemCorrected->data(obs, ObsColumn::TextCodeExplanationsRole).toStringList());
        
        c->setEnabled((mItemCorrected->flags() & Qt::ItemIsEditable));
        
        const QVariant valueType = mItemCorrected->data(obs, ObsColumn::ValueTypeRole);
        c->setEditable(valueType.toInt() != ObsColumn::TextCode);
        
        QString currentText = mItemCorrected->data(obs ,Qt::EditRole).toString();
        const int idx = c->findText(currentText);
        // if it is valid, adjust the combobox
        if(idx >= 0)
          c->setCurrentIndex(idx);
        else
          c->setCurrentText(currentText);
      }
      setFBF(c, mItemCorrected, obs);
    }
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
        ui->comboCorrected
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
  METLIBS_LOG_SCOPE();
  if (not (mDA and mItemCorrected and mSensorTime.valid()))
    return;
  if (not (mItemCorrected->flags() & Qt::ItemIsEditable))
    return;
  
  EditDataPtr obs = mDA->findE(mSensorTime);
  if (obs)
    mItemCorrected->setData(obs, mDA, mSensorTime, ui->comboCorrected->currentText(), Qt::EditRole);
}
