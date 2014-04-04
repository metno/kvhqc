
#include "EtaProgressDialog.hh"

#include "EtaProgressBar.hh"

#define MILOGGER_CATEGORY "kvhqc.EtaProgressDialog"
#include "util/HqcLogging.hh"

EtaProgressDialog::EtaProgressDialog(QWidget* parent)
  : QProgressDialog(parent)
  , mBar(new EtaProgressBar(this))
{
  setBar(mBar);
}

void EtaProgressDialog::setValue(int v)
{
  if (v <= 1)
    mBar->start();
  QProgressDialog::setValue(v);
}
