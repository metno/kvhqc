// -*- c++ -*-

#include "../TypeInformation.h"
#include <boost/algorithm/string.hpp>
#include <list>
#include <sstream>
#include <cctype>

template<class App> TypeInformation<App>
*TypeInformation<App>::stInfo = NULL;

template<class App> 
const TypeInformation<App>
*TypeInformation<App>::getInstance( App *app )
{
  if ( stInfo == NULL ) {
    if ( app != NULL )
      stInfo = new TypeInformation<App>( app );
  }
  return stInfo;
}

template<class App>
TypeInformation<App>::TypeInformation( App *app )
{
  std::list<kvalobs::kvTypes> typ;
  app->getKvTypes( typ );
  for ( std::list<kvalobs::kvTypes>::const_iterator it = typ.begin();
	it != typ.end();  it++ )
    types[ it->typeID() ] = *it;
}

template<class App>
TypeInformation<App>::~TypeInformation( )
{
}

static inline QString printableInfo( const kvalobs::kvTypes &typ, bool generated ) 
{
  std::ostringstream stream;
  std::vector<std::string> formats;
  std::string f = typ.format();
  boost::split(formats, f, boost::is_any_of(" ,"));
  stream << formats[0] << "-stasjon";
  if ( generated )
    stream << " generert av kvalobs";

  return QString::fromStdString( stream.str() );
}

template<class App>
QString TypeInformation<App>::getInfo( int typeID ) const
{
  std::map<int, kvalobs::kvTypes>::const_iterator st =
    types.find( abs( typeID ) );
  if ( st == types.end() )
    return QString::number( typeID );
  kvalobs::kvTypes s = st->second;

  return printableInfo( s, typeID < 0 );
}


template<class App>
QString TypeInformation<App>::getAll( ) const
{
  QString ret = "";
  for ( std::map<int, kvalobs::kvTypes>::const_iterator it = types.begin();
	it != types.end();  it++ )
    ret += printableInfo( it->second, it->second.typeID() < 0 );
  return ret;
}

template<class App>
const kvalobs::kvTypes 
*TypeInformation<App>::operator[]( int typeID ) const
{ 
  std::map<int, kvalobs::kvTypes>::const_iterator st = 
    types.find( typeID );
  if ( st = types.end() )
    return NULL;
  else 
    return &st->second;
}

