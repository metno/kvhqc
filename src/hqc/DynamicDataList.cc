
#include "DynamicDataList.hh"

#include "common/DataListModel.hh"
#include "ViewChanges.hh"
#include "util/ChangeReplay.hh"

#include <QtGui/QPushButton>
#include <QtXml/QDomElement>

#include "ui_datalist.h"

#define MILOGGER_CATEGORY "kvhqc.DynamicDataList"
#include "common/ObsLogging.hh"

DynamicDataList::DynamicDataList(QWidget* parent)
  : DataList(parent)
{
  mButtonEarlier = new QPushButton("+", this);
  mButtonEarlier->setToolTip(tr("Earlier"));
  connect(mButtonEarlier, SIGNAL(clicked()), this, SLOT(onEarlier()));
  ui->table->addScrollBarWidget(mButtonEarlier, Qt::AlignTop);

  mButtonLater = new QPushButton("+", this);
  mButtonLater->setToolTip(tr("Later"));
  connect(mButtonLater, SIGNAL(clicked()), this, SLOT(onLater()));
  ui->table->addScrollBarWidget(mButtonLater, Qt::AlignBottom);
}

DynamicDataList::~DynamicDataList()
{
}

void DynamicDataList::retranslateUi()
{
  mButtonEarlier->setToolTip(tr("Earlier"));
  mButtonLater->setToolTip(tr("Later"));
  DataList::retranslateUi();
}

void DynamicDataList::doNavigateTo()
{
  METLIBS_LOG_TIME();
  const SensorTime& cst = currentSensorTime();

  // updateColumns === changed sensor
  const bool updateColumns = (not mStoreSensorTime.valid()
      or not eq_Sensor()(mStoreSensorTime.sensor, cst.sensor));

  if (updateColumns or not mTableModel.get() or mTableModel->findIndexes(cst).empty()) {
    if (updateColumns)
      storeChanges();

    mTimeLimits = ViewChanges::defaultTimeLimits(cst);
    mOriginalTimeLimits = mTimeLimits;
    if (updateColumns)
      replayXML(ViewChanges::fetch(cst.sensor, viewType(), viewId()));
    updateModel(makeModel());
  }
  DataList::doNavigateTo();
}

std::string DynamicDataList::viewId() const
{
  return "1";
}

void DynamicDataList::storeChanges()
{
  METLIBS_LOG_SCOPE();
  if (mStoreSensorTime.valid() and not eq_SensorTime()(mStoreSensorTime, currentSensorTime())) {
    ViewChanges::store(mStoreSensorTime.sensor, viewType(), viewId(), changesXML());
    mStoreSensorTime = currentSensorTime();
  }
}

namespace /* anonymous */ {
static const char T_ATTR_START[] = "start";
static const char T_ATTR_END[]   = "end";
static const char E_TAG_CHANGES[] = "changes";
static const char E_TAG_TSHIFT[]  = "timeshift";
} // anonymous namespace


std::string DynamicDataList::changesXML()
{
  METLIBS_LOG_SCOPE();
  QDomDocument doc("changes");
  QDomElement doc_changes = doc.createElement(E_TAG_CHANGES);
  doc.appendChild(doc_changes);

  changes(doc_changes);

  if (doc_changes.hasChildNodes()) {
    METLIBS_LOG_DEBUG("changes for " << mStoreSensorTime << ": " << doc.toString());
    return doc.toString().toStdString();
  } else {
    return "";
  }
}

void DynamicDataList::replayXML(const std::string& changesXML)
{
  METLIBS_LOG_SCOPE();
  if (changesXML.empty())
    return;

  QDomDocument doc;
  doc.setContent(QString::fromStdString(changesXML));

  const QDomElement doc_changes = doc.documentElement();
  replay(doc_changes);
}

void DynamicDataList::changes(QDomElement& doc_changes)
{
  METLIBS_LOG_SCOPE();
  if (mOriginalTimeLimits.t0() != mTimeLimits.t0() or mOriginalTimeLimits.t1() != mTimeLimits.t1()) {
    QDomDocument doc = doc_changes.ownerDocument();
    QDomElement doc_timeshift = doc.createElement(E_TAG_TSHIFT);
    doc_timeshift.setAttribute(T_ATTR_START, (mTimeLimits.t0() - mOriginalTimeLimits.t0()).hours());
    doc_timeshift.setAttribute(T_ATTR_END,   (mTimeLimits.t1() - mOriginalTimeLimits.t1()).hours());
    doc_changes.appendChild(doc_timeshift);
  }
}

void DynamicDataList::replay(const QDomElement& doc_changes)
{
  TimeSpan newTimeLimits(mOriginalTimeLimits);
  const QDomElement doc_timeshift = doc_changes.firstChildElement(E_TAG_TSHIFT);
  if (not doc_timeshift.isNull()) {
    const int dT0 = doc_timeshift.attribute(T_ATTR_START).toInt();
    const int dT1 = doc_timeshift.attribute(T_ATTR_END)  .toInt();
    const timeutil::ptime t0 = mOriginalTimeLimits.t0() + boost::posix_time::hours(dT0);
    const timeutil::ptime t1 = mOriginalTimeLimits.t1() + boost::posix_time::hours(dT1);
    newTimeLimits = TimeSpan(t0, t1);
  }
  mTimeLimits = newTimeLimits;
}

void DynamicDataList::onEarlier()
{
  mTimeLimits = TimeSpan(mTimeLimits.t0() - boost::posix_time::hours(24), mTimeLimits.t1());
  updateModel(makeModel());
}

void DynamicDataList::onLater()
{
  mTimeLimits = TimeSpan(mTimeLimits.t0(), mTimeLimits.t1() + boost::posix_time::hours(24));
  updateModel(makeModel());
}
