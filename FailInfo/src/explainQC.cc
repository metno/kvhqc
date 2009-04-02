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
#include "explainQC.h"
#include <algorithm>
#include <map>
#include <list>
#include <fstream>
#include <sstream>
#include <qstring.h>
//#include <kvservice/qt/kvQtApp.h>
#include <kvcpp/KvApp.h>
#include <kvalobs/kvStationParam.h>
#include <kvalobs/kvQCFlagTypes.h>
#include <puTools/miString>

using namespace std;
using namespace kvalobs;
using namespace miutil;

namespace QC
{

  std::string errorFunc( const cFailedParam &, const kvalobs::kvData & )
  {
    return "Ukjent parameter.";
  }

  namespace QC1
  {
    class Limits
    {
      typedef list<kvStationParam> StationParams;
      typedef StationParams::const_iterator CIStationParams;
      typedef map<int, StationParams> StationParamsByStation;
      typedef StationParamsByStation::iterator IStationParamsByStation;
      typedef StationParamsByStation::const_iterator CIStationParamsByStation;

      StationParamsByStation sParams;
    public:
      const kvStationParam * getStParam( const kvData &data )
      {
    	for ( int i = 0; i < 2; ++i ) {

    	  int station = i ? 0 : data.stationID();

    	  CIStationParamsByStation st = sParams.find( station );
    	  if ( st == sParams.end() ) {

    	    cerr << "kvApp->getKvStationParam( " << station << " );\n";

    	    // Hvis oppslag er gjort, men ingen resultat: tom liste som innhold
    	    bool ok = kvservice::KvApp::
    	      kvApp->getKvStationParam( sParams[ station ], station );
    	    if ( ! ok )
    	      throw runtime_error( "No contact with kvalobs." );
    	    st = sParams.find( station );
    	    assert( st != sParams.end() );
    	  }
    	  const StationParams & sp = st->second;

    	  int day = data.obstime().dayOfYear();
    	  if ( day < 1 )
    	    day = 1;
    	  else if ( day > 365 )
    	    day = 365;

          const int & paramID = data.paramID();
    	  for ( CIStationParams it = sp.begin(); it != sp.end(); ++it )
    	    if ( paramID == it->paramID() and day >= it->fromday() and day <= it->today() )
    	      return &* it;
    	}
    	return 0;
      }
    };

    static string getLimits( const cFailedParam &, const kvData &data )
    {
      static Limits limits;
      const kvStationParam * stParam = 0;
      try {
    	stParam = limits.getStParam( data );
      }
      catch ( runtime_error & e ) {
    	return "Feil i kontakt med kvalobs: Ingen detaljer.";
      }

      if ( ! stParam )
	   return "Ingen relevante detaljer tilgjengelig";

      miString st =
	    stParam->metadata().substr( stParam->metadata().find( '\n' ) );
      st.trim();
      st.replace( ";", " - " );

      return string( st );
    }

    static string getExplanation( const cFailedParam &failDetail )
    {
      typedef map<cFailedParam, string, cFailedParam::altLess> ExplMap;
      static ExplMap explMap;

      if ( explMap.empty() ) {
	QString path = QString(getenv("HQCDIR"));
	QString filename = path + "/FailInfo/var/qc1/faildetail.txt";
	static const int bufSize = 512;

	ifstream fs( filename );
	if ( not fs.good() ) {
	  fs.close();
	  return string("Finner ikke datafil: ") + filename.toStdString();
	}
	char buffer[ bufSize ];

	while ( not fs.eof() ) {

	  fs.getline( buffer, bufSize );
	  string line = buffer;

	  if ( line.empty() )
	    continue;

	  string::size_type sepPos = line.find( '\t' );

	  cFailedParam key = line.substr( 0, sepPos );

	  while ( sepPos < line.size() and line[sepPos] == '\t' )
	    sepPos++;

	  string val;
	  if ( sepPos != string::npos )
	    val = line.substr( sepPos );
	  else
	    val = "";

	  cout << key.toString() << " - " << val << endl;

	  explMap[key] = val;
	}
	fs.close();
      }

      cout << "Inn: " << failDetail.toString() << endl;

      ExplMap::const_iterator result = explMap.find( failDetail );
      if ( result == explMap.end() )
	return "Ingen relevante detaljer tilgjengelig";
      else
	return result->second;
    }


    string fr( const cFailedParam &failDetail, const kvData &data )
    {
      return getLimits( failDetail, data );
    }

    string fcc( const cFailedParam &failDetail, const kvData & )
    {
      return getExplanation( failDetail.toString() );
    }

    string fs( const cFailedParam &failDetail, const kvData &data )
    {
      return getLimits( failDetail, data );
    }

    string fnum( const cFailedParam &failDetail, const kvData &data )
    {
      return getLimits( failDetail, data );
    }

    string fpos( const cFailedParam &, const kvData & )
    {
      return "fpos";
    }

    string fcp( const cFailedParam &failDetail, const kvData & )
    {
      return getExplanation( failDetail.toString() );
    }

    string fd( const cFailedParam &, const kvData &data )
    {
      kvData &Data = const_cast<kvData &>(data);
      kvControlInfo ctrl;
      Data.controlinfo( ctrl );

      int val = ctrl.flag( kvQCFlagTypes::f_fd );

      //if ( val >= 2 )
      ostringstream ss;
      ss << "fd=" << val << ": Doit";

      return ss.str();
    }

    string fcombi(  const cFailedParam &failDetail, const kvData & )
    {
      return getExplanation( failDetail.toString() );
    }

    static const int groups_ = 10;
    static FailGroupList::value_type _failGroup[ groups_ ] = {
      FailGroupList::value_type( "0",
				 FailGroup( "Forhandskvalifisering",
					    errorFunc ) ),
      FailGroupList::value_type( "1",
				 FailGroup( "Grenseverdikontroll (fr)",
					    fr ) ),
      FailGroupList::value_type( "2",
				 FailGroup( "Formell konsistenskontroll (fcc)",
					    fcc ) ),
      FailGroupList::value_type( "3a",
				 FailGroup( "Sprangkontroll, step (fs)",
					    fs ) ),
      FailGroupList::value_type( "3b",
				 FailGroup( "Sprangkontroll, freeze (fs)",
					    fs ) ),
      FailGroupList::value_type( "4",
				 FailGroup( "Prognostisk romkontroll (fnum)",
					    fnum ) ),
      FailGroupList::value_type( "5",
				 FailGroup( "Meldingskontroll (fpos)",
					    fpos ) ),
      FailGroupList::value_type( "6",
				 FailGroup( "Klimatologisk konsistenskontroll (fcp)",
					    fcp ) ),
      FailGroupList::value_type( "7",
				 FailGroup( "Identifisering av oppsamlet verdi (fd)",
					    fd ) ),
      FailGroupList::value_type( "9",
				 FailGroup( "Kombinert vurdering",
					    fcombi ) )
    };
    FailGroupList failGroup( _failGroup, &_failGroup[ groups_ ] );
  }

  FailGroup::FailGroup( )
    : explanation( "Ukjent feilgruppe" )
    , getDetailExplanation( errorFunc )
  {
  }

  FailGroup::FailGroup( const string &explanation,
			Function detailExpl )
    : explanation( explanation )
    , getDetailExplanation( detailExpl )
  {
  }

  static FailExplanation::value_type _failCheck[1] = {
    FailExplanation::value_type( "QC1", QC1::failGroup )
  };

  FailExplanation failExpl( _failCheck, &_failCheck[1] );
}
