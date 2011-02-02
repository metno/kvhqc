/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 Copyright (C) 2010 met.no

 Contact information:
 Norwegian Meteorological Institute
 Box 43 Blindern
 0313 OSLO
 NORWAY
 email: kvalobs-dev@met.no

 This file is part of KVALOBS

 KVALOBS is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation; either version 2 
 of the License, or (at your option) any later version.
 
 KVALOBS is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License along 
 with KVALOBS; if not, write to the Free Software Foundation Inc., 
 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <KvalobsDataDelegate.h>
#include <KvalobsDataModel.h>
#include <hqcmain.h>
#include <QDoubleValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>

namespace model
{

  KvalobsDataDelegate::KvalobsDataDelegate(QObject * parent) :
    QStyledItemDelegate(parent)
  {
    setup_();
  }

  KvalobsDataDelegate::~KvalobsDataDelegate()
  {
  }

  namespace
  {
    class InputValidator : public QDoubleValidator
    {
    public:
      InputValidator() : QDoubleValidator(0) {}
      virtual State validate(QString & input, int & pos) const
      {
        if ( input.isEmpty() )
          return Acceptable;
        return QDoubleValidator::validate(input, pos);
      }
    };
  }

  QWidget * KvalobsDataDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
  {
    Editor * ret = new Editor(parent);
    //static QDoubleValidator validator(0);
    static InputValidator validator;
    ret->setValidator(& validator);
    return ret;
  }

  void KvalobsDataDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
  {
    Editor * e = static_cast<Editor *>(editor);

    QVariant value = index.model()->data(index, Qt::EditRole);
    bool ok;
    double f = value.toDouble(& ok);
    if ( ok )
      e->setText(QString::number(f));
    else
      e->clear();
  }

  void KvalobsDataDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
  {
    Editor * e = static_cast<Editor *>(editor);
    QString enteredValue = e->text();
    float newValue = -32766;
    if ( not enteredValue.isEmpty() ) {
        bool ok;
        newValue = enteredValue.toFloat(& ok);
        if ( not ok )
          return;
    }

    const KvalobsDataModel * kvalobsModel = static_cast<KvalobsDataModel *>(model);
    const KvalobsData * kvalobsData = kvalobsModel->kvalobsData(index);
    if ( ! kvalobsData ) {
        QMessageBox::information(editor, "Internal error", "Unable to modify data", QMessageBox::Ok);
        return;
    }
    int paramid = kvalobsModel->getParameter(index).paramid;

    int stationid = kvalobsData->stnr();
    const miutil::miTime & obt = kvalobsData->otime();
    int typ = kvalobsData->typeId(paramid);
    double orig = kvalobsData->orig(paramid);
    double corr = kvalobsData->corr(paramid);
    QString oldCorVal = QString::number(corr, 'f', 1);

    if ( std::abs(newValue - corr) < 0.00005 )
      return;

    if (stationid > 99999)
      {
        QMessageBox::information(editor, "Utenlandsk stasjon",
            "Utenlandske stasjoner kan ikke korrigeres", QMessageBox::Ok,
            Qt::NoButton);
        return;
      }

    if (abs(typ) > 999) {
        HqcMainWindow * hqcm = getHqcMainWindow(editor);
        typ = hqcm->findTypeId(typ, stationid, paramid, obt);
    }
    if (typ == -32767) {
        QMessageBox::information(editor, "Ulovlig parameter",
            "Denne parameteren fins ikke i obs_pgm\nfor denne stasjonen",
            QMessageBox::Ok, Qt::NoButton);
        return;
    }

    if ( highMap.contains(paramid) and lowMap.contains(paramid) ) {
      float uplim = highMap[paramid];
      float downlim = lowMap[paramid];
      if ((newValue > uplim || newValue < downlim) && newValue != -32766 && paramid != 105)
        {
          QMessageBox::information(editor, "Ulovlig verdi",
              "Verdien er utenfor fysikalske grenser", QMessageBox::Ok,
              Qt::NoButton);
          return;
        }
    }
    if (!legalTime(obt.hour(), paramid, newValue))
      {
        QMessageBox::information(editor, "Ulovlig tidspunkt",
            "Denne parameteren kan ikke lagres ved dette tidspunktet",
            QMessageBox::Ok, Qt::NoButton);
        return;
      }
    if (!legalValue(newValue, paramid))
      {
        QMessageBox::information(editor, "Ulovlig verdi",
            "Lovlige verdier er -5 og -6", QMessageBox::Ok, Qt::NoButton);
        return;
      }

    QString newCorVal;
    newCorVal = newCorVal.setNum(newValue, 'f', 1);
    QString changeVal;
    if (newValue == -32766)
      changeVal = "Vil du forkaste " + oldCorVal + " ?";
    else
      changeVal = "Vil du endre " + oldCorVal + " til " + newCorVal + " ?";

    int corrMb = QMessageBox::information(editor, "Korrigering", changeVal, "Ja",  "Nei", "");
    if (corrMb == 1) // "Nei"
        return;

    QStyledItemDelegate::setModelData(editor, model, index);
  }

  void KvalobsDataDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const
  {
      editor->setGeometry(option.rect);
  }

  bool KvalobsDataDelegate::legalTime(int hour, int par, double val) const
  {
    bool lT = true;
    if ( (par == 214 || par == 216 || par == 224 || par == 109) &&
         !(hour == 6 || hour == 18) || par == 110 && hour != 6 ) lT = false;
    if ( val == -32766.0 ) lT = true; //Always possible to delete
    return lT;
  }

  bool KvalobsDataDelegate::legalValue(double val, int par) const
  {
    bool lT = true;
    if ( par == 105 && ( val != -5.0 && val != -6.0 && val != -32766.0 && val < 0.0 ) )
      lT = false;
    return lT;
  }

  void KvalobsDataDelegate::setup_()
  {
    QString path = QString(getenv("HQCDIR"));
    if ( path.isEmpty() ) {
      cerr << "Intet environment" << endl;
      return;
    }
    QString limitsFile = path + "/etc/kvhqc/slimits";
    QFile limits(limitsFile);
    if ( !limits.open(QIODevice::ReadOnly) ) {
      qDebug() << "kan ikke åpne " << qPrintable(limitsFile);
      return;
    }
    QTextStream limitStream(&limits);
    int par, dum;
    float low, high;
    while ( limitStream.atEnd() == 0 ) {
      limitStream >> par >> dum >> low >> high;
      lowMap[par] = low;
      highMap[par] = high;
    }
  }
}
