
#ifndef UTIL_GUI_ETAPROGRESSBAR_HH
#define UTIL_GUI_ETAPROGRESSBAR_HH 1

#include <QtGui/QProgressBar>

class EtaProgressBar : public QProgressBar {
  Q_OBJECT;
public:
  EtaProgressBar(QWidget* parent=0)
    : QProgressBar(parent) { }

  virtual QString text() const;

  void start();

private:
  mutable qint64 mStart;
};

#endif // UTIL_GUI_ETAPROGRESSBAR_HH
