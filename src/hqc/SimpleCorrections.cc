
#include "SimpleCorrections.hh"

#include "ChecksTableModel.hh"

#include "common/AcceptReject.hh"
#include "common/ColumnFactory.hh"
#include "common/KvHelpers.hh"
#include "common/ModelData.hh"
#include "common/HqcApplication.hh"

#include "util/ToolTipStringListModel.hh"

#include <boost/make_shared.hpp>

#include "ui_singleobservation.h"

#define MILOGGER_CATEGORY "kvhqc.SimpleCorrections"
#include "common/ObsLogging.hh"

SimpleCorrections::SimpleCorrections(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui_SingleObservation)
  , mDA(hqcApp->editAccess())
  , mMA(hqcApp->modelAccess())
{
  METLIBS_LOG_SCOPE();
  ui->setupUi(this);
  ToolTipStringListModel* ttl = new ToolTipStringListModel(ui->comboCorrected);
  ui->comboCorrected->setModel(ttl);
  ui->tableChecks->horizontalHeader()->setResizeMode(QHeaderView::Interactive);

  mChecksModel = new ChecksTableModel(this);
  ui->tableChecks->setModel(mChecksModel);
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
  
  ui->tableChecks->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

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

  if (mDA) {
    METLIBS_LOG_DEBUG("requesting " << mSensorTime);
    mObsBuffer = boost::make_shared<SingleObsBuffer>(mSensorTime);
    connect(mObsBuffer.get(), SIGNAL(updateDataEnd(const ObsData_pv&)), this, SLOT(onDataChanged()));
    connect(mObsBuffer.get(), SIGNAL(dropDataEnd(const SensorTime_v&)), this, SLOT(onDataChanged()));
    connect(mObsBuffer.get(), SIGNAL(bufferCompleted(bool)), this, SLOT(update()));
    mObsBuffer->postRequest(mDA);
  } else {
    update();
  }
}

namespace /* anonymous */
{
static void setFBF(QWidget* w, DataItem_p item, ObsData_p obs)
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
  ObsData_p obs;
  if (s.valid()) {
    ui->textStation->setText(QString::number(s.stationId));
    ui->textStation->setToolTip(Helpers::stationInfo(s.stationId));

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

    ui->textParam->setText(Helpers::paramName(s.paramId));
    ui->textParam->setToolTip(Helpers::paramInfo(s.paramId));

    ui->textType->setText(QString::number(s.typeId));
    ui->textType->setToolTip(Helpers::typeInfo(s.typeId));

    ui->textObstime->setText(QString::fromStdString(timeutil::to_iso_extended_string(mSensorTime.time)));

    ObsData_p bobs = mObsBuffer->get();
    // maybe the observation is from an old request?
    if (bobs and eq_SensorTime()(bobs->sensorTime(), mSensorTime))
      obs = bobs;
    if (mMA)
      mdl = mMA->find(mSensorTime);
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

  mChecksModel->showChecks(obs);
  ui->tableChecks->resizeColumnsToContents();
}

void SimpleCorrections::enableEditing()
{
  METLIBS_LOG_SCOPE();

  ObsData_p obs = mObsBuffer ? mObsBuffer->get() : ObsData_p();
  if (not obs) {
    setEnabled(false);
    return;
  }
  setEnabled(true);
  const int p = AcceptReject::possibilities(obs);
  METLIBS_LOG_DEBUG("possibilities = " << p);

  ui->buttonReject   ->setEnabled((p & AcceptReject::CAN_REJECT) != 0);
  ui->buttonAcceptOriginal   ->setEnabled((p & AcceptReject::CAN_ACCEPT_ORIGINAL) != 0);
  ui->buttonAcceptCorrected   ->setEnabled((p & AcceptReject::CAN_ACCEPT_CORRECTED) != 0);

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
  if (obs) {
    if (not (mItemCorrected->flags(obs) & Qt::ItemIsEditable))
      return;
    if (not mItemCorrected->setData(obs, mDA, mSensorTime, ui->comboCorrected->currentText(), Qt::EditRole)) {
      update();
    }
  }
}

void SimpleCorrections::onStartEditor()
{
}

void SimpleCorrections::onQc2Toggled(bool)
{
}
