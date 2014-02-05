
#include "UserSettings.hh"

#include "common/HqcApplication.hh"
#include "common/HqcUserConfig.hh"

#include <QtGui/QColorDialog>

#include "ui_user_settings.h"

UserSettings::UserSettings(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::UserSettings)
{
  ui->setupUi(this);
  initDataOrigUI2Colors();
}

UserSettings::~UserSettings()
{
}

void UserSettings::accept()
{
  HqcUserConfig* uc = hqcApp->userConfig();

  uc->setDataOrigUI2Background(1, mColorUI2_1);
  uc->setDataOrigUI2Background(2, mColorUI2_2);
  uc->setDataOrigUI2Background(3, mColorUI2_3);
  uc->setDataOrigUI2Background(9, mColorUI2_9);

  QDialog::accept();
}

void UserSettings::initDataOrigUI2Colors()
{
  HqcUserConfig* uc = hqcApp->userConfig();

  mColorUI2_1 = uc->dataOrigUI2Background(1);
  mColorUI2_2 = uc->dataOrigUI2Background(2);
  mColorUI2_3 = uc->dataOrigUI2Background(3);
  mColorUI2_9 = uc->dataOrigUI2Background(9);

  ui->labelUI2_1->setPalette(QPalette(mColorUI2_1));
  ui->labelUI2_2->setPalette(QPalette(mColorUI2_2));
  ui->labelUI2_3->setPalette(QPalette(mColorUI2_3));
  ui->labelUI2_9->setPalette(QPalette(mColorUI2_9));
}

void UserSettings::onOriginalColorUI2(QLabel* label, QColor& colorUI2)
{
  const QColor color = QColorDialog::getColor(colorUI2, this);
  if (color.isValid()) {
    colorUI2 = color;
    label->setPalette(QPalette(colorUI2));
  }
}

void UserSettings::onOriginalColorUI2_1()
{
  onOriginalColorUI2(ui->labelUI2_1, mColorUI2_1);
}

void UserSettings::onOriginalColorUI2_2()
{
  onOriginalColorUI2(ui->labelUI2_2, mColorUI2_2);
}

void UserSettings::onOriginalColorUI2_3()
{
  onOriginalColorUI2(ui->labelUI2_3, mColorUI2_3);
}

void UserSettings::onOriginalColorUI2_9()
{
  onOriginalColorUI2(ui->labelUI2_9, mColorUI2_9);
}
