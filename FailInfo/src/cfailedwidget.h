/********************************************************************************
** Form generated from reading UI file 'cfailedwidget.ui'
**
** Created: Fri Feb 4 10:15:34 2011
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef CFAILEDWIDGET_H
#define CFAILEDWIDGET_H

#include <Qt3Support/Q3Header>
#include <Qt3Support/Q3ListView>
#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_cFailedWidget : public QWidget
{
public:
    QHBoxLayout *hboxLayout;
    Q3ListView *cfailedList;

    void setupUi(QWidget *cFailedWidget)
    {
        if (cFailedWidget->objectName().isEmpty())
            cFailedWidget->setObjectName(QString::fromUtf8("cFailedWidget"));
        cFailedWidget->resize(347, 150);
        hboxLayout = new QHBoxLayout(cFailedWidget);
        hboxLayout->setSpacing(6);
        hboxLayout->setContentsMargins(11, 11, 11, 11);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        cfailedList = new Q3ListView(cFailedWidget);
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

        QMetaObject::connectSlotsByName(cFailedWidget);
    } // setupUi

    void retranslateUi(QWidget *cFailedWidget)
    {
        cFailedWidget->setWindowTitle(QApplication::translate("cFailedWidget", "Form1", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_WHATSTHIS
        cFailedWidget->setProperty("whatsThis", QVariant(QApplication::translate("cFailedWidget", "Dette er en liste over de automatiske kontrollene som har for\303\245rsaket at observasjonen ble merket som mistenkelig.", 0, QApplication::UnicodeUTF8)));
#endif // QT_NO_WHATSTHIS
        cfailedList->header()->setLabel(0, QApplication::translate("cFailedWidget", "Kontroll", 0, QApplication::UnicodeUTF8));
        cfailedList->header()->setLabel(1, QApplication::translate("cFailedWidget", "Forklaring", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class cFailedWidget: public Ui_cFailedWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // CFAILEDWIDGET_H
