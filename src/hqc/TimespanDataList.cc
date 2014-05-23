
#include "TimespanDataList.hh"

#include "common/DataListModel.hh"
#include "ViewChanges.hh"
#include "util/ChangeReplay.hh"
#include "util/Helpers.hh"

#include <QtCore/QSignalMapper>
#include <QtGui/QMenu>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QtXml/QDomElement>

#include "ui_datalist.h"

#define MILOGGER_CATEGORY "kvhqc.TimespanDataList"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {
const int HOUR = 3600, DAY = 24*HOUR;

QString timeActionText(int step)
{
  QString t = (step < 0)
      ? qApp->translate("TimespanDataList", "%1 earlier")
      : qApp->translate("TimespanDataList", "%1 later");
  return t.arg(Helpers::timeStepAsText(step));
}

QAction* makeTimeAction(QMenu* menu, int timeStep, QSignalMapper* mapper)
{
  QIcon icon(timeStep < 0 ? "icons:earlier.svg" : "icons:later.svg");
  QAction* a = menu->addAction(icon, timeActionText(timeStep));
  a->setData(QVariant(timeStep));
  QObject::connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
  mapper->setMapping(a, a);
  return a;
}

static const char T_ATTR_START[] = "start";
static const char T_ATTR_END[]   = "end";
static const char E_TAG_TSHIFT[]  = "timeshift";
} // anonymous namespace

// ########################################################################

TimespanDataList::TimespanDataList(QWidget* parent)
  : DataList(parent)
{
  mMenuTime = new QMenu(this);
  mMenuStart = mMenuTime->addMenu(tr("Change start time"));
  mMenuEnd   = mMenuTime->addMenu(tr("Change end time"));
  mActionResetTime = mMenuTime->addAction(tr("Reset time range"));
  connect(mActionResetTime, SIGNAL(triggered()), this, SLOT(onResetTime()));

  mMapperStart = new QSignalMapper(this);
  mMapperEnd = new QSignalMapper(this);
  connect(mMapperStart, SIGNAL(mapped(QObject*)), this, SLOT(onChangeStart(QObject*)));
  connect(mMapperEnd,   SIGNAL(mapped(QObject*)), this, SLOT(onChangeEnd(QObject*)));

  const int steps[] = { 1*HOUR, 12*HOUR, 1*DAY, 3*DAY, 0 };
  for (int i=0; steps[i] > 0; ++i) {
    mTimeActions.append(makeTimeAction(mMenuStart, -steps[i], mMapperStart));
    mTimeActions.append(makeTimeAction(mMenuEnd,    steps[i], mMapperEnd));
  }
  mMenuStart->addSeparator();
  mMenuEnd->addSeparator();
  for (int i=0; steps[i] > 0; ++i) {
    mTimeActions.append(makeTimeAction(mMenuStart,  steps[i], mMapperStart));
    mTimeActions.append(makeTimeAction(mMenuEnd,   -steps[i], mMapperEnd));
  }

  mButtonTime = new QToolButton(this);
  mButtonTime->setIcon(QIcon("icons:earlier.svg"));
  mButtonTime->setToolTip(tr("Change start end end times"));
  mButtonTime->setMenu(mMenuTime);
  mButtonTime->setPopupMode(QToolButton::InstantPopup);
  ui->layoutButtons->addWidget(mButtonTime);
}

TimespanDataList::~TimespanDataList()
{
}

void TimespanDataList::retranslateUi()
{
  mButtonTime->setToolTip(tr("Change start end end times"));
  mMenuStart->setTitle(tr("Change start time"));
  mMenuEnd->setTitle(tr("Change end time"));
  mActionResetTime->setText(tr("Reset time range"));

  for (int i=0; i<mTimeActions.size(); ++i) {
    QAction* a = mTimeActions.at(i);
    a->setText(timeActionText(a->data().toInt()));
  }

  DataList::retranslateUi();
}

SensorTime TimespanDataList::sensorSwitch() const
{
  METLIBS_LOG_SCOPE();
  const SensorTime& sst = storeSensorTime(), cst = currentSensorTime();
  METLIBS_LOG_DEBUG(LOGVAL(sst) << LOGVAL(sst));
  if (eq_Sensor()(sst.sensor, cst.sensor) and timeSpan().contains(cst.time))
    return sst;
  else
    return cst;
}

void TimespanDataList::switchSensorPrepare()
{
  mTimeLimits = ViewChanges::defaultTimeLimits(currentSensorTime());
  mOriginalTimeLimits = mTimeLimits;
}

void TimespanDataList::loadChangesXML(const QDomElement& doc_changes)
{
  const QDomElement doc_timeshift = doc_changes.firstChildElement(E_TAG_TSHIFT);
  if (not doc_timeshift.isNull()) {
    const int dT0 = doc_timeshift.attribute(T_ATTR_START).toInt();
    const int dT1 = doc_timeshift.attribute(T_ATTR_END)  .toInt();
    const timeutil::ptime t0 = mOriginalTimeLimits.t0() + boost::posix_time::hours(dT0);
    const timeutil::ptime t1 = mOriginalTimeLimits.t1() + boost::posix_time::hours(dT1);
    mTimeLimits = TimeSpan(t0, t1);
  }
}

void TimespanDataList::switchSensorDone()
{
  updateModel(makeModel());
}

void TimespanDataList::storeChangesXML(QDomElement& doc_changes)
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

void TimespanDataList::onChangeStart(QObject* action)
{
  METLIBS_LOG_SCOPE();
  const int step = static_cast<QAction*>(action)->data().toInt();
  METLIBS_LOG_DEBUG(LOGVAL(mTimeLimits) << LOGVAL(step));

  Time t0 = mTimeLimits.t0() + boost::posix_time::seconds(step);
  Time t1 = mTimeLimits.t1();
  if (t0 >= t1)
    t1 = t0 + boost::posix_time::seconds(step);
  mTimeLimits = TimeSpan(t0, t1);
  METLIBS_LOG_DEBUG(LOGVAL(mTimeLimits));

  updateModel(makeModel());
}

void TimespanDataList::onChangeEnd(QObject* action)
{
  METLIBS_LOG_SCOPE();
  const int step = static_cast<QAction*>(action)->data().toInt();
  METLIBS_LOG_DEBUG(LOGVAL(mTimeLimits) << LOGVAL(step));

  Time t1 = mTimeLimits.t1() + boost::posix_time::seconds(step);
  Time t0 = mTimeLimits.t0();
  if (t1 <= t0)
    t0 = t1 + boost::posix_time::seconds(step);
  mTimeLimits = TimeSpan(t0, t1);
  METLIBS_LOG_DEBUG(LOGVAL(mTimeLimits));

  updateModel(makeModel());
}

void TimespanDataList::onResetTime()
{
  METLIBS_LOG_SCOPE();
  mTimeLimits = mOriginalTimeLimits;
  updateModel(makeModel());
}