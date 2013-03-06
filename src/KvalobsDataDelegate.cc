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

#include "KvalobsDataDelegate.h"

#include "Functors.hh"
#include "hqcmain.h"
#include "KvalobsDataModel.h"
#include "KvMetaDataBuffer.hh"

#include <QDoubleValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

//#define NDEBUG
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
}

KvalobsDataDelegate::~KvalobsDataDelegate()
{
}

QWidget * KvalobsDataDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
{
    QLineEdit * ret = new QLineEdit(parent);
    ret->setValidator(mValidator);
    return ret;
}

void KvalobsDataDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    QLineEdit * e = static_cast<QLineEdit *>(editor);

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
    LOG_SCOPE("KvalobsDataDelegate");

    QLineEdit * e = static_cast<QLineEdit *>(editor);
    QString enteredValue = e->text();
    bool ok = true;
    float newValue = enteredValue.isEmpty() ? -32766 : enteredValue.toFloat(&ok);
    if (not ok)
        return;

    const KvalobsDataModel * kvalobsModel = static_cast<KvalobsDataModel*>(model);
    const KvalobsData * kvalobsData = kvalobsModel->kvalobsData(index);
    if (not kvalobsData) {
        QMessageBox::information(editor, tr("Internal error"),
                                 tr("Unable to modify data."), QMessageBox::Ok);
        return;
    }
    const int paramid = kvalobsModel->getParameter(index).paramid;
    const double corr = kvalobsData->corr(paramid);
    if (Helpers::float_eq()(newValue, corr))
        return;

    const timeutil::ptime& obt = kvalobsData->otime();
    int typ = kvalobsData->typeId(paramid);
    if (abs(typ) > 999) {
        const int stationid = kvalobsData->stnr();
        typ = mainWindow->findTypeId(typ, stationid, paramid, obt);
    }
    if (typ == -32767) {
        QMessageBox::information(mainWindow, tr("Illegal parameter"),
                                 tr("This parameter is not in obs_pgm for this station."),
                                 QMessageBox::Ok, Qt::NoButton);
        return;
    }

    if (not KvMetaDataBuffer::instance()->checkPhysicalLimits(paramid, newValue)) {
        QMessageBox::information(mainWindow, tr("Illegal value"),
                                 tr("Value outside physical limits."), QMessageBox::Ok,
                                 Qt::NoButton);
        return;
    }
    if (not legalTime(obt.time_of_day().hours(), paramid, newValue)) {
        QMessageBox::information(mainWindow, tr("Illegal time"),
                                 tr("This parameter cannot be saved for this time."),
                                 QMessageBox::Ok, Qt::NoButton);
        return;
    }
    if (not legalValue(newValue, paramid)) {
        QMessageBox::information(editor, tr("Illegal value"),
                                 tr("Legal values are -5 and -6"), QMessageBox::Ok, Qt::NoButton);
        return;
    }

    const QString oldCorVal = QString::number(corr,     'f', 1);
    const QString newCorVal = QString::number(newValue, 'f', 1);
    QString changeVal;
    if (newValue == -32766)
        changeVal = tr("Do you want to reject %1?").arg(oldCorVal);
    else
        changeVal = tr("Do you want to change %1 to %2?").arg(oldCorVal).arg(newCorVal);

    if (QMessageBox::No == QMessageBox::question(mainWindow, tr("Correction"), changeVal,
                                                 QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes))
        return;

    LOG4SCOPE_DEBUG("calling setModelData");
    QStyledItemDelegate::setModelData(editor, model, index);
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

} // namespace model
