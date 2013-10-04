#include "getStations.hh"
#include <common/KvMetaDataBuffer.hh>


namespace kvalobs
{

inline const StationList & getStations()
{
	static StationList stations;
	if ( stations.empty() ) {
		typedef std::list<kvalobs::kvStation> StList;
        const StList & st_list = KvMetaDataBuffer::instance()->allStations();
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
