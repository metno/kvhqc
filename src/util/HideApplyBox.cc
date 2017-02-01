
#include "HideApplyBox.hh"

#include "ui_hideapplybox.h"

HideApplyBox::HideApplyBox(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::HideApplyBox)
{
  ui->setupUi(this);
  
  connect(ui->hideButton, SIGNAL(clicked()), this, SIGNAL(hide()));
  connect(ui->applyButton, SIGNAL(clicked()), this, SIGNAL(apply()));
  
  connect(ui->applyHideButton, SIGNAL(clicked()), this, SLOT(hideApply()));
}

HideApplyBox::~HideApplyBox()
{
}

void HideApplyBox::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    ui->retranslateUi(this);
  QWidget::changeEvent(event);
}

void HideApplyBox::setCanApply(bool enabled)
{
  ui->applyButton->setEnabled(enabled);
  ui->applyHideButton->setEnabled(enabled);
}

void HideApplyBox::hideApply()
{
  Q_EMIT hide();
  Q_EMIT apply();
}
