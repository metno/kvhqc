
#include "DynamicDataView.hh"

#include "ViewChanges.hh"
#include "util/ChangeReplay.hh"
#include "util/Helpers.hh"

#include <QDomElement>

#define MILOGGER_CATEGORY "kvhqc.DynamicDataView"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {
static const char E_TAG_CHANGES[] = "changes";
} // anonymous namespace

DynamicDataView::DynamicDataView(QWidget* parent)
  : AbstractDataView(parent)
{
}

DynamicDataView::~DynamicDataView()
{
}

void DynamicDataView::doNavigateTo()
{
  METLIBS_LOG_TIME(LOGMYTYPE());
  const SensorTime storeST = sensorSwitch();
  const bool sv = mStoreST.valid(), sw = sv and not eq_SensorTime()(mStoreST, storeST);
  METLIBS_LOG_DEBUG(LOGVAL(mStoreST) << LOGVAL(storeST) << LOGVAL(sv) << LOGVAL(sw));
  mStoreST = storeST;
  if (sw or not sv)
    doSensorSwitch();
}

void DynamicDataView::doSensorSwitch()
{
  loadChanges();
}

SensorTime DynamicDataView::sensorSwitch() const
{
  return currentSensorTime();
}

std::string DynamicDataView::viewId() const
{
  return "1";
}

void DynamicDataView::storeChangesXML(QDomElement&)
{
}

void DynamicDataView::loadChangesXML(const QDomElement&)
{
}

void DynamicDataView::storeChanges()
{
  METLIBS_LOG_SCOPE(LOGMYTYPE());
  if (mStoreST.valid()) {
    QDomDocument doc;
    QDomElement doc_changes = doc.createElement(E_TAG_CHANGES);
    doc.appendChild(doc_changes);

    storeChangesXML(doc_changes);

    const std::string vt = viewType();
    if (doc_changes.hasChildNodes()) {
      const std::string xml = doc.toString().toStdString();
      ViewChanges::store(mStoreST.sensor, vt, viewId(), xml);
    } else {
      ViewChanges::forget(mStoreST.sensor, vt, viewId());
    }
  }
}

void DynamicDataView::loadChanges()
{
  METLIBS_LOG_SCOPE(LOGMYTYPE());

  const std::string stored = ViewChanges::fetch(mStoreST.sensor, viewType(), viewId());
  if (not stored.empty()) {
    QDomDocument doc;
    doc.setContent(QString::fromStdString(stored));
    const QDomElement doc_changes = doc.documentElement();
    loadChangesXML(doc_changes);
  }
}
