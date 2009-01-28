/********************************************************************************
** Form generated from reading ui file 'cfailedwidget.ui'
**
** Created: Thu May 22 07:53:40 2008
**      by: Qt User Interface Compiler version 4.2.3
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef CFAILEDWIDGET_H
#define CFAILEDWIDGET_H

#include <Qt3Support/Q3Header>
#include <Qt3Support/Q3ListView>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QWidget>
#include <Qt3Support/Q3MimeSourceFactory>

class Ui_cFailedWidget : public QWidget
{
  Q_OBJECT
public:
    QHBoxLayout *hboxLayout;
    Q3ListView *cfailedList;

    void setupUi(QWidget *cFailedWidget)
    {
    cFailedWidget->setObjectName(QString::fromUtf8("cFailedWidget"));
    hboxLayout = new QHBoxLayout(cFailedWidget);
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(11);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    cfailedList = new Q3ListView(cFailedWidget,"cfailedList");
    cfailedList->addColumn(QApplication::translate("cFailedWidget", "Kontroll", 0, QApplication::UnicodeUTF8));
    cfailedList->header()->setClickEnabled(true, cfailedList->header()->count() - 1);
    cfailedList->header()->setResizeEnabled(true, cfailedList->header()->count() - 1);
    cfailedList->addColumn(QApplication::translate("cFailedWidget", "Forklaring", 0, QApplication::UnicodeUTF8));
    cfailedList->header()->setClickEnabled(true, cfailedList->header()->count() - 1);
    cfailedList->header()->setResizeEnabled(true, cfailedList->header()->count() - 1);
    cfailedList->setObjectName(QString::fromUtf8("cfailedList"));
    cfailedList->setAllColumnsShowFocus(true);
    cfailedList->setResizeMode(Q3ListView::LastColumn);
    cfailedList->setTreeStepSize(20);

    hboxLayout->addWidget(cfailedList);


    retranslateUi(cFailedWidget);

    QSize size(347, 150);
    size = size.expandedTo(cFailedWidget->minimumSizeHint());
    cFailedWidget->resize(size);


    QMetaObject::connectSlotsByName(cFailedWidget);
    } // setupUi

    void retranslateUi(QWidget *cFailedWidget)
    {
    cFailedWidget->setWindowTitle(QApplication::translate("cFailedWidget", "Form1", 0, QApplication::UnicodeUTF8));
    cFailedWidget->setProperty("whatsThis", QVariant(QApplication::translate("cFailedWidget", "Dette er en liste over de automatiske kontrollene som har for\303\245rsaket at observasjonen ble merket som mistenkelig.", 0, QApplication::UnicodeUTF8)));
    cfailedList->header()->setLabel(0, QApplication::translate("cFailedWidget", "Kontroll", 0, QApplication::UnicodeUTF8));
    cfailedList->header()->setLabel(1, QApplication::translate("cFailedWidget", "Forklaring", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(cFailedWidget);
    } // retranslateUi

};

namespace Ui {
    class cFailedWidget: public Ui_cFailedWidget {};
} // namespace Ui

#endif // CFAILEDWIDGET_H
