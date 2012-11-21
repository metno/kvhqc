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

#include "FunctionLogger.hh"
#include "hqc_paths.hh"

#include <kvcpp/KvApp.h>
#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvStationParam.h>
#include <kvalobs/kvQCFlagTypes.h>

#include <QtCore/qstring.h>

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <map>
#include <list>
#include <fstream>
#include <sstream>

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
	  std::string comma(",");
          // UNUSED const int & paramID = data.paramID();
	  int icomma = data.cfailed().find(comma);

	  std::string qcx;
	  if ( icomma > 0 )
	    qcx = data.cfailed().substr(0, data.cfailed().find(','));
	  else
	    qcx = data.cfailed();
    	  for ( CIStationParams it = sp.begin(); it != sp.end(); ++it ) {
	    if ( qcx == it->qcx() and day >= it->fromday() and day <= it->today() )
    	      return &* it;
	  }
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

      std::string st;
      if ( data.controlinfo().flag(kvalobs::flag::fnum) == 6 )
	st = "Observasjon mangler.  Modellverdi er satt inn";
      else if ( data.controlinfo().flag(kvalobs::flag::fnum) > 1 ) {
	st = "Grenseverdier for avvik fra modell :";
	st +=  stParam->metadata().substr( stParam->metadata().find( '\n' ) + 1 );
        boost::algorithm::trim(st);
        boost::algorithm::replace_all(st, ";", " - ");
      }
      else if ( data.controlinfo().flag(kvalobs::flag::fs) == 3 ) {
	st = "Samme verdi mer enn ";
	st +=  stParam->metadata().substr( stParam->metadata().find( '\n' ) + 1 );
	st += " ganger";
	boost::algorithm::trim(st);
      }
      else {
	st =  stParam->metadata().substr( stParam->metadata().find( '\n' ) );
	boost::algorithm::trim(st);
	boost::algorithm::replace_all(st, ";", " - ");
      }
      cerr << "st = '" << st << '\'' << endl;
      return st;
    }

    static string getExplanation( const cFailedParam &failDetail )
    {
      typedef map<cFailedParam, string, cFailedParam::altLess> ExplMap;
      static ExplMap explMap;

      if ( explMap.empty() ) {
	QString filename = ::hqc::getPath(::hqc::DATADIR) + "/faildetail.txt";
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
    } //getExplanation


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

      int val = ctrl.flag( kvalobs::flag::fd );

      //if ( val >= 2 )
      ostringstream ss;
      ss << "fd=" << val << ": Doit";

      return ss.str();
    }

    string fcombi(  const cFailedParam &failDetail, const kvData & data )
    {
      kvData &Data = const_cast<kvData &>(data);
      kvControlInfo ctrl;
      Data.controlinfo( ctrl );

      int val = ctrl.flag( kvalobs::flag::fcombi );

      //if ( val >= 2 )
      ostringstream ss;
      ss << "fcombi = " << val << ": Doit";

      return ss.str();
      //      return getExplanation( failDetail.toString() );
    }

    string fmis( const cFailedParam &, const kvData & )
    {
      return "fmis";
    }

    string ftime( const cFailedParam &failDetail, const kvData & )
    {
      return getExplanation( failDetail.toString() );
    }

    ////    static const int groups_ = 10;
    static const int groups_ = 12;
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
					    fcombi ) ),
      FailGroupList::value_type( "10",
				 FailGroup( "Manglende observasjon",
					    fmis ) ),
      FailGroupList::value_type( "11",
				 FailGroup( "Tidsserietilpasning",
					    ftime ) ),
    };
    FailGroupList failGroup( _failGroup, &_failGroup[ groups_ ] );
  }

  namespace QC2
  {
    static string getExplanation( const cFailedParam &failDetail )
    {
      typedef map<cFailedParam, string, cFailedParam::altLess> ExplMap;
      static ExplMap explMap;

      if ( explMap.empty() ) {
	QString filename = ::hqc::getPath(::hqc::DATADIR) + "/faildetail.txt";
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

    string ftime( const cFailedParam &failDetail, const kvData & )
    {
      return getExplanation( failDetail.toString() );
    }

  static FailGroupList::value_type _qc2d_failGroup[] = {
      FailGroupList::value_type( "2", FailGroup( "Tidsserietilpasning", ftime ) )
  };
  FailGroupList qc2d_failGroup(_qc2d_failGroup, boost::end(_qc2d_failGroup));

  static FailGroupList::value_type _qc2h_failGroup[] = {
      FailGroupList::value_type( "1", FailGroup( "Pluviometer", ftime ) )
  };
  FailGroupList qc2h_failGroup(_qc2h_failGroup, boost::end(_qc2h_failGroup));
  } // namespace QC2




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

static FailExplanation::value_type _failCheck[] = {
    FailExplanation::value_type( "QC1",  QC1::failGroup ),
//    FailExplanation::value_type( "QC2",  QC2::failGroup ),
    FailExplanation::value_type( "QC2d", QC2::qc2d_failGroup ),
    FailExplanation::value_type( "QC2h", QC2::qc2h_failGroup )
};

FailExplanation failExpl(_failCheck, boost::end(_failCheck));

} // namespace QC
