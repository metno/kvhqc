
#ifndef BUSY_LABEL_HH
#define BUSY_LABEL_HH 1

#include "AnimatedLabel.hh"

class BusyLabel : public AnimatedLabel
{
  Q_OBJECT;

public:
  BusyLabel(QWidget* parent = 0);

public Q_SLOTS:
  void setBusy(bool busy);
};
     
#endif // BUSY_LABEL_HH
