
#include "SimpleCorrections.hh"

#include "AcceptReject.hh"
#include "ChecksTableModel.hh"
#include "ColumnFactory.hh"
#include "ModelData.hh"
#include "ToolTipStringListModel.hh"

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

    QIcon iconAccept("icons:accept.svg"), iconReject("icons:reject.svg");
    ui->buttonAcceptCorrected   ->setIcon(iconAccept);
    ui->buttonAcceptCorrectedQC2->setIcon(iconAccept);
    ui->buttonAcceptOriginal    ->setIcon(iconAccept);
    ui->buttonAcceptOriginalQC2 ->setIcon(iconAccept);
    ui->buttonReject   ->setIcon(iconReject);
    ui->buttonRejectQC2->setIcon(iconReject);

    update();
}

SimpleCorrections::~SimpleCorrections()
{
  if (mDA and mSensorTime.valid())
    mDA->removeSubscription(ObsSubscription(mSensorTime.sensor.stationId, TimeRange(mSensorTime.time, mSensorTime.time)));
}

void SimpleCorrections::setDataAccess(EditAccessPtr eda, ModelAccessPtr mda)
{
  if (mSensorTime.valid() and eda != mDA) {
    const ObsSubscription sub(mSensorTime.sensor.stationId, TimeRange(mSensorTime.time, mSensorTime.time));
    if (eda)
      eda->addSubscription(sub);
    if (mDA)
      mDA->removeSubscription(sub);
  }

  DataView::setDataAccess(eda, mda);

  mChecksModel.reset(new ChecksTableModel(eda));
  ui->tableChecks->setModel(mChecksModel.get());
  update();
}

void SimpleCorrections::navigateTo(const SensorTime& st)
{
    METLIBS_LOG_SCOPE();
    METLIBS_LOG_DEBUG(LOGVAL(st));
    
    mChecksModel->navigateTo(st);
    ui->tableChecks->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

    if (eq_SensorTime()(mSensorTime, st))
        return;

    if (mDA) {
      if (st.valid())
        mDA->addSubscription(ObsSubscription(st.sensor.stationId, TimeRange(st.time, st.time)));
      if (mSensorTime.valid())
        mDA->removeSubscription(ObsSubscription(mSensorTime.sensor.stationId, TimeRange(mSensorTime.time, mSensorTime.time)));
    }

    const bool changedSensor = (not eq_Sensor()(mSensorTime.sensor, st.sensor));

    mSensorTime = st;
    METLIBS_LOG_DEBUG(LOGVAL(mSensorTime));

    if (changedSensor) {
      mItemFlags     = ColumnFactory::itemForSensor(mDA, mSensorTime.sensor, ObsColumn::NEW_CONTROLINFO);
      mItemOriginal  = ColumnFactory::itemForSensor(mDA, mSensorTime.sensor, ObsColumn::ORIGINAL);
      mItemCorrected = ColumnFactory::itemForSensor(mDA, mSensorTime.sensor, ObsColumn::NEW_CORRECTED);
    }

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
    const SensorTime& st = obs->sensorTime();
    const QVariant vFont = item->data(obs, st, Qt::FontRole);
    if (vFont.isValid())
      font = vFont.value<QFont>();
    
    const QVariant vFG = item->data(obs, st, Qt::ForegroundRole);
    if (vFG.isValid())
      palette.setColor(w->foregroundRole(), vFG.value<QBrush>());

    const QVariant vBG = item->data(obs, st, Qt::BackgroundRole);
    if (vBG.isValid())
      palette.setColor(w->backgroundRole(), vBG.value<QBrush>());

    toolTip = item->data(obs, st, Qt::ToolTipRole).toString();
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
        if (mDA and not obs) METLIBS_LOG_DEBUG("mDA but no obs at " << mSensorTime);
        mdl = mMA ? mMA->find(mSensorTime) : ModelDataPtr();
    } else {
        ui->textStation->setText("");
        ui->textParam->setText("");
        ui->textType->setText("");

        ui->textObstime->setText("");
    }

    ui->textFlags->setText((obs and mItemFlags) ? mItemFlags->data(obs, mSensorTime, Qt::DisplayRole).toString() : "");
    setFBF(ui->textFlags, mItemFlags, obs);

    ui->textOriginal->setText((obs and mItemOriginal) ? mItemOriginal->data(obs, mSensorTime, Qt::DisplayRole).toString() : "");
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
        ttl->setStringList(mItemCorrected->data(obs, mSensorTime, ObsColumn::TextCodesRole).toStringList());
        ttl->setToolTipList(mItemCorrected->data(obs, mSensorTime, ObsColumn::TextCodeExplanationsRole).toStringList());
        
        c->setEnabled((mItemCorrected->flags(obs) & Qt::ItemIsEditable));
        
        const QVariant valueType = mItemCorrected->data(obs, mSensorTime, ObsColumn::ValueTypeRole);
        c->setEditable(valueType.toInt() != ObsColumn::TextCode);
        
        const Qt::ItemDataRole role = Qt::DisplayRole;
        QVariant currentText = mItemCorrected->data(obs, mSensorTime, role).toString();
        const int idx = c->findData(currentText, role);
        // if it is valid, adjust the combobox
        if(idx >= 0)
          c->setCurrentIndex(idx);
        else
          c->setCurrentText(currentText.toString());
      }
      setFBF(c, mItemCorrected, obs);
    }
    enableEditing();
}

void SimpleCorrections::enableEditing()
{
    METLIBS_LOG_SCOPE();

    EditDataPtr obs = (mDA and mSensorTime.sensor.valid()) ? mDA->findE(mSensorTime) : EditDataPtr();
    if (not obs) {
        setEnabled(false);
        return;
    }
    setEnabled(true);
    const int p = AcceptReject::possibilities(obs);
    METLIBS_LOG_DEBUG("possibilities = " << p);

    ui->buttonReject   ->setEnabled((p & AcceptReject::CAN_REJECT) != 0);
    ui->buttonRejectQC2->setEnabled((p & AcceptReject::CAN_REJECT) != 0);

    ui->buttonAcceptOriginal   ->setEnabled((p & AcceptReject::CAN_ACCEPT_ORIGINAL) != 0);
    ui->buttonAcceptOriginalQC2->setEnabled((p & AcceptReject::CAN_ACCEPT_ORIGINAL) != 0);

    ui->buttonAcceptCorrected   ->setEnabled((p & AcceptReject::CAN_ACCEPT_CORRECTED) != 0);
    ui->buttonAcceptCorrectedQC2->setEnabled((p & AcceptReject::CAN_ACCEPT_CORRECTED) != 0);

    ui->comboCorrected->setEnabled((p & AcceptReject::CAN_CORRECT) != 0);
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
  mDA->newVersion();
  AcceptReject::accept_original(mDA, mSensorTime, false);
}

void SimpleCorrections::onAcceptOriginalQC2()
{
  mDA->newVersion();
  AcceptReject::accept_original(mDA, mSensorTime, true);
}

void SimpleCorrections::onAcceptCorrected()
{
  mDA->newVersion();
  AcceptReject::accept_corrected(mDA, mSensorTime, false);
}

void SimpleCorrections::onAcceptCorrectedQC2()
{
  mDA->newVersion();
  AcceptReject::accept_corrected(mDA, mSensorTime, true);
}

void SimpleCorrections::onReject()
{
  mDA->newVersion();
  AcceptReject::reject(mDA, mSensorTime, false);
}

void SimpleCorrections::onRejectQC2()
{
  mDA->newVersion();
  AcceptReject::reject(mDA, mSensorTime, true);
}

void SimpleCorrections::onNewCorrected()
{
  METLIBS_LOG_SCOPE();
  if (not (mDA and mItemCorrected and mSensorTime.valid()))
    return;
  
  EditDataPtr obs = mDA->findE(mSensorTime);
  if (obs) {
    if (not (mItemCorrected->flags(obs) & Qt::ItemIsEditable))
      return;
    if (not mItemCorrected->setData(obs, mDA, mSensorTime, ui->comboCorrected->currentText(), Qt::EditRole)) {
      update();
    }
  }
}
