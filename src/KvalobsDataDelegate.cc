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
#include "hqc_paths.hh"
#include <QDoubleValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#define NDEBUG
#include "debug.hh"

namespace { /* anonymous */

class InputValidator : public QDoubleValidator
{
public:
    InputValidator(QObject* parent) : QDoubleValidator(parent) {}
    virtual State validate(QString & input, int & pos) const
        {
            if (input.isEmpty())
                return Acceptable;
            return QDoubleValidator::validate(input, pos);
        }
};
} // anonymous namespace

namespace model
{

KvalobsDataDelegate::KvalobsDataDelegate(QObject * parent)
    : QStyledItemDelegate(parent)
    , mainWindow(getHqcMainWindow(parent))
    , mValidator(new InputValidator(this))
  {
    setup_();
  }

  KvalobsDataDelegate::~KvalobsDataDelegate()
  {
  }

  QWidget * KvalobsDataDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
  {
    Editor * ret = new Editor(parent);
    ret->setValidator(mValidator);
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
    LOG_SCOPE();
    kvalobs::DataReinserter<kvservice::KvApp> *reinserter = mainWindow->getReinserter();
#ifdef NDEBUG
    if (not reinserter) {
        QMessageBox::critical( editor,
                               "Ikke autentisert",
                               "Du er ikke autentisert som operatør.\n"
                               "Kan ikke lagre data.",
                               QMessageBox::Ok,
                               Qt::NoButton );
        return;
    }
#endif

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
    const timeutil::ptime& obt = kvalobsData->otime();
    int typ = kvalobsData->typeId(paramid);
    // UNUSED double orig = kvalobsData->orig(paramid);
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
    if (!legalTime(obt.time_of_day().hours(), paramid, newValue))
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

    DBGL;
    QStyledItemDelegate::setModelData(editor, model, index);
    kvalobs::kvData kd = kvalobsData->getKvData(paramid);
    std::list<kvalobs::kvData> modData(1, kd);
    DBGV(kd);
    if (reinserter) {
        CKvalObs::CDataSource::Result_var result;
        result = reinserter->insert( modData );
    }
  }

  void KvalobsDataDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & /*index*/) const
  {
      editor->setGeometry(option.rect);
  }

  bool KvalobsDataDelegate::legalTime(int hour, int par, double val) const
  {
      if (val == -32766.0)
          return true; // always possible to delete
      if (((par == 214 or par == 216 or par == 224 or par == 109) && hour != 6 and hour != 18)
          or (par == 110 && hour != 6))
      {
          return false;
      }
      return true;
  }

  bool KvalobsDataDelegate::legalValue(double val, int par) const
  {
      if ( par == 105 && ( val != -5.0 && val != -6.0 && val != -32766.0 && val < 0.0 ) )
          return false;
      return true;
  }

  QString KvalobsDataDelegate::displayText(const QVariant& value, const QLocale& locale) const
  {
    if ( value.type() == QVariant::Double )
      return locale.toString(value.toDouble(), 'f', 1);
    else
      return locale.toString(value.toInt());
  }

  void KvalobsDataDelegate::setup_()
  {
    QString limitsFile = hqc::getPath(::hqc::CONFDIR) + "/slimits";
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
