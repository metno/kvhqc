#include "../StationInformation.h"
#include <list>
#include <sstream>
#include <cctype>

template<class App> StationInformation<App>
*StationInformation<App>::stInfo = NULL;

template<class App> 
const StationInformation<App>
*StationInformation<App>::getInstance( App *app )
{
  if ( stInfo == NULL ) {
    if ( app != NULL )
      stInfo = new StationInformation<App>( app );
  }
  return stInfo;
}

template<class App>
StationInformation<App>::StationInformation( App *app )
{
  std::list<kvalobs::kvStation> st;
  app->getKvStations( st );
  for ( std::list<kvalobs::kvStation>::const_iterator it = st.begin();
	it != st.end();  it++ )
    stations[ it->stationID() ] = *it;
}

template<class App>
StationInformation<App>::~StationInformation( )
{
}

static inline QString printableInfo( const kvalobs::kvStation &st ) 
{
  std::ostringstream stream;
  stream << st.stationID() << " " << st.name() << ", "
	 << st.height() << " moh.";

  return QString( stream.str() );
}

template<class App>
QString StationInformation<App>::getInfo( int stationID ) const
{
  std::map<int, kvalobs::kvStation>::const_iterator st =
    stations.find( stationID );
  if ( st == stations.end() )
    return QString::number( stationID );
  kvalobs::kvStation s = st->second;

  return printableInfo( s );
}


template<class App>
QString StationInformation<App>::getAll( ) const
{
  QString ret = "";
  for ( std::map<int, kvalobs::kvStation>::const_iterator it = stations.begin();
	it != stations.end();  it++ )
    ret += printableInfo( it->second );
  return ret;
}

template<class App>
const kvalobs::kvStation 
*StationInformation<App>::operator[]( int stationID ) const
{ 
  std::map<int, kvalobs::kvStation>::const_iterator st = 
    stations.find( stationID );
  if ( st == stations.end() )
    return NULL;
  else 
    return &st->second;
}

