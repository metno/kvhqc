#ifndef AUTHENTICATIONDIALOG_H
#define AUTHENTICATIONDIALOG_H

#include <qvariant.h>


#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

class Ui_AuthenticationDialog
{
public:
    QVBoxLayout *vboxLayout;
    QLabel *textLabel1;
    QHBoxLayout *hboxLayout;
    QVBoxLayout *vboxLayout1;
    QLabel *textLabel2;
    QLabel *textLabel3;
    QVBoxLayout *vboxLayout2;
    QLineEdit *username;
    QLineEdit *password;
    QHBoxLayout *hboxLayout1;
    QSpacerItem *spacerItem;
    QPushButton *buttonOk;
    QPushButton *buttonCancel;

    void setupUi(QDialog *AuthenticationDialog)
    {
    if (AuthenticationDialog->objectName().isEmpty())
        AuthenticationDialog->setObjectName(QString::fromUtf8("AuthenticationDialog"));
    AuthenticationDialog->resize(265, 132);
    AuthenticationDialog->setSizeGripEnabled(false);
    vboxLayout = new QVBoxLayout(AuthenticationDialog);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(11);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    textLabel1 = new QLabel(AuthenticationDialog);
    textLabel1->setObjectName(QString::fromUtf8("textLabel1"));
    textLabel1->setWordWrap(false);

    vboxLayout->addWidget(textLabel1);

    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setSpacing(6);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    textLabel2 = new QLabel(AuthenticationDialog);
    textLabel2->setObjectName(QString::fromUtf8("textLabel2"));
    textLabel2->setWordWrap(false);

    vboxLayout1->addWidget(textLabel2);

    textLabel3 = new QLabel(AuthenticationDialog);
    textLabel3->setObjectName(QString::fromUtf8("textLabel3"));
    textLabel3->setWordWrap(false);

    vboxLayout1->addWidget(textLabel3);


    hboxLayout->addLayout(vboxLayout1);

    vboxLayout2 = new QVBoxLayout();
    vboxLayout2->setSpacing(6);
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    username = new QLineEdit(AuthenticationDialog);
    username->setObjectName(QString::fromUtf8("username"));
    username->setMaxLength(32);

    vboxLayout2->addWidget(username);

    password = new QLineEdit(AuthenticationDialog);
    password->setObjectName(QString::fromUtf8("password"));
    password->setMaxLength(32);
    password->setEchoMode(QLineEdit::Password);

    vboxLayout2->addWidget(password);


    hboxLayout->addLayout(vboxLayout2);


    vboxLayout->addLayout(hboxLayout);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    spacerItem = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout1->addItem(spacerItem);

    buttonOk = new QPushButton(AuthenticationDialog);
    buttonOk->setObjectName(QString::fromUtf8("buttonOk"));
    buttonOk->setAutoDefault(true);
    buttonOk->setDefault(true);

    hboxLayout1->addWidget(buttonOk);

    buttonCancel = new QPushButton(AuthenticationDialog);
    buttonCancel->setObjectName(QString::fromUtf8("buttonCancel"));
    buttonCancel->setAutoDefault(true);

    hboxLayout1->addWidget(buttonCancel);


    vboxLayout->addLayout(hboxLayout1);


    retranslateUi(AuthenticationDialog);
    QObject::connect(buttonOk, SIGNAL(clicked()), AuthenticationDialog, SLOT(doAuthenticate()));
    QObject::connect(buttonCancel, SIGNAL(clicked()), AuthenticationDialog, SLOT(reject()));

    QMetaObject::connectSlotsByName(AuthenticationDialog);
    } // setupUi

    void retranslateUi(QDialog *AuthenticationDialog)
    {
    AuthenticationDialog->setWindowTitle(QApplication::translate("AuthenticationDialog", "Logg inn", 0, QApplication::UnicodeUTF8));
    textLabel1->setText(QApplication::translate("AuthenticationDialog", "Vennligst skriv inn brukernavn og passord:", 0, QApplication::UnicodeUTF8));
    textLabel2->setText(QApplication::translate("AuthenticationDialog", "Brukernavn:", 0, QApplication::UnicodeUTF8));
    textLabel3->setText(QApplication::translate("AuthenticationDialog", "Passord:", 0, QApplication::UnicodeUTF8));
    buttonOk->setText(QApplication::translate("AuthenticationDialog", "&OK", 0, QApplication::UnicodeUTF8));
    buttonOk->setShortcut(QString());
    buttonCancel->setText(QApplication::translate("AuthenticationDialog", "&Avbryt", 0, QApplication::UnicodeUTF8));
    buttonCancel->setShortcut(QApplication::translate("AuthenticationDialog", "Alt+A", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(AuthenticationDialog);
    } // retranslateUi

};

namespace Ui {
    class AuthenticationDialog: public Ui_AuthenticationDialog {};
} // namespace Ui

class AuthenticationDialog : public QDialog, public Ui::AuthenticationDialog
{
    Q_OBJECT

public:
    AuthenticationDialog(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~AuthenticationDialog();

protected slots:
    virtual void languageChange();

    virtual void doAuthenticate();


};

#endif // AUTHENTICATIONDIALOG_H
