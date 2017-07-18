
#include "SimpleCorrections.hh"

#include "ChecksTableModel.hh"
#include "DataHistoryTableModel.hh"

#include "common/AcceptReject.hh"
#include "common/ColumnFactory.hh"
#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/ModelData.hh"

#include "util/ToolTipStringListModel.hh"

#include <QEvent>
#include <QHeaderView>

#include "ui_singleobservation.h"

#define MILOGGER_CATEGORY "kvhqc.SimpleCorrections"
#include "common/ObsLogging.hh"

SimpleCorrections::SimpleCorrections(EditAccess_p eda, ModelAccess_p mda, QWidget* parent)
  : QWidget(parent)
  , ui(new Ui_SingleObservation)
  , mDA(eda)
  , mMA(mda)
  , mModelBuffer(std::make_shared<ModelBuffer>(mMA))
{
  METLIBS_LOG_SCOPE();
  ui->setupUi(this);
  ToolTipStringListModel* ttl = new ToolTipStringListModel(ui->comboCorrected);
  ui->comboCorrected->setModel(ttl);
  ui->tableChecks->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

  mChecksModel = new ChecksTableModel(this);
  ui->tableChecks->setModel(mChecksModel);

  mHistoryModel = new DataHistoryTableModel(KvMetaDataBuffer::instance()->handler(), this); // FIXME
  ui->tableHistory->setModel(mHistoryModel);
  connect(mHistoryModel, SIGNAL(modelReset()), this, SLOT(onHistoryTableUpdated()));

  // FIXME bad things happen if this slot is called while the user is
  // editing the corrected value, or after editing and just before
  // pressing return
  connect(mModelBuffer.get(), SIGNAL(received(const ModelData_pv&)),
      this, SLOT(update()));

  update();
}

void SimpleCorrections::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    ui->retranslateUi(this);
  QWidget::changeEvent(event);
}

SimpleCorrections::~SimpleCorrections()
{
}

void SimpleCorrections::navigateTo(const SensorTime& st)
{
  METLIBS_LOG_TIME(LOGVAL(st));
  
  if (eq_SensorTime()(mSensorTime, st))
    return;

  const bool changedSensor = (not eq_Sensor()(mSensorTime.sensor, st.sensor));

  mSensorTime = st;
  METLIBS_LOG_DEBUG(LOGVAL(mSensorTime));

  if (changedSensor) {
    mItemFlags     = ColumnFactory::itemForSensor(mDA, mSensorTime.sensor, ObsColumn::NEW_CONTROLINFO);
    mItemOriginal  = ColumnFactory::itemForSensor(mDA, mSensorTime.sensor, ObsColumn::ORIGINAL);
    mItemCorrected = ColumnFactory::itemForSensor(mDA, mSensorTime.sensor, ObsColumn::NEW_CORRECTED);
  }

  mModelBuffer->clear();
  if (mDA) {
    METLIBS_LOG_DEBUG("requesting " << mSensorTime);
    mObsBuffer = std::make_shared<SingleObsBuffer>(mSensorTime);
    connect(mObsBuffer.get(), SIGNAL(newDataEnd(const ObsData_pv&)), this, SLOT(onDataChanged()));
    connect(mObsBuffer.get(), SIGNAL(updateDataEnd(const ObsData_pv&)), this, SLOT(onDataChanged()));
    connect(mObsBuffer.get(), SIGNAL(dropDataEnd(const SensorTime_v&)), this, SLOT(onDataChanged()));
    connect(mObsBuffer.get(), SIGNAL(bufferCompleted(const QString&)), this, SLOT(update()));
    mObsBuffer->postRequest(mDA);
  } else {
    update();
  }
}

namespace /* anonymous */
{
static void setFBF(QWidget* w, DataItem_p item, ObsData_p obs, const SensorTime& st)
{
  if (not (w and w->parentWidget()))
    return;
  
  QPalette palette = w->parentWidget()->palette();
  QFont font = w->parentWidget()->font();
  QString toolTip;

  if (item) {
    const QVariant vFont = item->data(obs, st, Qt::FontRole);
    if (vFont.isValid())
      font = vFont.value<QFont>();
    
    const QVariant vFG = item->data(obs, st, Qt::ForegroundRole);
    if (vFG.isValid())
      palette.setColor(w->foregroundRole(), vFG.value<QColor>());

    const QVariant vBG = item->data(obs, st, Qt::BackgroundRole);
    if (vBG.isValid())
      palette.setColor(w->backgroundRole(), vBG.value<QColor>());

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
  ModelData_p mdl;
  ObsData_p obs;
  if (mSensorTime.valid()) {
    ui->textStation->setText(QString::number(s.stationId));
    ui->textStation->setToolTip(KvMetaDataBuffer::instance()->stationInfo(s.stationId));

    const bool showLevel = (s.level != 0);
    ui->labelLevel->setVisible(showLevel);
    ui->textLevel->setVisible(showLevel);
    if (showLevel)
      ui->textLevel->setText(QString::number(s.level));

    const bool showSensorNr = (s.sensor != 0);
    ui->labelSensorNr->setVisible(showSensorNr);
    ui->textSensorNr->setVisible(showSensorNr);
    if (showSensorNr)
      ui->textSensorNr->setText(QString::number(s.sensor));

    ui->textParam->setText(KvMetaDataBuffer::instance()->paramName(s.paramId));
    ui->textParam->setToolTip(KvMetaDataBuffer::instance()->paramInfo(s.paramId));

    ui->textType->setText(QString::number(s.typeId));
    ui->textType->setToolTip(KvMetaDataBuffer::instance()->typeInfo(s.typeId));

    ui->textObstime->setText(QString::fromStdString(timeutil::to_iso_extended_string(mSensorTime.time)));

    ObsData_p bobs = mObsBuffer->get();
    // maybe the observation is from an old request?
    if (bobs and eq_SensorTime()(bobs->sensorTime(), mSensorTime))
      obs = bobs;
    mdl = mModelBuffer->get(mSensorTime);
  } else {
    ui->textStation->setText("");
    ui->textStation->setToolTip("");

    ui->labelLevel->setVisible(false);
    ui->textLevel->setVisible(false);
    ui->labelSensorNr->setVisible(false);
    ui->textSensorNr->setVisible(false);

    ui->textParam->setText("");
    ui->textParam->setToolTip("");
    ui->textType->setText("");
    ui->textType->setToolTip("");

    ui->textObstime->setText("");
  }

  ui->textObstime->setToolTip(obs ? tr("tbtime: %1").arg(QString::fromStdString(timeutil::to_iso_extended_string(obs->tbtime()))) : "");

  ui->textFlags->setText((obs and mItemFlags) ? mItemFlags->data(obs, mSensorTime, Qt::DisplayRole).toString() : "");
  setFBF(ui->textFlags, mItemFlags, obs, mSensorTime);

  ui->textOriginal->setText((obs and mItemOriginal) ? mItemOriginal->data(obs, mSensorTime, Qt::DisplayRole).toString() : "");
  setFBF(ui->textOriginal, mItemOriginal, obs, mSensorTime);

  ui->textModel->setText(mdl ? QString::number(mdl->value()) : "");

  { QComboBox*& c = ui->comboCorrected;
    ToolTipStringListModel* ttl = static_cast<ToolTipStringListModel*>(c->model());
    if (not mItemCorrected) {
      ttl->setStringList(QStringList());
      ttl->setToolTipList(QStringList());
      c->setEditable(false);
      c->setEnabled(false);
      c->setEditText("");
    } else {
      // FIXME this is almost identical to ObsDelegate code
      ttl->setStringList(mItemCorrected->data(obs, mSensorTime, ObsColumn::TextCodesRole).toStringList());
      ttl->setToolTipList(mItemCorrected->data(obs, mSensorTime, ObsColumn::TextCodeExplanationsRole).toStringList());
        
      c->setEnabled((mItemCorrected->flags(obs) & Qt::ItemIsEditable));
        
      const QVariant valueType = mItemCorrected->data(obs, mSensorTime, ObsColumn::ValueTypeRole);
      c->setEditable(valueType.toInt() != ObsColumn::TextCode);
        
      const Qt::ItemDataRole role = Qt::DisplayRole;
      QString currentText = mItemCorrected->data(obs, mSensorTime, role).toString();
      const int idx = c->findData(currentText, role);
      METLIBS_LOG_DEBUG(LOGVAL(obs) << LOGVAL(currentText) << LOGVAL(idx));
      // if it is valid, adjust the combobox
      if(idx >= 0)
        c->setCurrentIndex(idx);
      else
        c->setEditText(currentText);
    }
    setFBF(c, mItemCorrected, obs, mSensorTime);
  }
  enableEditing();

  mChecksModel->showChecks(obs);
  ui->tableChecks->resizeColumnsToContents();

  mHistoryModel->showHistory(mSensorTime);
}

void SimpleCorrections::enableEditing()
{
  METLIBS_LOG_SCOPE();

  setEnabled(mSensorTime.valid());

  ObsData_p obs = mObsBuffer ? mObsBuffer->get() : ObsData_p();

  const int p = AcceptReject::possibilities(obs);
  METLIBS_LOG_DEBUG("possibilities = " << p);

  ui->buttonReject         ->setEnabled((p & AcceptReject::CAN_REJECT) != 0);
  ui->buttonAcceptOriginal ->setEnabled((p & AcceptReject::CAN_ACCEPT_ORIGINAL) != 0);
  ui->buttonAcceptCorrected->setEnabled((p & AcceptReject::CAN_ACCEPT_CORRECTED) != 0);

  bool enableAcceptModel = (p & AcceptReject::CAN_ACCEPT_CORRECTED) != 0;
  if (enableAcceptModel) {
    ModelData_p mdl = mModelBuffer->get(mSensorTime);
    enableAcceptModel = (bool)mdl;
  }
  ui->buttonAcceptModel->setEnabled(enableAcceptModel);

  ui->comboCorrected->setEnabled((p & AcceptReject::CAN_CORRECT) != 0);
}

void SimpleCorrections::onDataChanged()
{
  METLIBS_LOG_SCOPE();
  update();
}

void SimpleCorrections::onAcceptOriginal()
{
  mDA->newVersion();
  if (mObsBuffer->get())
    AcceptReject::accept_original(mDA, mObsBuffer->get());
}

void SimpleCorrections::onAcceptCorrected()
{
  mDA->newVersion();
  if (mObsBuffer->get())
    AcceptReject::accept_corrected(mDA, mObsBuffer->get(), ui->checkQC2->isChecked());
}

void SimpleCorrections::onAcceptModel()
{
  mDA->newVersion();
  if (mObsBuffer->get())
    AcceptReject::accept_model(mDA, mMA, mObsBuffer->get(), ui->checkQC2->isChecked());
}

void SimpleCorrections::onReject()
{
  mDA->newVersion();
  if (mObsBuffer->get())
    AcceptReject::reject(mDA, mObsBuffer->get(), ui->checkQC2->isChecked());
}

void SimpleCorrections::onNewCorrected()
{
  METLIBS_LOG_SCOPE();
  if (not (mDA and mItemCorrected and mSensorTime.valid()))
    return;
  
  ObsData_p obs = mObsBuffer->get();
  if (obs and not (mItemCorrected->flags(obs) & Qt::ItemIsEditable))
    return;
  if (not mItemCorrected->setData(obs, mDA, mSensorTime, ui->comboCorrected->currentText(), Qt::EditRole))
    update();
}

void SimpleCorrections::onStartEditor()
{
}

void SimpleCorrections::onQc2Toggled(bool)
{
}

void SimpleCorrections::onHistoryTableUpdated()
{
  ui->tableHistory->resizeColumnsToContents();
}
