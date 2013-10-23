#include <common/KvMetaDataBuffer.hh>
#include <boost/foreach.hpp>
#include <stdexcept>
#include <list>

#include <cassert>

namespace kvalobs
{

namespace
{
struct insert_if_newer_
{
  ObsPgm & op;
  insert_if_newer_( ObsPgm & op_ ) : op( op_ ) {}
  void operator()( const kvalobs::kvObsPgm & data )
  {
    IObsPgm find = op.find( data );
    if ( find == op.end() )
      op.insert( data );
    else {
      if ( data.fromtime() > find->fromtime() ) {
        op.erase( find );
        op.insert( data );
      }
    }
  }
};

struct matches_time
{
	const boost::posix_time::ptime t_;
	explicit matches_time(const boost::posix_time::ptime & t) : t_(t) {}
	bool operator () (const kvalobs::kvObsPgm & opgm) const
	{
		if ( opgm.fromtime() <= t_ )
		{
			if ( opgm.totime().is_not_a_date_time() )
				return true;
			return t_ < opgm.totime();
		}
		return false;
	}
};
}

template<typename Functor>
void getObsPgmWith(ObsPgm & out, const boost::posix_time::ptime & forTime, Functor & func)
{
    std::set<long> stationids;
    BOOST_FOREACH(const kvalobs::kvStation & station, KvMetaDataBuffer::instance()->allStations())
        stationids.insert(station.stationID());
    KvMetaDataBuffer::instance()->findObsPgm(stationids);
    BOOST_FOREACH(long station, stationids)
    {
        KvMetaDataBuffer::ObsPgmList getObsPgmList = KvMetaDataBuffer::instance()->findObsPgm(station);

        matches_time mt(forTime);
        for (KvMetaDataBuffer::ObsPgmList::const_iterator it = getObsPgmList.begin(); it != getObsPgmList.end(); ++it)
            if ( mt(*it) and func(*it) )
                out.insert(*it);
    }
 }

}
