#include "getStations.hh"
#include <kvcpp/KvApp.h>

//#include <iostream>

namespace kvalobs
{

inline const StationList & getStations()
{
	static StationList stations;
	if ( stations.empty() ) {
		assert( kvservice::KvApp::kvApp );
		typedef std::list<kvalobs::kvStation> StList;
		StList st_list;
		bool result = kvservice::KvApp::kvApp->getKvStations( st_list );
		if ( ! result )
			throw std::runtime_error( "Cannot get stations from kvalobs" );
		for ( StList::const_iterator it = st_list.begin(); it != st_list.end(); ++ it )
			stations.insert( * it );
	}
	return stations;
}

template<typename Functor>
void getStationsWith( StationList & out, Functor & func )
{
	const StationList & st = getStations();
    for ( StationList::const_iterator it = st.begin(); it != st.end(); ++ it )
		if ( func( * it ) )
			out.insert( * it );
}

}
