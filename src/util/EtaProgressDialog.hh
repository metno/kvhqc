
#ifndef UTIL_GUI_ETAPROGRESSDIALOG_HH
#define UTIL_GUI_ETAPROGRESSDIALOG_HH 1

#include <QtGui/QProgressDialog>

class EtaProgressBar;

class EtaProgressDialog : public QProgressDialog {
public:
  EtaProgressDialog(QWidget* parent=0);

  void setValue(int value);

private:
  EtaProgressBar* mBar;
};

#endif // UTIL_GUI_ETAPROGRESSDIALOG_HH
