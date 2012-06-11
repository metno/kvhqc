/*
HQC - Free Software for Manual Quality Control of Meteorological Observations

$Id$

Copyright (C) 2007 met.no

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
#ifndef __WatchRR__VxKvDataTableItem_h__
#define __WatchRR__VxKvDataTableItem_h__

#include "RRTableItem.h"
#include "dataconsistencyverifier.h"
#include "enums.h"
#include <utility>
#include <vector>
#include <map>
#include <q3table.h>
#include <qstring.h>
#include <qstringlist.h>
#include <kvalobs/kvData.h>
#include <istream>

namespace WatchRR
{
  class VxKvDataTableItem
        : public RRComboTableItem
  {
  public:
    VxKvDataTableItem( Q3Table * table, KvDataProvider::Data data_ );

    virtual ~VxKvDataTableItem();

    virtual QString explain( ) const;

    QString getExplanations() const;

    /**
     * \brief Will return false if the system has been unable to
     * find the possible values for this cell.
     */
    virtual bool has_valid_parameter_list() const;

    virtual bool had_invalid_value_and_not_modified() const;

    virtual void getCellValue( CellValueProvider::Data &cellList ) const;

    using CellValueProvider::getCellValue;

  protected:

    /**
     * \brief Get the textual symbol for a set of parameters
     */
    QString getSymbol( KvDataProvider::Data data_ ) const;

    /**
      * Will not list VxS parameters with corrected value "1" if original value 
      * was missing.
     */
    virtual bool equal( const kvalobs::kvData & d, float v ) const;

    struct VxParam
    {
      float kvalobsCode;
      QString metCode;
      QString explanation;
    };
    friend std::istream &operator>>( std::istream &s, VxParam &p );

    typedef std::map<QString, const VxParam> VxParams;
    static VxParams vxparams;

    friend class HelpDialogImpl;

    enum { Low=0xB0, Mid=0xB9, High=0xB2 } degree; // alt: Low=0xBA

    static const QString VxExplFile;
    //    static const std::string VxExplFile;

    static const char *VxSExpl[ 3 ];

  private:
    static QStringList selections;
  };
}


#endif // __WatchRR__VxKvDataTableItem_h__


