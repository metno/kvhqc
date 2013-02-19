/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

Copyright (C) 2013 met.no

Contact information:
Norwegian Meteorological Institute
Box 43 Blindern
0313 OSLO
NORWAY
email: kvalobs-dev@met.no

This file is part of HQC

HQC is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

HQC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with HQC; if not, write to the Free Software Foundation Inc.,
51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "missingtable.h"
#include "errorlist.h"
#include "KvMetaDataBuffer.hh"

MissingTable::MissingTable(QWidget* parent, ErrorList* el)
  : Q3Table( 1000, 100, parent, "table" )
{
  setCaption(tr("Missing list"));
  // UNUSED int antRow = 0;
  QString fTyp = "";
  setNumRows( 0 );
  setNumCols(12);
  horizontalHeader()->setLabel(0, tr("Stnr"));
  horizontalHeader()->setLabel(1, tr("Name"));
  horizontalHeader()->setLabel(2, tr("  Mt"));
  horizontalHeader()->setLabel(3, tr("  Dy"));
  horizontalHeader()->setLabel(4, tr("  Hr"));
  horizontalHeader()->setLabel(5, tr("Para"));
  horizontalHeader()->setLabel(6, tr("Type"));
  horizontalHeader()->setLabel(7, tr("Orig.d"));
  horizontalHeader()->setLabel(8, tr("Corr.d"));
  horizontalHeader()->setLabel(9, tr("mod.v"));
  horizontalHeader()->setLabel(10, tr("Flags"));
  horizontalHeader()->setLabel(11, tr("Fl.v"));
  int numRows = el->mList.size();
  setNumRows( numRows );
  QString strDat;

  for ( int insRow = 0; insRow < numRows; insRow++ ) {
    strDat = strDat.setNum(el->missList[insRow].stnr);
    Q3TableItem* snIt = new Q3TableItem(this, Q3TableItem::Never,strDat);
    setItem(insRow,0,snIt);

    strDat = el->missList[insRow].name.left(8);
    Q3TableItem* naIt = new Q3TableItem(this, Q3TableItem::Never,strDat);
    setItem(insRow,1,naIt);

    const QString isoTime = QString::fromStdString(timeutil::to_iso_extended_string(el->missList[insRow].obstime));
    strDat = isoTime.mid(5,2);
    Q3TableItem* moIt = new Q3TableItem(this, Q3TableItem::Never,strDat);
    setItem(insRow,2,moIt);

    strDat = isoTime.mid(8,2);
    Q3TableItem* dyIt = new Q3TableItem(this, Q3TableItem::Never,strDat);
    setItem(insRow,3,dyIt);

    strDat = isoTime.mid(11,2);
    Q3TableItem* clIt = new Q3TableItem(this, Q3TableItem::Never,strDat);
    setItem(insRow,4,clIt);

    QString parName = "???";
    try {
        parName = QString::fromStdString(KvMetaDataBuffer::instance()->findParam(el->missList[insRow].parNo).name());
    } catch (std::runtime_error&) {
        // unknown parameter
    }
    strDat = parName;
    Q3TableItem* paIt = new Q3TableItem(this, Q3TableItem::Never,strDat);
    setItem(insRow,5,paIt);

    strDat = strDat.setNum(el->missList[insRow].typeId);
    Q3TableItem* tiIt = new Q3TableItem(this, Q3TableItem::Never,strDat);
    setItem(insRow,6,tiIt);

    strDat = strDat.setNum(el->missList[insRow].orig,'f',1);
    Q3TableItem* ogIt = new Q3TableItem(this, Q3TableItem::Never,strDat);
    setItem(insRow,7,ogIt);

    strDat = strDat.setNum(el->missList[insRow].corr,'f',1);
    Q3TableItem* coIt = new Q3TableItem(this, Q3TableItem::Never,strDat);
    setItem(insRow,8,coIt);

    strDat = strDat.setNum(el->missList[insRow].morig,'f',1);
    Q3TableItem* mlIt = new Q3TableItem(this, Q3TableItem::Never,strDat);
    setItem(insRow,9,mlIt);

    strDat = el->missList[insRow].flTyp;
    Q3TableItem* fiIt = new Q3TableItem(this, Q3TableItem::Never,strDat);
    setItem(insRow,10,fiIt);

    strDat = strDat.setNum(el->missList[insRow].flg);
    Q3TableItem* fgIt = new Q3TableItem(this, Q3TableItem::Never,strDat);
    setItem(insRow,11,fgIt);
  }
  for ( int icol = 0; icol < 12; icol++ )
    adjustColumn(icol);
}
