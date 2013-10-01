
#ifndef UTIL_ETAPROGRESSBAR_HH
#define UTIL_ETAPROGRESSBAR_HH 1

#include <QtGui/QProgressBar>

class EtaProgressBar : public QProgressBar {
  Q_OBJECT;
public:
  EtaProgressBar(QWidget* parent=0)
    : QProgressBar(parent) { }

  virtual QString text() const;

private:
  mutable qint64 mStart;
};

#endif // UTIL_ETAPROGRESSBAR_HH
