
#include "UserSettings.hh"

#include "common/HqcApplication.hh"
#include "common/HqcUserConfig.hh"

#include <QColorDialog>

#include "ui_user_settings.h"

#define MILOGGER_CATEGORY "kvhqc.UserSettings"
#include "util/HqcLogging.hh"

UserSettings::UserSettings(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::UserSettings)
{
  ui->setupUi(this);
  initDataOrigUI2Colors();
  initLanguage();
}

UserSettings::~UserSettings()
{
}

void UserSettings::accept()
{
  acceptDataOrigUI2Colors();
  acceptLanguage();
  QDialog::accept();
}

void UserSettings::acceptDataOrigUI2Colors()
{
  HqcUserConfig* uc = hqcApp->userConfig();

  uc->setDataOrigUI2Background(1, mColorUI2_1);
  uc->setDataOrigUI2Background(2, mColorUI2_2);
  uc->setDataOrigUI2Background(3, mColorUI2_3);
  uc->setDataOrigUI2Background(9, mColorUI2_9);
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

void UserSettings::initLanguage()
{
  ui->comboLanguage->addItem(tr("System"), "");

  const QString language = hqcApp->savedLanguage();
  const QStringList available = hqcApp->availableLanguages();
  int index = 0, current = 0; // start at 0, which is "system"
  Q_FOREACH(QString l, available) {
    index += 1;
    const QLocale locale(l);
    ui->comboLanguage->addItem(locale.nativeLanguageName(), l);
    if (l == language)
      current = index;
  }
  ui->comboLanguage->setCurrentIndex(current);
}

void UserSettings::acceptLanguage()
{
  const int current = ui->comboLanguage->currentIndex();
  if (current >= 0) {
    const QString l = ui->comboLanguage->itemData(current).toString();
    hqcApp->saveLanguage(l);
  }
}
