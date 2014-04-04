
#include "EtaProgressBar.hh"

#include <QtCore/QDateTime>

#define MILOGGER_CATEGORY "kvhqc.EtaProgressBar"
#include "util/HqcLogging.hh"

void EtaProgressBar::start()
{
  mStart = QDateTime::currentMSecsSinceEpoch();
}

QString EtaProgressBar::text() const
{
  const int v = value(), mi = minimum(), ma = maximum();
  if (mi >= ma or v <= mi or v >= ma or v<=1)
    return "";

  const qint64 now = QDateTime::currentMSecsSinceEpoch();
  const qint64 elapsed = (now - mStart);
  if (elapsed <= 5000)
    return "...";

  const int eta = int(0.001f * elapsed * float(ma-v) / float(v-mi));

  const std::div_t seconds = std::div(eta, 60);
  return QString(tr("ETA %1:%2")).arg(seconds.quot).arg(seconds.rem, 2, 10, QChar('0'));
}
